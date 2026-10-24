#include "../editing_ww.h"
#include <algorithm>
#include <stdexcept>

extern HGE *hge;

State::Viewport::Viewport(EditingWW *phOwn, int x, int y, int w, int h) {
    hOwn = phOwn;
    iX = x;
    iY = y;
    iW = std::max(1, w);
    iH = std::max(1, h);
    bRedrawViewport = true;

    target = hge->Target_Create(iW, iH, false);
    if (!target) throw std::runtime_error("Cannot create map viewport render target");
    sprViewport = new hgeSprite(hge->Target_GetTexture(target), 0, 0, iW, iH);

    hFocuser = new vpFoc();
    hFocuser->setDimension(gcn::Rectangle(0, 0, iW, iH));
#ifdef WAPMAP_ENABLE_IMGUI
    // In ImGui mode this widget only captures legacy map input.
    hFocuser->setOpaque(false);
#endif
    hFocuser->addMouseListener(hOwn);
}

State::Viewport::~Viewport() {
    delete sprViewport;
    hge->Target_Free(target);
#ifdef WAPMAP_ENABLE_IMGUI
    if (compositeTarget) hge->Target_Free(compositeTarget);
#endif
}

void State::Viewport::Resize(int w, int h) {
    w = std::max(1, w);
    h = std::max(1, h);
    if (w == iW && h == iH) return;
#ifdef WAPMAP_ENABLE_IMGUI
    const bool compositeEnabled = compositeTarget != nullptr;
    HTARGET resizedComposite = nullptr;
    if (compositeEnabled) {
        resizedComposite = hge->Target_Create(w, h, false);
        if (!resizedComposite) throw std::runtime_error("Cannot resize map viewport composite target");
    }
#endif
    HTARGET resizedTarget = hge->Target_Create(w, h, false);
    if (!resizedTarget) {
#ifdef WAPMAP_ENABLE_IMGUI
        if (resizedComposite) hge->Target_Free(resizedComposite);
#endif
        throw std::runtime_error("Cannot resize map viewport render target");
    }

    delete sprViewport;
    hge->Target_Free(target);
#ifdef WAPMAP_ENABLE_IMGUI
    if (compositeTarget) hge->Target_Free(compositeTarget);
    compositeTarget = resizedComposite;
#endif

    iW = w;
    iH = h;
    bRedrawViewport = true;

    target = resizedTarget;
    sprViewport = new hgeSprite(hge->Target_GetTexture(target), 0, 0, iW, iH);
    hFocuser->setWidth(iW);
    hFocuser->setHeight(iH);//(gcn::Rectangle(0, 0, iW, iH));
}

void State::Viewport::MarkToRedraw() {
    bRedrawViewport = true;
}

void State::Viewport::SetRenderBounds(int x, int y, int w, int h) noexcept {
    iX = x; iY = y; iW = w; iH = h;
}

void State::Viewport::ClipScreen() {
    hge->Gfx_SetClipping(iX, iY, iW, iH);
}

void State::Viewport::Update() {
    if (!bRedrawViewport) return;

    if (!hge->Gfx_BeginScene(target)) throw std::runtime_error("Cannot begin map viewport scene");

    hOwn->RenderToViewportBuffer();

    hge->Gfx_EndScene();

    bRedrawViewport = false;
}

#ifdef WAPMAP_ENABLE_IMGUI
void State::Viewport::SetCompositeEnabled(const bool enabled) {
    if (enabled == (compositeTarget != nullptr)) return;
    if (enabled) {
        compositeTarget = hge->Target_Create(iW, iH, false);
        if (!compositeTarget) throw std::runtime_error("Cannot create map viewport composite target");
    } else {
        hge->Target_Free(compositeTarget);
        compositeTarget = nullptr;
    }
}

void State::Viewport::UpdateComposite() {
    if (!compositeTarget) throw std::logic_error("Map viewport composite target is not enabled");
    if (!hge->Gfx_BeginScene(compositeTarget))
        throw std::runtime_error("Cannot begin map viewport composite scene");
    hOwn->DrawViewport();
    hge->Gfx_EndScene();
}

void State::Viewport::Render() {
    sprViewport->Render(iX, iY);
}

void State::Viewport::SetPos(int x, int y) {
    iX = x;
    iY = y;
    hFocuser->setPosition(iX, iY);
}

void State::Viewport::GfxLost() {
    sprViewport->SetTexture(hge->Target_GetTexture(target));
}

unsigned int State::Viewport::GetTextureHandle(const bool composite) const {
    if (composite && !compositeTarget)
        throw std::logic_error("Map viewport composite target is not enabled");
    return hge->Texture_GetNativeHandle(
        hge->Target_GetTexture(composite ? compositeTarget : target));
}
#endif
