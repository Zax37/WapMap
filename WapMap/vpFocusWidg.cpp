#include "vpFocusWidg.h"

#include "hge.h"

using namespace gcn;

extern HGE *hge;

vpFoc::vpFoc() {
    addMouseListener(this);
    bMouseOver = 0;
    setFocusable(1);
}

vpFoc::~vpFoc() {
}

void vpFoc::mousePressed(MouseEvent &mouseEvent) {
    if (mouseEvent.getSource() != this) {
        return;
    }
    mouseEvent.consume();
}

void vpFoc::mouseExited(MouseEvent &mouseEvent) {
    bMouseOver = 0;
}

void vpFoc::mouseEntered(MouseEvent &mouseEvent) {
    bMouseOver = 1;
}

void vpFoc::mouseReleased(MouseEvent &mouseEvent) {
    mouseEvent.consume();
}

void vpFoc::mouseDragged(DragEvent &mouseEvent) {
    if (mouseEvent.isConsumed() || mouseEvent.getSource() != this) {
        return;
    }
    mouseEvent.consume();
}
