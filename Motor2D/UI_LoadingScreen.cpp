#include "UI_LoadingScreen.h"
#include "j1App.h"
#include "j1Window.h"
#include "j1Render.h"
#include "j1Gui.h"
#include "UI_Text.h"
#include "j1Fonts.h"

LoadingScreen::LoadingScreen(): UI_Element(0, 0, App->win->width, App->win->height)
{

}


LoadingScreen::~LoadingScreen()
{
}

void LoadingScreen::BlitElement(bool use_camera)
{
	if (isLoading)
	{
		App->render->DrawQuad(section, Black, true, use_camera);

		BlitChilds(use_camera);
	}
}

void LoadingScreen::loadingMessage(const char* text)
{
	if (this->text != nullptr)
		RELEASE(this->text);

	this->text = App->gui->createText(0, 0, text, App->font->getFont("BebasNeue-Regular.ttf", 70), false);
	this->text->setColor({ 255,255,255,255 });

	childs.clear();
	appendChild(this->text, true);
	
	isLoading = true;
}

void LoadingScreen::endLoadingScreen()
{
	isLoading = false;
}
