#ifndef __J1LIGHT_H__
#define	__J1LIGHT_H__

#include "j1Module.h"
#include "Color.h"
#include "j1Timer.h"
#include "p2Point.h"

#include <list>

#include "SDL/include/SDL.h"

using namespace std;

struct SDL_Rect;

enum Light_State{

	NONE,
	DAY,
	NIGHT,
	LIGHT_OBJECT,

};

struct LightNode {

	LightNode(int x, int y, int w, int h, float r, float g, float b, float alpha)
	{
		Source = new SDL_Rect({ x,y,w,h });
		lColor = new Color({ r,g,b,alpha });
		type = NONE;
	}

	SDL_Rect* Source;
	Color* lColor;
	Light_State type;
	bool isLight = false;

};

class Light : public j1Module
{
public:
	Light();
	~Light();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update(float dt);

	bool PostUpdate(float dt);

	// Called before quitting
	bool CleanUp();

	int DayNight();

	void addLight();

public:

	SDL_Rect light_screen = {0, 0, 0, 0};

	list<LightNode*> screen_rects;
	list<LightNode*> all_Lights;

	Color light;

	Light_State light_state;

	j1Timer Light_changer;

	SDL_Point mouse;

	fPoint test_mouse;

};

#endif // !__J1LIGHT_H__

