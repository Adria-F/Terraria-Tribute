#include "UI_Text.h"
#include "j1App.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Render.h"
#include "Brofiler\Brofiler.h"

Text::Text(std::string text, int x, int y, _TTF_Font* font): UI_Element(x, y, 0, 0),
text(text),
font(font)
{
	color = { 0,0,0,255 };
	createTexture();
}

Text::~Text()
{
	if (texture != nullptr)
	{
		App->tex->UnLoad(texture);
		texture = nullptr;
	}
}

void Text::createTexture()
{
	if (texture != nullptr)
	{
		App->tex->UnLoad(texture);
		texture = nullptr;
	}

	texture = App->font->Print(text.c_str(), &color, font); //Normal texture
	App->tex->GetSize(texture, tex_width, tex_height);
	section.w = tex_width;
	section.h = tex_height;
}

void Text::BlitElement(bool use_camera)
{
	if (active)
	{
		BROFILER_CATEGORY("Text Blit", Profiler::Color::Fuchsia);

		if (texture != nullptr)
		{
			iPoint globalPos = getGlobalPosition();

			App->render->Blit(texture, globalPos.x, globalPos.y, NULL, use_camera);
		}
	}
}

void Text::setColor(SDL_Color color)
{
	if (this->color.r != color.r || this->color.g != color.g || this->color.b != color.b || this->color.a != color.a)
	{
		this->color = color;
		createTexture();
	}
}
