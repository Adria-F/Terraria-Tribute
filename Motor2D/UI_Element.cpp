#include "UI_Element.h"

iPoint UI_Element::getGlobalPosition()
{
	iPoint ret = { (int)local_position.x, (int)local_position.y };

	if (parent != nullptr)
		ret += parent->getGlobalPosition();

	return ret;
}

void UI_Element::appendChild(UI_Element* child, bool center)
{
	if (center)
	{
		fPoint child_pos(section.w / 2, section.h / 2);
		child_pos.x -= child->section.w / 2;
		child_pos.y -= child->section.h / 2;
		child->local_position = child_pos;
	}
	child->parent = this;
	childs.push_back(child);
}

void UI_Element::BlitChilds(bool use_camera)
{
	for (std::list<UI_Element*>::iterator it_e = childs.begin(); it_e != childs.end(); it_e++)
	{
		if ((*it_e)->active)
			(*it_e)->BlitElement(use_camera);
	}
}