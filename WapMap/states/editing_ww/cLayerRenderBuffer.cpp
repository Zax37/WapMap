#include "../editing_ww.h"
#include "../../globals.h"
#include "../../conf.h"

extern HGE * hge;

State::cLayerRenderBuffer::cLayerRenderBuffer(State::EditingWW * hown, Viewport * nvPort, WWD::Plane * npl)
{
	hOwner = hown;
	hPlane = npl;
	vPort = nvPort;
	hTarget = hge->Target_Create(hPlane->GetPlaneWidthPx(), hPlane->GetPlaneHeightPx(), 0);
	sprTarget = new hgeSprite(hge->Target_GetTexture(hTarget), 0, 0, hPlane->GetPlaneWidthPx(), hPlane->GetPlaneHeightPx());
	bRedraw = 1;
	iEntityCnt = 0;
	Update();
}

State::cLayerRenderBuffer::~cLayerRenderBuffer()
{
	delete sprTarget;
	hge->Target_Free(hTarget);
}

void State::cLayerRenderBuffer::Render(int x, int y, float fZoom)
{
	//sprTarget->SetTexture(hge->Target_GetTexture(hTarget));
	sprTarget->RenderEx(x, y, 0, fZoom);
	//sprTarget->Render(x, y);
}

void State::cLayerRenderBuffer::Update()
{
	if (!bRedraw) return;

	hge->Gfx_BeginScene(hTarget);

	hge->Gfx_Clear(0x01000000);

	GV->sprBlank->RenderStretch(0, 0, sprTarget->GetWidth(), sprTarget->GetHeight());
	iEntityCnt = hOwner->RenderLayer(hPlane, 1);

	hge->Gfx_EndScene();

	bRedraw = 0;
}

void State::cLayerRenderBuffer::GfxLost()
{
	sprTarget->SetTexture(hge->Target_GetTexture(hTarget));
	bRedraw = 1;
}

int State::cLayerRenderBuffer::GetEntityCount(int x1, int y1, int x2, int y2)
{
	int rcount = 0;
	for (int y = y1; y <= y2; y++)
		for (int x = x1; x <= x2; x++)
			if (hPlane->GetTile(x, y) != 0 && !hPlane->GetTile(x, y)->IsInvisible())
				rcount++;
	return rcount;
}
