#ifndef WAPMAP_CFIELD_H
#define WAPMAP_CFIELD_H

#include "../../WapMap/globals.h"
#include "../../WapMap/cInterfaceSheet.h"

extern HGE *hge;
extern cInterfaceSheet *_ghGfxInterface;

namespace SHR {
    class Field {
    public:
        void renderFrame(int dx, int dy, int w, int h, unsigned char alpha, bool version) {
            for (int i = 0; i < 8; i++)
                _ghGfxInterface->sprTextField[version][i]->SetColor(SETA(0xFFFFFF, alpha));

            _ghGfxInterface->sprTextField[version][0]->Render(dx - 3, dy - 3);
            _ghGfxInterface->sprTextField[version][1]->RenderStretch(dx + 4, dy - 3, dx + w - 4, dy + 4);
            _ghGfxInterface->sprTextField[version][2]->Render(dx + w - 4, dy - 3);

            _ghGfxInterface->sprTextField[version][3]->RenderStretch(dx - 3, dy + 4, dx + 1, dy + h - 4);
            _ghGfxInterface->sprTextField[version][4]->RenderStretch(dx + w - 1, dy + 4, dx + w + 3, dy + h - 4);

            _ghGfxInterface->sprTextField[version][5]->Render(dx - 3, dy + h - 4);
            _ghGfxInterface->sprTextField[version][6]->RenderStretch(dx + 4, dy + h - 4, dx + w - 4, dy + h + 3);
            _ghGfxInterface->sprTextField[version][7]->Render(dx + w - 4, dy + h - 4);

            hgeQuad q;
            q.tex = 0;
            q.blend = BLEND_DEFAULT;
            for (int i = 0; i < 4; i++) q.v[i].z = 0;
            q.v[0].col = q.v[1].col = q.v[2].col = q.v[3].col = SETA(0x232323, alpha);
            q.v[0].x = dx + 1;
            q.v[0].y = dy + 4;
            q.v[1].x = dx + w - 1;
            q.v[1].y = dy + 4;
            q.v[2].x = dx + w - 1;
            q.v[2].y = dy + h - 4;
            q.v[3].x = dx + 1;
            q.v[3].y = dy + h - 4;
            hge->Gfx_RenderQuad(&q);
        }

        void drawCaret(gcn::Graphics *graphics, int x) {
            // Check the current clip area as a clip area with a different
            // size than the widget might have been pushed (which is the
            // case in the draw method when we push a clip area after we have
            // drawn a border).
            const gcn::Rectangle clipArea = graphics->getCurrentClipArea();

            graphics->setColor(0x3393e6);
            graphics->drawLine(x + 2, clipArea.height - 2, x + 2, 1);
        }
    };
}


#endif //WAPMAP_CFIELD_H
