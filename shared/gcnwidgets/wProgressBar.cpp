#include "wProgressBar.h"

#include "guichan/exception.hpp"
#include "guichan/font.hpp"
#include "guichan/graphics.hpp"

namespace SHR {
    ProgressBar::ProgressBar(guiParts *phGfx) {
        hGfx = phGfx;
        iProgress = iEnd = 0;
        cOpacity = 255;
    }

    float ProgressBar::getPercentageProgress() {
        if (iProgress == 0)
            return 0;
        else
            return 100.0f / (float(iEnd) / float(iProgress));
    }

    void ProgressBar::draw(Graphics *graphics) {
        float perc = getPercentageProgress();

        int x, y;
        getAbsolutePosition(x, y);

        DWORD col = ARGB(cOpacity, 255, 255, 255);

        if (perc == 0) {
            hGfx->sprPBarEL->SetColor(col);
            hGfx->sprPBarEL->Render(x, y);
            hGfx->sprPBarEM->SetColor(col);
            hGfx->sprPBarEM->RenderStretch(x + 3, y, x + getWidth() - 3, y + getHeight());
            hGfx->sprPBarER->SetColor(col);
            hGfx->sprPBarER->Render(x + getWidth() - 3, y);
        } else if (perc >= 100) {
            hGfx->sprPBarFL->SetColor(col);
            hGfx->sprPBarFL->Render(x, y);
            hGfx->sprPBarFM->SetColor(col);
            hGfx->sprPBarFM->RenderStretch(x + 3, y, x + getWidth() - 3, y + getHeight());
            hGfx->sprPBarFR->SetColor(col);
            hGfx->sprPBarFR->Render(x + getWidth() - 3, y);
        } else {
            int fullen = perc / 100.0f * getWidth();
            hGfx->sprPBarFL->SetColor(col);
            hGfx->sprPBarFL->Render(x, y);
            hGfx->sprPBarFM->SetColor(col);
            hGfx->sprPBarFM->RenderStretch(x + 3, y, x + fullen, y + getHeight());
            hGfx->sprPBarFR->SetColor(col);
            hGfx->sprPBarFR->Render(x + fullen - 3, y);
            hGfx->sprPBarEM->SetColor(col);
            hGfx->sprPBarEM->RenderStretch(x + fullen, y, x + getWidth() - 3, y + getHeight());
            hGfx->sprPBarER->SetColor(col);
            hGfx->sprPBarER->Render(x + getWidth() - 3, y);
        }
    }
}
