#ifndef SHR_SLIDER_H
#define SHR_SLIDER_H

#include "guichan/keylistener.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"
#include <vector>

using namespace gcn;

namespace SHR {
    class GCN_CORE_DECLSPEC Slider :
            public gcn::Widget,
            public MouseListener,
            public KeyListener {
    public:

        /**
         * Draw orientations for the slider. A slider can be drawn vertically or
         * horizontally.
         */
        enum Orientation {
            HORIZONTAL = 0,
            VERTICAL
        };

        enum Style {
            DEFAULT = 0,
            BALL,
            POINTER
        };

        /**
         * Constructor. The default start value of the slider scale is zero.
         *
         * @param scaleEnd The end value of the slider scale.
         */
        Slider(double scaleEnd = 1.0);

        /**
         * Constructor.
         *
         * @param scaleStart The start value of the slider scale.
         * @param scaleEnd The end value of the slider scale.
         */
        Slider(double scaleStart, double scaleEnd);

        /**
         * Destructor.
         */
        virtual ~Slider() {}

        /**
         * Sets the scale of the slider.
         *
         * @param scaleStart The start value of the scale.
         * @param scaleEnd tThe end of value the scale.
         * @see getScaleStart, getScaleEnd
         */
        void setScale(double scaleStart, double scaleEnd);

        /**
         * Gets the start value of the scale.
         *
         * @return The start value of the scale.
         * @see setScaleStart, setScale
         */
        double getScaleStart() const;

        /**
         * Sets the start value of the scale.
         *
         * @param scaleStart The start value of the scale.
         * @see getScaleStart
         */
        void setScaleStart(double scaleStart);

        /**
         * Gets the end value of the scale.
         *
         * @return The end value of the scale.
         * @see setScaleEnd, setScale
         */
        double getScaleEnd() const;

        /**
         * Sets the end value of the scale.
         *
         * @param scaleEnd The end value of the scale.
         * @see getScaleEnd
         */
        void setScaleEnd(double scaleEnd);

        /**
         * Gets the current selected value.
         *
         * @return The current selected value.
         * @see setValue
         */
        double getValue() const;

        /**
         * Sets the current selected value.
         *
         * @param value The current selected value.
         * @see getValue
         */
        void setValue(double value);

        /**
         * Sets the length of the marker.
         *
         * @param length The length for the marker.
         * @see getMarkerLength
         */
        void setMarkerLength(int length);

        /**
         * Gets the length of the marker.
         *
         * @return The length of the marker.
         * @see setMarkerLength
         */
        int getMarkerLength() const;

        void adjustMarkerLength();

        /**
         * Sets the orientation of the slider. A slider can be drawn vertically
         * or horizontally.
         *
         * @param orientation The orientation of the slider.
         * @see getOrientation
         */
        void setOrientation(Orientation orientation);

        /**
         * Gets the orientation of the slider. A slider can be drawn vertically
         * or horizontally.
         *
         * @return The orientation of the slider.
         * @see setOrientation
         */
        Orientation getOrientation() const;

        void setStyle(Style st);

        Style getStyle() const;

        /**
         * Sets the step length. The step length is used when the keys LEFT
         * and RIGHT are pressed to step in the scale.
         *
         * @param length The step length.
         * @see getStepLength
         */
        void setStepLength(double length);

        /**
         * Gets the step length. The step length is used when the keys LEFT
         * and RIGHT are pressed to step in the scale.
         *
         * @return the step length.
         * @see setStepLength
         */
        double getStepLength() const;


        // Inherited from Widget

        virtual void draw(Graphics *graphics);


        // Inherited from MouseListener.

        virtual void mouseEntered(MouseEvent &mouseEvent);

        virtual void mouseExited(MouseEvent &mouseEvent);

        virtual void mousePressed(MouseEvent &mouseEvent);

        virtual void mouseDragged(DragEvent &mouseEvent);

        virtual void mouseWheelMovedUp(MouseEvent &mouseEvent);

        virtual void mouseWheelMovedDown(MouseEvent &mouseEvent);


        // Inherited from KeyListener

        virtual void keyPressed(KeyEvent &keyEvent);

        void addKeyValue(double f);

        void clearKeyValues();

        void setForceKeyValue(bool b);

        bool isForceKeyValue();

    protected:
        /**
         * Draws the marker.
         *
         * @param graphics A graphics object to draw with.
         */
        virtual void drawMarker(gcn::Graphics *graphics);

        /**
         * Converts a marker position to a value in the scale.
         *
         * @param position The position to convert.
         * @return A scale value corresponding to the position.
         * @see valueToMarkerPosition
         */
        virtual double markerPositionToValue(int position) const;

        /**
         * Converts a value to a marker position.
         *
         * @param value The value to convert.
         * @return A marker position corresponding to the value.
         * @see markerPositionToValue
         */
        virtual int valueToMarkerPosition(double value) const;

        /**
         * Gets the marker position of the current selected value.
         *
         * @return The marker position of the current selected value.
         */
        virtual int getMarkerPosition() const;

        /**
         * True if the slider is dragged, false otherwise.
         */
        bool mDragged;

        /**
         * Holds the current selected value.
         */
        double mValue;

        /**
         * Holds the step length. The step length is used when the keys LEFT
         * and RIGHT are pressed to step in the scale.
         */
        double mStepLength;

        /**
         * Holds the length of the marker.
         */
        int mMarkerLength;

        /**
         * Holds the start value of the scale.
         */
        double mScaleStart;

        /**
         * Holds the end value of the scale.
         */
        double mScaleEnd;

        /**
         * Holds the orientation of the slider. A slider can be drawn
         * vertically or horizontally.
         */
        Orientation mOrientation;
        Style mStyle;

        float fTimer;

        bool mHasMouse;
        std::vector<double> vKeys;
        bool mForceKeyValue;
    };
}

#endif // end GCN_SLIDER_HPP
