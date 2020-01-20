#include "CodeEditor.h"
#include "../globals.h"
#include "../../shared/gcnWidgets/wContainer.h"
#include "../../shared/commonFunc.h"
#include "../returncodes.h"
#include "../langID.h"
#include "../databanks/logics.h"
#include "../../shared/Scintilla/Scintilla.h"
#include "../../shared/Scintilla/SciLexer.h"
#include "editing_ww.h"
#include <Windows.h>

extern HGE * hge;

State::CodeEditor::CodeEditor(cCustomLogic * hLog, bool bAllowNameChange, std::string strLogicName)
{
	bNameChange = bAllowNameChange;
	if (hLog == 0) {
		bChildrenLogic = 1;
		cFile n;
		n.hFeed = 0;
		n.strPath = "";
		if (strLogicName.length() == 0) {
			hLogic = new cCustomLogic(n, (bAllowNameChange ? GETL2S("Win_CodeEditor", "NewLogic")
				: "main"));
		}
		else {
			hLogic = new cCustomLogic(n, strLogicName);
		}
	}
	else {
		hLogic = hLog;
		bChildrenLogic = 0;
	}
}

State::CodeEditor::~CodeEditor()
{
}

bool State::CodeEditor::Opaque()
{
	return 0;
}

LRESULT State::CodeEditor::SendEditor(UINT Msg, WPARAM wParam, LPARAM lParam) {
	return ::SendMessage(hwndScintilla, Msg, wParam, lParam);
}

void State::CodeEditor::SetStyle(int style, DWORD fore, DWORD back, bool setback, int size, const char *face) {
	SendEditor(SCI_STYLESETFORE, style, fore);
	if (setback)
		SendEditor(SCI_STYLESETBACK, style, back);
	if (size >= 1)
		SendEditor(SCI_STYLESETSIZE, style, size);
	if (face)
		SendEditor(SCI_STYLESETFONT, style, reinterpret_cast<LPARAM>(face));
}

