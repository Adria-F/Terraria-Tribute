#include "j1Map.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1CollisionManager.h"
#include "j1Gui.h"
#include "j1Light.h"
#include "UI_LoadingScreen.h"
#include "j1EntityManager.h"
#include "Entity.h"
#include <time.h>

j1Map::j1Map()
{
}

j1Map::~j1Map()
{
}

bool j1Map::Start()
{
	srand(time(NULL));

	pugi::xml_document mapData;
	App->LoadConfig(mapData, "maps/mapData.xml");
	if (mapData)
	{
		loadBlocksConnectionsData(mapData.child("connections"));
		loadBlocksTextures(mapData.child("textures"));
	}
	pugi::xml_document worldData;
	App->LoadConfig(worldData, "worldData.xml");
	if (worldData)
		loadWorldData(worldData.child("generationRules"));

	return true;
}

bool j1Map::PreUpdate()
{
	startFrameTime = App->getExecutionTime();

	if (generatingMap)
	{
		switch (state)
		{
		case TO_START_GENERATING:
			newGenerationState(GENERATE_FLAT_MAP);
			break;
		case GENERATE_FLAT_MAP:
			generateFlatMap();
			break;
		case GENERATE_CAVES:
			generateCaves(10);
			break;
		case CLEAN_NOISE:
			cleanMapNoise(worldData.cleanMapIterations);
			break;
		case GENERATE_BIOMES:
			generateBiomes();
			break;
		case CLEAN_MAP:
			cleanLonelyBlocks();
			break;
		case CREATE_COLLIDERS:
			createColliders();
			break;
		case CONNECT_BLOCKS:
			updateBlocksConnections();
			break;
		}
	}

	return true;
}

bool j1Map::Update(float dt)
{
	if (generatingMap)
		return true;

	App->render->DrawQuad({ 0,0, worldData.world_width*BLOCK_SIZE, worldData.world_height*BLOCK_SIZE }, Cyan);

	int chunck_counter = 0;

	//Camera culling
	iPoint firstBlock = WorldToMap({ App->render->camera.x, App->render->camera.y });
	iPoint lastBlock = WorldToMap({ App->render->camera.x+App->render->camera.w, App->render->camera.y+App->render->camera.h });
	lastBlock.x += 1;
	lastBlock.y += 1;

	int alpha= App->lightEngine->DayNight();

	for (int i = firstBlock.x; i < lastBlock.x; i++)
	{
		for (int j = firstBlock.y; j < lastBlock.y; j++)
		{
			block* block = getBlockAt(i, j);

			if (block == nullptr)
			{
				continue;
			}

			iPoint pos = MapToWorld(block->position);
			block->lColor->a = alpha;

			if (block->type == AIR)
			{
				App->render->DrawQuad({ pos.x , pos.y, BLOCK_SIZE, BLOCK_SIZE }, *(block->lColor));
				continue;
			}

			Color color = { 150, 90, 60, 255 }; //Brown
			if (block->type == GRASS)
				color = { 0, 230, 0, 255 }; //Green
			else if (block->type == STONE)
				color = { 150, 150, 150, 255 }; //Gray

			if ((App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN || App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) && App->input->collidingMouse({ pos.x, pos.y, BLOCK_SIZE, BLOCK_SIZE }, true))
			{
				removeBlock(block);
			}

			if (false)
			{
				App->render->DrawQuad({ pos.x, pos.y, BLOCK_SIZE, BLOCK_SIZE }, Black);
				App->render->DrawQuad({ pos.x + BLOCK_BORDER, pos.y + BLOCK_BORDER, BLOCK_SIZE - BLOCK_BORDER, BLOCK_SIZE - BLOCK_BORDER }, color);
			}
			else
			{
				App->render->Blit(getBlockTexture(block->type), pos.x, pos.y, &block->section);
				App->render->DrawQuad({ pos.x, pos.y, BLOCK_SIZE, BLOCK_SIZE}, *(block->lColor));
			}
			 
		}
		chunck_counter++;
		if (App->render->debug)
			App->render->DrawLine(chunck_counter*CHUNCK_WIDTH*BLOCK_SIZE - 1, 0, chunck_counter*CHUNCK_WIDTH*BLOCK_SIZE - 1, worldData.world_height*BLOCK_SIZE, Red);
	}

	return true;
}

