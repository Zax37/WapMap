#ifndef VP_FOCUS
#define VP_FOCUS

#include <string>

#include "guichan/mouselistener.hpp"
#include "guichan/platform.hpp"
#include "guichan/widgets/container.hpp"

using namespace gcn;

class GCN_CORE_DECLSPEC vpFoc : public Container,
                                public MouseListener {
public:
    vpFoc();

    virtual ~vpFoc();

    virtual void mousePressed(MouseEvent &mouseEvent);

    virtual void mouseDragged(DragEvent &mouseEvent);

    virtual void mouseReleased(MouseEvent &mouseEvent);

    virtual void mouseEntered(MouseEvent &mouseEvent);

    virtual void mouseExited(MouseEvent &mouseEvent);

    bool isMouseOver() { return bMouseOver; };
protected:
    bool bMouseOver;
};

#endif // end GCN_Win_HPP
