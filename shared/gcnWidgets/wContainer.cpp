#include "wContainer.h"

#include "guichan/exception.hpp"
#include "guichan/graphics.hpp"

#ifdef WAP_MAP

#include "../../WapMap/globals.h"

#endif

#ifdef WAP_GRAPHIC
#include "../../WapGraphic/globals.h"
#endif

extern HGE *hge;

using namespace gcn;

namespace SHR {

    Container::Container() {
        mOpaque = true;
        mForegroundColor = 0xFF00FF;
        bHide = 0;
        mAlphaMod = 1.0f;
        addWidgetListener(this);
    }

    Container::~Container() {

    }

    void Container::setShow(bool b) {
        bHide = !b;
        if (!isVisible() && b) {
            setVisible(1);
            mAlphaMod = 0.0f;
        }
    }

    void Container::widgetShown(const Event &event) {
        bHide = 0;
    }

    void Container::widgetHidden(const Event &event) {
        mAlphaMod = 0;
        bHide = 0;
    }

    void Container::draw(Graphics *graphics) {
        if (!bHide && mAlphaMod < 1.0f) {
            mAlphaMod += hge->Timer_GetDelta() * 5.0f;
            if (mAlphaMod > 1.0f) mAlphaMod = 1.0f;
        } else if (bHide && (mAlphaMod > 0.0f || isVisible())) {
            mAlphaMod -= hge->Timer_GetDelta() * 5.0f;
            if (mAlphaMod <= 0.0f) {
                mAlphaMod = 0.0f;
                setVisible(0);
                bHide = 0;
            }
        }

        if (isOpaque()) {
            graphics->setColor(GV->colBaseGCN);
            graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
        }

        drawChildren(graphics);
    }

    void Container::setOpaque(bool opaque) {
        mOpaque = opaque;
    }

    bool Container::isOpaque() const {
        return mOpaque;
    }

    void Container::add(Widget *widget) {
        BasicContainer::add(widget);
    }

    void Container::add(Widget *widget, int x, int y) {
        widget->setPosition(x, y);
        BasicContainer::add(widget);
    }

    void Container::remove(Widget *widget) {
        BasicContainer::remove(widget);
    }

    void Container::clear() {
        BasicContainer::clear();
    }

    gcn::Widget *Container::findWidgetById(const std::string &id) {
        return BasicContainer::findWidgetById(id);
    }
}
