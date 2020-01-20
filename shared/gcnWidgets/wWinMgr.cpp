#include "wWinMgr.h"

#include "guichan/exception.hpp"
#include "guichan/graphics.hpp"

namespace SHR
{

	WinMgr::WinMgr()
	{
		mOpaque = true;
	}

	WinMgr::~WinMgr()
	{

	}

	void WinMgr::draw(Graphics* graphics)
	{
		if (isOpaque())
		{
			graphics->setColor(getBaseColor());
			graphics->fillRectangle(Rectangle(0, 0, getWidth(), getHeight()));
		}

		drawChildren(graphics);
	}

	void WinMgr::setOpaque(bool opaque)
	{
		mOpaque = opaque;
	}

	bool WinMgr::isOpaque() const
	{
		return mOpaque;
	}

	void WinMgr::add(Widget* widget)
	{
		BasicContainer::add(widget);
	}

	void WinMgr::add(Widget* widget, int x, int y)
	{
		widget->setPosition(x, y);
		BasicContainer::add(widget);
	}

	void WinMgr::remove(Widget* widget)
	{
		BasicContainer::remove(widget);
	}

	void WinMgr::clear()
	{
		BasicContainer::clear();
	}

	Widget* WinMgr::findWidgetById(const std::string &id)
	{
		return BasicContainer::findWidgetById(id);
	}
}
