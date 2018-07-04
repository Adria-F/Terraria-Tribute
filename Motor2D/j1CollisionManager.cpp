#include "j1CollisionManager.h"
#include "j1App.h"


j1CollisionManager::j1CollisionManager()
{
}

j1CollisionManager::~j1CollisionManager()
{
}

bool j1CollisionManager::Update(float dt)
{
	for (std::list<Collider*>::iterator it_c1 = colliders.begin(); it_c1 != colliders.end(); it_c1++)
	{
		std::list<Collider*>::iterator it_c1Copy = it_c1;
		for (std::list<Collider*>::iterator it_c2 = ++it_c1Copy; it_c2 != colliders.end(); it_c2++)
		{
			collisionType type = checkCollision((*it_c1), (*it_c2));
			Collision collision((*it_c1), (*it_c2), type);

			int collisionExists = collisionAlreadyExists(collision);

			if (type != NO_COLLISION) //There is a collision
			{
				if (collisionExists == -1) //Collision did not exist previously
				{
					if ((*it_c1)->callback != nullptr)
						(*it_c1)->callback->OnCollision((*it_c1), (*it_c2), type);
					if ((*it_c2)->callback != nullptr)
						(*it_c2)->callback->OnCollision((*it_c2), (*it_c1), inverseCollision(type));

					onGoingCollisions.push_back(collision); //Add it to the existing collisions
				}
				else //Collision did exist previously
				{
					if ((*it_c1)->callback != nullptr)
						(*it_c1)->callback->DuringCollision((*it_c1), (*it_c2), onGoingCollisions[collisionExists].type);
					if ((*it_c2)->callback != nullptr)
						(*it_c2)->callback->DuringCollision((*it_c2), (*it_c1), inverseCollision(onGoingCollisions[collisionExists].type));
				}
			}
			else if (type == NO_COLLISION && collisionExists != -1) //Collision existed previously but they are no longer colliding
			{
				if ((*it_c1)->callback != nullptr)
					(*it_c1)->callback->OnEndCollision((*it_c1), (*it_c2), onGoingCollisions[collisionExists].type);
				if ((*it_c2)->callback != nullptr)
					(*it_c2)->callback->OnEndCollision((*it_c2), (*it_c1), inverseCollision(onGoingCollisions[collisionExists].type));

				onGoingCollisions.erase(onGoingCollisions.begin() + collisionExists); //Remove it from the existing collisions
			}
		}
	}

	return true;
}

bool j1CollisionManager::PostUpdate(float dt)
{
	if (App->render->debug) //Draw colliders in debug
	{
		for (std::list<Collider*>::iterator it_c = colliders.begin(); it_c != colliders.end(); it_c++)
		{
			Color color = { 255,255,255, 155 };
			switch ((*it_c)->type)
			{
			case PLAYER_COLLIDER:
				color = { 0,255,0,155 };
				break;
			case FLOOR_COLLIDER:
				color = { 0,0,255,155 };
				break;
			}

			App->render->DrawQuad((*it_c)->section, color);
		}
	}

	return true;
}

bool j1CollisionManager::CleanUp()
{
	colliders.clear();

	return true;
}

Collider* j1CollisionManager::AddCollider(int x, int y, int w, int h, colliderType type, j1Module* callback)
{
	Collider* ret = nullptr;
	
	ret = new Collider({ x,y,w,h }, callback);
	ret->type = type;
	colliders.push_back(ret);

	return ret;
}

int j1CollisionManager::collisionAlreadyExists(const Collision& collision)
{
	int ret = -1;

	for (int i = 0; i < onGoingCollisions.size(); i++)
	{
		if (onGoingCollisions[i] == collision)
		{
			ret = i;
			break;
		}
	}

	return ret;
}

collisionType j1CollisionManager::checkCollision(Collider* c1, Collider* c2) const
{
	collisionType ret = NO_COLLISION;

	//If there is collision
	if (c1->section.x < c2->section.x + c2->section.w &&
		c1->section.x + c1->section.w > c2->section.x &&
		c1->section.y < c2->section.y + c2->section.h &&
		c1->section.h + c1->section.y > c2->section.y)
	{
		ret = UNKNOWN_COLLISION;

		if (c1->section.y > c1->previousPos.y || c2->section.y < c2->previousPos.y) //C1 is moving up or C2 is moving down
			ret = BOTTOM_COLLISION;
		else if (c1->section.y < c1->previousPos.y || c2->section.y > c2->previousPos.y) //C1 is moving dow or C2 is moving up
			ret = TOP_COLLISION;
		else if (c1->section.x > c1->previousPos.x || c2->section.x < c2->previousPos.x) //C1 is moving right or C2 is moving left
			ret = RIGHT_COLLISION;
		else if (c1->section.x < c1->previousPos.x || c2->section.x > c2->previousPos.x) //C1 is moving left or C2 is moving right
			ret = LEFT_COLLISION;
	}


	return ret;
}

collisionType j1CollisionManager::inverseCollision(collisionType original) const
{
	collisionType ret = UNKNOWN_COLLISION;

	switch (original)
	{
	case NO_COLLISION:
		ret = NO_COLLISION;
		break;
	case TOP_COLLISION:
		ret = BOTTOM_COLLISION;
		break;
	case BOTTOM_COLLISION:
		ret = TOP_COLLISION;
		break;
	case LEFT_COLLISION:
		ret = RIGHT_COLLISION;
		break;
	case RIGHT_COLLISION:
		ret = LEFT_COLLISION;
		break;
	}

	return ret;
}

Collider::Collider(SDL_Rect section, j1Module* callback) : section(section), previousPos({ section.x, section.y }), callback(callback)
{
}

void Collider::updatePosition(int x, int y, iPoint offset)
{
	previousPos = { section.x, section.y };
	section.x = x + offset.x;
	section.y = y + offset.y;
}

bool Collision::operator==(const Collision& other)
{
	return (this->c1 == other.c1 && this->c2 == other.c2);
}
