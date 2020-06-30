#include "about.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"

extern HGE *hge;

static const int usedLibsCreditsBlockWidth = 507;
static const int usedLibsCreditsBlockLineSpacing = 6;

static const char* const usedLibsCreditLines[] {
        "HGE 1.81 - Copyright (c) 2003-2008 Relish Games.",
        "BASS 2.3 - Copyright (c) 2003-2012 un4seen developments. All rights reserved.",
        "Guichan 0.8.3 - Copyright (c) 2004-2007 Olof Naessen and Per Larsson. All rights reserved.",
        "libcurl 7.21.6 - Copyright (c) 1996 - 2012, Daniel Stenberg, <daniel@haxx.se>.",
        "hashlib++ 0.3.2 - Copyright (c) 2007-2011 Benjamin Grudelbach.",
        "Lua 5.1.4 - Copyright (c) 1994-2012 Lua.org, PUC-Rio.",
        "SFML 2.5.1 - Copyright (c) 2007-2018 Laurent Gomila.",
        "zlib 1.2.5 - Copyright (c) 1995-2012 Jean-loup Gailly and Mark Adler.",
        "ziplib - Copyright (c) 2013 Petr Benes.",
        "JSON for Modern C++ - Copyright (c) 2013-2019 Niels Lohmann <http://nlohmann.me>."
};

winAbout::winAbout() {
    myWin = new SHR::Win(&GV->gcnParts, GETL(Lang_About));
    int h = GV->sprLogoBig->GetHeight() + 105 + 22 * 4;
    for (auto line : usedLibsCreditLines) {
        h += GV->fntMyriad16->GetStringBlockHeight(usedLibsCreditsBlockWidth, line) + usedLibsCreditsBlockLineSpacing;
    }
    myWin->setDimension(gcn::Rectangle(0, 0, usedLibsCreditsBlockWidth + 2 * 32, h));
    myWin->setVisible(0);
    vp = new WIDG::Viewport(this, 0);
    myWin->add(vp, 0, 0);
    myWin->setClose(1);
}

winAbout::~winAbout() {
    delete myWin;
}

void winAbout::Think() {

}

void winAbout::Draw(int piCode) {
    unsigned char alpha = myWin->getAlpha();
    int dx, dy, logoW = GV->sprLogoBig->GetWidth();
    myWin->getAbsolutePosition(dx, dy);

    dx += 16;

    GV->RenderLogoWithVersion(dx + logoW / 2 + 12, dy + 77, alpha);

    dx += 18;
    dy += GV->sprLogoBig->GetHeight() + 64;

    hge->Gfx_RenderLine(dx, dy, dx + usedLibsCreditsBlockWidth - 2, dy, GV->colLineBright);

    dy += 16;

    GV->fntMyriad16->printf(dx, dy, HGETEXT_LEFT, "%s kijanek6 & Zax37.",
                            0, GETL2S("WinAuthors", "Func_1"));
    dy += 22;
    GV->fntMyriad16->printf(dx, dy, HGETEXT_LEFT, "%s Tomalla.",
                            0, GETL2S("WinAuthors", "Func_2"));
    dy += 22;
    GV->fntMyriad16->printf(dx, dy, HGETEXT_LEFT, "%s.",
                            0, GETL2S("WinAuthors", "Func_3"));
    dy += 32;

    hge->Gfx_RenderLine(dx, dy, dx + usedLibsCreditsBlockWidth - 2, dy, GV->colLineBright);

    dy += 16;

    for (auto line : usedLibsCreditLines) {
        GV->fntMyriad16->printfb(dx, dy, usedLibsCreditsBlockWidth, 200, 0, 0, line);
        dy += GV->fntMyriad16->GetStringBlockHeight(usedLibsCreditsBlockWidth, line) + usedLibsCreditsBlockLineSpacing;
    }
}

void winAbout::Open() {
    myWin->setPosition((hge->System_GetState(HGE_SCREENWIDTH) - myWin->getWidth()) / 2,
                       (hge->System_GetState(HGE_SCREENHEIGHT) - myWin->getHeight()) / 2);
    myWin->setVisible(true);
    myWin->getParent()->moveToTop(myWin);
}

void winAbout::Close() {
    myWin->setVisible(false);
}
