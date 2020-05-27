#include "wSlider.h"

#include "guichan/graphics.hpp"
#include "guichan/key.hpp"
#include "guichan/mouseinput.hpp"
#include "../../WapMap/cInterfaceSheet.h"

extern cInterfaceSheet *_ghGfxInterface;
extern HGE *hge;

using namespace gcn;

namespace SHR {
    void sliderDrawBar(int dx, int dy, bool orient, int size, int type, DWORD col) {
        for (int i = 0; i < 3; i++)
            _ghGfxInterface->sprScrollbar[orient][type][i]->SetColor(col);

        _ghGfxInterface->sprScrollbar[orient][type][0]->Render(dx, dy);
        if (orient) {
            _ghGfxInterface->sprScrollbar[orient][type][1]->RenderStretch(dx, dy + 9, dx + 16, dy + size - 9);
            _ghGfxInterface->sprScrollbar[orient][type][2]->Render(dx, dy + size - 9);
        } else {
            _ghGfxInterface->sprScrollbar[orient][type][1]->RenderStretch(dx + 9, dy, dx + size - 9, dy + 16);
            _ghGfxInterface->sprScrollbar[orient][type][2]->Render(dx + size - 9, dy);
        }

    }

    Slider::Slider(double scaleEnd) {
        mDragged = false;

        mScaleStart = 0;
        mScaleEnd = scaleEnd;

        setFocusable(true);
        //setFrameSize(1);
        setOrientation(HORIZONTAL);
        setValue(0);
        setStepLength(scaleEnd / 10);
        setMarkerLength(10);

        addMouseListener(this);
        addKeyListener(this);
        mStyle = DEFAULT;
        fTimer = 0;
        mHasMouse = 0;
        mForceKeyValue = 0;
    }

    Slider::Slider(double scaleStart, double scaleEnd) {
        mDragged = false;

        mScaleStart = scaleStart;
        mScaleEnd = scaleEnd;

        setFocusable(true);
        //setFrameSize(1);
        setOrientation(HORIZONTAL);
        setValue(scaleStart);
        setStepLength((scaleEnd - scaleStart) / 10);
        setMarkerLength(10);

        addMouseListener(this);
        addKeyListener(this);
        mStyle = DEFAULT;
        fTimer = 0;
        mHasMouse = 0;
        mForceKeyValue = 0;
    }

    void Slider::setScale(double scaleStart, double scaleEnd) {
        mScaleStart = scaleStart;
        mScaleEnd = scaleEnd;
    }

    double Slider::getScaleStart() const {
        return mScaleStart;
    }

    void Slider::setScaleStart(double scaleStart) {
        mScaleStart = scaleStart;
    }

    double Slider::getScaleEnd() const {
        return mScaleEnd;
    }

    void Slider::setScaleEnd(double scaleEnd) {
        mScaleEnd = scaleEnd;
    }

    bool Slider::showHand() {
        int dx, dy;
        getAbsolutePosition(dx, dy);
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        if (mStyle == DEFAULT) {
            bool orient = mOrientation == VERTICAL;
            if (orient)
                dy += getHeight() - getMarkerPosition() - getMarkerLength();
            else
                dx += getMarkerPosition();
            int markdimx = (orient ? 16 : getMarkerLength()),
                    markdimy = (orient ? getMarkerLength() : 16);
            return (mHasMouse && mx > dx && my > dy && mx < dx + markdimx && my < dy + markdimy);
        } else {
            int markspr = (mStyle == POINTER);
            int v = getMarkerPosition();
            if (mOrientation == HORIZONTAL)
                return (mHasMouse && mx > dx + v && mx < dx + v + getMarkerLength() && my > dy && my < dy + 12);
            else if (mOrientation == VERTICAL)
                return (mHasMouse && mx > dx && mx < dx + 12 && my > dy + v && my < dy + v + getMarkerLength());
        }
    }

