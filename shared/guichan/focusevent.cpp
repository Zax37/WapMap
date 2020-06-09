#include "guichan/focusevent.hpp"

namespace gcn {
    FocusEvent::FocusEvent(Widget* source, Widget* other, bool isKeyboardFocus)
                : Event(source),
                  mIsKeyboardFocus(isKeyboardFocus) {
        mWasStopped = false;
        mOtherOne = other;
    }

    FocusEvent::~FocusEvent() {

    }

    void FocusEvent::stopEvent() const {
        ((FocusEvent*)this)->mWasStopped = true;
    }

    bool FocusEvent::wasStopped() const {
        return mWasStopped;
    }

    bool FocusEvent::isKeyboardFocus() const {
        return mIsKeyboardFocus;
    }

    Widget* FocusEvent::getOtherSource() const {
        return mOtherOne;
    }
}
