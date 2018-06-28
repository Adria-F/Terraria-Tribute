#include "j1EntityManager.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
#include "Entity.h"
#include "Player.h"
#include "j1CollisionManager.h"

j1EntityManager::~j1EntityManager()
{
	std::list<Entity*>::iterator it_e;
	it_e = entities.begin();
	while (it_e != entities.end())
	{
		RELEASE((*it_e));
		it_e++;
	}
	entities.clear();
}

bool j1EntityManager::Start()
{
	 Entity* player = new Player();
	 entities.push_back(player);

	return true;
}

bool j1EntityManager::Update(float dt)
{
	for (std::list<Entity*>::iterator it_e = entities.begin(); it_e != entities.end(); it_e++)
	{
		if ((*it_e)->active)
		{
			(*it_e)->Update();
			//CAMERA CULLING
			(*it_e)->Draw(dt);
		}
	}

	for (std::list<Entity*>::iterator it_e = entities.begin(); it_e != entities.end(); it_e++)
	{
		if ((*it_e)->active)
		{
			(*it_e)->physicsUpdate(dt);
		}
	}

	return true;
}

bool j1EntityManager::CleanUp()
{
	std::list<Entity*>::iterator it_e = entities.begin();
	while (it_e != entities.end())
	{
		RELEASE((*it_e));
		it_e++;
	}
	entities.clear();

	return true;
}

bool j1EntityManager::OnCollision(Collider* c1, Collider* c2, collisionType type)
{
	Entity* entity = getEntityByCollider(c1);
	if (entity != nullptr)
		entity->OnCollision(c1, c2, type);

	return true;
}

bool j1EntityManager::OnEndCollision(Collider* c1, Collider* c2, collisionType type)
{
	Entity* entity = getEntityByCollider(c1);
	if (entity != nullptr)
		entity->OnEndCollision(c1, c2, type);

	return true;
}

Entity* j1EntityManager::getEntity(int id) const
{
	Entity* ret = nullptr;

	for (std::list<Entity*>::const_iterator it_e = entities.begin(); it_e != entities.end(); it_e++)
	{
		if ((*it_e)->id == id)
		{
			ret = (*it_e);
			break;
		}
	}

	return ret;
}

Entity* j1EntityManager::getEntityByCollider(Collider * c) const
{
	Entity* ret = nullptr;

	for (std::list<Entity*>::const_iterator it_e = entities.begin(); it_e != entities.end(); it_e++)
	{
		if ((*it_e)->collider == c)
		{
			ret = (*it_e);
			break;
		}
	}

	return ret;
}