    void Slider::draw(gcn::Graphics *graphics) {
        Color shadowColor = getBaseColor() - 0x101010;
        int alpha = getBaseColor().a;
        shadowColor.a = alpha;

        int dx, dy;
        getAbsolutePosition(dx, dy);

        float mx, my;
        hge->Input_GetMousePos(&mx, &my);

        if (mStyle == DEFAULT) {
            bool orient = (mOrientation == VERTICAL);
            sliderDrawBar(dx, dy, orient, (orient ? getHeight() : getWidth()), 0, 0xFFFFFFFF);
            if (orient)
                dy += getHeight() - getMarkerPosition() - getMarkerLength();
            else
                dx += getMarkerPosition();
            if (isEnabled())
                sliderDrawBar(dx, dy, orient, getMarkerLength(), 1, 0xFFFFFFFF);
            int markDimX = (orient ? 16 : getMarkerLength()),
                markDimY = (orient ? getMarkerLength() : 16);
            bool markerFocus = (mHasMouse && mx > dx && my > dy && mx < dx + markDimX && my < dy + markDimY);

            if (markerFocus && hge->Input_GetKeyState(HGEK_LBUTTON) && fTimer < 0.4f) {
                fTimer += hge->Timer_GetDelta();
                if (fTimer > 0.4f) fTimer = 0.4f;
            } else if (!(markerFocus && hge->Input_GetKeyState(HGEK_LBUTTON)) && fTimer > 0.2f) {
                fTimer -= hge->Timer_GetDelta();
                if (fTimer < 0.2f) fTimer = 0.2f;
            } else if (markerFocus && fTimer < 0.2f) {
                fTimer += hge->Timer_GetDelta();
                if (fTimer > 0.2f) fTimer = 0.2f;
            } else if (!markerFocus && fTimer > 0.0f) {
                fTimer -= hge->Timer_GetDelta();
                if (fTimer < 0.0f) fTimer = 0.0f;
            }
            if (isEnabled())
                sliderDrawBar(dx, dy + 1, orient, getMarkerLength(), 2,
                              SETA(0xFFFFFFFF, (unsigned char) (fTimer * 2.5f * 255.0f)));
        } else {
            int markSpr = (mStyle == POINTER);
            if (mOrientation == HORIZONTAL) {
                _ghGfxInterface->sprSliderBG[0]->Render(dx, dy + 3);
                _ghGfxInterface->sprSliderBG[1]->RenderStretch(dx + 5, dy + 3, dx + getWidth() - 5, dy + 8);
                _ghGfxInterface->sprSliderBG[2]->Render(dx + getWidth() - 5, dy + 3);

                for (double vKey : vKeys) {
                    int off = valueToMarkerPosition(vKey) + 7;
                    //off += getStepLength()
                    hge->Gfx_RenderLine(dx + off, dy + 11, dx + off, dy + 16, 0xFF606162);
                }

                int v = getMarkerPosition();

                bool markerFocus = (mx > dx + v && mx < dx + v + getMarkerLength() && my > dy && my < dy + 12 &&
                                    mHasMouse);

                if (markerFocus && hge->Input_GetKeyState(HGEK_LBUTTON) && fTimer < 0.4f) {
                    fTimer += hge->Timer_GetDelta();
                    if (fTimer > 0.4f) fTimer = 0.4f;
                } else if (!(markerFocus && hge->Input_GetKeyState(HGEK_LBUTTON)) && fTimer > 0.2f) {
                    fTimer -= hge->Timer_GetDelta();
                    if (fTimer < 0.2f) fTimer = 0.2f;
                } else if (markerFocus && fTimer < 0.2f) {
                    fTimer += hge->Timer_GetDelta();
                    if (fTimer > 0.2f) fTimer = 0.2f;
                } else if (!markerFocus && fTimer > 0.0f) {
                    fTimer -= hge->Timer_GetDelta();
                    if (fTimer < 0.0f) fTimer = 0.0f;
                }

                _ghGfxInterface->sprSlider[markSpr][0]->Render(dx + v, dy);
                if (fTimer > 0.0f) {
                    _ghGfxInterface->sprSlider[markSpr][1]->SetColor(
                            SETA(0xFFFFFFFF, (unsigned char) (fTimer * 2.5f * 255.0f)));
                    _ghGfxInterface->sprSlider[markSpr][1]->Render(dx + v, dy);
                }
            } else if (mOrientation == VERTICAL) {
                //todo
            }

        }
    }

