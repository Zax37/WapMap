#include "wTabbedArea.h"

#include "guichan/font.hpp"
#include "guichan/graphics.hpp"
#include "wTabbedArea.h"
#include "wButton.h"
#include "wLabel.h"
#include "wTab.h"

#include "../../WapMap/globals.h"
#include "../../WapMap/cInterfaceSheet.h"

extern cInterfaceSheet *_ghGfxInterface;

namespace SHR {
    Tab::Tab()
            : mHasMouse(false),
              mTabbedArea(NULL) {
        mLabel = new Lab();
        mLabel->setPosition(4, 4);
        add(mLabel);

        addMouseListener(this);
    }

    Tab::~Tab() {
        delete mLabel;
    }

    void Tab::adjustSize() {
        setSize(mLabel->getWidth() + 8,
                mLabel->getHeight() + 8);

        if (mTabbedArea != NULL) {
            mTabbedArea->adjustTabPositions();
        }
    }

    void Tab::setTabbedArea(TabbedArea *tabbedArea) {
        mTabbedArea = tabbedArea;
    }

    TabbedArea *Tab::getTabbedArea() {
        return mTabbedArea;
    }

    void Tab::setCaption(const std::string &caption) {
        mLabel->setCaption(caption);
        mLabel->adjustSize();
        adjustSize();
    }

    const std::string &Tab::getCaption() const {
        return mLabel->getCaption();
    }

    void Tab::draw(Graphics *graphics) {
        /*const Color &faceColor = 0x747474;//getBaseColor();
        const int alpha = getBaseColor().a;
        Color highlightColor = faceColor + 0x303030;
        highlightColor.a = alpha;
        Color shadowColor = faceColor - 0x303030;
        shadowColor.a = alpha;

        Color borderColor;
        Color baseColor;

		if (selected) {
            // Draw a border.
            graphics->setColor(highlightColor);
            graphics->drawLine(0, 0, getWidth() - 1, 0);
            graphics->drawLine(0, 1, 0, getHeight() - 1);
            graphics->setColor(shadowColor);
            graphics->drawLine(getWidth() - 1, 1, getWidth() - 1, getHeight() - 1);

            borderColor = highlightColor;
            baseColor = faceColor;
        } else {
            // Draw a border.
            /*graphics->setColor(shadowColor);
            graphics->drawLine(0, 0, getWidth() - 1, 0);
            graphics->drawLine(0, 1, 0, getHeight() - 1);
            graphics->drawLine(getWidth() - 1, 1, getWidth() - 1, getHeight() - 1);

            baseColor = faceColor - 0x151515;
            baseColor.a = alpha;
        }

        // Push a clip area so the other drawings don't need to worry
        // about the border.
        graphics->pushClipArea(gcn::Rectangle(1, 1, getWidth() - 2, getHeight() - 1));
        const gcn::Rectangle currentClipArea = graphics->getCurrentClipArea();

        graphics->setColor(baseColor);
        graphics->fillRectangle(gcn::Rectangle(0,
                                               0,
                                               currentClipArea.width,
                                               currentClipArea.height));*/

		int state = (mTabbedArea != NULL && mTabbedArea->isTabSelected(this)) ? 0
			: mHasMouse ? 2 : 1;

		int x, y;
		getAbsolutePosition(x, y);
		_ghGfxInterface->sprBreadcrumb[state][1]->RenderStretch(x, y, x + getWidth(), y + getHeight());

		graphics->setColor(GV->colLineDark);
		graphics->drawLine(getWidth() - 1, 1, getWidth() - 1, getHeight() - 1);
		if (state) {
			graphics->setColor(GV->colLineBright);
		}
		graphics->drawLine(0, 1, 0, getHeight() - 1);

        drawChildren(graphics);

        /*if (mTabbedArea != NULL
            && mTabbedArea->isFocused()
            && mTabbedArea->isTabSelected(this)) {
            graphics->setColor(Color(0x000000));
            graphics->drawRectangle(gcn::Rectangle(2,
                                                   2,
                                                   currentClipArea.width - 4,
                                                   currentClipArea.height - 4));
        }

        graphics->popClipArea();*/
    }

    void Tab::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = true;
    }

    void Tab::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = false;
    }
}

