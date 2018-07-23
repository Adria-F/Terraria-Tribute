#include "j1Light.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
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

	for (int i = 0; i <= App->render->camera.w/32; i++)
	{
		for (int j = 0; j <= App->render->camera.h/32; j++)
		{
			screen_rects.push_back(new LightNode( i*32, j*32, 32, 32,0,0,0,0));
		}
	}

	return true;
}

bool Light::Update(float dt) 
{

	DayNight();

	App->input->GetMousePosition(mouse.x, mouse.y);	

	for (list<LightNode*>::iterator it = screen_rects.begin(); it != screen_rects.end(); it++)
	{
		if (SDL_PointInRect(&mouse, (*it)->Source))
		{
			(*it)->lColor->a = 0;
		}
		else
		{
			*(*it)->lColor = light;
		}
	}


	return true;
}

bool Light::PostUpdate(float dt) 
{
	for (list<LightNode*>::iterator it = screen_rects.begin(); it != screen_rects.end(); it++)
	{
		App->render->DrawQuad(*(*it)->Source, *(*it)->lColor, true, false);
	}

	return true;
}

bool Light::CleanUp(){
	return true;

}

int Light::DayNight()
{
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

	return 0;
}