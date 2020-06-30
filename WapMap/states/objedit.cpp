#include "objedit.h"
#include "editing_ww.h"
#include "../cObjectUserData.h"
#include "../langID.h"
#include "../databanks/imageSets.h"

extern HGE *hge;

namespace ObjEdit {
    void cObjEditAL::action(const gcn::ActionEvent &actionEvent) {
        if (actionEvent.getSource() == m_hOwn->_butAddNext) {
            m_hOwn->Save();
            m_hOwn->SaveChanges();
            m_hOwn->bKill = 1;
            m_hOwn->_bAddNext = 1;
            return;
        } else if (actionEvent.getSource() == m_hOwn->_butSave) {
            m_hOwn->Save();
            m_hOwn->SaveChanges();
            m_hOwn->bKill = 1;
            m_hOwn->hState->bEditObjDelete = 0;
            return;
        }
        m_hOwn->Action(actionEvent);
    }

    cObjEdit::cObjEdit(WWD::Object *obj, State::EditingWW *st) {
        dwHighlightColor = 0xFFFF0000;
        bAllowDragging = 1;
        _bDragging = 0;
        _bAddNext = 0;
        bObjectSaved = 0;
        bChangesMade = 0;
        hOrigObj = obj;
        hState = st;
        hAL = new cObjEditAL(this);
		vpCB = new cObjEditVP(this);
		vpAdv = new WIDG::Viewport(vpCB, 0);
        bKill = 0;
        hTempObj = new WWD::Object(hOrigObj);
        hState->GetActivePlane()->AddObjectAndCalcID(hTempObj);
        hTempObj->SetUserData(new cObjUserData(hTempObj));
        hState->hPlaneData[hState->GetActivePlaneID()]->ObjectData.hQuadTree->UpdateObject(hTempObj);
        hState->vObjectsPicked[0] = hTempObj;

        _butAddNext = new SHR::But(GV->hGfxInterface, GETL2S("WinAddObjBatch", "AddNext"));
        _butAddNext->setDimension(gcn::Rectangle(0, 0, 100, 33));
        _butAddNext->addActionListener(hAL);

        _butSave = new SHR::But(GV->hGfxInterface, GETL(Lang_Save));
        _butSave->setDimension(gcn::Rectangle(0, 0, 100, 33));
        _butSave->addActionListener(hAL);

        _iMoveInitX = -1;
        _iMoveInitY = -1;
    }

    cObjEdit::~cObjEdit() {
        if (hState->bEditObjDelete) {
            if (!bObjectSaved) {
                hState->GetActivePlane()->DeleteObject(hOrigObj);
                hOrigObj = 0;
                hState->vObjectsPicked.clear();
            } else {
                hState->MarkUnsaved();
            }
        }
        hState->GetActivePlane()->DeleteObject(hTempObj);
        delete _butAddNext;
        delete _butSave;
        delete vpAdv;
        delete vpCB;
        delete hAL;
        if (hOrigObj != 0) {
            hState->vObjectsPicked.clear();
            hState->vObjectsPicked.push_back(hOrigObj);
        }
    }

    void cObjEdit::Think(bool bMouseConsumed) {
        _Think(bMouseConsumed);
        _butAddNext->setEnabled(_butSave->isEnabled());
        float mx, my;
        hge->Input_GetMousePos(&mx, &my);
        bool bMouseIn = 1;
        if (mx < hState->vPort->GetX() || my < hState->vPort->GetY() ||
            mx > hState->vPort->GetX() + hState->vPort->GetWidth() ||
            my > hState->vPort->GetY() + hState->vPort->GetHeight())
            bMouseIn = 0;

        if (!bMouseConsumed && bMouseIn) {
            int wmx = GV->editState->Scr2WrdX(GV->editState->GetActivePlane(), mx),
                wmy = GV->editState->Scr2WrdY(GV->editState->GetActivePlane(), my);
            if (_bDragging) {
                int destx = wmx - _iDragOffX,
                    desty = wmy - _iDragOffY;
                if (hge->Input_GetKeyState(HGEK_CTRL)) {
                    int diffx = wmx - _iMoveInitX,
                        diffy = wmy - _iMoveInitY;
                    int diffmodulox = wmx % 64;
                    int diffmoduloy = wmy % 64;
                    int modx = 0, mody = 0;

                    if (diffmodulox > 16 && diffmodulox < 48) modx += 32;
                    else if (diffmodulox >= 48) modx += 64;

                    if (diffmoduloy > 16 && diffmoduloy < 48) mody += 32;
                    else if (diffmoduloy >= 48) mody += 64;

                    destx = _iMoveInitX + diffx - (diffmodulox - modx);
                    desty = _iMoveInitY + diffy - (diffmoduloy - mody);
                } else if (hge->Input_GetKeyState(HGEK_SHIFT)) {
                    int diffx = wmx - _iMoveInitX,
                            diffy = wmy - _iMoveInitY;
                    float ratio = (diffy == 0 ? 2 : float(diffx) / float(diffy));
                    if (ratio >= -0.50f && ratio <= 0.50)
                        destx = _iMoveInitX;
                    else if (ratio > 1.5f || ratio < -1.5f) {
                        desty = _iMoveInitY;
                    } else {
                        int diff = std::min(abs(diffx), abs(diffy));
                        if (diffy < 0) {
                            if (ratio < -0.5f && ratio > -1.5f) { //upright
                                destx = _iMoveInitX + diff;
                                desty = _iMoveInitY - diff;
                            } else if (ratio > 0.5f && ratio < 1.5f) { //upleft
                                destx = _iMoveInitX - diff;
                                desty = _iMoveInitY - diff;
                            }
                        } else {
                            if (ratio < -0.5f && ratio > -1.5f) { //downleft
                                destx = _iMoveInitX - diff;
                                desty = _iMoveInitY + diff;
                            } else if (ratio > 0.5f && ratio < 1.5f) { //downright
                                destx = _iMoveInitX + diff;
                                desty = _iMoveInitY + diff;
                            }
                        }
                    }
                }
                if (hTempObj->GetParam(WWD::Param_LocationX) != destx ||
                    hTempObj->GetParam(WWD::Param_LocationY) != desty) {
                    int sourceX = hTempObj->GetParam(WWD::Param_LocationX),
                        sourceY = hTempObj->GetParam(WWD::Param_LocationY);
                    GV->editState->vPort->MarkToRedraw();
                    hTempObj->SetParam(WWD::Param_LocationX, destx);
                    hTempObj->SetParam(WWD::Param_LocationY, desty);
                    GetUserDataFromObj(hTempObj)->SyncToObj();
                    ObjectMovedInsideEasyEdit(sourceX, sourceY);
                }
            }
            if (_bDragging && !hge->Input_GetKeyState(HGEK_LBUTTON)) {
                _bDragging = 0;
                _iMoveInitX = -1;
                _iMoveInitY = -1;
            }
            WWD::Rect r = GV->editState->SprBank->GetObjectRenderRect(hTempObj);
            r.x2 += r.x1;
            r.y2 += r.y1;
            if (wmx > r.x1 && wmx < r.x2 && wmy > r.y1 && wmy < r.y2) {
                if (bAllowDragging) {
                    if (hge->Input_KeyDown(HGEK_LBUTTON)) {
                        _bDragging = 1;
                        if (_iMoveInitX == -1)
                            _iMoveInitX = hTempObj->GetParam(WWD::Param_LocationX);
                        if (_iMoveInitY == -1)
                            _iMoveInitY = hTempObj->GetParam(WWD::Param_LocationY);
                        _iDragOffX = wmx - hTempObj->GetParam(WWD::Param_LocationX);
                        _iDragOffY = wmy - hTempObj->GetParam(WWD::Param_LocationY);
                    }
                }
            }
        }
    }

