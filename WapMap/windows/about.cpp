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
	GV->sprLogoBig->Render(dx + myWin->getWidth() / 2 - 141, dy + 60 - 28);
	GV->fntMyriad10->SetColor(0xFFFFFFFF);
	GV->fntMyriad10->Render(dx + myWin->getWidth() / 2 - 25, dy + 60 + 31, HGETEXT_RIGHT, WA_VERSTRING, 0);

	GV->sprUsedLibs->Render(dx + 350, dy + 315);

	GV->fntMyriad13->printf(dx + 150, dy + 125, HGETEXT_LEFT,
		"~y~kijanek6 ~w~ - %s\n\n~y~Zax37 ~w~- %s\n\n~y~Tomalla~w~ - %s\n\n~y~wisztom, Zax37 ~w~- %s\n\n~y~Teo Phil~w~, ~y~Zuczek~w~ & ~y~The Claw Recluse ~w~- %s\n\n~w~Copyright (c) 2010-2013 kijanek6. Reborn in 2020 thanks to Zax37.",
		0,
		GETL2S("WinAuthors", "Func_1"),
		GETL2S("WinAuthors", "Func_2"),
		GETL2S("WinAuthors", "Func_3"),
		GETL2S("WinAuthors", "Func_4"),
		GETL2S("WinAuthors", "Func_5"),
		GETL2S("WinAuthors", "Version"),
		WA_AUTHORVERSTRING);

	hge->Gfx_RenderLine(dx, dy + 305, dx + myWin->getWidth(), dy + 305, GV->colLineDark);
	hge->Gfx_RenderLine(dx, dy + 306, dx + myWin->getWidth(), dy + 306, GV->colLineBright);

	const int block_width = 340;
	const int block_height = 200;

	int yoff = 315, y_spacing = 5;
	int h = GV->fntMyriad13->GetHeightb(340, "HGE 1.81 - Copyright (c) 2003-2008 Relish Games.");
	if (h > GV->fntMyriad13->GetHeight()) h -= GV->fntMyriad13->GetHeight();
	GV->fntMyriad13->printfb(dx + 5, dy + yoff, block_width, block_height, HGETEXT_LEFT | HGETEXT_TOP, 0,
		"HGE 1.81 - Copyright (c) 2003-2008 Relish Games.");
	yoff += h + y_spacing;

	h = GV->fntMyriad13->GetHeightb(340, "BASS 2.3 - Copyright (c) 2003-2012 un4seen developments. All rights reserved.");
	if (h > 40) h -= GV->fntMyriad13->GetHeight();
	GV->fntMyriad13->printfb(dx + 5, dy + yoff, block_width, block_height, HGETEXT_LEFT | HGETEXT_TOP, 0,
		"BASS 2.3 - Copyright (c) 2003-2012 un4seen developments. All rights reserved.", 0);
	yoff += h + y_spacing;

	h = GV->fntMyriad13->GetHeightb(340, "Guichan 0.8.3 - Copyright (c) 2004-2007 Olof Naessen and Per Larsson. All rights reserved.");
	if (h > 40) h -= GV->fntMyriad13->GetHeight();
	GV->fntMyriad13->printfb(dx + 5, dy + yoff, block_width, block_height, HGETEXT_LEFT | HGETEXT_TOP, 0,
		"Guichan 0.8.3 - Copyright (c) 2004-2007 Olof Naessen and Per Larsson. All rights reserved.", 0);
	yoff += h + y_spacing;

	h = GV->fntMyriad13->GetHeightb(340, "libcurl 7.21.6 - Copyright (c) 1996 - 2012, Daniel Stenberg, <daniel@haxx.se>.");
	if (h > 40) h -= GV->fntMyriad13->GetHeight();
	GV->fntMyriad13->printfb(dx + 5, dy + yoff, block_width, block_height, HGETEXT_LEFT | HGETEXT_TOP, 0,
		"libcurl 7.21.6 - Copyright (c) 1996 - 2012, Daniel Stenberg, <daniel@haxx.se>.", 0);
	yoff += h + y_spacing;

	h = GV->fntMyriad13->GetHeightb(340, "hashlib++ 0.3.2 - Copyright (c) 2007-2011 Benjamin Grudelbach.");
	if (h > 40) h -= GV->fntMyriad13->GetHeight();
	GV->fntMyriad13->printfb(dx + 5, dy + yoff, block_width, block_height, HGETEXT_LEFT | HGETEXT_TOP, 0,
		"hashlib++ 0.3.2 - Copyright (c) 2007-2011 Benjamin Grudelbach.", 0);
	yoff += h + y_spacing;

	h = GV->fntMyriad13->GetHeightb(340, "Lua 5.1.4 - Copyright (c) 1994-2012 Lua.org, PUC-Rio.");
	if (h > 40) h -= GV->fntMyriad13->GetHeight();
	GV->fntMyriad13->printfb(dx + 5, dy + yoff, block_width, block_height, HGETEXT_LEFT | HGETEXT_TOP, 0,
		"Lua 5.1.4 - Copyright (c) 1994-2012 Lua.org, PUC-Rio.", 0);

	y_spacing = -5;
	yoff += h + y_spacing;

	h = GV->fntMyriad13->GetHeightb(340, "SFML 2.5.1 - Copyright (c) 2007-2018 Laurent Gomila.");
	if (h > 40) h -= GV->fntMyriad13->GetHeight();
	GV->fntMyriad13->printfb(dx + 5, dy + yoff, block_width, block_height, HGETEXT_LEFT | HGETEXT_TOP, 0,
		"SFML 2.5.1 - Copyright (c) 2007-2018 Laurent Gomila.", 0);
	yoff += h + y_spacing;

	h = GV->fntMyriad13->GetHeightb(340, "zlib 1.2.5 - Copyright (c) 1995-2012 Jean-loup Gailly and Mark Adler.");
	if (h > 40) h -= GV->fntMyriad13->GetHeight();
	GV->fntMyriad13->printfb(dx + 5, dy + yoff, block_width, block_height, HGETEXT_LEFT | HGETEXT_TOP, 0,
		"zlib 1.2.5 - Copyright (c) 1995-2012 Jean-loup Gailly and Mark Adler.", 0);
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
