#ifndef H_STATE_ERROR
#define H_STATE_ERROR

#include "../../shared/cStateMgr.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wTextField.h"
#include "../../shared/cWWD.h"
#include "guichan.hpp"

#define ST_DIALOG_ICON_QUESTION 1
#define ST_DIALOG_ICON_INFO     2
#define ST_DIALOG_ICON_WARNING  3
#define ST_DIALOG_ICON_ERROR    4

#define ST_DIALOG_BUT_OK            1
#define ST_DIALOG_BUT_YESNO         2
#define ST_DIALOG_BUT_OKCANCEL      4
#define ST_DIALOG_BUT_YESNOCANCEL   8
#define ST_DIALOG_BUT_RETRYSKIP     16

#define ST_DIALOG_BUT_MASK      63

#define ST_DIALOG_TEXT_FIELD    64

#define RETURN_OK               0
#define RETURN_CANCEL           1
#define RETURN_YES              2
#define RETURN_NO               3
#define RETURN_RETRY            4
#define RETURN_SKIP             5

namespace State {
    template <class RC>
    const ReturnCode<RC>& ShowDialog(const char* pszTitle, const char* pszErrorString, int piIcon, int piButtons = ST_DIALOG_BUT_OK, const char* textFieldInitValue = 0);

    int MessageBox(const char *pszTitle, const char *pszErrorString, int piIcon, int piButtons = ST_DIALOG_BUT_OK);

    const TextReturnCode& InputDialog(const char *pszTitle, const char *pszErrorString, int piButtons = ST_DIALOG_BUT_OK, const char* textFieldInitValue = 0);

    class Dialog : public SHR::cState, gcn::KeyListener, gcn::ActionListener {
    public:
        Dialog(const char *pszTitle, const char *pszErrorString, int piIcon, int piButtons = ST_DIALOG_BUT_OK, const char* textFieldInitValue = 0);

        void Init() override;

        void Destroy() override;

        bool Think() override;

        bool Render() override;

        void GainFocus(ReturnCode<void> code, bool bFlipped) override;

        void action(const gcn::ActionEvent &actionEvent) override;

        void keyReleased(KeyEvent& keyEvent) override;

    private:
        gcn::Gui *gui;
        SHR::Win *win;
        int iIcon, iButtons;
        SHR::But *butOK, *butYes, *butNo, *butCancel, *butRetry, *butSkip;
        SHR::TextField *textField;
        char *szMessage, *szTitle;
        int messageHeight;
        const char *initialText;
    };
}

#endif