    bool cObjEdit::ChangesMade() {
        if (strcmp(hTempObj->GetName(), hOrigObj->GetName()) != 0)
            return 1;
        if (strcmp(hTempObj->GetLogic(), hOrigObj->GetLogic()) != 0)
            return 1;
        if (strcmp(hTempObj->GetImageSet(), hOrigObj->GetImageSet()) != 0)
            return 1;
        if (strcmp(hTempObj->GetAnim(), hOrigObj->GetAnim()) != 0)
            return 1;

        for (int i = 0; i < 8; i++) {
            if (hTempObj->GetUserValue(i) != hOrigObj->GetUserValue(i))
                return 1;
        }

        for (int i = 0; i < OBJ_PARAMS_CNT; i++) {
            if ((WWD::OBJ_PARAMS) i == WWD::Param_ID) continue;
            if (hTempObj->GetParam((WWD::OBJ_PARAMS) i) != hOrigObj->GetParam((WWD::OBJ_PARAMS) i))
                return 1;
        }

        if (hTempObj->GetMoveRect() != hOrigObj->GetMoveRect())
            return 1;
        if (hTempObj->GetHitRect() != hOrigObj->GetHitRect())
            return 1;
        if (hTempObj->GetAttackRect() != hOrigObj->GetAttackRect())
            return 1;
        if (hTempObj->GetClipRect() != hOrigObj->GetClipRect())
            return 1;
        if (hTempObj->GetUserRect(0) != hOrigObj->GetUserRect(0))
            return 1;
        if (hTempObj->GetUserRect(1) != hOrigObj->GetUserRect(1))
            return 1;

        if (hTempObj->GetDrawFlags() != hOrigObj->GetDrawFlags())
            return 1;
        if (hTempObj->GetDynamicFlags() != hOrigObj->GetDynamicFlags())
            return 1;
        if (hTempObj->GetAddFlags() != hOrigObj->GetAddFlags())
            return 1;
        if (hTempObj->GetHitTypeFlags() != hOrigObj->GetHitTypeFlags())
            return 1;
        if (hTempObj->GetTypeFlags() != hOrigObj->GetTypeFlags())
            return 1;
        if (hTempObj->GetUserFlags() != hOrigObj->GetUserFlags())
            return 1;

        return 0;
    }

	bool cObjEdit::IsAnyInputFocused()
	{
		return win->gotFocus();
	}

	void cObjEdit::GetWindowPosition(int& x, int& y)
	{
        x = win->getX();
        y = win->getY();
	}

	void cObjEdit::SetWindowPosition(int x, int y)
	{
        win->setX(x);
        win->setY(y);
	}

    void cObjEdit::SaveChanges() {
        bObjectSaved = true;

        if (!hState->bEditObjDelete) {
            bChangesMade = ChangesMade();
            if (!bChangesMade) return;
        }

        WWD::Object *tmpptr;
        tmpptr = hTempObj;
        hTempObj = hOrigObj;
        hOrigObj = tmpptr;
        int iOrigid = hTempObj->GetParam(WWD::Param_ID);
        hOrigObj->SetParam(WWD::Param_ID, iOrigid);
        hState->MarkUnsaved();
        hState->vPort->MarkToRedraw();
    }

	void cObjEditVP::Draw(int iCode)
	{
		m_hOwn->Draw();
	}
}
