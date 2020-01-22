#include "error.h"
#include "../globals.h"
#include "../../shared/gcnWidgets/wContainer.h"
#include "../../shared/commonFunc.h"
#include "../returncodes.h"
#include "../langID.h"

extern HGE * hge;

namespace State {
	class ErrorActionListener : public gcn::ActionListener
	{
	private:
		Error * m_hOwn;
	public:
		void action(const gcn::ActionEvent &actionEvent)
		{
			if (actionEvent.getSource() == m_hOwn->butOK) {
				if (m_hOwn->standalone) {
					m_hOwn->stop = 1;
					m_hOwn->ret = RETURN_OK;
				}
				else {
					returnCode * rc = new returnCode;
					rc->Type = RC_ErrorState;
					rc->Ptr = RETURN_OK;
					m_hOwn->_popMe((int)rc);
				}
			}
			else if (actionEvent.getSource() == m_hOwn->butCancel) {
				if (m_hOwn->standalone) {
					m_hOwn->stop = 1;
					m_hOwn->ret = RETURN_CANCEL;
				}
				else {
					returnCode * rc = new returnCode;
					rc->Type = RC_ErrorState;
					rc->Ptr = RETURN_CANCEL;
					m_hOwn->_popMe((int)rc);
				}
			}
			else if (actionEvent.getSource() == m_hOwn->butYes) {
				if (m_hOwn->standalone) {
					m_hOwn->stop = 1;
					m_hOwn->ret = RETURN_YES;
				}
				else {
					returnCode * rc = new returnCode;
					rc->Type = RC_ErrorState;
					rc->Ptr = RETURN_YES;
					m_hOwn->_popMe((int)rc);
				}
			}
			else if (actionEvent.getSource() == m_hOwn->butNo) {
				if (m_hOwn->standalone) {
					m_hOwn->stop = 1;
					m_hOwn->ret = RETURN_NO;
				}
				else {
					returnCode * rc = new returnCode;
					rc->Type = RC_ErrorState;
					rc->Ptr = RETURN_NO;
					m_hOwn->_popMe((int)rc);
				}
			}
			/*if( actionEvent.getSource() == m_hOwn->butExit ){
			 m_hOwn->bExit = 1;
			}*/
		}
		ErrorActionListener(Error * owner)
		{
			m_hOwn = owner;
		}
	};

	int ErrorStandalone(const char * pszTitle, const char * pszErrorString, int piIcon, int piButtons, bool pbOpaque)
	{
		Error * er = new Error(pszTitle, pszErrorString, piIcon, piButtons, pbOpaque, 1);
		er->Init();
		while (1) {
			bool stop = er->Think();
			GV->StateMgr->ManualRenderStart();
			er->Render();
			GV->StateMgr->ManualRenderStop();
			if (stop) break;
		}
		bool ret = er->GetReturn();
		er->Destroy();
		delete er;
		return ret;
	}
}

State::Error::Error(const char * pszTitle, const char * pszErrorString, int piIcon, int piButtons, bool pbOpaque, bool pbStandalone)
{
	iIcon = piIcon;
	iButtons = piButtons;
	bOpaque = pbOpaque;
	szTitle = new char[strlen(pszTitle) + 1];
	strcpy(szTitle, pszTitle);
	szErrorString = new char[strlen(pszErrorString) + 1];
	strcpy(szErrorString, pszErrorString);
	standalone = pbStandalone;
	stop = 0;
	ret = 0;
}

State::Error::~Error()
{
}

bool State::Error::Opaque()
{
	return bOpaque;
}

void State::Error::Init()
{
	//MainActionListener * al = new MainActionListener(this);
	gui = new gcn::Gui();
	gui->setGraphics(GV->gcnGraphics);
	gui->setInput(GV->gcnInput);
	ErrorActionListener * al = new ErrorActionListener(this);

	winError = new SHR::Win(&GV->gcnParts, szTitle);
	int height = 100;//GV->fntMyriad13->GetHeightb(330, szErrorString)+75;
	winError->setDimension(gcn::Rectangle(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 200, hge->System_GetState(HGE_SCREENHEIGHT) / 2 - height / 2, 400, height));
	winError->setMovable(0);

	butOK = 0;
	butYes = 0;
	butNo = 0;
	butCancel = 0;

	if (iButtons == ST_ER_BUT_OK) {
		butOK = new SHR::But(GV->hGfxInterface, GETL(Lang_OK));
		butOK->setDimension(gcn::Rectangle(0, 0, 100, 33));
		butOK->addActionListener(al);
		winError->add(butOK, 150, height - 50);
	}
	else if (iButtons == ST_ER_BUT_YESNO) {
		butYes = new SHR::But(GV->hGfxInterface, GETL(Lang_Yes));
		butYes->setDimension(gcn::Rectangle(0, 0, 100, 33));
		butYes->addActionListener(al);
		winError->add(butYes, 75, height - 50);
		butNo = new SHR::But(GV->hGfxInterface, GETL(Lang_No));
		butNo->setDimension(gcn::Rectangle(0, 0, 100, 33));
		butNo->addActionListener(al);
		winError->add(butNo, 225, height - 50);
	}
	else if (iButtons == ST_ER_BUT_OKCANCEL) {
		butOK = new SHR::But(GV->hGfxInterface, GETL(Lang_OK));
		butOK->setDimension(gcn::Rectangle(0, 0, 100, 33));
		butOK->addActionListener(al);
		winError->add(butOK, 75, height - 50);
		butCancel = new SHR::But(GV->hGfxInterface, GETL(Lang_Cancel));
		butCancel->setDimension(gcn::Rectangle(0, 0, 100, 33));
		butCancel->addActionListener(al);
		winError->add(butCancel, 225, height - 50);
	}

	gui->setTop(winError);

	GV->IF->EnableCursor(1);
}

void State::Error::Destroy()
{
	delete[] szErrorString;
	delete[] szTitle;
	if (iButtons == ST_ER_BUT_OK)
		delete butOK;
	else if (iButtons == ST_ER_BUT_YESNO) {
		delete butYes, butNo;
	}
	else if (iButtons == ST_ER_BUT_OKCANCEL) {
		delete butOK, butCancel;
	}
	delete winError;
	delete gui;
}

bool State::Error::Think()
{
	GV->Console->Think();
	try {
		gui->logic();
	}
	catch (gcn::Exception& exc) {
		GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(), exc.getLine());
	}
	if (stop) return 1;
	return 0;
}

bool State::Error::Render()
{
	if (bOpaque)
		hge->Gfx_Clear(0xFF000000);
	try {
		gui->draw();
	}
	catch (gcn::Exception& exc) {
		GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(), exc.getLine());
	}
	switch (iIcon) {
	case ST_ER_ICON_WARNING:
		GV->gcnParts.sprIconWarning->Render(winError->getX() + 5, winError->getY() + 25);
		break;
	case ST_ER_ICON_INFO:
		GV->gcnParts.sprIconInfo->Render(winError->getX() + 5, winError->getY() + 25);
		break;
	case ST_ER_ICON_FATAL:
		GV->gcnParts.sprIconError->Render(winError->getX() + 5, winError->getY() + 25);
		break;
	}
	GV->fntMyriad13->printfb(winError->getX() + 75, winError->getY() + 25, 330, 100, HGETEXT_LEFT | HGETEXT_TOP, 0, szErrorString);
	GV->IF->Render();
	GV->Console->Render();
	return 0;
}

void State::Error::GainFocus(int iReturnCode, bool bFlipped)
{

}
