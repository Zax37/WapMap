#include "wLabel.h"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include <hgeFont.h>
#include "guichan/hge/hgeimagefont.hpp"

namespace SHR
{
	Lab::Lab()
	{
		mAlignment = Graphics::LEFT;
		mColor = 0xFFa1a1a1;
	}

	Lab::Lab(const std::string& caption)
	{
		mCaption = caption;
		mAlignment = Graphics::LEFT;

		setWidth(getFont()->getWidth(caption));
		setHeight(getFont()->getHeight() + 2);
		mColor = 0xFFa1a1a1;
	}

	const std::string &Lab::getCaption() const
	{
		return mCaption;
	}

	void Lab::setCaption(const std::string& caption)
	{
		mCaption = caption;
	}

	void Lab::setAlignment(Graphics::Alignment alignment)
	{
		mAlignment = alignment;
	}

	Graphics::Alignment Lab::getAlignment() const
	{
		return mAlignment;
	}

	void Lab::draw(Graphics* graphics)
	{
		int textX;
		//int textY = getHeight() / 2 - getFont()->getHeight() / 2;
		int textY = 0;

		int align;

		switch (getAlignment())
		{
		case Graphics::LEFT:
			textX = 0;
			align = HGETEXT_LEFT;
			break;
		case Graphics::CENTER:
			textX = getWidth() / 2;
			align = HGETEXT_CENTER;
			break;
		case Graphics::RIGHT:
			textX = getWidth();
			align = HGETEXT_RIGHT;
			break;
		default:
			throw GCN_EXCEPTION("Unknown alignment.");
		}
		int totalx, totaly;
		getAbsolutePosition(totalx, totaly);

		hgeFont * fnt = ((HGEImageFont*)getFont())->getHandleHGE();

		fnt->SetColor(SETA(mColor, getAlpha()));

		fnt->printfb(totalx, totaly, getWidth(), getHeight(), align | HGETEXT_TOP, 0, "%s", getCaption().c_str());

		//graphics->setFont(getFont());
		//graphics->setColor(getForegroundColor());
		//graphics->drawText(getCaption(), textX, textY, getAlignment());
	}

	void Lab::adjustSize()
	{
		setWidth(getFont()->getWidth(getCaption()));
		setHeight(getFont()->getHeight() + 2);
	}
}
