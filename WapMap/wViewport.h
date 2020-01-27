#ifndef WIDG_VIEWPORT_H
#define WIDG_VIEWPORT_H

#include <string>

#include "guichan/graphics.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"

using namespace gcn;

namespace WIDG {
    class VpCallback {
    public:
        virtual void Draw(int piCode) {};
    };

    class GCN_CORE_DECLSPEC Viewport : public gcn::Widget {
    public:
        Viewport(VpCallback *phCallback, int piCode);

        virtual void draw(Graphics *graphics);

    protected:
        VpCallback *hCallback;
        int iCode;
    };
}

#endif // end GCN_Viewport_HPP
