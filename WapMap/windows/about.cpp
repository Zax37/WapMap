#include "about.h"
#include "../globals.h"
#include "../langID.h"
#include "../states/editing_ww.h"
#include "../cInterfaceSheet.h"
#include "../conf.h"

extern HGE * hge;


winAbout::winAbout()
{
    myWin = new SHR::Win(&GV->gcnParts, GETL(Lang_About));
    myWin->setDimension(gcn::Rectangle(0, 0, 700, 570));
    myWin->setVisible(0);
    vp = new WIDG::Viewport(this, 0);
    myWin->add(vp, 0, 0);
    myWin->setClose(1);
}

winAbout::~winAbout()
{
    delete myWin;
}

void winAbout::Think()
{

}

void winAbout::Draw(int piCode)
{
    int dx, dy;
    myWin->getAbsolutePosition(dx, dy);
    GV->sprLogoBig->Render(dx+myWin->getWidth()/2-151, dy+50-28);
    GV->fntMyriad10->SetColor(0xFFFFFFFF);
    GV->fntMyriad10->Render(dx+myWin->getWidth()/2-35, dy+50+31, HGETEXT_RIGHT, WA_VERSTRING);

    GV->sprUsedLibs->Render(dx+350, dy+300);

    GV->fntMyriad13->SetColor(0xFF000000);
    GV->fntMyriad13->printf(dx+350, dy+115, HGETEXT_CENTER,
                            "~y~kijanek6 ~l~ - %s\n~y~Zax37 ~l~- %s\n~y~Tomalla~l~ - %s\n~y~Zax37, wisztom ~l~- %s\n~y~Teo Phil~l~, ~y~Zuczek~l~ & ~y~The Claw Recluse ~l~- %s\n~y~Zax37 ~l~- BrushMaker~l~\n\nCopyright (c) 2010-2012 by Kijedi Studio\n%s ~w~%s~l~ compiled on ~w~%s %s~l~.",
                            0,
                            GETL2S("WinAuthors", "Func_1"),
                            GETL2S("WinAuthors", "Func_2"),
                            GETL2S("WinAuthors", "Func_3"),
                            GETL2S("WinAuthors", "Func_4"),
                            GETL2S("WinAuthors", "Func_5"),
                            GETL2S("WinAuthors", "Version"),
                            WA_AUTHORVERSTRING,
                            __TIME__,
                            __DATE__);

    hge->Gfx_RenderLine(dx, dy+290, dx+myWin->getWidth(), dy+290, GV->colLineDark);
    hge->Gfx_RenderLine(dx, dy+291, dx+myWin->getWidth(), dy+291, GV->colLineBright);

    int yoff = 300;
    int h = GV->fntMyriad13->GetHeightb(340, "HGE 1.81 - Copyright (c) 2003-2008 Relish Games.");
    if( h > GV->fntMyriad13->GetHeight() ) h -= GV->fntMyriad13->GetHeight();
    GV->fntMyriad13->printfb(dx+5, dy+yoff, 340, 200, HGETEXT_LEFT|HGETEXT_TOP, 0,
                             "HGE 1.81 - Copyright (c) 2003-2008 Relish Games.");
    yoff += h;

    h = GV->fntMyriad13->GetHeightb(340, "BASS 2.3 - Copyright © 2003-2012 un4seen developments. All rights reserved.");
    if( h > 40 ) h -= GV->fntMyriad13->GetHeight();
    GV->fntMyriad13->printfb(dx+5, dy+yoff, 340, 200, HGETEXT_LEFT|HGETEXT_TOP, 0,
                            "BASS 2.3 - Copyright © 2003-2012 un4seen developments. All rights reserved.", 0);
    yoff += h;

    h = GV->fntMyriad13->GetHeightb(340, "Guichan 0.8.3 - Copyright (c) 2004-2007 Olof Naessén and Per Larsson. All rights reserved.");
    if( h > 40 ) h -= GV->fntMyriad13->GetHeight();
    GV->fntMyriad13->printfb(dx+5, dy+yoff, 340, 200, HGETEXT_LEFT|HGETEXT_TOP, 0,
                            "Guichan 0.8.3 - Copyright (c) 2004-2007 Olof Naessén and Per Larsson. All rights reserved.", 0);
    yoff += h;

    h = GV->fntMyriad13->GetHeightb(340, "libcurl 7.21.6 - Copyright (c) 1996 - 2012, Daniel Stenberg, <daniel@haxx.se>.");
    if( h > 40 ) h -= GV->fntMyriad13->GetHeight();
    GV->fntMyriad13->printfb(dx+5, dy+yoff, 340, 200, HGETEXT_LEFT|HGETEXT_TOP, 0,
                            "libcurl 7.21.6 - Copyright (c) 1996 - 2012, Daniel Stenberg, <daniel@haxx.se>.", 0);
    yoff += h;

    h = GV->fntMyriad13->GetHeightb(340, "GD 2.0.33 - Copyright (c) 1994-2012 Boutell.Com, Inc. All rights reserved.");
    if( h > 40 ) h -= GV->fntMyriad13->GetHeight();
    GV->fntMyriad13->printfb(dx+5, dy+yoff, 340, 200, HGETEXT_LEFT|HGETEXT_TOP, 0,
                            "GD 2.0.33 - Copyright (c) 1994-2012 Boutell.Com, Inc. All rights reserved.", 0);
    yoff += h;

    h = GV->fntMyriad13->GetHeightb(340, "hashlib++ 0.3.2 - Copyright (c) 2007-2011 Benjamin Grüdelbach.");
    if( h > 40 ) h -= GV->fntMyriad13->GetHeight();
    GV->fntMyriad13->printfb(dx+5, dy+yoff, 340, 200, HGETEXT_LEFT|HGETEXT_TOP, 0,
                            "hashlib++ 0.3.2 - Copyright (c) 2007-2011 Benjamin Grüdelbach.", 0);
    yoff += h;

    h = GV->fntMyriad13->GetHeightb(340, "Lua 5.1.4 - Copyright © 1994–2012 Lua.org, PUC-Rio.");
    if( h > 40 ) h -= GV->fntMyriad13->GetHeight();
    GV->fntMyriad13->printfb(dx+5, dy+yoff, 340, 200, HGETEXT_LEFT|HGETEXT_TOP, 0,
                            "Lua 5.1.4 - Copyright © 1994–2012 Lua.org, PUC-Rio.", 0);
    yoff += h;

    h = GV->fntMyriad13->GetHeightb(340, "zlib 1.2.5 - Copyright (C) 1995-2012 Jean-loup Gailly and Mark Adler.");
    if( h > 40 ) h -= GV->fntMyriad13->GetHeight();
    GV->fntMyriad13->printfb(dx+5, dy+yoff, 500, 200, HGETEXT_LEFT|HGETEXT_TOP, 0,
                            "zlib 1.2.5 - Copyright (C) 1995-2012 Jean-loup Gailly and Mark Adler.", 0);
}

void winAbout::Open()
{
    myWin->setVisible(1);
    myWin->getParent()->moveToTop(myWin);
}

void winAbout::Close()
{
    myWin->setVisible(0);
}
