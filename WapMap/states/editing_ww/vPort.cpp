#include "../editing_ww.h"

extern HGE * hge;

State::Viewport::Viewport(EditingWW * phOwn, int x, int y, int w, int h)
{
	hOwn = phOwn;
	iX = x;
	iY = y;
	iW = w;
	iH = h;
	bRedrawViewport = 1;

	target = hge->Target_Create(iW, iH, 0);
	sprViewport = new hgeSprite(hge->Target_GetTexture(target), 0, 0, iW, iH);

	hFocuser = new vpFoc();
	hFocuser->setDimension(gcn::Rectangle(0, 0, iW, iH));
}

State::Viewport::~Viewport()
{
	delete sprViewport;
	hge->Target_Free(target);
}

void State::Viewport::Resize(int w, int h)
{
	delete sprViewport;
	hge->Target_Free(target);

	iW = w;
	iH = h;
	bRedrawViewport = 1;

	target = hge->Target_Create(iW, iH, 0);
	sprViewport = new hgeSprite(hge->Target_GetTexture(target), 0, 0, iW, iH);
	hFocuser->setWidth(iW);
	hFocuser->setHeight(iH);//(gcn::Rectangle(0, 0, iW, iH));
}

void State::Viewport::MarkToRedraw(bool mark)
{
	bRedrawViewport = mark;
}

void State::Viewport::ClipScreen()
{
	hge->Gfx_SetClipping(iX, iY, iW, iH);
}

void State::Viewport::Update()
{
	if (!bRedrawViewport) return;

	hge->Gfx_BeginScene(target);

	hOwn->RenderToViewportBuffer();

	hge->Gfx_EndScene();

	bRedrawViewport = 0;
}

void State::Viewport::Render()
{
	sprViewport->Render(iX, iY);
}

void State::Viewport::SetPos(int x, int y)
{
	iX = x;
	iY = y;
	hFocuser->setPosition(iX, iY);
}

void State::Viewport::GfxLost()
{
	sprViewport->SetTexture(hge->Target_GetTexture(target));
}