bool j1Map::CleanUp()
{
	chuncks.clear();

	return true;
}

void j1Map::loadWorldData(pugi::xml_node data)
{
	worldData.world_width = data.child("worldWidth").attribute("value").as_int();
	worldData.world_height = data.child("worldHeight").attribute("value").as_int();

	worldData.groundStartHeight = data.child("groundStartHeight").attribute("value").as_int();
	worldData.stoneStartHeight = data.child("stoneStartHeight").attribute("value").as_int();
	worldData.mapMaxVariation = data.child("mapMaxVariation").attribute("value").as_int();

	worldData.maxCaveSize = { data.child("maxCaveSize").attribute("width").as_int(), data.child("maxCaveSize").attribute("height").as_int() };
	worldData.minCaveSize = { data.child("minCaveSize").attribute("width").as_int(), data.child("minCaveSize").attribute("height").as_int() };
	worldData.cavesMinSeparation = data.child("cavesMinSeparation").attribute("value").as_int();
	worldData.cleanMapIterations = data.child("cleanMapIterations").attribute("value").as_int();
}

void j1Map::generateFlatMap()
{
	fillPerlinList(grassPerlin);
	fillPerlinList(stonePerlin, 1, 0);
	while (App->getExecutionTime() - startFrameTime < FRAME_TIME)
	{
		if (chuncks.size() * CHUNCK_WIDTH >= worldData.world_width || (chuncks.size() >= CHUNCKS_LOADED && CHUNCKS_LOADED != 0))
		{
			newGenerationState(GENERATE_CAVES);
			break;
		}
		chunck* curr_chunck = new chunck();
		for (int i = 0; i < CHUNCK_WIDTH; i++)
		{
			for (int j = 0; j < worldData.world_height; j++)
			{
				block* curr_block = new block();
				curr_block->position = { i + (CHUNCK_WIDTH*(int)chuncks.size()), j };
				if (curr_block->position.y == (worldData.groundStartHeight + (int)(worldData.mapMaxVariation*grassPerlin[curr_block->position.x])))
					curr_block->type = GRASS;
				else if (curr_block->position.y >(worldData.groundStartHeight + (int)(worldData.mapMaxVariation*stonePerlin[curr_block->position.x])) + worldData.stoneStartHeight)
					curr_block->type = STONE;
				else if (curr_block->position.y >(worldData.groundStartHeight + (int)(worldData.mapMaxVariation*grassPerlin[curr_block->position.x])))
					curr_block->type = DIRT;

				curr_block->Source = new SDL_Rect({ i + (CHUNCK_WIDTH*(int)chuncks.size()), j ,16,16 });
				curr_block->lColor = new Color(0, 0, 0, 255);

				curr_chunck->blocks.push_back(curr_block);
			}
		}
		chuncks.push_back(curr_chunck);
	}
}

void j1Map::createColliders()
{
	int firstBlock = 0;
	int lastBlock = 0;
	int currY = 0;

	Collider* currCollider = nullptr;

	bool firstFound = false;

	for (int j = 0; j < worldData.world_height; j++)
	{
		for (int i = 0; i < worldData.world_width; i++)
		{
			block* currBlock = getBlockAt(i, j);
			if (currBlock == nullptr || currBlock->collider != nullptr || currBlock->type == AIR || currBlock->getNeighborsType(AIR) == 0)
				continue;

			iPoint worldPos = MapToWorld({ currBlock->position.x, currBlock->position.y });
			currCollider = App->collisions->AddCollider(worldPos.x, worldPos.y, currBlock->section.w, currBlock->section.h, FLOOR_COLLIDER, true);
			currBlock->collider = currCollider;
		}
	}

	newGenerationState(CONNECT_BLOCKS);
}

