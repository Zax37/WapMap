#include "dialog.h"
#include "../globals.h"
#include "../../shared/gcnWidgets/wContainer.h"
#include "../../shared/commonFunc.h"
#include "../returncodes.h"
#include "../langID.h"

extern HGE *hge;

namespace State {
    int MessageBox(const char *pszTitle, const char *pszErrorString, int piIcon, int piButtons)
    {
        return ShowDialog<void>(pszTitle, pszErrorString, piIcon, piButtons).value;
    }

    const TextReturnCode& InputDialog(const char *pszTitle, const char *pszErrorString, int piButtons, const char* textFieldInitValue) {
        return ShowDialog<std::string>(pszTitle, pszErrorString, 0, piButtons | ST_DIALOG_TEXT_FIELD, textFieldInitValue);
    }

    template <class RC>
    const ReturnCode<RC>& ShowDialog(const char* pszTitle, const char* pszErrorString, int piIcon, int piButtons, const char* textFieldInitValue) {
        auto dialog = new Dialog(pszTitle, pszErrorString, piIcon, piButtons, textFieldInitValue);
        dialog->Init();
        GV->StateMgr->Push(dialog);
        do {
            hge->System_DoManualMainLoop();
        } while (GV->StateMgr->GetState() == dialog);
        return GV->StateMgr->GetReturnCode<RC>();
    }
}

State::Dialog::Dialog(const char *pszTitle, const char *pszErrorString, int piIcon, int piButtons, const char* textFieldInitValue) {
    iIcon = piIcon;
    iButtons = piButtons;
    szTitle = new char[strlen(pszTitle) + 1];
    strcpy(szTitle, pszTitle);
    szMessage = new char[strlen(pszErrorString) + 1];
    strcpy(szMessage, pszErrorString);
    initialText = textFieldInitValue;
}

void State::Dialog::Init() {
    gui = new gcn::Gui();
    gui->setGraphics(GV->gcnGraphics);
    gui->setInput(GV->gcnInput);
    gui->addGlobalKeyListener(this);

    win = new SHR::Win(&GV->gcnParts, szTitle);
    messageHeight = GV->fntMyriad16->GetStringBlockHeight(350, szMessage) + GV->fntMyriad16->GetHeight();

    int width = (iIcon ? 85 : 15) + GV->fntMyriad16->GetStringBlockWidth(350, szMessage);
    if (width < 230) {
        width = 230;
    }

    int height = (iButtons & ST_DIALOG_TEXT_FIELD)
            ? messageHeight + 105
            : iIcon
                ? std::max(messageHeight, 64) + 70
                : messageHeight + 70;

    win->setDimension(gcn::Rectangle(hge->System_GetState(HGE_SCREENWIDTH) / 2 - width / 2,
            hge->System_GetState(HGE_SCREENHEIGHT) / 2 - height / 2, width, height));
    win->setMovable(false);

    butOK = 0;
    butYes = 0;
    butNo = 0;
    butCancel = 0;
    textField = iButtons & ST_DIALOG_TEXT_FIELD
            ? (initialText
                ? new SHR::TextField(initialText)
                : new SHR::TextField())
            : 0;

    if (textField) {
        textField->setDimension(gcn::Rectangle(0, 0, width - (iIcon ? 90 : 15), 20));
        win->add(textField, iIcon ? 72 : 5, messageHeight + 12);
    }

    switch (iButtons & ST_DIALOG_BUT_MASK) {
        case ST_DIALOG_BUT_OK:
            butOK = new SHR::But(GV->hGfxInterface, GETL(Lang_OK));
            butOK->setDimension(gcn::Rectangle(0, 0, 100, 33));
            butOK->addActionListener(this);
            win->add(butOK, width / 2 - 50, height - 55);
        break;
        case ST_DIALOG_BUT_YESNO:
            butYes = new SHR::But(GV->hGfxInterface, GETL(Lang_Yes));
            butYes->setDimension(gcn::Rectangle(0, 0, 100, 33));
            butYes->addActionListener(this);
            win->add(butYes, width / 2 - 110, height - 55);
            butNo = new SHR::But(GV->hGfxInterface, GETL(Lang_No));
            butNo->setDimension(gcn::Rectangle(0, 0, 100, 33));
            butNo->addActionListener(this);
            win->add(butNo, width / 2 + 5, height - 55);
        break;
        case ST_DIALOG_BUT_YESNOCANCEL:
            butYes = new SHR::But(GV->hGfxInterface, GETL(Lang_Yes));
            butYes->setDimension(gcn::Rectangle(0, 0, 100, 33));
            butYes->addActionListener(this);
            win->add(butYes, width / 2 - 155, height - 55);
            butNo = new SHR::But(GV->hGfxInterface, GETL(Lang_No));
            butNo->setDimension(gcn::Rectangle(0, 0, 100, 33));
            butNo->addActionListener(this);
            win->add(butNo, width / 2 - 50, height - 55);
            butCancel = new SHR::But(GV->hGfxInterface, GETL(Lang_Cancel));
            butCancel->setDimension(gcn::Rectangle(0, 0, 100, 33));
            butCancel->addActionListener(this);
            win->add(butCancel, width / 2 + 55, height - 55);
        break;
        case ST_DIALOG_BUT_OKCANCEL:
            butOK = new SHR::But(GV->hGfxInterface, GETL(Lang_OK));
            butOK->setDimension(gcn::Rectangle(0, 0, 100, 33));
            butOK->addActionListener(this);
            win->add(butOK, width / 2 - 110, height - 55);
            butCancel = new SHR::But(GV->hGfxInterface, GETL(Lang_Cancel));
            butCancel->setDimension(gcn::Rectangle(0, 0, 100, 33));
            butCancel->addActionListener(this);
            win->add(butCancel, width / 2 + 5, height - 55);
        break;
        case ST_DIALOG_BUT_RETRYSKIP:
            butRetry = new SHR::But(GV->hGfxInterface, GETL(Lang_Retry));
            butRetry->setDimension(gcn::Rectangle(0, 0, 100, 33));
            butRetry->addActionListener(this);
            win->add(butRetry, width / 2 - 110, height - 55);
            butSkip = new SHR::But(GV->hGfxInterface, GETL(Lang_Skip));
            butSkip->setDimension(gcn::Rectangle(0, 0, 100, 33));
            butSkip->addActionListener(this);
            win->add(butSkip, width / 2 + 5, height - 55);
        break;
    }

    gui->setTop(win);
    if (textField) {
        textField->requestFocus();
    }
}

