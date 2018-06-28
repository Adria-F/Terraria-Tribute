#ifndef __J1ENTITYMANAGER_H__
#define __J1ENTITYMANAGER_H__

#include "j1Module.h"
#include <list>

#define DEFAULT_ENTITY_SPEED 150

class Entity;

class j1EntityManager : public j1Module
{
public:
	j1EntityManager()
	{}
	~j1EntityManager();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	bool OnCollision(Collider* c1, Collider* c2, collisionType type);
	bool OnEndCollision(Collider* c1, Collider* c2, collisionType type);

	Entity* getEntity(int id) const;
	Entity* getEntityByCollider(Collider* c) const;

public:

	std::list<Entity*> entities;
};

#endif / __J1ENTITYMANAGER_H__