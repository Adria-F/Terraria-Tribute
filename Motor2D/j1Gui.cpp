#include "j1Gui.h"
#include "UI_Element.h"
#include "UI_Text.h"
#include "UI_LoadingScreen.h"
#include "j1App.h"

j1Gui::~j1Gui()
{
	std::list<UI_Element*>::iterator it_e;
	it_e = elements.begin();
	while (it_e != elements.end())
	{
		RELEASE((*it_e));
		it_e++;
	}
	elements.clear();
}

bool j1Gui::Start()
{
	loadingScreen = new LoadingScreen();

	return true;
}

bool j1Gui::PostUpdate(float dt)
{
	for (std::list<UI_Element*>::iterator it_e = elements.begin(); it_e != elements.end(); it_e++)
	{
		if ((*it_e)->active)
			(*it_e)->BlitElement();
	}

	loadingScreen->BlitElement();

	return true;
}

bool j1Gui::CleanUp()
{
	return true;
}

Text* j1Gui::createText(int x, int y, const char* text, _TTF_Font* font, bool addToElements)
{
	Text* ret = new Text(text, x, y, font);

	if (addToElements)
		elements.push_back(ret);

	return ret;
}

UI_Element* j1Gui::getElement(int id)
{
	UI_Element* ret = nullptr;

	for (std::list<UI_Element*>::iterator it_e = elements.begin(); it_e != elements.end(); it_e++)
	{
		if ((*it_e)->id == id)
		{
			ret = (*it_e);
			break;
		}
	}

	return ret;
}