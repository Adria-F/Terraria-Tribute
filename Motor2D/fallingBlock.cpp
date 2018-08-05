#include "fallingBlock.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1CollisionManager.h"
#include "j1Map.h"
#include "j1EntityManager.h"

fallingBlock::fallingBlock(int x, int y, block* Block): Entity(x, y, 16, 16, FALLING_BLOCK)
{
	texture = App->map->getBlockTexture(Block->type);
	section = Block->section;
	type = Block->type;
	onFloor = false;
	collider->type = FALLING_BLOCK_COLLIDER;
}

fallingBlock::~fallingBlock()
{
}

void fallingBlock::Draw(float dt)
{
	App->render->Blit(texture, position.x, position.y, &section);
}

void fallingBlock::OnCollision(Collider* c1, Collider* c2, collisionType type)
{
	iPoint newBlockPos = App->map->WorldToMap({ (int)position.x, (int)position.y });
	App->map->setBlock(newBlockPos.x, newBlockPos.y, this->type);

	to_delete = true;
}