void j1Map::generateCaves(int amount)
{
	BROFILER_CATEGORY("Generate Cave", Profiler::Color::Blue);
		
	std::vector<SDL_Rect> caveList;

	while (App->getExecutionTime() - startFrameTime < FRAME_TIME)
	{
		if (iterations >= amount)
		{
			newGenerationState(CLEAN_NOISE);
			iterations = 0;
			break;
		}
		int caveX = rand() % worldData.world_width;
		int caveY = rand() % (worldData.world_height - worldData.groundStartHeight + 1) + worldData.groundStartHeight;
		int caveW = rand() % (worldData.maxCaveSize.x - worldData.minCaveSize.x + 1) + worldData.minCaveSize.x;
		int caveH = rand() % (worldData.maxCaveSize.y - worldData.minCaveSize.y + 1) + worldData.minCaveSize.y;
		SDL_Rect cave = { caveX, caveY, caveW, caveH };

		while (collidingWithList(cave, caveList, worldData.cavesMinSeparation))
		{
			int caveX = rand() % worldData.world_width;
			int caveY = rand() % (worldData.world_height - worldData.groundStartHeight + 1) + worldData.groundStartHeight;
			int caveW = rand() % (worldData.maxCaveSize.x - worldData.minCaveSize.x + 1) + worldData.minCaveSize.x;
			int caveH = rand() % (worldData.maxCaveSize.y - worldData.minCaveSize.y + 1) + worldData.minCaveSize.y;

			cave = { caveX, caveY, caveW, caveH };
		}

		caveList.push_back(cave);
		fillCaveMap(cave);
		iterations++;
	}
}

void j1Map::fillCaveMap(SDL_Rect area)
{
	bool previousAir = false;
	int tendence = 50;
	bool topExtra = false;
	bool bottomExtra = false;
	int extraX = 0;
	for (int i = area.x; i < area.x + area.w; i++)
	{
		for (int j = area.y; (j < area.y + area.h || bottomExtra); j++)
		{
			//Some calculations to avoid the margins of the cave being squared
			extraX = i;
			if (extraX == area.x) //First X position | negative X
			{
				while (rand() % 100 > 25) //25%
					extraX--;
			}
			else if (extraX == area.x + area.w - 1) //Last X position | positive X
			{
				while (rand() % 100 > 25) //25%
					extraX++;
			}
			if (!topExtra)
			{
				while (rand() % 100 > 25) //25%
					j--;
				topExtra = true; //negative Y
			}
			if (j >= (area.y + area.h - 1))
			{
				bottomExtra = (rand() % 100 > 25); //25% | positive Y
			}

			//Remove some random blocks with a decreasing tendence
			int repetition = 0; //If repeated block state, decrease the tendence
			bool lastAir = false;
			if ((rand() % 100) > tendence) //Higher tendence less air chance | lower more air chance
			{
				block* Block = getBlockAt(extraX, j);
				if (Block != nullptr)
				{
					removeBlock(Block, false);
					if (lastAir)
						repetition++;
					else
					{
						lastAir = true;
						repetition = 0;
					}
				}
				tendence = 25 + repetition;
			}
			else
			{
				tendence = 75 - repetition;
				if (lastAir)
				{
					lastAir = false;
					repetition = 0;
				}
				else
					repetition++;
			}
		}
		topExtra = false;
	}
}

void j1Map::generateBiomes()
{
	//Select an X to spawn the biome
	bool spawn = false;
	int x = 0;
	while (!spawn)
	{
		bool spawn = (rand() % 100 < 2);
		if (spawn)
			break;

		x++;
		if (x > worldData.world_width)
			x = 0;
	}

	int y = 0;
	block* Block = getBlockAt(x, y);
	while (Block->type == AIR)
	{
		y++;
		Block = getBlockAt(x, y);	//Need to check if returns nullptr
	}

	//BIOME SIZE
	int W = 50;
	int H = 50;

	for (int i = x - W / 2; i < x + W / 2; i++)
	{
		for (int j = y - H / 2; j < y + H / 2; j++)
		{
			block* Block2 = getBlockAt(i, j);
			if (Block2 != nullptr && Block2->type != AIR && Block2->position.DistanceTo(Block->position) <= 23)
			{
				setBlock(Block2, SAND);
			}
		}
	}

	newGenerationState(CREATE_COLLIDERS);
}

