#ifndef __J1_MODULE_H__
#define __J1_MODULE_H__

#include "j1Module.h"
#include <list>

class Text;

class j1Debug : public j1Module
{
public:
	j1Debug();
	~j1Debug();

	bool Start();

	bool Update(float dt);

public:

	std::list<Text*> constants;
	Text* block_t = nullptr;
	Text* chunck_t = nullptr;
	Text* biome_t = nullptr;
};

#endif // !__J1_MODULE_H__