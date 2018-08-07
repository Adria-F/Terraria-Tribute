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
