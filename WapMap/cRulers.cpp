#include "cRulers.h"
#include "globals.h"
#include "states/editing_ww.h"
#include "cAppMenu.h"
#include "../shared/commonFunc.h"

extern HGE *hge;

cRulers::cRulers() {
    bVisible = 0;
}

cRulers::~cRulers() {

}

void cRulers::SetVisible(bool b) {
    if (bVisible != b) {
        bVisible = b;
        GV->editState->hAppMenu->NotifyRulersSwitch();
        if (GV->editState->hParser != NULL) {
            if (b) {
                GV->editState->fCamX += 19 * GV->editState->fZoom;
                GV->editState->fCamY += 19 * GV->editState->fZoom;
            } else {
                GV->editState->fCamX -= 19 * GV->editState->fZoom;
                GV->editState->fCamY -= 19 * GV->editState->fZoom;
            }
        }
    }
}

void cRulers::Think() {
    if (GV->editState->hParser == NULL || !bVisible || GV->editState->iActiveTool != EWW_TOOL_NONE ||
        !GV->editState->bShowGuideLines)
        return;
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    if (GV->editState->conMain->getWidgetAt(mx, my) != NULL) return;
    bool orient;
    int drawY = LAY_MDI_Y + LAY_MDI_H + 1;
    if (mx > 0 && mx < 20 && my > drawY + 17 && my < GV->editState->vPort->GetHeight())
        orient = GUIDE_VERTICAL;
    else if (mx > 17 && mx < GV->editState->vPort->GetWidth() + 17 && my > drawY && my < drawY + 20)
        orient = GUIDE_HORIZONTAL;
    else {
        if (GV->editState->iManipulatedGuide != -1 && !hge->Input_GetKeyState(HGEK_LBUTTON)) {
            GV->editState->MDI->GetActiveDoc()->vGuides.erase(
                GV->editState->MDI->GetActiveDoc()->vGuides.begin() + GV->editState->iManipulatedGuide);
            GV->editState->iManipulatedGuide = -1;
        }
        return;
    }

    if (hge->Input_KeyDown(HGEK_LBUTTON)) {
        stGuideLine gl;
        gl.bOrient = orient;
        gl.iPos = -(17 - (orient == GUIDE_HORIZONTAL ? my - drawY
                                                     : mx));
        GV->editState->MDI->GetActiveDoc()->vGuides.push_back(gl);
        GV->editState->iManipulatedGuide = GV->editState->MDI->GetActiveDoc()->vGuides.size() - 1;
    }
    if (GV->editState->iManipulatedGuide != -1) {
        if (hge->Input_GetKeyState(HGEK_LBUTTON)) {
            stGuideLine gl = GV->editState->MDI->GetActiveDoc()->vGuides[GV->editState->iManipulatedGuide];
            gl.iPos = -((gl.bOrient == GUIDE_HORIZONTAL ? my - drawY
                                                        : mx));
            GV->editState->MDI->GetActiveDoc()->vGuides[GV->editState->iManipulatedGuide] = gl;
        } else {
            GV->editState->MDI->GetActiveDoc()->vGuides.erase(
                    GV->editState->MDI->GetActiveDoc()->vGuides.begin() + GV->editState->iManipulatedGuide);
            GV->editState->iManipulatedGuide = -1;
        }
    }
}

