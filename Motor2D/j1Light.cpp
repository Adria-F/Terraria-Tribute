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

	return true;
}

bool Light::PostUpdate(float dt) 
{
	int radius = 128;

	DayNight();

	App->input->GetMousePosition(mouse.x, mouse.y);
	test_mouse.x = mouse.x;
	test_mouse.y = mouse.y;

	for (list<LightNode*>::iterator it = screen_rects.begin(); it != screen_rects.end(); it++)
	{
		fPoint center = { (float)((*it)->Source->x + (*it)->Source->w / 2),(float)((*it)->Source->y + (*it)->Source->h / 2) };

		if (SDL_PointInRect(&mouse, (*it)->Source))
		{
			(*it)->lColor->a = 0;
		}
		else
		{
			if (App->render->camera.y < 500)
			{
				*(*it)->lColor = light;
			}
			else
			{
				(*it)->lColor->a = 200;
			}
		}

		if (test_mouse.DistanceTo(center) <= 128 && light.a>70)
		{
			(*it)->lColor->a = 70;
		}

		if (test_mouse.DistanceTo(center) <= 112 && light.a>60)
		{
			(*it)->lColor->a = 60;
		}

		if (test_mouse.DistanceTo(center) <= 96 && light.a>50)
		{
			(*it)->lColor->a = 50;
		}

		if (test_mouse.DistanceTo(center) <= 80 && light.a>40)
		{
			(*it)->lColor->a = 40;
		}

		if (test_mouse.DistanceTo(center) <= 64 && light.a>30)
		{
			(*it)->lColor->a = 30;
		}

		if (test_mouse.DistanceTo(center) <= 48 && light.a>20)
		{
			(*it)->lColor->a = 20;
		}

		if (test_mouse.DistanceTo(center) <= 32 && light.a>10)
		{
			(*it)->lColor->a = 10;
		}

		if (test_mouse.DistanceTo(center) <= 16 && light.a>0)
		{
			(*it)->lColor->a = 0;
		}

		App->render->DrawQuad(*(*it)->Source, *(*it)->lColor, true, false);

	}

	LOG("%d", light.a);
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
