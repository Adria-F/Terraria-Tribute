#ifndef __j1FONTS_H__
#define __j1FONTS_H__

#include "j1Module.h"
#include <list>
#include <map>

#define DEFAULT_FONT "fonts/BebasNeue-Regular.ttf"
#define DEFAULT_FONT_SIZE 18
#define FONTS_FOLDER "fonts"

struct SDL_Texture;
struct SDL_Color;
struct _TTF_Font;

struct FontData
{
	FontData(const char* path, int size): path(path), size(size)
	{}

	std::string path = "";
	int size = 12;

	bool operator<(const FontData& other) const
	{
		return (this->size < other.size);
	}
};

class j1Fonts : public j1Module
{
public:

	j1Fonts();

	// Destructor
	virtual ~j1Fonts();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before quitting
	bool CleanUp();

	_TTF_Font* const getFont(const char* file, int size);

	// Load Font
	_TTF_Font* const Load(const char* path, int size = 12);

	// Create a surface from text
	SDL_Texture* Print(const char* text, SDL_Color* color, _TTF_Font* font = NULL);

	bool CalcSize(const char* text, int& width, int& height, _TTF_Font* font = NULL) const;

public:

	std::map<FontData, _TTF_Font*> fonts;
	_TTF_Font*				default_font = nullptr;
};


#endif // __j1FONTS_H__