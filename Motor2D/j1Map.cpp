#include "j1Map.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1Textures.h"
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

	generateMap();

	return true;
}

bool j1Map::PreUpdate()
{
	if (App->input->GetKey(SDL_SCANCODE_G) == KEY_DOWN)
	{
		LOG("------------ New Map ------------");
		chuncks.clear();
		generateMap();
	}

	return true;
}

bool j1Map::Update(float dt)
{
	App->render->DrawQuad({ 0,0, worldData.world_width*BLOCK_SIZE, worldData.world_height*BLOCK_SIZE }, Cyan);

	int chunck_counter = 0;

	//Camera culling
	iPoint firstBlock = WorldToMap({ App->render->camera.x, App->render->camera.y });
	iPoint lastBlock = WorldToMap({ App->render->camera.x+App->render->camera.w, App->render->camera.y+App->render->camera.h });
	lastBlock.x += 1;
	lastBlock.y += 1;

	for (int i = firstBlock.x; i < lastBlock.x; i++)
	{
		for (int j = firstBlock.y; j < lastBlock.y; j++)
		{
			block* block = getBlockAt(i, j);
			if (block == nullptr || block->type == AIR)
				continue;

			iPoint pos = MapToWorld(block->position);
			Color color = { 150, 90, 60, 255 }; //Brown
			if (block->type == GRASS)
				color = { 0, 230, 0, 255 }; //Green
			else if (block->type == STONE)
				color = { 150, 150, 150, 255 }; //Gray

			if ((App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN || App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) && App->input->collidingMouse({ pos.x, pos.y, BLOCK_SIZE, BLOCK_SIZE }, true))
				removeBlock(block);

			if (App->render->debug)
			{
				App->render->DrawQuad({ pos.x, pos.y, BLOCK_SIZE, BLOCK_SIZE }, Black);
				App->render->DrawQuad({ pos.x + BLOCK_BORDER, pos.y + BLOCK_BORDER, BLOCK_SIZE - BLOCK_BORDER, BLOCK_SIZE - BLOCK_BORDER }, color);
			}
			else
			{
				App->render->Blit(getBlockTexture(block->type), pos.x, pos.y, &block->section);
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
	worldData.cleanCaveIterations = data.child("cleanCaveIterations").attribute("value").as_int();
}

void j1Map::generateMap()
{
	BROFILER_CATEGORY("Generate Map", Profiler::Color::DarkGreen);

	generateFlatMap();
	generateCaves(10);
	cleanMapNoise(worldData.cleanCaveIterations);
	cleanLonelyBlocks();

	updateBlocksConnections();
}

void j1Map::generateFlatMap()
{
	fillPerlinList(grassPerlin);
	fillPerlinList(stonePerlin, 1, 0);
	while (chuncks.size() * CHUNCK_WIDTH < worldData.world_width && (chuncks.size() < CHUNCKS_LOADED || CHUNCKS_LOADED == 0))
	{
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
				curr_chunck->blocks.push_back(curr_block);
			}
		}
		chuncks.push_back(curr_chunck);
	}
}

void j1Map::generateCaves(int amount)
{
	BROFILER_CATEGORY("Generate Cave", Profiler::Color::Blue);
		
	std::vector<SDL_Rect> caveList;

	int times = 0;
	while (times < amount)
	{
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
		times++;
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
					Block->type = AIR;
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

void j1Map::cleanMapNoise(int iterations)
{
	int times = 0;
	while (times < iterations)
	{
		for (int i = 0; i < worldData.world_width; i++)
		{
			for (int j = 0; j < worldData.world_height; j++)
			{
				convertBlockIntoNeighbors(i, j);
			}
		}
		times++;
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
	for (int i = 0; i < chuncks.size(); i++)
	{
		for (int j = 0; j < chuncks[i]->blocks.size(); j++)
		{
			block* block = chuncks[i]->blocks[j];
			for (int i = 0; i < MAX_TYPE; i++)
			{
				if (block->getNeighborsType((blockType)i) == 4)
					setBlock(block, (blockType)i);
			}
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
	{
		Block->type = AIR;

		std::vector<block*> neighbors = Block->getNeighbors();
		for (int i = 0; i < neighbors.size(); i++)
			neighbors[i]->updateSection();
	}
}

void j1Map::removeBlock(block* Block)
{
	Block->type = AIR;

	std::vector<block*> neighbors = Block->getNeighbors();
	for (int i = 0; i < neighbors.size(); i++)
		neighbors[i]->updateSection();
}

void j1Map::setBlock(int x, int y, blockType type)
{
	block* Block = getBlockAt(x, y);
	if (Block != nullptr)
		Block->type = type;
}

void j1Map::setBlock(block* Block, blockType type)
{
	Block->type = type;
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
