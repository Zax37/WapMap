#include "dialog.h"
#include "../globals.h"
#include "../../shared/gcnWidgets/wContainer.h"
#include "../../shared/commonFunc.h"
#include "../returncodes.h"
#include "../langID.h"
#ifdef WAPMAP_ENABLE_IMGUI
#include "../imguiTheme.h"
#include "../legacyText.h"
#include "../cInterfaceSheet.h"
#endif

extern HGE *hge;

namespace State {
    int MessageBox(const char *pszTitle, const char *pszErrorString, int piIcon, int piButtons) {
        return ShowDialog(pszTitle, pszErrorString, piIcon, piButtons);
    }

    int InputDialog(const char *pszTitle, const char *pszErrorString, int piButtons, const char* textFieldInitValue) {
        return ShowDialog(pszTitle, pszErrorString, 0, piButtons | ST_DIALOG_TEXT_FIELD, textFieldInitValue);
    }

    const ReturnCode& ShowDialog(const char* pszTitle, const char* pszErrorString, int piIcon, int piButtons, const char* textFieldInitValue) {
        auto dialog = new Dialog(pszTitle, pszErrorString, piIcon, piButtons, textFieldInitValue);
        GV->StateMgr->Push(dialog);
        do {
            hge->System_DoManualMainLoop();
        } while (GV->StateMgr->GetState() == dialog);
        return GV->StateMgr->GetReturnCode();
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
#ifdef WAPMAP_ENABLE_IMGUI
    if (iButtons & ST_DIALOG_TEXT_FIELD) {
        const auto text = LegacyText::ToUtf8(initialText ? initialText : "", LegacyText::CodePage(GV->Lang->GetCode()));
        nativeText.assign(std::max(size_t(513), text.size() + 1), 0);
        std::copy(text.begin(), text.end(), nativeText.begin());
    }
    return;
#endif
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
#ifdef WAPMAP_ENABLE_IMGUI
    return;
#endif
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
#ifdef WAPMAP_ENABLE_IMGUI
    if (nativeResult >= 0) _popMe({ReturnCodeType::DialogState, nativeResult});
    return false;
#endif
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
#ifdef WAPMAP_ENABLE_IMGUI
    return RenderImGui();
#endif
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

void State::Dialog::GainFocus(const ReturnCode& code, bool bFlipped) {

}

#ifdef WAPMAP_ENABLE_IMGUI
bool State::Dialog::RenderImGui() {
    if (nativeResult >= 0) return false;
    const auto cp = LegacyText::CodePage(GV->Lang->GetCode());
    const auto title = LegacyText::ToUtf8(szTitle, cp) + "###WapMapDialog" + std::to_string(reinterpret_cast<std::uintptr_t>(this));
    std::vector<std::pair<const char*, int>> buttons;
    switch (iButtons & ST_DIALOG_BUT_MASK) {
        case ST_DIALOG_BUT_YESNO: buttons = {{GETL(Lang_Yes), RETURN_YES}, {GETL(Lang_No), RETURN_NO}}; break;
        case ST_DIALOG_BUT_YESNOCANCEL: buttons = {{GETL(Lang_Yes), RETURN_YES}, {GETL(Lang_No), RETURN_NO}, {GETL(Lang_Cancel), RETURN_CANCEL}}; break;
        case ST_DIALOG_BUT_OKCANCEL: buttons = {{GETL(Lang_OK), RETURN_OK}, {GETL(Lang_Cancel), RETURN_CANCEL}}; break;
        case ST_DIALOG_BUT_RETRYSKIP: buttons = {{GETL(Lang_Retry), RETURN_RETRY}, {GETL(Lang_Skip), RETURN_SKIP}}; break;
        default: buttons = {{GETL(Lang_OK), RETURN_OK}}; break;
    }
    const auto* viewport = ImGui::GetMainViewport();
    const float left = iIcon ? 82.f : 12.f;
    const float buttonWidth = static_cast<float>(buttons.size()) * 108 - 8;
    const float width = std::max(buttonWidth + 24, std::max(240.f,
        left + 12 + std::min(350.f, GV->fntMyriad16->GetStringBlockWidth(350, szMessage))));
    const float textHeight = GV->fntMyriad16->GetStringBlockHeight(static_cast<int>(width - left - 12), szMessage);
    const bool input = (iButtons & ST_DIALOG_TEXT_FIELD) != 0;
    const float height = std::max(180.f, std::min(std::max(textHeight, iIcon ? 64.f : 0.f) +
        (input ? 134.f : 88.f), viewport->WorkSize.y - 20));
    if (nativePosition) {
        ImGui::OpenPopup(title.c_str());
        nativePosition = false;
    }
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({width, height}, ImGuiCond_Always);
    ImGui::SetNextWindowScroll({0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, IM_COL32(0, 0, 0, 145));
    const bool shown = ImGui::BeginPopupModal(title.c_str(), nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleColor(); ImGui::PopStyleVar();
    if (!shown) return false;
    WapMapImGui::WindowChrome(*hge, *GV->fntMyriad16, nullptr, szTitle);
    hgeSprite* icon = iIcon == ST_DIALOG_ICON_QUESTION ? GV->gcnParts.sprIconQuestion :
        iIcon == ST_DIALOG_ICON_INFO ? GV->gcnParts.sprIconInfo :
        iIcon == ST_DIALOG_ICON_WARNING ? GV->gcnParts.sprIconWarning :
        iIcon == ST_DIALOG_ICON_ERROR ? GV->gcnParts.sprIconError : nullptr;
    const auto origin = ImGui::GetWindowPos();
    if (icon) WapMapImGui::Sprite(ImGui::GetWindowDrawList(), *hge, icon, {origin.x + 8, origin.y + 36});
    ImGui::SetCursorPos({left, 35});
    if (ImGui::BeginChild("message", {width - left - 12, height - (input ? 134.f : 88.f)}, ImGuiChildFlags_NavFlattened))
        WapMapImGui::WrappedBitmapMarkupText(*hge, *GV->fntMyriad16, szMessage, width - left - 28);
    ImGui::EndChild();
    bool submitted = false;
    std::optional<std::string> encoded = std::string{};
    if (input) {
        ImGui::SetCursorPos({12, height - 78});
        ImGui::SetNextItemWidth(width - 24);
        if (nativeFocus) ImGui::SetKeyboardFocusHere();
        submitted = ImGui::InputText("##input", nativeText.data(), nativeText.size(),
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
        encoded = LegacyText::FromUtf8(nativeText.data(), cp, std::max(size_t(128), initialText ? std::strlen(initialText) : size_t(0)));
        if (!encoded) {
            ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(220, 75, 75, 255));
            if (ImGui::IsItemHovered()) {
                const auto message = LegacyText::ToUtf8(GETL2S("MapProperties", "InvalidText"), cp);
                ImGui::SetTooltip("%s", message.c_str());
            }
        }
    }
    const auto finish = [&](int result) {
        if (input && encoded) GV->szLastInputDialogText = *encoded;
        nativeResult = result;
        ImGui::CloseCurrentPopup();
    };
    for (size_t i = 0; i < buttons.size(); ++i) {
        ImGui::SetCursorPos({(width - buttonWidth) / 2 + static_cast<float>(i) * 108, height - 45});
        const bool cancel = buttons[i].second == RETURN_CANCEL || buttons[i].second == RETURN_NO || buttons[i].second == RETURN_SKIP;
        ImGui::BeginDisabled(!cancel && !encoded);
        if (WapMapImGui::BitmapButton(*hge, *GV->fntMyriad16, *GV->hGfxInterface, buttons[i].first, {100, 33})) finish(buttons[i].second);
        if (nativeFocus && !input && i == 0) ImGui::SetItemDefaultFocus();
        ImGui::EndDisabled();
    }
    if (nativeResult < 0 && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        if (submitted && encoded) finish(buttons.front().second);
        const int escapeResult = buttons.back().second;
        const bool canEscape = encoded || escapeResult == RETURN_CANCEL || escapeResult == RETURN_NO || escapeResult == RETURN_SKIP;
        if (nativeResult < 0 && canEscape && ImGui::IsKeyPressed(ImGuiKey_Escape)) finish(escapeResult);
    }
    nativeFocus = false;
    ImGui::EndPopup();
    return false;
}
#endif

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
        GV->szLastInputDialogText = textField->getText();
    }
    _popMe({ ReturnCodeType::DialogState, value });
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