void cRulers::Render() {
    if (!bVisible) return;
    float zoom = GV->editState->fZoom;
    int iStartX, iStartY;
    if (GV->editState->hParser == NULL) {
        iStartX = 0;
        iStartY = 0;
    } else {
        iStartX = GV->editState->fCamX;
        iStartY = GV->editState->fCamY;
        if (iStartX < 0) iStartX = 0;
        if (iStartY < 0) iStartY = 0;
    }
    hgeQuad q;
    q.tex = 0;
    q.blend = BLEND_DEFAULT;

    int drawY = LAY_MDI_Y + LAY_MDI_H + 1;

    //separator from mdi
    hge->Gfx_RenderLine(0, drawY, hge->System_GetState(HGE_SCREENWIDTH), drawY, GV->colLineBright);
    SHR::SetQuad(&q, 0xFF202020, 1, drawY,
                 19, drawY + 18);
    hge->Gfx_RenderQuad(&q);
    for (int rl = 0; rl < 2; rl++) {//1 vertical 0 horizontal
        int drX = 1 + 18 * (!rl), drY = drawY + (rl * 18);
        SHR::SetQuad(&q, 0xFF303030, drX, drY, drX + (rl ? 18 : GV->editState->vPort->GetWidth() + 18),
                     drY + (rl ? GV->editState->vPort->GetHeight() : 18));
        hge->Gfx_SetClipping(drX, drY, q.v[2].x - drX, q.v[2].y - drY);
        int step;
        int mode = GV->editState->GetActiveMode();
        if (mode == EWW_MODE_TILE) {
            if (zoom > 0.75) step = 16;
            else step = 32;
        } else {
            if (zoom < 0.20) step = 250;
            else if (zoom < 0.40) step = 100;
            else if (zoom < 0.75) step = 25;
            else if (zoom < 1.50) step = 10;
            else step = 5;
        }
        for (int i = 0; i < (rl ? GV->editState->vPort->GetHeight() / (step * zoom)
                                : (GV->editState->vPort->GetWidth() + 20) / (step * zoom)) + 1; i++) {
            int modx = (rl ? iStartY : iStartX) % step;
            int constoff = ((rl ? GV->editState->fCamY : GV->editState->fCamX) < 0) *
                           (rl ? GV->editState->fCamY : GV->editState->fCamX) * -1;
            int firstid = (rl ? iStartY : iStartX) / step;
            int type = 0, id = firstid + i;
            if (mode == EWW_MODE_OBJECT) {
                if (step == 250) {
                    if (id % 4 == 0)
                        type = 3;
                    else if (id % 2 == 0)
                        type = 2;
                    else
                        type = 1;
                } else if (step == 100) {
                    if (id % 10 == 0)
                        type = 3;
                    else if (id % 5 == 0)
                        type = 2;
                    else
                        type = 1;
                } else if (step == 25) {
                    if (id % 40 == 0)
                        type = 3;
                    else if (id % 10 == 0)
                        type = 2;
                    else if (id % 4 == 0)
                        type = 1;
                } else if (step == 10) {
                    if (id % 100 == 0)
                        type = 3;
                    else if (id % 25 == 0)
                        type = 2;
                    else if (id % 10 == 0)
                        type = 1;
                } else if (step == 5) {
                    if (id % 20 == 0)
                        type = 3;
                    else if (id % 10 == 0)
                        type = 2;
                    else if (id % 5 == 0)
                        type = 1;
                }
            } else {
                if (step == 32) {
                    if (zoom < 0.25) {
                        if (id % 20 == 0) type = 3;
                        else if (id % 10 == 0) type = 1;
                        else if (id % 2 == 0) type = 0;
                        else
                            continue;
                    } else {
                        if (id % 20 == 0)
                            type = 3;
                        else if (id % 10 == 0)
                            type = 2;
                        else if (id % 2 == 0)
                            type = 1;
                    }
                } else if (step == 16) {
                    if (id % 20 == 0) type = 3;
                    else if (id % 4 == 0) type = 2;
                    else if (id % 2 == 0) type = 1;
                }
            }
            int pos = (mode == EWW_MODE_TILE ? id * step / 64 : id * step);
            char text[64];
            if (rl == 1 && pos > 999)
                sprintf(text, "%.1fk", float(pos) / 1000.0f);
            else
                sprintf(text, "%d", pos);

            int linex = drX + (rl ? 17 : (i * step - modx) * zoom + constoff),
                liney = drY + (rl ? (i * step - modx) * zoom + constoff : 17);
            if (type == 3) {
                hge->Gfx_RenderLine(linex - 17 * rl, liney - 17 * (!rl), linex, liney, 0xFF717171);
                hge->Gfx_RenderLine(linex - 17 * rl + 1 * (!rl), liney - 17 * (!rl) + 1 * rl, linex + 1 * (!rl),
                                    liney + 1 * rl, 0xFF717171);
                GV->fntMinimal->SetColor(0xFF717171);
                GV->fntMinimal->printf(linex + (rl ? -16 : 3), liney + (rl ? -2 : -20), HGETEXT_LEFT, text, 0);
            } else {
                int h = 3;
                if (type == 1) h = 10;
                else if (type == 2) h = 17;
                hge->Gfx_RenderLine(linex - h * rl, liney - h * (!rl), linex, liney, 0xFF616161);
                if (type == 2 && step < 250) {
                    GV->fntMinimal->SetColor(0xFF616161);
                    GV->fntMinimal->printf(linex + (rl ? -16 : 3), liney + (rl ? -3 : -20), HGETEXT_LEFT, text, 0);
                }
            }
        }
        hge->Gfx_SetClipping();
    }
    //vertical ruler viewport separator
    /*hge->Gfx_RenderLine(19, drawY,
                        19, GV->editState->vPort->GetY() + GV->editState->vPort->GetHeight(),
                        0xFF616161);*/
    //vertical ruler upper separator fix
    /*hge->Gfx_RenderLine(0, drawY + 18,
                        18, drawY + 18,
                        0xFF616161);*/

    if (GV->editState->iManipulatedGuide != -1) {
        stGuideLine gl = GV->editState->MDI->GetActiveDoc()->vGuides[GV->editState->iManipulatedGuide];
        if (gl.iPos < 0 && gl.iPos != -100) {
            if (gl.bOrient == GUIDE_HORIZONTAL) {
                hge->Gfx_RenderLine(19, drawY - gl.iPos, 18 + GV->editState->vPort->GetWidth(), drawY - gl.iPos,
                                    0xFF30DBD9);
            } else {
                hge->Gfx_RenderLine(-gl.iPos, drawY + 18, -gl.iPos, drawY + 18 + GV->editState->vPort->GetHeight(),
                                    0xFF30DBD9);
            }
        }
    }
}
