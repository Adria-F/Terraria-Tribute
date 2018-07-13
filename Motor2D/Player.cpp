#include "Player.h"
#include "j1App.h"
#include "j1CollisionManager.h"
#include "j1Input.h"
#include "j1Textures.h"

Player::Player(): Entity(0, 300, 40, 54, PLAYER) //586
{
	hair = App->tex->Load("assets/player/Player_Hair_1.png");
	head = App->tex->Load("assets/player/Player_Head.png");
	eyes = App->tex->Load("assets/player/Player_Eyes.png");
	eyeWhite = App->tex->Load("assets/player/Player_Eye_Whites.png");
	body = App->tex->Load("assets/player/Player_Shirt.png");
	hands = App->tex->Load("assets/player/Player_Undershirt.png");
	pants = App->tex->Load("assets/player/Player_Pants.png");
	feet = App->tex->Load("assets/player/Player_Shoes.png");

	idle = new Animation();
	idle->PushBack({ 0,0,40,54 });
	
	jump = new Animation();
	jump->PushBack({ 0,56 * 5,40,54 });

	walk = new Animation();
	for (int i = 6; i < 14; i++)
		walk->PushBack({ 0,0+(56*i),40,54 });
	walk->speed = 6.0f;

	animation = walk;

	collider->section.w = 24;
	collider->section.h = 45;
	collider_offset = { 8, 9 };
	collider->type = PLAYER_COLLIDER;
}

Player::~Player()
{
}

void Player::Update()
{
	

	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		state = MOVING;
		facing = LEFT;
		
		if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT)
		{
			if (onFloor)
				state = JUMPING;
		}
	}

	else if (state == MOVING && facing == LEFT)
		state = IDLE;

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		state = MOVING;
		facing = RIGHT;
		
		if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT)
		{
			if (onFloor)
				state = JUMPING;
		}
	}
	else if (state == MOVING && facing == RIGHT)
		state = IDLE;

	Entity::Update();
}

void Player::Draw(float dt)
{
	animationManager();

	SDL_RendererFlip flip = SDL_FLIP_NONE;
	if (facing == LEFT)
		flip = SDL_FLIP_HORIZONTAL;

	App->render->Blit(head, position.x, position.y, &animation->GetCurrentFrame(), true, flip);
	App->render->Blit(hair, position.x, position.y, &section, true, flip);
	App->render->Blit(eyes, position.x, position.y, &animation->GetCurrentFrame(), true, flip);
	App->render->Blit(eyeWhite, position.x, position.y, &animation->GetCurrentFrame(), true, flip);
	App->render->Blit(body, position.x, position.y, &animation->GetCurrentFrame(), true, flip);
	App->render->Blit(pants, position.x, position.y, &animation->GetCurrentFrame(), true, flip);
	App->render->Blit(feet, position.x, position.y, &animation->GetCurrentFrame(), true, flip);

	if (state == MOVING)
		animation->skipFrame(dt);

}

void Player::animationManager()
{
	switch (state)
	{
	case IDLE:
		animation = idle;
		break;
	case MOVING:
		animation = walk;
		break;
	case JUMPING:
		animation = jump;
		break;
	case FALLING:
		animation = jump;
		break;
	}
}
