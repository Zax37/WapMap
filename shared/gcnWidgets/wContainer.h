#ifndef SHR_CONT_H
#define SHR_CONT_H

#include <list>

#include "guichan/basicContainer.hpp"
#include "guichan/graphics.hpp"
#include "guichan/platform.hpp"
#include "guichan/mouselistener.hpp"
#include "guichan/widgetlistener.hpp"

using namespace gcn;

namespace SHR {
    class GCN_CORE_DECLSPEC Container : public gcn::BasicContainer,
                                       public gcn::WidgetListener {
    public:
        Container();

        virtual ~Container();

        void setOpaque(bool opaque);

        bool isOpaque() const;

        virtual void add(Widget *widget);

        virtual void add(Widget *widget, int x, int y);

        virtual void remove(Widget *widget);

        virtual void clear();

        virtual Widget *findWidgetById(const std::string &id);

        virtual void draw(Graphics *graphics);

        virtual void widgetShown(const Event &event);

        virtual void widgetHidden(const Event &event);

        void setShow(bool b);

    protected:
        bool mOpaque;
        bool bHide;
    };
}

#endif
