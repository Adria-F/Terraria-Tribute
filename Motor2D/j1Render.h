#ifndef __j1RENDER_H__
#define __j1RENDER_H__

#include "SDL/include/SDL.h"
#include "p2Point.h"
#include "j1Module.h"
#include "Color.h"

#define CAMERA_SPEED 1000
#define CULLING_MARGIN 10

class j1Render : public j1Module
{
public:

	j1Render();

	// Destructor
	virtual ~j1Render();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();
	bool PostUpdate(float dt);

	// Called before quitting
	bool CleanUp();

	// Load / Save
	bool Load(pugi::xml_node&);
	bool Save(pugi::xml_node&) const;

	iPoint j1Render::ScreenToWorld(int x, int y) const;
	// Blit
	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();
	bool Blit(SDL_Texture* texture, int x, int y, const SDL_Rect* section = NULL, bool use_camera = true,SDL_RendererFlip flip = SDL_FLIP_NONE, float speed = 1.0f, double angle = 0, int pivot_x = INT_MAX, int pivot_y = INT_MAX) const;
	bool DrawQuad(SDL_Rect rect, Color& color, bool filled = true, bool use_camera = true) const;
	bool DrawLine(int x1, int y1, int x2, int y2, Color& color, bool use_camera = true) const;
	bool DrawCircle(int x1, int y1, int redius, Color& color, bool use_camera = true) const;

	// Set background color
	void SetBackgroundColor(SDL_Color color);

	void manageCameraMovement(float dt);

	bool collisionWithCamera(SDL_Rect rect);

public:

	SDL_Renderer*	renderer = nullptr;
	SDL_Rect		camera = { 0,0,0,0 };
	SDL_Rect		viewport = { 0,0,0,0 };
	SDL_Color		background = { 0,0,0,0 };

	bool debug = false;
};

#endif // __j1RENDER_H__