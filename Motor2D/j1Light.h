#ifndef __J1LIGHT_H__
#define	__J1LIGHT_H__

#include "j1Module.h"
#include "Color.h"
#include "j1Timer.h"

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

public:

	SDL_Rect light_screen = {0, 0, 0, 0};

	list<SDL_Rect*> screen_rects;

	Color light;

	Light_State light_state;

	j1Timer Light_changer;

};

#endif // !__J1LIGHT_H__