void j1Map::cleanMapNoise(int iterations)
{
	while (App->getExecutionTime() - startFrameTime < FRAME_TIME)
	{
		if (this->iterations >= iterations)
		{
			newGenerationState(GENERATE_BIOMES);
			this->iterations = 0;
			lastBlockOperated = { 0,0 };
			break;
		}

		convertBlockIntoNeighbors(lastBlockOperated.x, lastBlockOperated.y);
		lastBlockOperated.y++;
		if (lastBlockOperated.y >= worldData.world_height)
		{
			lastBlockOperated.y = 0;
			lastBlockOperated.x++;
		}
		if (lastBlockOperated.x >= worldData.world_width)
		{
			lastBlockOperated.x = 0;
			this->iterations++;
		}
	}
}

void j1Map::convertBlockIntoNeighbors(int x, int y)
{
	block* original = getBlockAt(x, y);
	if (original != nullptr)
	{
		int noAirAmount = (original->type == AIR)? 8 : 9;
		for (int i = x - 1; i <= x + 1; i++)
		{
			for (int j = y - 1; j <= y + 1; j++)
			{
				block* Block = getBlockAt(i, j);
				if (Block != nullptr && Block->type == AIR)
				{
					noAirAmount--;
				}
			}
		}

		if (noAirAmount >= 5)
		{
			if (original->type == AIR)
				original->type = STONE;
		}
		else
			original->type = AIR;
	}
}

//Needed for the convertBlockIntoNeighbors() function
blockType j1Map::moreRepeatedNeighbor(block * Block)
{
	return blockType();
}

bool j1Map::collidingWithList(SDL_Rect rect, std::vector<SDL_Rect> list, int margin)
{
	bool ret = false;

	for (int i = 0; i < list.size(); i++)
	{
		if (rect.x - margin < list[i].x + list[i].w &&
			rect.x + rect.w > list[i].x - margin &&
			rect.y - margin< list[i].y + list[i].h &&
			rect.y + rect.h > list[i].y - margin
			)
		{
			ret = true;
			break;
		}
	}

	return ret;
}

//NEEDS OPTIMIZATION | Currently checking for all possible blocks surrounding
void j1Map::cleanLonelyBlocks()
{
	while (App->getExecutionTime() - startFrameTime < FRAME_TIME)
	{
		block* block = chuncks[lastBlockOperated.x]->blocks[lastBlockOperated.y];
		for (int i = 0; i < MAX_TYPE; i++)
		{
			if (block->getNeighborsType((blockType)i) == 4)
				setBlock(block, (blockType)i);
		}
		/*if (block->type == AIR && block->collider != nullptr)
		{
			App->collisions->removeCollider(block->collider);
			block->collider = nullptr;
		}*/

		lastBlockOperated.y++;
		if (lastBlockOperated.y >= chuncks[lastBlockOperated.x]->blocks.size())
		{
			lastBlockOperated.y = 0;
			lastBlockOperated.x++;
		}
		if (lastBlockOperated.x >= chuncks.size())
		{
			lastBlockOperated.x = 0;
			newGenerationState(CREATE_COLLIDERS);
			break;
		}
	}
}

void j1Map::updateBlocksConnections()
{
	for (int i = 0; i < chuncks.size(); i++)
	{
		for (int j = 0; j < chuncks[i]->blocks.size(); j++)
		{
			chuncks[i]->blocks[j]->updateSection();
		}
	}

	newGenerationState(GENERATION_COMPLETED);
}

SDL_Rect j1Map::getSectionFromBlockConnections(std::vector<bool> connections)
{
	SDL_Rect ret = { 0,0,0,0 };

	std::map<std::vector<bool>, SDL_Rect>::iterator it = tile_sections.find(connections);
	if (it != tile_sections.end())
		ret = (*it).second;

	return ret;
}

iPoint j1Map::MapToWorld(iPoint pos)
{
	return {pos.x*BLOCK_SIZE, pos.y*BLOCK_SIZE };
}

iPoint j1Map::WorldToMap(iPoint pos)
{
	return { pos.x/BLOCK_SIZE, pos.y/BLOCK_SIZE };
}

void j1Map::removeBlock(int x, int y)
{
	block* Block = getBlockAt(x, y);
	if (Block != nullptr)
		removeBlock(Block);
}

