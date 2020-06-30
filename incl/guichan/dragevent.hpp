#ifndef WAPMAP_DRAGEVENT_HPP
#define WAPMAP_DRAGEVENT_HPP

#include "guichan/mouseevent.hpp"

namespace gcn
{
    class GCN_CORE_DECLSPEC DragEvent : public MouseEvent {
    public:
        DragEvent(const MouseEvent& mouseEvent, unsigned dist, unsigned time)
            : MouseEvent(mouseEvent), dist(dist), time(time) {}

        unsigned getDragDistance() { return dist; }

        unsigned getDragTime() { return time; }

    private:
        unsigned dist, time;
    };

}

#endif //WAPMAP_DRAGEVENT_HPP
