#include "../editing_ww.h"
#include "../../globals.h"
#include "../../conf.h"
#include "../../../shared/commonFunc.h"
extern HGE * hge;

namespace State {
	void EditingWWvpCallback::Draw(int iCode)
	{
		if (iCode == VP_VIEWPORT) {
			m_hOwn->DrawViewport();
		}
		else if (iCode == VP_WORLD) {
			m_hOwn->DrawWorldP();
		}
		else if (iCode == VP_DB) {
			m_hOwn->DrawDB();
		}
		else if (iCode == VP_TILEPROP) {
			m_hOwn->DrawTileProperties();
		}
		else if (iCode == VP_OBJSEARCH) {
			m_hOwn->DrawObjSearch();
		}
		else if (iCode == VP_FILLCOLOR) {
			m_hOwn->DrawSelectFillColor();
		}
		else if (iCode == VP_NEWMAP) {
			m_hOwn->NewMap_Think();
		}
		else if (iCode == VP_FIRSTRUN) {
			m_hOwn->FirstRun_Think();
		}
		else if (iCode == VP_PLANEPROPERTIES) {
			m_hOwn->DrawPlaneProperties();
		}
		else if (iCode == VP_WELCOMESCREEN) {
			m_hOwn->DrawWelcomeScreen();
		}
		else if (iCode == VP_CRASHRETRIEVE) {
			m_hOwn->DrawCrashRetrieve();
		}
		else if (iCode == VP_TILEPICKER) {
			m_hOwn->DrawTilePicker();
		}
	}

	EditingWWvpCallback::EditingWWvpCallback(EditingWW * owner)
	{
		m_hOwn = owner;
	}
}
