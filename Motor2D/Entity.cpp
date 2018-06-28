#include "Entity.h"
#include "j1Module.h"
#include "j1App.h"
#include "j1CollisionManager.h"

Entity::Entity(float x, float y, int w, int h, entity_type type) : position({ x,y }), type(type), section({0,0,w,h})
{
	collider = App->collisions->AddCollider(x, y, section.w, section.h, NO_COLLIDER, (j1Module*)App->entitymanager);
}

void Entity::Draw(float dt)
{
	Color usingColor = White;
	switch (type)
	{
	case PLAYER:
		usingColor = Blue;
		break;
	case ENEMY:
		usingColor = Red;
		break;
	}

	App->render->DrawQuad({ (int)position.x, (int)position.y, section.w, section.h }, usingColor);
}

bool Entity::physicsUpdate(float dt)
{
	if (state == MOVING)
	{
		if (facing == RIGHT)
		{
			if (X_speed < maxXSpeed)
				X_speed += DEFAULT_SPEED_INCREMENT;
			if (X_speed > maxXSpeed)
				X_speed = maxXSpeed;
		}
		else if (facing == LEFT)
		{
			if (X_speed > -maxXSpeed)
				X_speed -= DEFAULT_SPEED_INCREMENT;
			if (X_speed < -maxXSpeed)
				X_speed = -maxXSpeed;
		}
	}
	else
	{
		if (X_speed > 0)
			X_speed -= DEFAULT_SPEED_INCREMENT;
		else if (X_speed < 0)
			X_speed += DEFAULT_SPEED_INCREMENT;
	}

	if (state == JUMPING && Y_speed == 0)
	{
		if (onFloor)
			Y_speed = -DEFAULT_JUMP_FORCE;
		else
		{
			state = FALLING;
		}
	}

	if (!onFloor && state != FALLING && state != JUMPING)
	{
		state = FALLING;
	}

	if (state == JUMPING || state == FALLING)
	{
		if (Y_speed < maxYSpeed)
		{
			if (Y_speed < 0)
			{
				Y_speed += GRAVITY;
				if (Y_speed > 0)
					Y_speed = 0;
			}
			else
				Y_speed += GRAVITY;
		}
		if (Y_speed > maxYSpeed)
			Y_speed = maxYSpeed;
	}

	position.x += X_speed * dt;
	position.y += Y_speed * dt;

	collider->updatePosition(position.x, position.y, collider_offset);

	return true;
}

void Entity::OnCollision(Collider* c1, Collider* c2, collisionType type)
{
	if (c2->type == FLOOR_COLLIDER && type == TOP_COLLISION)
	{
		Y_speed = 0;
		position.y = c2->section.y -c1->section.h - collider_offset.y + 1;
		onFloor = true;
		if (state == FALLING)
			state = IDLE;
	}
}

void Entity::OnEndCollision(Collider* c1, Collider* c2, collisionType type)
{
	if (c2->type == FLOOR_COLLIDER && type == TOP_COLLISION)
		onFloor = false;
}
