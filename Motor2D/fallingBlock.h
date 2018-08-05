#ifndef __FALLINGBLOCK_H__
#define __FALLINGBLOCK_H__

#include "Entity.h"

struct block;
enum blockType;

class fallingBlock : public Entity
{
public:
	fallingBlock(int x, int y, block* Block);
	~fallingBlock();

	void Draw(float dt);

	void OnCollision(Collider* c1, Collider* c2, collisionType type);

public:

	SDL_Texture* texture = nullptr;
	SDL_Rect section = { 0,0,16,16 };
	blockType type = (blockType)0;
};

#endif // !__FALLINGBLOCK_H__