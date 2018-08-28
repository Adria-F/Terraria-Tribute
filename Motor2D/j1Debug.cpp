#include "j1Debug.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Gui.h"
#include "j1Input.h"
#include "j1Map.h"
#include "j1Fonts.h"
#include "UI_Text.h"
#include "j1Window.h"

j1Debug::j1Debug()
{
}

j1Debug::~j1Debug()
{
}

bool j1Debug::Start()
{
	constants.push_back(App->gui->createText(10, 10, "Block: ", App->font->getFont("BebasNeue-Regular.ttf", 20), false));
	constants.push_back(App->gui->createText(10, 40, "Chunck: ", App->font->getFont("BebasNeue-Regular.ttf", 20), false));
	constants.push_back(App->gui->createText(10, 70, "Biome: ", App->font->getFont("BebasNeue-Regular.ttf", 20), false));

	for (std::list<Text*>::iterator it_t = constants.begin(); it_t != constants.end(); it_t++)
	{
		(*it_t)->setColor({ 255,255,255,255 });
	}

	return true;
}

bool j1Debug::Update(float dt)
{
	if (App->render->debug)
	{
		iPoint mousePos;
		App->input->GetMousePosition(mousePos.x, mousePos.y);
		mousePos.x += App->render->camera.x;
		mousePos.y += App->render->camera.y;
		mousePos = App->map->WorldToMap(mousePos);

		chunck* Chunck = App->map->getChunckAt(mousePos.x);
		RELEASE(chunck_t);
		RELEASE(block_t);
		RELEASE(biome_t);
		std::string block_s = std::to_string(mousePos.x) + "," + std::to_string(mousePos.y);
		block_t = App->gui->createText(60, 10, block_s.c_str(), App->font->getFont("BebasNeue-Regular.ttf", 20), false);
		block_t->setColor({ 255,255,255,255 });
		chunck_t = App->gui->createText(70, 40, std::to_string(Chunck->id).c_str(), App->font->getFont("BebasNeue-Regular.ttf", 20), false);
		chunck_t->setColor({ 255,255,255,255 });
		biome_t = App->gui->createText(60, 70, "Plains", App->font->getFont("BebasNeue-Regular.ttf", 20), false);
		biome_t->setColor({ 0,255,0,255 });
		
		App->render->DrawQuad({ 0,0,App->win->width, 110 }, Color(0, 0, 0, 75), true, false);

		for (std::list<Text*>::iterator it_t = constants.begin(); it_t != constants.end(); it_t++)
		{
			(*it_t)->BlitElement();
		}

		block_t->BlitElement();
		chunck_t->BlitElement();
		biome_t->BlitElement();
	}

	return true;
}
