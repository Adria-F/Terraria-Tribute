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

	return true;
}

bool Light::Update(float dt) {

	/*if (Light_changer.Read() >= 1000) {

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

bool Light::PostUpdate(float dt) {

	App->render->DrawQuad(light_screen, light, true, false);

	return true;
}

bool Light::CleanUp(){
	return true;

}