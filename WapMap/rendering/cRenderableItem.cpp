#include "cRenderableItem.h"
#include "../../shared/cWWD.h"
#include "../globals.h"
#include "../states/editing_ww.h"
#include "../cObjectUserData.h"

bool RenderableItemSortZ(cRenderableItem *a, cRenderableItem *b) {
    if (a->GetZ() == b->GetZ()) {
        if (a->GetPriority() == b->GetPriority()) {
            return a->GetUniqueOrderID() < b->GetUniqueOrderID();
        }
        return (a->GetPriority() < b->GetPriority());
    }
    return (a->GetZ() < b->GetZ());
}

cPlaneRenderWrapper::cPlaneRenderWrapper(WWD::Plane *h, int index) {
    hPlane = h;
    iIndex = index;
}

cPlaneRenderWrapper::~cPlaneRenderWrapper() {

}

int cPlaneRenderWrapper::GetZ() {
    return hPlane->GetZCoord();
}

int cPlaneRenderWrapper::GetPriority() {
    return 1;
}

int cPlaneRenderWrapper::Render() {
    return GV->editState->RenderPlane(hPlane, iIndex);
}

int cPlaneRenderWrapper::GetUniqueOrderID() {
    return iIndex;
}

cObjectRenderWrapper::cObjectRenderWrapper(WWD::Object *h) {
    hObject = h;
}

cObjectRenderWrapper::~cObjectRenderWrapper() {

}

int cObjectRenderWrapper::GetZ() {
    return GetUserDataFromObj(hObject)->GetZ();
}

int cObjectRenderWrapper::GetPriority() {
    return 2;
}

int cObjectRenderWrapper::Render() {
    return GV->editState->RenderObject(hObject, GV->editState->Wrd2ScrXrb(GV->editState->plMain,
                                       GetUserDataFromObj(hObject)->GetX()), GV->editState->Wrd2ScrYrb(GV->editState->plMain,
                                       GetUserDataFromObj(hObject)->GetY()), 0xFFFFFFFF);
}

int cObjectRenderWrapper::GetUniqueOrderID() {
    return hObject->GetParam(WWD::Param_ID);
}
