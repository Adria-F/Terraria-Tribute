#ifndef __UI_LOADINGSCREEN__
#define __UI_LOADINGSCREEN__

#include "UI_Element.h"

class Text;

class LoadingScreen : public UI_Element
{
public:
	LoadingScreen();
	~LoadingScreen();

	void BlitElement(bool use_camera = false);

	void loadingMessage(const char* text);
	void endLoadingScreen();

public:

	bool isLoading = false;
	Text* text = nullptr;
};

#endif // !__UI_LOADINGSCREEN__