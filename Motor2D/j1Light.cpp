#include "j1Light.h"
#include "j1App.h"
#include "j1Render.h"
#include "Color.h"


Light::Light() : j1Module()
{
	name = "Light";
}


Light::~Light()
{
}

bool Light::Start() {

	light_screen.x = App->render->camera.x;
	light_screen.y = App->render->camera.y;
	light_screen.h = App->render->camera.h;
	light_screen.w = App->render->camera.w;

	light = { 0,0,0,0 };

	light_state = DAY;

	Light_changer.Start();

	for (int i = 0; i <= App->render->camera.w/16; i++)
	{
		for (int j = 0; j <= App->render->camera.h/16; j++)
		{
			screen_rects.push_back(new SDL_Rect({ i*16, j*16, 16, 16 }));
		}
	}

	return true;
}

bool Light::Update(float dt) {

	/*if (Light_changer.Read() >= 10) {

		if (light_state==NIGHT)
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
	}*/


	return true;
}

bool Light::PostUpdate(float dt) 
{

	for (list<SDL_Rect*>::iterator it = screen_rects.begin(); it != screen_rects.end(); it++)
	{
		SDL_Rect aux_rect = { (*it)->x,(*it)->y,(*it)->w,(*it)->h };
		App->render->DrawQuad(aux_rect, light, true, false);
	}

	return true;
}

bool Light::CleanUp(){
	return true;

}