void j1Map::removeBlock(block* Block, bool updateSurroundings)
{
	Block->type = AIR;
	if (Block->collider != nullptr)
	{
		App->collisions->removeCollider(Block->collider); //It will change when blocks can be placed
		Block->collider = nullptr;
	}

	if (updateSurroundings)
	{
		std::vector<block*> neighbors = Block->getNeighbors();
		for (int i = 0; i < neighbors.size(); i++)
		{
			neighbors[i]->updateSection();
			neighbors[i]->createCollider();
		}

		block* N_neighbor = getBlockAt(Block->position.x, Block->position.y - 1);
		if (N_neighbor != nullptr && N_neighbor->type != AIR && N_neighbor->falling_block)
			convertIntoFallingBlock(N_neighbor);
	}
}

void j1Map::setBlock(int x, int y, blockType type)
{
	block* Block = getBlockAt(x, y);
	if (Block != nullptr)
		setBlock(Block, type);
}

void j1Map::setBlock(block* Block, blockType type)
{
	blockType previousType = Block->type;

	Block->type = type;
	if (type >= FIRST_FALLING_BLOCK)
		Block->falling_block = true;
	else
		Block->falling_block = false;

	if (previousType == AIR && type != AIR)
	{
		Block->createCollider();

		Block->updateSection();
		std::vector<block*> neighbors = Block->getNeighbors();
		for (int i = 0; i < neighbors.size(); i++)
		{
			neighbors[i]->updateSection();
			if (neighbors[i]->getNeighborsType(AIR) == 0 && neighbors[i]->collider != nullptr)
			{
				App->collisions->removeCollider(neighbors[i]->collider);
				neighbors[i]->collider = nullptr;
			}

		}
	}
}

void j1Map::convertIntoFallingBlock(block* Block)
{
	App->entitymanager->createFallingBlock(Block);
	removeBlock(Block);
}

SDL_Texture* j1Map::getBlockTexture(blockType type)
{
	SDL_Texture* ret = nullptr;

	std::map<blockType, SDL_Texture*>::iterator it = blocks_textures.find(type);
	if (it != blocks_textures.end())
		ret = (*it).second;

	return ret;
}

chunck* j1Map::getChunckAt(int x)
{
	chunck* ret = nullptr;

	int num = x/CHUNCK_WIDTH;

	if (num < chuncks.size())
		ret = chuncks[num];

	return ret;
}

block* j1Map::getBlockAt(int x, int y)
{
	block* ret = nullptr;
	chunck* chunck = getChunckAt(x);

	if (chunck != nullptr && x < worldData.world_width && x >= 0 && y < worldData.world_height && y >= 0)
	{
		int position = y + (x-chunck->blocks[0]->position.x)*worldData.world_height;

		if (position < chunck->blocks.size())
			ret = chunck->blocks[position];
	}

	return ret;
}

void j1Map::loadBlocksTextures(pugi::xml_node textures)
{
	for (pugi::xml_node texture = textures.child("texture"); texture; texture = texture.next_sibling("texture"))
	{
		blocks_textures.insert(std::pair<blockType, SDL_Texture*>((blockType)texture.attribute("blockID").as_int(), App->tex->Load(texture.attribute("path").as_string())));
	}
}

void j1Map::loadBlocksConnectionsData(pugi::xml_node connections)
{
	for (pugi::xml_node connection = connections.child("connection"); connection; connection = connection.next_sibling("connection"))
	{
		std::vector<bool> directions;
		directions.push_back(connection.attribute("N").as_bool(false));
		directions.push_back(connection.attribute("S").as_bool(false));
		directions.push_back(connection.attribute("E").as_bool(false));
		directions.push_back(connection.attribute("W").as_bool(false));

		SDL_Rect section = {connection.child("section").attribute("x").as_int(0),
			connection.child("section").attribute("y").as_int(0) ,
			connection.child("section").attribute("w").as_int(0) ,
			connection.child("section").attribute("h").as_int(0) };

		tile_sections.insert(std::pair<std::vector<bool>, SDL_Rect>(directions, section));
	}
}