void State::CodeEditor::Init()
{
	for (byte i = 0; i < 255; i++)
		bKeyboardBuffer[i] = 0;
	bEditFocus = 0;
	//MainActionListener * al = new MainActionListener(this);
	gui = new gcn::Gui();
	gui->setGraphics(GV->gcnGraphics);
	gui->setInput(GV->gcnInput);

	winMe = new SHR::Win(&GV->gcnParts, "");
	int windowH = hge->System_GetState(HGE_SCREENHEIGHT) - 100;
	winMe->setDimension(gcn::Rectangle(hge->System_GetState(HGE_SCREENWIDTH) / 2 - 375, hge->System_GetState(HGE_SCREENHEIGHT) / 2 - windowH / 2, 750, windowH));
	winMe->setMovable(0);
	winMe->addActionListener(this);
	winMe->setClose(1);

	SetDirty(0);

	/*butUndo = new SHR::But(GV->hGfxInterface, GV->sprIcons[Icon_Undo]);
	butUndo->setDimension(gcn::Rectangle(0,0,32,32));
	butUndo->addActionListener(this);
	butUndo->setEnabled(0);
	butUndo->setRenderBG(0);
	winMe->add(butUndo, 5, 10);
	butRedo = new SHR::But(GV->hGfxInterface, GV->sprIcons[Icon_Redo]);
	butRedo->setDimension(gcn::Rectangle(0,0,32,32));
	butRedo->addActionListener(this);
	butRedo->setEnabled(0);
	butRedo->setRenderBG(0);
	winMe->add(butRedo, 5+32, 10);*/

	butSave = new SHR::But(GV->hGfxInterface, GETL2S("Win_CodeEditor", "Save"));
	butSave->setDimension(gcn::Rectangle(0, 0, 150, 30));
	butSave->addActionListener(this);
	winMe->add(butSave, 370 - 150, windowH - 20 - 30 - 10);

	butCancel = new SHR::But(GV->hGfxInterface, GETL2S("Win_CodeEditor", "Cancel"));
	butCancel->setDimension(gcn::Rectangle(0, 0, 150, 30));
	butCancel->addActionListener(this);
	winMe->add(butCancel, 380, windowH - 20 - 30 - 10);

	gui->setTop(winMe);

	GV->IF->EnableCursor(1);
	bExit = 0;

	if (bNameChange) {
		labName = new SHR::Lab(GETL2S("Win_CodeEditor", "Name"));
		labName->adjustSize();
		tfName = new SHR::TextField(hLogic->GetName());
		tfName->setDimension(gcn::Rectangle(0, 0, 250, 20));
		tfName->addActionListener(this);
		winMe->add(labName, 5, 13);
		winMe->add(tfName, 10 + labName->getWidth(), 13);
	}

	hwndScintilla = CreateWindowEx(0,
		"Scintilla", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN,
		winMe->getX() + 5, winMe->getY() + 25 + 32 * bNameChange, winMe->getWidth() - 10, winMe->getHeight() - 80 - 32 * bNameChange, hge->System_GetState(HGE_HWND), 0, 0, NULL);

	char* buffer = new char[hLogic->GetContent().length() + 1];
	strcpy(buffer, hLogic->GetContent().c_str());
	SendEditor(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
	SendEditor(SCI_SETMARGINWIDTHN, 0, SendEditor(SCI_TEXTWIDTH, STYLE_LINENUMBER, reinterpret_cast<LPARAM>(static_cast<const char*>("_99999"))));

	SendEditor(SCI_SETLEXER, SCLEX_LUA);
	SetStyle(STYLE_DEFAULT, RGB(0xa1, 0xa1, 0xa1), RGB(0x13, 0x13, 0x13), 1, 8, "Lucida Console");
	SendEditor(SCI_STYLECLEARALL);

	SetStyle(STYLE_LINENUMBER, RGB(255, 255, 255), RGB(0x30, 0x30, 0x30), 1);

	SendEditor(SCI_SETCARETFORE, RGB(255, 255, 255));

	SetStyle(SCE_LUA_COMMENT, RGB(0x3f, 0xdf, 0x0d));
	SetStyle(SCE_LUA_COMMENTLINE, RGB(0, 0x80, 0));
	SetStyle(SCE_LUA_COMMENTDOC, RGB(0x0d, 0xdf, 0x9c));
	SetStyle(SCE_LUA_NUMBER, RGB(255, 255, 255));
	SetStyle(SCE_LUA_STRING, RGB(255, 255, 255));
	SetStyle(SCE_LUA_CHARACTER, RGB(255, 255, 255));
	SetStyle(SCE_LUA_LITERALSTRING, RGB(255, 255, 255));
	SetStyle(SCE_LUA_PREPROCESSOR, RGB(0, 255, 0));
	SetStyle(SCE_LUA_OPERATOR, RGB(0xc1, 0x57, 0x36));
	//SetStyle(SCE_LUA_IDENTIFIER, RGB(255, 0, 255));
	SetStyle(SCE_LUA_STRINGEOL, RGB(0, 0, 255));
	SetStyle(SCE_LUA_LABEL, RGB(0, 255, 255));
	SetStyle(SCE_LUA_WORD, RGB(0x02, 0xb8, 0x8f));
	SetStyle(SCE_LUA_WORD2, RGB(0x02, 0xb8, 0x8f));
	SetStyle(SCE_LUA_WORD3, RGB(0x02, 0xb8, 0x8f));
	SetStyle(SCE_LUA_WORD4, RGB(0x02, 0xb8, 0x8f));
	SetStyle(SCE_LUA_WORD5, RGB(0x02, 0xb8, 0x8f));
	SetStyle(SCE_LUA_WORD6, RGB(0xFF, 0, 0));

	SendEditor(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>("and break do else elseif end false for function goto if in local nil not or repeat return then true until while"));
	SendEditor(SCI_STYLESETBOLD, SCE_LUA_WORD, 1);

	SetStyle(SCE_LUA_WORD7, RGB(0xE1, 0xE8, 0x1A));
	SendEditor(SCI_SETKEYWORDS, 6, reinterpret_cast<LPARAM>("main hit attack init destroy draw"));
	SendEditor(SCI_STYLESETBOLD, SCE_LUA_WORD7, 1);

	SetStyle(SCE_LUA_WORD8, RGB(0xE1, 0xE8, 0x1A));
	SendEditor(SCI_SETKEYWORDS, 7, reinterpret_cast<LPARAM>("SetAnimation GetClaw GetInput CreateObject SetFrame SetImage AlwaysActive GetObject ObjectBelow ClawJump AnimationStep"));
	/*
	#define SCE_LUA_DEFAULT 0
	#define SCE_LUA_COMMENT 1
	#define SCE_LUA_COMMENTLINE 2
	#define SCE_LUA_COMMENTDOC 3
	#define SCE_LUA_NUMBER 4
	#define SCE_LUA_WORD 5
	#define SCE_LUA_STRING 6
	#define SCE_LUA_CHARACTER 7
	#define SCE_LUA_LITERALSTRING 8
	#define SCE_LUA_PREPROCESSOR 9
	#define SCE_LUA_OPERATOR 10
	#define SCE_LUA_IDENTIFIER 11
	#define SCE_LUA_STRINGEOL 12
	#define SCE_LUA_WORD2 13
	#define SCE_LUA_WORD3 14
	#define SCE_LUA_WORD4 15
	#define SCE_LUA_WORD5 16
	#define SCE_LUA_WORD6 17
	#define SCE_LUA_WORD7 18
	#define SCE_LUA_WORD8 19
	#define SCE_LUA_LABEL 20
	*/

	//folding
	SendEditor(SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
	SendEditor(SCI_SETPROPERTY, (WPARAM)"fold.compact", (LPARAM)"0");
	SendEditor(SCI_SETPROPERTY, (WPARAM)"fold.comment", (LPARAM)"1");
	SendEditor(SCI_SETPROPERTY, (WPARAM)"fold.preprocessor", (LPARAM)"1");
	SendEditor(SCI_SETMARGINWIDTHN, 1, 0);
	SendEditor(SCI_SETMARGINTYPEN, 1, SC_MARGIN_SYMBOL);
	SendEditor(SCI_SETMARGINMASKN, 1, SC_MASK_FOLDERS);
	SendEditor(SCI_SETMARGINWIDTHN, 1, 20);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY);
	SendEditor(SCI_SETFOLDFLAGS, 16, 0);
	SendEditor(SCI_SETMARGINSENSITIVEN, 1, 1);
	SendEditor(SCI_SETFOLDMARGINCOLOUR, 1, RGB(0x30, 0x30, 0x30));
	SendEditor(SCI_SETFOLDMARGINHICOLOUR, 1, RGB(0x40, 0x40, 0x40));

	SendEditor(SCI_ADDTEXT, hLogic->GetContent().length(), reinterpret_cast<LPARAM>(static_cast<char *>(buffer)));
	SendEditor(SCI_EMPTYUNDOBUFFER);
	SendEditor(SCI_SETSAVEPOINT);
	hge->System_SetState(HGE_HIDEMOUSE, 0);
	delete[] buffer;
}

void State::CodeEditor::Destroy()
{
	if (bChildrenLogic && !bExitSave)
		delete hLogic;
	hge->System_SetState(HGE_HIDEMOUSE, 1);
	//delete butUndo;
	//delete butRedo;
	if (bNameChange) {
		delete tfName;
		delete labName;
	}
	delete butSave;
	delete butCancel;
	DestroyWindow(hwndScintilla);
	delete winMe;
	delete gui;
}

void State::CodeEditor::OS_Notify()
{
	/*SCNotification * notify = (SCNotification*)hge->System_GetNotifyParam();
	if( notify->nmhdr.code == SCN_MARGINCLICK ){
	 const int modifiers = notify->modifiers;
	 const int position = notify->position;
	 const int margin = notify->margin;
	 const int line_number = SendEditor(SCI_LINEFROMPOSITION, position, 0);

	 switch( margin ){
	  case 1:
	   {
		SendEditor(SCI_TOGGLEFOLD, line_number, 0);
	   }
	  break;
	 }
	}else if( notify->nmhdr.code == SCN_FOCUSIN ){
	 bEditFocus = 1;
	 if( bNameChange && tfName->isFocused() )
	  tfName->_getFocusHandler()->focusNone();
	}else if( notify->nmhdr.code == SCN_FOCUSOUT ){
	 bEditFocus = 0;
	}else if( notify->nmhdr.code == SCN_SAVEPOINTREACHED ){
	 SetDirty(0);
	}else if( notify->nmhdr.code == SCN_SAVEPOINTLEFT ){
	 SetDirty(1);
	}*/
}

void State::CodeEditor::SetDirty(bool b)
{
	bDirty = b;
	char tmp[256];
	std::string name = (!strcmp(hLogic->GetName(), "main") ? GETL2S("Win_CodeEditor", "GlobalScript")
		: std::string(hLogic->GetName()) + ".lua");
	if (bDirty)
		sprintf(tmp, "%s - %s*", GETL2S("Win_CodeEditor", "WinCaption"), name.c_str());
	else
		sprintf(tmp, "%s - %s", GETL2S("Win_CodeEditor", "WinCaption"), name.c_str());
	winMe->setCaption(tmp);
}

bool State::CodeEditor::Think()
{
	//butUndo->setEnabled(SendEditor(SCI_CANUNDO));
	//butRedo->setEnabled(SendEditor(SCI_CANREDO));
	if (bEditFocus) {
		int c = 0;
		byte states[255];
		for (byte i = 0; i < 255; i++) {
			SHORT code = GetAsyncKeyState(i);
			states[i] = (code & 0x8000) ? 0x80 : 0;
		}
		for (byte i = 0; i < 255; i++) {
			if (states[i] & 0x80 && !(bKeyboardBuffer[i] & 0x80)) {
				WORD buf;
				unsigned int scan = MapVirtualKey(i, 0);
				scan = scan & 0x7FFFFFFF;
				int val = ToAscii(i, scan, states, &buf, 0);
				char chr[3] = { '\0', '\0', '\0' };
				if (val == 1) {
					chr[0] = (buf & 0x00FF);
				}
				else if (val == 2) {
					chr[0] = (buf & 0xFF00 >> 8);
					chr[1] = (buf & 0x00FF);
				}
				if (chr[1] != '\0' && iscntrl(chr[1])) chr[1] = '\0';
				if (chr[0] != '\0' && iscntrl(chr[0])) {
					chr[0] = '\0';
					if (chr[1] != '\0') {
						chr[0] = chr[1];
						chr[1] = '\0';
					}
				}
				if (chr[0] != '\0') {
					char * ptr = setlocale(LC_ALL, NULL);
					SendEditor(SCI_ADDTEXT, 1 + (chr[1] != '\0'), (LPARAM)chr);
				}
			}
		}
		memcpy(&bKeyboardBuffer, &states, 255);
	}
	GV->Console->Think();
	try {
		gui->logic();
	}
	catch (gcn::Exception& exc) {
		GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(), exc.getLine());
	}
	if (bExit) {
		if (bExitSave) {
			int len = SendEditor(SCI_GETLENGTH);
			char* buffer = new char[len + 1];
			SendEditor(SCI_GETTEXT, len + 1, (LPARAM)buffer);
			hLogic->SetContent(buffer);
			delete[] buffer;
		}
		stCodeEditorRC * myRC = new stCodeEditorRC;
		myRC->hLogic = (bChildrenLogic && !bExitSave ? 0
			: hLogic);
		myRC->bNewLogic = bChildrenLogic;
		myRC->bSaved = bExitSave;
		myRC->bChanges = bDirty;

		returnCode * rc = new returnCode;
		rc->Type = RC_CodeEditor;
		rc->Ptr = (int)myRC;
		_popMe((int)rc);
	}
	return 0;
}

