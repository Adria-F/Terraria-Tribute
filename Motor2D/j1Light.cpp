#include "j1Light.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1Map.h"
#include "Color.h"

#include <time.h>
#include <stdlib.h>

Light::Light() : j1Module()
{
	name = "Light";
}


Light::~Light()
{
}

bool Light::Start() {

	Light_changer.Start();

	light_state = DAY;

	for (int i = 0; i <= App->render->camera.w/16; i++)
	{
		for (int j = 0; j <= App->render->camera.h/16; j++)
		{
			screen_rects.push_back(new LightNode( i*16, j*16, 16, 16,0,0,0,0));
		}
	}

	return true;
}

bool Light::Update(float dt) 
{
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
	{
		addLight();
	}

	return true;
}

bool Light::PostUpdate(float dt) 
{
	int radius = 128;

	LOG("%d", light.a);
	return true;
}

bool Light::CleanUp(){
	return true;

}

int Light::DayNight()
{
	int alpha = light.a;

	if (Light_changer.Read() >= 100) {

		if (light_state == NIGHT)
		{
			light.a++;
			if (light.a >= 200)
			{
				light_state = DAY;
			}
		}

		if (light_state == DAY)
		{
			light.a--;
			if (light.a <= 0)
			{
				light_state = NIGHT;
			}
		}
		Light_changer.Start();
	}

	return alpha;
}


void Light::addLight() {


	iPoint mouse;
	iPoint mouse_world;

	vector<block*> blocks;

	App->input->GetMousePosition(mouse.x, mouse.y);
	mouse.x += App->render->camera.x;
	mouse.y += App->render->camera.y;
	mouse_world = App->map->WorldToMap(mouse);

	block* block = App->map->getBlockAt(mouse_world.x, mouse_world.y);
	iPoint center = { block->position.x,block->position.y };
	blocks = App->map->getRadiusNeighbors(5,center.x,center.y);

	if (block)
	{
		if (block->isLight )
		{
			block->isLight = false;
			
			for (int i = 0; i < blocks.size(); i++)
				if (blocks[i])
				{
					blocks[i]->isLight = false;
				}
		}
		else
		{
			block->isLight = true;
			block->lColor->a = 0;

			for (int i = 0; i < blocks.size(); i++)
				if (blocks[i])
				{
					iPoint pos = blocks[i]->position;
					blocks[i]->isLight = true;

					blocks[i]->lColor->a = (20*pos.DistanceTo(center));
					blocks[i]->alpha = (20 * pos.DistanceTo(center));		
				}
		}
	}
}
