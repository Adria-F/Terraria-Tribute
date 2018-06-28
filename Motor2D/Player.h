#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Entity.h"

class Player : public Entity
{
public:
	Player();
	~Player();

	void Update();

	void Draw(float dt);
	void animationManager();

private:

	Animation* idle = nullptr;
	Animation* walk = nullptr;
	Animation* jump = nullptr;
	Animation* dig = nullptr;

	SDL_Texture* hair = nullptr;
	SDL_Texture* head = nullptr;
	SDL_Texture* eyes = nullptr;
	SDL_Texture* eyeWhite = nullptr;
	SDL_Texture* body = nullptr;
	SDL_Texture* hands = nullptr;
	SDL_Texture* pants = nullptr;
	SDL_Texture* feet = nullptr;
};

#endif //__PLAYER_H__