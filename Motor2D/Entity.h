#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "p2Point.h"
#include "j1Render.h"
#include "p2Animation.h"

#define DEFAULT_SPEED_INCREMENT 10
#define DEFAULT_JUMP_FORCE 750
#define GRAVITY 40

struct Collider;

enum entity_type
{
	NO_ENTITY = 0,

	PLAYER,
	ENEMY
};

enum entity_state
{
	IDLE = 0,

	MOVING,

	JUMPING,

	FALLING
};

enum faceLooking
{
	RIGHT = 0,
	LEFT
};

class Entity
{
public:
	Entity(float x, float y, int w, int h, entity_type type);
	~Entity()
	{}

	virtual void Update() //Calculate movement of the frame
	{}

	bool physicsUpdate(float dt); //Do the movement

	//Check collisions and act accordingly
	void OnCollision(Collider* c1, Collider* c2, collisionType type); 
	void DuringCollision(Collider* c1, Collider* c2, collisionType type);
	void OnEndCollision(Collider* c1, Collider* c2, collisionType type);

	virtual void Draw(float dt); //Draw the correct position


public:

	uint id = 0;
	entity_type type = NO_ENTITY;
	bool active = true;

	fPoint position = { 0.0f,0.0f };
	SDL_Rect section = { 0,0,20,30 };
	
	Collider* collider = nullptr;
	iPoint collider_offset = { 0,0 };
	bool onFloor = false;

	entity_state state = IDLE;
	faceLooking facing = RIGHT;
	Animation* animation = nullptr;

	float X_speed = 0.0f;
	float Y_speed = 0.0f;

	float maxXSpeed = 200.0f;
	float maxYSpeed = 500.0f;

	
};

#endif // __ENTITY_H__