    void Slider::addKeyValue(double f) {
        vKeys.push_back(f);
    }

    void Slider::clearKeyValues() {
        vKeys.clear();
    }

    void Slider::drawMarker(gcn::Graphics *graphics) {
        gcn::Color faceColor = 0x5f5f5f;
        Color highlightColor, shadowColor;
        int alpha = getBaseColor().a;
        highlightColor = faceColor + 0x303030;
        highlightColor.a = alpha;
        shadowColor = faceColor - 0x303030;
        shadowColor.a = alpha;

        graphics->setColor(faceColor);

        if (getOrientation() == HORIZONTAL) {
            int v = getMarkerPosition();
            graphics->fillRectangle(gcn::Rectangle(v + 1, 1, getMarkerLength() - 2, getHeight() - 2));
            graphics->setColor(highlightColor);
            graphics->drawLine(v, 0, v + getMarkerLength() - 1, 0);
            graphics->drawLine(v, 0, v, getHeight() - 1);
            graphics->setColor(shadowColor);
            graphics->drawLine(v + getMarkerLength() - 1, 1, v + getMarkerLength() - 1, getHeight() - 1);
            graphics->drawLine(v + 1, getHeight() - 1, v + getMarkerLength() - 1, getHeight() - 1);

            if (isFocused()) {
                graphics->setColor(getForegroundColor());
                graphics->drawRectangle(gcn::Rectangle(v + 2, 2, getMarkerLength() - 4, getHeight() - 4));
            }
        } else {
            int v = (getHeight() - getMarkerLength()) - getMarkerPosition();
            graphics->fillRectangle(gcn::Rectangle(1, v + 1, getWidth() - 2, getMarkerLength() - 2));
            graphics->setColor(highlightColor);
            graphics->drawLine(0, v, 0, v + getMarkerLength() - 1);
            graphics->drawLine(0, v, getWidth() - 1, v);
            graphics->setColor(shadowColor);
            graphics->drawLine(1, v + getMarkerLength() - 1, getWidth() - 1, v + getMarkerLength() - 1);
            graphics->drawLine(getWidth() - 1, v + 1, getWidth() - 1, v + getMarkerLength() - 1);

            if (isFocused()) {
                graphics->setColor(getForegroundColor());
                graphics->drawRectangle(gcn::Rectangle(2, v + 2, getWidth() - 4, getMarkerLength() - 4));
            }
        }
    }

    void Slider::mouseEntered(MouseEvent &mouseEvent) {
        mHasMouse = 1;
    }

    void Slider::mouseExited(MouseEvent &mouseEvent) {
        mHasMouse = 0;
    }

    void Slider::mousePressed(MouseEvent &mouseEvent) {
        if (mouseEvent.getButton() == gcn::MouseEvent::LEFT
            && mouseEvent.getX() >= 0
            && mouseEvent.getX() <= getWidth()
            && mouseEvent.getY() >= 0
            && mouseEvent.getY() <= getHeight()) {
            if (getOrientation() == HORIZONTAL) {
                setValue(markerPositionToValue(mouseEvent.getX() - getMarkerLength() / 2));
            } else {
                setValue(markerPositionToValue(getHeight() - mouseEvent.getY() - getMarkerLength() / 2));
            }

            distributeActionEvent();
        }
    }

    void Slider::setForceKeyValue(bool b) {
        mForceKeyValue = b;
    }

    bool Slider::isForceKeyValue() {
        return mForceKeyValue;
    }

    void Slider::mouseDragged(MouseEvent &mouseEvent) {
        if (getOrientation() == HORIZONTAL) {
            int npos = mouseEvent.getX() - getMarkerLength() / 2;
            for (size_t i = 0; i < vKeys.size(); i++) {
                int kpos = valueToMarkerPosition(vKeys[i]);
                float spanl = kpos - 5, spanr = kpos + 5;
                if (mForceKeyValue && getScaleEnd() != getScaleStart()) {
                    spanl = (i > 0 ? (vKeys[i] - vKeys[i - 1]) / 2 : vKeys[i]);
                    spanr = (i < vKeys.size() - 1 ? (vKeys[i + 1] - vKeys[i]) / 2 : getScaleEnd() - vKeys[i]);
                    spanl = valueToMarkerPosition(vKeys[i] - spanl);
                    spanr = valueToMarkerPosition(vKeys[i] + spanr);
                    //spanr = float(getWidth())/float(getScaleEnd()-getScaleStart())*float(spanr);
                }
                if (npos >= spanl && npos <= spanr) {
                    npos = kpos;
                    break;
                }
            }
            setValue(markerPositionToValue(npos));
        } else {
            setValue(markerPositionToValue(getHeight() - mouseEvent.getY() - getMarkerLength() / 2));
        }

        distributeActionEvent();

        mouseEvent.consume();
    }

