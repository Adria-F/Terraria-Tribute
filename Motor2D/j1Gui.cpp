#include "j1Gui.h"
#include "UI_Element.h"
#include "UI_TextBox.h"
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

	return true;
}

bool j1Gui::Update(float dt)
{
	for (std::list<UI_Element*>::iterator it_e = elements.begin(); it_e != elements.end(); it_e++)
	{
		if ((*it_e)->active)
			(*it_e)->BlitElement();
	}

	return true;
}

bool j1Gui::CleanUp()
{
	return true;
}

TextBox* j1Gui::createTextBox(int x, int y)
{
	TextBox* ret = new TextBox(x, y, 0, 0);
	elements.push_back(ret);

	ret->id = elements.size();

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