void State::Dialog::Destroy() {
    delete[] szMessage;
    delete[] szTitle;
    if (textField) delete textField;
    switch (iButtons & ST_DIALOG_BUT_MASK) {
        case ST_DIALOG_BUT_OK:
            delete butOK;
        break;
        case ST_DIALOG_BUT_YESNO:
            delete butYes;
            delete butNo;
        break;
        case ST_DIALOG_BUT_YESNOCANCEL:
            delete butYes;
            delete butNo;
            delete butCancel;
        break;
        case ST_DIALOG_BUT_OKCANCEL:
            delete butOK;
            delete butCancel;
        break;
        case ST_DIALOG_BUT_RETRYSKIP:
            delete butRetry;
            delete butSkip;
        break;
    }
    delete win;
    delete gui;
}

bool State::Dialog::Think() {
    GV->Console->Think();
    try {
        gui->logic();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }
    return false;
}

bool State::Dialog::Render() {
    try {
        gui->draw();
    }
    catch (gcn::Exception &exc) {
        GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(),
                            exc.getLine());
    }

    int offX = win->getX() + 5, offY = win->getY() + 25;

    if (iIcon) {
        switch (iIcon) {
            case ST_DIALOG_ICON_QUESTION:
                GV->gcnParts.sprIconQuestion->Render(offX, offY);
                break;
            case ST_DIALOG_ICON_INFO:
                GV->gcnParts.sprIconInfo->Render(offX, offY);
                break;
            case ST_DIALOG_ICON_WARNING:
                GV->gcnParts.sprIconWarning->Render(offX, offY);
                break;
            case ST_DIALOG_ICON_ERROR:
                GV->gcnParts.sprIconError->Render(offX, offY);
                break;
        }
        offX += 70;

        if (messageHeight < 64) {
            if (textField) {
                offY += (64 - messageHeight - 20) / 2;
            } else {
                offY += (64 - messageHeight) / 2;
            }
        }
    } else {
        offX += 3;
    }
    offY += 7;

    GV->fntMyriad16->printfb(offX, offY, 350, 100, HGETEXT_LEFT | HGETEXT_TOP, 0,
                             szMessage);
    GV->Console->Render();
    return false;
}

void State::Dialog::GainFocus(ReturnCode<void> code, bool bFlipped) {

}

void State::Dialog::action(const gcn::ActionEvent &actionEvent) {
    int value = 0;
    if (actionEvent.getSource() == butOK) {
        value = RETURN_OK;
    } else if (actionEvent.getSource() == butCancel) {
        value = RETURN_CANCEL;
    } else if (actionEvent.getSource() == butYes) {
        value = RETURN_YES;
    } else if (actionEvent.getSource() == butNo) {
        value = RETURN_NO;
    } else if (actionEvent.getSource() == butRetry) {
        value = RETURN_RETRY;
    } else if (actionEvent.getSource() == butSkip) {
        value = RETURN_SKIP;
    }

    if (iButtons & ST_DIALOG_TEXT_FIELD) {
        _popMe(TextReturnCode{ ReturnCodeType::DialogState, value, textField->getText() });
    } else {
        _popMe({ ReturnCodeType::DialogState, value });
    }
}

void State::Dialog::keyReleased(KeyEvent &keyEvent) {
    switch(keyEvent.getKey().getValue()) {
        case Key::ENTER:
            switch (iButtons & ST_DIALOG_BUT_MASK) {
                case ST_DIALOG_BUT_OK:
                case ST_DIALOG_BUT_OKCANCEL:
                    butOK->simulatePress();
                    break;
                case ST_DIALOG_BUT_YESNO:
                    butYes->simulatePress();
                    break;
                case ST_DIALOG_BUT_RETRYSKIP:
                    butRetry->simulatePress();
                    break;
            }
            break;
        case Key::ESCAPE:
            switch (iButtons & ST_DIALOG_BUT_MASK) {
                case ST_DIALOG_BUT_OK:
                    butOK->simulatePress();
                    break;
                case ST_DIALOG_BUT_YESNO:
                    butNo->simulatePress();
                    break;
                case ST_DIALOG_BUT_OKCANCEL:
                    butCancel->simulatePress();
                    break;
                case ST_DIALOG_BUT_RETRYSKIP:
                    butSkip->simulatePress();
                    break;
            }
            break;
    }
    keyEvent.consume();
}
