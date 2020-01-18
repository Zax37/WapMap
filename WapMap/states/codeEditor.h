#ifndef H_STATE_CODEEDITOR
#define H_STATE_CODEEDITOR

#include "../../shared/cStateMgr.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wTextField.h"
#include "../../shared/cWWD.h"
#include "guichan.hpp"

class cCustomLogic;

struct stCodeEditorRC
{
 cCustomLogic * hLogic;
 bool bNewLogic;
 bool bSaved;
 bool bChanges;
};

namespace State {

 class CodeEditor : public SHR::cState, gcn::ActionListener {
  public:
   CodeEditor(cCustomLogic * hLog, bool bAllowNameChange, std::string strLogicName);
   ~CodeEditor();
   virtual bool Opaque();
   virtual void Init();
   virtual void Destroy();
   virtual bool Think();
   virtual bool Render();
   virtual void GainFocus(int iReturnCode, bool bFlipped);
   virtual void OS_Notify();

   virtual void action(const gcn::ActionEvent &actionEvent);

   virtual void AppFocus(bool bGain);

   void SetDirty(bool b);
  private:
   byte bKeyboardBuffer[255];
   bool bExit, bFocus, bEditFocus, bDirty, bChildrenLogic, bExitSave, bNameChange;
   gcn::Gui * gui;
   SHR::Win * winMe;
   cCustomLogic * hLogic;
   HWND hwndScintilla;
   SHR::But * butSave, * butCancel;
   SHR::But * butUndo, * butRedo;
   SHR::Lab * labName;
   SHR::TextField * tfName;

   LRESULT SendEditor(UINT Msg, WPARAM wParam=0, LPARAM lParam=0);
   void SetStyle(int style, DWORD fore, DWORD back=RGB(255,255,255), bool setback=0, int size=-1, const char *face=0);
 };
};

#endif
