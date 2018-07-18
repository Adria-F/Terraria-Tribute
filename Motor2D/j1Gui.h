#ifndef __J1GUI_H__
#define __J1GUI_H__

#include "j1Module.h"
#include <list>

#define DEFAULT_UI_SPEED 150

class UI_Element;
class Text;
class LoadingScreen;

struct _TTF_Font;

class j1Gui : public j1Module
{
public:
	j1Gui()
	{}
	~j1Gui();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PostUpdate(float dt);

	// Called before quitting
	bool CleanUp();

	Text* createText(int x, int y, const char* text, _TTF_Font* font, bool addToElements = true);
	
	UI_Element* getElement(int id);

public:

	std::list<UI_Element*> elements;

	LoadingScreen* loadingScreen = nullptr;

};

#endif // __J1GUI_H__