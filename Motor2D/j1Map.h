#ifndef __J1MAP_H__
#define __J1MAP_H__

#include "j1Module.h"
#include "p2Point.h"
#include "j1Render.h"
#include <vector>
#include <map>

#define WORLD_WIDTH 400
#define WORLD_HEIGHT 100
#define GROUND_START_HEIGHT 20
#define STONE_START_HEIGHT 20
#define CHUNCK_WIDTH 8
#define BLOCK_SIZE 16
#define CHUNCKS_LOADED 0 //0 for unlimited
#define MAP_MAX_VARIATION 5
#define CLEAN_MAP_ITERATIONS 3

#define BLOCK_BORDER 1
#define CHUNCK_BORDER 3

//In Blocks
#define MAX_CAVE_SIZE {40, 30}
#define MIN_CAVE_SIZE {20, 15}
#define CAVES_MIN_SEPARATION 10

#define FRAME_TIME 15

#define FIRST_FALLING_BLOCK 20

#define FIRST_VEGETATION_BLOCK 50

struct SDL_Texture;
struct Collider;
struct Entity;

enum blockType
{
	AIR = 0,

	//STATICS
	GRASS,
	DIRT,
	STONE,

	//FALLING
	SAND = FIRST_FALLING_BLOCK,

	//VEGETATION
	TREE = FIRST_VEGETATION_BLOCK,
	CACTUS,

	MAX_TYPE
};

enum vegetationType
{
	TREE_PLANT,
	CACTUS_PLANT
};

enum generatingState
{
	TO_START_GENERATING = 0,

	GENERATE_FLAT_MAP,
	GENERATE_CAVES,
	CLEAN_NOISE,
	GENERATE_BIOMES,
	CLEAN_MAP,
	CREATE_COLLIDERS,
	CONNECT_BLOCKS,

	GENERATION_COMPLETED
};

enum biomeType
{
	PLAINS,
	DESERT
};

struct WorldData
{
	int world_width = WORLD_WIDTH;
	int world_height = WORLD_HEIGHT;

	int groundStartHeight = GROUND_START_HEIGHT;
	int stoneStartHeight = STONE_START_HEIGHT;

	int mapMaxVariation = MAP_MAX_VARIATION;

	iPoint maxCaveSize = MAX_CAVE_SIZE;
	iPoint minCaveSize = MIN_CAVE_SIZE;
	int cavesMinSeparation = CAVES_MIN_SEPARATION;
	int cleanMapIterations = CLEAN_MAP_ITERATIONS;
};

struct block
{
	blockType type = AIR;
	iPoint position = { 0,0 };
	SDL_Rect section = { 0,0,16,16 };
	Collider* collider = nullptr;

	SDL_Rect* Source;
	Color* lColor;
	int alpha;

	bool isLight = false;

	bool falling_block = false;

	int getNeighborsType(blockType type);
	std::vector<block*> getNeighbors();
	void updateSection();
	void createCollider();
};

struct chunck
{
	chunck(int chuncksAmount)
	{
		id = chuncksAmount + 1;
	}

	~chunck()
	{
		blocks.clear();
	}

	int id = 0;
	std::vector<block*> blocks;
	biomeType biome = PLAINS;
};

class j1Map : public j1Module
{
public:

	j1Map();
	~j1Map();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	void loadWorldData(pugi::xml_node data);

	void generateMap();
	void generateFlatMap();
	void createColliders();

	void generateCaves(int amount = 1);
	bool collidingWithList(SDL_Rect rect, std::vector<SDL_Rect> list, int margin = 0);
	void fillCaveMap(SDL_Rect area);

	void generateBiomes();

	void spawnVegetation(int x, int y, vegetationType type, int minHeight = 4, int maxHeight = 10);

	void cleanMapNoise(int iterations = 1);
	void convertBlockIntoNeighbors(int x, int y);
	blockType moreRepeatedNeighbor(block* Block);

	void cleanLonelyBlocks();
	void updateBlocksConnections();

	SDL_Rect getSectionFromBlockConnections(std::vector<bool> connections);

	iPoint MapToWorld(iPoint pos);
	iPoint WorldToMap(iPoint pos);

	void removeBlock(int x, int y);
	//Enter the block in order to avoid looking for it
	void removeBlock(block* Block, bool updateSurroundings = true);

	void setBlock(int x, int y, blockType type);
	//Enter the block in order to avoid looking for it
	void setBlock(block* Block, blockType type);

	void convertIntoFallingBlock(block* Block);

	SDL_Texture* getBlockTexture(blockType type);

	//Map coordinates
	chunck* getChunckAt(int x);
	//Map coordinates
	block* getBlockAt(int x, int y);
	
	std::vector<block*> getRadiusNeighbors(int radius,int x, int y);
	std::vector<block*> getOvalNeighbors(iPoint center, int radius);

	void loadBlocksTextures(pugi::xml_node textures);
	void loadBlocksConnectionsData(pugi::xml_node connections);

	//maxValue/minValue: range of noise in scales from 0 to 1
	//increment: percentage of maximum variation per block
	//tendenceMargin: 100-(probability) of the variation going to the same direction (up/down)
	void fillPerlinList(std::vector<float>& perlinList, int maxValue = 1, int minValue = -1, int increment = 10, int tendenceMargin = 20);

	void newGenerationState(generatingState newState);

public:

	std::vector<chunck*> chuncks;
	std::map<blockType, SDL_Texture*> blocks_textures;

	//bolean order {N, S, E, W}
	std::map<std::vector<bool>, SDL_Rect> tile_sections;

	WorldData worldData;

private:

	std::vector<float> grassPerlin;
	std::vector<float> stonePerlin;

	bool generatingMap = true;
	generatingState state = TO_START_GENERATING;
	int startFrameTime = 0;

	int iterations = 0;
	iPoint lastBlockOperated = { 0,0 };
};

#endif //__J1MAP_H__