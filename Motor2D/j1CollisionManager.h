#ifndef __J1_COLLISIONMANAGER_H__
#define __J1_COLLISIONMANAGER_H__

#include "j1Module.h"
#include "j1Render.h"
#include <list>
#include <vector>

enum colliderType
{
	NO_COLLIDER = 0,

	PLAYER_COLLIDER,
	FLOOR_COLLIDER,

	MAX_COLLIDERS
};

enum collisionType
{
	NO_COLLISION = 0,

	UNKNOWN_COLLISION,
	TOP_COLLISION,
	BOTTOM_COLLISION,
	LEFT_COLLISION,
	RIGHT_COLLISION
};

struct Collider
{
	Collider(SDL_Rect section, j1Module* callback = nullptr);

	SDL_Rect section = { 0,0,0,0 };
	colliderType type = NO_COLLIDER;

	j1Module* callback = nullptr;

	iPoint previousPos = { 0,0 };

	void updatePosition(int x, int y, iPoint offset = { 0,0 });
};

struct Collision
{
	Collision(Collider* c1, Collider* c2, collisionType type): c1(c1), c2(c2), type(type)
	{}

	bool operator==(const Collision& other);

	Collider* c1 = nullptr;
	Collider* c2 = nullptr;
	collisionType type = NO_COLLISION;
};

class j1CollisionManager : public j1Module
{
public:
	j1CollisionManager();
	~j1CollisionManager();

	bool PreUpdate();

	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	Collider* AddCollider(int x, int y, int w, int h, colliderType type = NO_COLLIDER, j1Module* callback = nullptr);

	int collisionAlreadyExists(const Collision& collision);

	//Return collision type of c1 with c2
	collisionType checkCollision(Collider* c1, Collider* c2) const;

public:

	std::list<Collider*> colliders;

	std::vector<Collision> onGoingCollisions;
};

#endif //__J1_COLLISIONMANAGER_H__