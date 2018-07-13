#include "j1CollisionManager.h"
#include "j1Map.h"
#include "j1App.h"

j1CollisionManager::j1CollisionManager()
{
}

j1CollisionManager::~j1CollisionManager()
{
}

bool j1CollisionManager::Update(float dt)
{
	for (std::list<Collider*>::iterator it_c1 = nonStatic_colliders.begin(); it_c1 != nonStatic_colliders.end(); it_c1++)
	{
		std::list<Collider*>::iterator it_c1Copy = it_c1;
		for (std::list<Collider*>::iterator it_c2 = ++it_c1Copy; it_c2 != nonStatic_colliders.end(); it_c2++)
		{
			//CAMERA CULLING

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

		//Check static colliders around
		iPoint firstBlock = App->map->WorldToMap({ (*it_c1)->section.x, (*it_c1)->section.y });
		iPoint lastBlock = App->map->WorldToMap({ (*it_c1)->section.x + (*it_c1)->section.w, (*it_c1)->section.y + (*it_c1)->section.h });
		//Add margin of 2 in order to detect when collisions end
		firstBlock.x -= 2;
		if (firstBlock.x < 0)
			firstBlock.x = 0;
		firstBlock.y -= 2;
		if (firstBlock.y < 0)
			firstBlock.y = 0;
		lastBlock.x += 2;
		lastBlock.y += 2;
		
		for (int i = firstBlock.x; i < lastBlock.x; i++)
		{
			for (int j = firstBlock.y; j < lastBlock.y; j++)
			{
				block* block = App->map->getBlockAt(i, j);
				if (block == nullptr || block->type == AIR)
					continue;

				Collider* c2 = block->collider;
				if (c2 != nullptr)
				{
					collisionType type = checkCollision((*it_c1), c2);
					Collision collision((*it_c1), c2, type);

					int collisionExists = collisionAlreadyExists(collision);

					if (type != NO_COLLISION) //There is a collision
					{
						if (collisionExists == -1) //Collision did not exist previously
						{
							if ((*it_c1)->callback != nullptr)
								(*it_c1)->callback->OnCollision((*it_c1), c2, type);

							onGoingCollisions.push_back(collision); //Add it to the existing collisions
						}
						else //Collision did exist previously
						{
							if (onGoingCollisions[collisionExists].type == UNKNOWN_COLLISION && type != UNKNOWN_COLLISION)
								onGoingCollisions[collisionExists].type = type;

							if ((*it_c1)->callback != nullptr)
								(*it_c1)->callback->DuringCollision((*it_c1), c2, onGoingCollisions[collisionExists].type);
						}
					}
					else if (type == NO_COLLISION && collisionExists != -1) //Collision existed previously but they are no longer colliding
					{
						if ((*it_c1)->callback != nullptr)
							(*it_c1)->callback->OnEndCollision((*it_c1), c2, onGoingCollisions[collisionExists].type);

						onGoingCollisions.erase(onGoingCollisions.begin() + collisionExists); //Remove it from the existing collisions
					}
				}
			}
		}
	}

	return true;
}

bool j1CollisionManager::PostUpdate(float dt)
{
	if (App->render->debug) //Draw colliders in debug
	{
		for (std::list<Collider*>::iterator it_c = static_colliders.begin(); it_c != static_colliders.end(); it_c++)
		{
			Color color = { 0,0,255,155 };
			App->render->DrawQuad((*it_c)->section, color);
		}
	
		for (std::list<Collider*>::iterator it_c = nonStatic_colliders.begin(); it_c != nonStatic_colliders.end(); it_c++)
		{
			Color color = { 0,255,0,155 };
			App->render->DrawQuad((*it_c)->section, color);		
		}
	}

	return true;
}

bool j1CollisionManager::CleanUp()
{
	static_colliders.clear();
	nonStatic_colliders.clear();

	return true;
}

Collider* j1CollisionManager::AddCollider(int x, int y, int w, int h, colliderType type, bool isStatic, j1Module* callback)
{
	Collider* ret = nullptr;
	
	ret = new Collider({ x,y,w,h }, isStatic, callback);
	ret->type = type;
	if (isStatic)
		static_colliders.push_back(ret);
	else
		nonStatic_colliders.push_back(ret);

	return ret;
}

void j1CollisionManager::removeCollider(Collider* collider)
{
	if (collider->moveType == STATIC)
		static_colliders.remove(collider);
	else
		nonStatic_colliders.remove(collider);

	RELEASE(collider);
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

		float b_distance = c1->section.y + c1->section.h - c2->section.y;
		float t_distance = c2->section.y + c2->section.h - c1->section.y;
		float r_distance = c1->section.x + c1->section.w - c2->section.x;
		float l_distance = c2->section.x + c2->section.w - c1->section.x;

		if (b_distance < t_distance && b_distance < r_distance && b_distance < l_distance)
			ret = BOTTOM_COLLISION;
		else if (t_distance < b_distance && t_distance < r_distance && t_distance < l_distance)
			ret = TOP_COLLISION;
		else if (r_distance < b_distance && r_distance < t_distance && r_distance < l_distance)
			ret = RIGHT_COLLISION;
		else if (l_distance < b_distance && l_distance < t_distance && l_distance < r_distance)
			ret = LEFT_COLLISION;



		/*if (c1->section.y > c1->previousPos.y || c2->section.y < c2->previousPos.y) //C1 is down up or C2 is moving up
			ret = BOTTOM_COLLISION;
		else if (c1->section.y < c1->previousPos.y || c2->section.y > c2->previousPos.y) //C1 is moving up or C2 is moving down
			ret = TOP_COLLISION;

		if (ret == UNKNOWN_COLLISION)
		{
			if (c1->section.x > c1->previousPos.x || c2->section.x < c2->previousPos.x) //C1 is moving right or C2 is moving left
				ret = RIGHT_COLLISION;
			else if (c1->section.x < c1->previousPos.x || c2->section.x > c2->previousPos.x) //C1 is moving left or C2 is moving right
				ret = LEFT_COLLISION;
		}
		else if (c1->section.x != c1->previousPos.x || c2->section.x != c2->previousPos.x)
		{
			int c1VSpeed = abs(c1->section.y - c1->previousPos.y);
			int c2VSpeed = abs(c2->section.y - c2->previousPos.y);
			int c1HSpeed = abs(c1->section.x - c1->previousPos.x);
			int c2HSpeed = abs(c2->section.x - c2->previousPos.x);

			int VSpeed = (c1VSpeed > c2VSpeed) ? c1VSpeed : c2VSpeed;
			int HSpeed = (c1HSpeed > c2HSpeed) ? c1HSpeed : c2HSpeed;

			if (VSpeed > HSpeed)
			{
				if ((c1->section.x > c1->previousPos.x || c2->section.x < c2->previousPos.x) && c2->previousPos.x >= c1->previousPos.x+c1->section.w) //C1 is moving right or C2 is moving left
					ret = RIGHT_COLLISION;
				else if ((c1->section.x < c1->previousPos.x || c2->section.x > c2->previousPos.x) && c1->previousPos.x >= c2->previousPos.x + c2->section.w) //C1 is moving left or C2 is moving right
					ret = LEFT_COLLISION;
			}

		}*/
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

Collider::Collider(SDL_Rect section, bool isStatic, j1Module* callback) : section(section), previousPos({ section.x, section.y }), callback(callback)
{
	if (!isStatic)
		moveType = NON_STATIC;
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
