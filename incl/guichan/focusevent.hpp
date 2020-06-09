#ifndef GCN_FOCUSEVENT_HPP
#define GCN_FOCUSEVENT_HPP

#include "guichan/event.hpp"

namespace gcn
{
    class Widget;

    /**
     * Represents a focus change event.
     */
    class GCN_CORE_DECLSPEC FocusEvent: public Event
    {
    public:

        /**
         * Constructor.
         *
         * @param source The source widget of the event.
         * @param other The other widget involved in the event.
         * @param isKeyboardFocus True if the source of focus is keyboard navigation, false otherwise.
         */
        FocusEvent(Widget* source, Widget* other, bool isKeyboardFocus);

        /**
         * Destructor.
         */
        virtual ~FocusEvent();

        /**
         * Stops the focus change from happening.
         */
        void stopEvent() const;

        /**
         * Checks if the event was stopped by one of the participants.
         *
         * @return True if the one of the participants stopped the event.
         *
         */
        bool wasStopped() const;

        /**
         * Checks if the source of focus is keyboard navigation.
         *
         * @return True if the source of focus is keyboard navigation,
         *         false otherwise.
         *
         */
        bool isKeyboardFocus() const;

        /**
         * Returns the other widget involved in event.
         *
         * @return Pointer to the other widget involved in focus change. Can be NULL.
         *
         */
        Widget* getOtherSource() const;

    protected:
        /**
         * True if any of the participants said stop.
         */
        bool mWasStopped;

        /**
         * True if the source of focus is keyboard navigation.
         */
        bool mIsKeyboardFocus;

        /**
         * Pointer to the other widget involved in focus change.
         */
        Widget* mOtherOne;
    };
}

#endif // end GCN_FOCUSEVENT_HPP