void j1Map::fillPerlinList(std::vector<float>& perlinList, int maxValue, int minValue, int increment, int tendenceMargin)
{
	perlinList.clear();

	float previousPerlin = maxValue + minValue;
	float previousVariation = 0.0f;
	int tendence = 50;
	for (int i = 0; i < (worldData.world_width*BLOCK_SIZE); i++)
	{
		float newPerlin = previousPerlin;
		while (newPerlin == previousPerlin)
		{
			newPerlin = rand() % increment;
			newPerlin /= 100;

			if (tendence != 0 && tendence != 100)
			{
				if (previousVariation < 0)
					tendence = 100 - tendenceMargin;
				else if (previousVariation > 0)
					tendence = tendenceMargin;
			}
			bool positive = (rand() % 100) > tendence;

			newPerlin = (positive) ? newPerlin : -newPerlin;
			previousVariation = newPerlin;
			newPerlin = previousPerlin + newPerlin;

			if (newPerlin > maxValue)
				tendence = 100;
			else if (newPerlin < minValue)
				tendence = 0;
			else
				tendence = 50;
		}
		previousPerlin = newPerlin;
		perlinList.push_back(previousPerlin);
	}
}

void j1Map::newGenerationState(generatingState newState)
{
	state = newState;

	switch (state)
	{
	case GENERATE_FLAT_MAP:
		App->gui->loadingScreen->loadingMessage("Generating basic map");
		break;
	case GENERATE_CAVES:
		App->gui->loadingScreen->loadingMessage("Generating caves");
		break;
	case CLEAN_NOISE:
		App->gui->loadingScreen->loadingMessage("Cleaning caves noise");
		break;
	case GENERATE_BIOMES:
		App->gui->loadingScreen->loadingMessage("Generating Biomes");
		break;
	case CLEAN_MAP:
		App->gui->loadingScreen->loadingMessage("Cleaning map noise");
		break;
	case CREATE_COLLIDERS:
		App->gui->loadingScreen->loadingMessage("Creating colliders");
		break;
	case CONNECT_BLOCKS:
		App->gui->loadingScreen->loadingMessage("Connecting Blocks");
		break;
	case GENERATION_COMPLETED:
		generatingMap = false;
		App->gui->loadingScreen->endLoadingScreen();
		App->entitymanager->createPlayer(0, 300);
		break;
	}
}

int block::getNeighborsType(blockType type)
{
	int ret = 0;

	std::vector<block*> neighbors = this->getNeighbors();

	for (int i = 0; i < neighbors.size(); i++)
		if (neighbors[i]->type == type)
			ret++;

	return ret;
}

std::vector<block*> block::getNeighbors()
{
	block* N_neighbor = App->map->getBlockAt(position.x, position.y - 1);
	block* S_neighbor = App->map->getBlockAt(position.x, position.y + 1);
	block* E_neighbor = App->map->getBlockAt(position.x + 1, position.y);
	block* W_neighbor = App->map->getBlockAt(position.x - 1, position.y);

	std::vector<block*> ret;
	if (N_neighbor != nullptr)
		ret.push_back(N_neighbor);
	if (S_neighbor != nullptr)
		ret.push_back(S_neighbor);
	if (E_neighbor != nullptr)
		ret.push_back(E_neighbor);
	if (W_neighbor != nullptr)
		ret.push_back(W_neighbor);

	return ret;
}

void block::updateSection()
{
	bool north = false;
	bool south = false;
	bool east = false;
	bool west = false;

	if (App->map->getBlockAt(position.x, position.y - 1) == nullptr || App->map->getBlockAt(position.x, position.y - 1)->type != AIR)
		north = true;
	if (App->map->getBlockAt(position.x, position.y + 1) == nullptr || App->map->getBlockAt(position.x, position.y + 1)->type != AIR)
		south = true;
	if (App->map->getBlockAt(position.x + 1, position.y) == nullptr || App->map->getBlockAt(position.x + 1, position.y)->type != AIR)
		east = true;
	if (App->map->getBlockAt(position.x - 1, position.y) == nullptr || App->map->getBlockAt(position.x - 1, position.y)->type != AIR)
		west = true;

	std::vector<bool> connections;
	connections.push_back(north);
	connections.push_back(south);
	connections.push_back(east);
	connections.push_back(west);

	section = App->map->getSectionFromBlockConnections(connections);	
}

void block::createCollider()
{
	if (collider == nullptr && type != AIR)
	{
		iPoint worldPos = App->map->MapToWorld({ position.x, position.y });
		collider = App->collisions->AddCollider(worldPos.x, worldPos.y, section.w, section.h, FLOOR_COLLIDER, true);
	}
	else
		LOG("This block already has a collider or it is AIR");
}
