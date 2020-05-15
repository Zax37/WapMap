#include "cTooltip.h"
#include "hge.h"
#include "../../WapMap/globals.h"

extern HGE *hge;

namespace SHR {
    cTooltip* cTooltip::currentlyDisplayedTooltip = NULL;

    cTooltip::cTooltip() {
        m_szTooltip = 0;
        m_fTooltipTimer = 0;
        m_iTooltipX = m_iTooltipY = -1;
        m_bTooltipVisible = 0;
        m_iTooltipLC = 0;
    }

    cTooltip::~cTooltip() {
        if (m_szTooltip != 0)
            delete[] m_szTooltip;

        if (currentlyDisplayedTooltip == this) {
            currentlyDisplayedTooltip = NULL;
        }
    }

    void cTooltip::SetTooltip(const char *n) {
        if (m_szTooltip != 0)
            delete[] m_szTooltip;
        m_szTooltip = new char[strlen(n) + 1];
        strcpy(m_szTooltip, n);

        char *pch;
        pch = strchr(m_szTooltip, '\n');
        m_iTooltipLC = 1;
        while (pch != NULL) {
            m_iTooltipLC++;
            pch = strchr(pch + 1, '\n');
        }
    }

    void cTooltip::RenderTooltip() {
        if (currentlyDisplayedTooltip) {
            currentlyDisplayedTooltip->_RenderTooltip();
        }
    }

    void cTooltip::_RenderTooltip() {
        if (!HasTooltip() || !IsTooltipVisible()) return;
        int w = GV->fntMyriad13->GetStringWidth(m_szTooltip),
            h = GV->fntMyriad13->GetHeight() * m_iTooltipLC;
        hgeQuad q;
        q.blend = BLEND_DEFAULT;
        q.tex = 0;
        q.v[0].z = q.v[1].z = q.v[2].z = q.v[3].z = 1.0f;
        q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = 0xFFf1edd7;
        int rx = m_iTooltipX, ry = m_iTooltipY;
        rx += GV->IF->GetCursor()->GetWidth();
        w += 6;
        h += 6;
        if (rx + w > hge->System_GetState(HGE_SCREENWIDTH)) {
            rx -= w + 12;
            rx -= GV->IF->GetCursor()->GetWidth();
        }
        if (ry + h > hge->System_GetState(HGE_SCREENHEIGHT))
            ry -= h;
        q.v[0].x = rx;
        q.v[0].y = ry;
        q.v[1].x = rx + w;
        q.v[1].y = ry;
        q.v[2].x = rx + w;
        q.v[2].y = ry + h;
        q.v[3].x = rx;
        q.v[3].y = ry + h;
        hge->Gfx_RenderQuad(&q);
        hge->Gfx_RenderLine(q.v[0].x, q.v[0].y, q.v[1].x, q.v[1].y, 0xFF000000);
        hge->Gfx_RenderLine(q.v[1].x, q.v[1].y, q.v[2].x, q.v[2].y, 0xFF000000);
        hge->Gfx_RenderLine(q.v[2].x, q.v[2].y, q.v[3].x, q.v[3].y, 0xFF000000);
        hge->Gfx_RenderLine(q.v[3].x, q.v[3].y, q.v[0].x, q.v[0].y, 0xFF000000);
        GV->fntMyriad13->SetColor(0xFF000000);
        GV->fntMyriad13->Render(rx + 3, ry + 3, HGETEXT_LEFT, m_szTooltip, 0);
    }

    void cTooltip::UpdateTooltip(bool hasmouse) {
        if (!hasmouse) {
            m_bTooltipVisible = 0;
            if (m_bTooltipVisible && currentlyDisplayedTooltip == this) {
                currentlyDisplayedTooltip = NULL;
            }
            return;
        }
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        if (m_iTooltipX == -1 && m_iTooltipY == -1) {
            m_iTooltipX = mx;
            m_iTooltipY = my;
        } else if (m_iTooltipX != mx || m_iTooltipY != my) {
            m_bTooltipVisible = 0;
            m_fTooltipTimer = 0;
            m_iTooltipX = -1;
            m_iTooltipY = -1;
        } else if (m_fTooltipTimer > SHR_TOOLTIP_DELAY) {
            m_bTooltipVisible = 1;
            currentlyDisplayedTooltip = this;
        } else {
            m_fTooltipTimer += hge->Timer_GetDelta();
        }
    }
}