bool State::CodeEditor::Render()
{
	try {
		gui->draw();
	}
	catch (gcn::Exception& exc) {
		GV->Console->Printf("~r~Guichan exception: ~w~%s (%s:%d)", exc.getMessage().c_str(), exc.getFilename().c_str(), exc.getLine());
	}

	if (bFocus && !bEditFocus) {
		float mx, my;
		hge->Input_GetMousePos(&mx, &my);
		gcn::Widget * w = gui->getTop(), *last = NULL;
		while (w != NULL) {
			last = w;
			int x, y;
			w->getAbsolutePosition(x, y);
			w = w->getWidgetAt(mx - x, my - y);
		}
		bool hand = 0;
		if (last != NULL) hand = last->showHand();
		SetCursor(LoadCursor(0, hand ? IDC_HAND : IDC_ARROW));
	}

	//GV->IF->Render();
	GV->Console->Render();
	return 0;
}

void State::CodeEditor::AppFocus(bool bGain)
{
	bFocus = bGain;
}

void State::CodeEditor::GainFocus(int iReturnCode, bool bFlipped)
{

}

void State::CodeEditor::action(const gcn::ActionEvent &actionEvent)
{
	if (actionEvent.getSource() == winMe || actionEvent.getSource() == butCancel) {
		//if( bDirty ){
		// if( MessageBox(hge->System_GetState(HGE_HWND), "Are you sure you want to exit code editor? Unsaved changes will be lost.", "WapMap", MB_YESNO|MB_ICONWARNING) == IDYES )
		bExit = 1;
		bExitSave = 0;
		//}
	}
	else if (actionEvent.getSource() == butSave) {
		if (bNameChange) {
			if (GV->editState->hCustomLogics->GetLogicByName(hLogic->GetName()) != 0 || !strcmp(hLogic->GetName(), "main")) {
				MessageBox(hge->System_GetState(HGE_HWND), GETL2S("Win_CodeEditor", "NameTaken"), "WapMap", MB_OK | MB_ICONERROR);
				return;
			}
		}
		bExit = 1;
		bExitSave = 1;
	}
	else if (bNameChange && actionEvent.getSource() == tfName) {
		hLogic->SetName(tfName->getText());
		SetDirty(bDirty);
	}
	/*else if( actionEvent.getSource() == butUndo ){
	 SendEditor(SCI_UNDO);
	}else if( actionEvent.getSource() == butRedo ){
	 SendEditor(SCI_REDO);
	}*/
}