    void Slider::setValue(double value) {
        if (value > getScaleEnd()) {
            mValue = getScaleEnd();
            return;
        }

        if (value < getScaleStart()) {
            mValue = getScaleStart();
            return;
        }

        mValue = value;
    }

    double Slider::getValue() const {
        return mValue;
    }

    int Slider::getMarkerLength() const {
        return mMarkerLength;
    }

    void Slider::adjustMarkerLength() {
        double len = mScaleEnd - mScaleStart;
        double scrollsize = (getOrientation() == HORIZONTAL ? getWidth() : getHeight()) - 5;
        double ratio = (len == 0 ? 1 : scrollsize / len);
        if (ratio > 1.0f) ratio = 1.0f;
        mMarkerLength = ratio * scrollsize;
        if (mMarkerLength < 40) mMarkerLength = 40;
    }

    void Slider::setMarkerLength(int length) {
        mMarkerLength = length;
    }

    void Slider::keyPressed(KeyEvent &keyEvent) {
        Key key = keyEvent.getKey();

        if (getOrientation() == HORIZONTAL) {
            if (key.getValue() == Key::RIGHT) {
                setValue(getValue() + getStepLength());
                distributeActionEvent();
                keyEvent.consume();
            } else if (key.getValue() == Key::LEFT) {
                setValue(getValue() - getStepLength());
                distributeActionEvent();
                keyEvent.consume();
            }
        } else {
            if (key.getValue() == Key::UP) {
                setValue(getValue() + getStepLength());
                distributeActionEvent();
                keyEvent.consume();
            } else if (key.getValue() == Key::DOWN) {
                setValue(getValue() - getStepLength());
                distributeActionEvent();
                keyEvent.consume();
            }
        }
    }

    void Slider::setStyle(Style st) {
        mStyle = st;
        if (st == POINTER || st == BALL)
            setMarkerLength(12);
    }

    Slider::Style Slider::getStyle() const {
        return mStyle;
    }

    void Slider::setOrientation(Slider::Orientation orientation) {
        mOrientation = orientation;
    }

    Slider::Orientation Slider::getOrientation() const {
        return mOrientation;
    }

    double Slider::markerPositionToValue(int v) const {
        int w;
        if (getOrientation() == HORIZONTAL) {
            w = getWidth();
        } else {
            w = getHeight();
        }

        double pos = v / ((double) w - getMarkerLength());
        return (1.0 - pos) * getScaleStart() + pos * getScaleEnd();

    }

    int Slider::valueToMarkerPosition(double value) const {
        int v;
        if (getOrientation() == HORIZONTAL) {
            v = getWidth();
        } else {
            v = getHeight();
        }

        int w = (int) ((v - getMarkerLength())
                       * (value - getScaleStart())
                       / (getScaleEnd() - getScaleStart()));

        if (w < 0) {
            return 0;
        }

        if (w > v - getMarkerLength()) {
            return v - getMarkerLength();
        }

        return w;
    }

    void Slider::setStepLength(double length) {
        mStepLength = length;
    }

    double Slider::getStepLength() const {
        return mStepLength;
    }

    int Slider::getMarkerPosition() const {
        return valueToMarkerPosition(getValue());
    }

    void Slider::mouseWheelMovedUp(MouseEvent &mouseEvent) {
        setValue(getValue() + getStepLength());
        distributeActionEvent();

        mouseEvent.consume();
    }

    void Slider::mouseWheelMovedDown(MouseEvent &mouseEvent) {
        setValue(getValue() - getStepLength());
        distributeActionEvent();

        mouseEvent.consume();
    }
}
