#include "cStateMgr.h"
#include "../WapMap/globals.h"

extern HGE *hge;

SHR::cStateMgr::cStateMgr() {
	m_bPush = m_bPop = m_bReplace = false;
	m_bFocused = true;
}

void SHR::cStateMgr::EnableBuffering(bool b) {
	if (bBuffering == b) return;
	if (b) {
		hBuffer = hge->Target_Create(hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT), false);
		sprBuffer = new hgeSprite(0, 0, 0, hge->System_GetState(HGE_SCREENWIDTH),
								  hge->System_GetState(HGE_SCREENHEIGHT));
	} else {
		hge->Target_Free(hBuffer);
		delete sprBuffer;
	}
	bBuffering = b;
}

void SHR::cStateMgr::ReloadShader() {
	if (!BufferingEnabled()) EnableBuffering(true);
	FILE *f = fopen("test.fx", "rb");
	if (f != NULL) {
		fclose(f);
		//myShader = hge->Shader_Create("test.fx", 0);
		if (myShader == NULL) {
			GV->Console->Print("~r~Failed to load shader. HGE printout:");
			GV->Console->Print(hge->System_GetErrorMessage());
		} else {
			GV->Console->Print("~g~Shader loaded.");
		}
		//HSHTECH tech = hge->Shader_GetTechnique(myShader, "std");
		//if( tech == NULL )
		// GV->Console->Print("~r~Failed to load shader technique.");
		//hge->Shader_SetTechnique(myShader, tech);
	}
}

void SHR::cStateMgr::Init() {
	bBuffering = false;
	hBuffer = NULL;
	sprBuffer = NULL;
	myShader = NULL;
}

SHR::cStateMgr::~cStateMgr() {
	for (auto & state : m_hStateStack) {
		state->Destroy();
		delete state;
	}
	EnableBuffering(false);
}

void SHR::cStateMgr::ReplaceFor(cState *phState) {
	m_bReplace = true;
	m_hNew = phState;
}

bool SHR::cStateMgr::Think() {
	bool bActionsDone = false;
	if (m_bReplace || m_bPop || m_bPush || m_bFlip)
		bActionsDone = true;

	while (bActionsDone) {
		bActionsDone = false;
		if (m_bReplace) {
			m_bPop = m_bPush = m_bReplace = false;
			m_hStateStack.back()->Destroy();
			delete m_hStateStack.back();
			m_hStateStack.pop_back();
			m_hStateStack.push_back(m_hNew);
			m_hNew->_hOwner = this;
			m_hNew->_bOnTop = true;
			m_hNew->Init();
			bActionsDone = true;
		}
		if (m_bPop) {
			m_bPop = false;
			m_hStateStack.back()->Destroy();
			delete m_hStateStack.back();
			m_hStateStack.pop_back();
			if (!m_hStateStack.empty()) {
				m_hStateStack.back()->_bOnTop = true;
				m_hStateStack.back()->GainFocus({m_ReturnCode.type, m_ReturnCode.value}, false);
			}
			bActionsDone = true;
		}
		if (m_bPush) {
			if (!m_hStateStack.empty())
				m_hStateStack.back()->_bOnTop = false;
			m_hStateStack.push_back(m_hNew);
			m_hNew->_hOwner = this;
			m_hNew->_bOnTop = true;
			m_bPush = false;
			m_hNew->Init();
			bActionsDone = true;
		}
		if (m_bFlip) {
			m_bFlip = false;
			if (m_hStateStack.size() > 1) {
				cState *oldon = m_hStateStack.back();
				m_hStateStack.pop_back();
				cState *oldunder = m_hStateStack.back();
				m_hStateStack.pop_back();
				m_hStateStack.push_back(oldon);
				m_hStateStack.push_back(oldunder);
				oldunder->_bOnTop = true;
				oldon->_bOnTop = false;
				oldunder->GainFocus({m_ReturnCode.type, m_ReturnCode.value}, true);
				bActionsDone = true;
			}
		}
		SHR::cTooltip::ResetTooltip();
	}

	if (m_hStateStack.empty()) {
		return false;
	}
	return m_hStateStack.back()->Think();
}

void SHR::cStateMgr::ManualRenderStart() {
	if (bBuffering)
		hge->Gfx_BeginScene(hBuffer);
	else
		hge->Gfx_BeginScene();
}

void SHR::cStateMgr::ManualRenderStop() {
	hge->Gfx_EndScene();

	if (bBuffering) {
		sprBuffer->SetTexture(hge->Target_GetTexture(hBuffer));

		hge->Gfx_BeginScene();

		/*if( myShader != NULL ){
		 sprBuffer->SetColor(0);
		 sprBuffer->Render(0,0);
		 sprBuffer->SetColor(0xFFFFFFFF);

		 hge->Shader_SetTexture(myShader, hge->Shader_GetParam(myShader, "g_myTex"), sprBuffer->GetTexture());

		 int passes = hge->Shader_Begin(myShader, HGESHADER_SAVE);
		 for(int i=0;i<passes;i++){
		  hge->Shader_BeginPass(myShader, i);
		  sprBuffer->Render(0,0);
		  hge->Gfx_FlushBuffer();
		  hge->Shader_EndPass(myShader);
		 }
		 hge->Shader_End(myShader);
		}else*/
		sprBuffer->Render(0, 0);
		hge->Gfx_EndScene();
	}
}

bool SHR::cStateMgr::Render() {
	if (m_hStateStack.empty()) {
		return false;
	}

	if (!m_hStateStack.back()->Opaque() && m_hStateStack.size() > 1) {
		m_hStateStack[m_hStateStack.size() - 2]->PreRender();
	}
	m_hStateStack.back()->PreRender();

	ManualRenderStart();

	bool ret = false;
	if (!m_hStateStack.back()->Opaque() && m_hStateStack.size() > 1) {
		ret = m_hStateStack[m_hStateStack.size() - 2]->Render();
	}
	ret = std::max(m_hStateStack.back()->Render(), ret);

	SHR::cTooltip::RenderTooltip();

	ManualRenderStop();
	return false;
}

bool SHR::cStateMgr::ExitFunc() {
	if (m_hStateStack.empty()) {
		return true;
	}
	return m_hStateStack.back()->PromptExit();
}

void SHR::cStateMgr::GfxRestore() {
	if (m_hStateStack.empty()) return;
	if (!m_hStateStack.back()->Opaque() && m_hStateStack.size() > 1) {
		m_hStateStack[m_hStateStack.size() - 2]->GfxRestore();
	}
	return m_hStateStack.back()->GfxRestore();
}

void SHR::cStateMgr::AppFocus(bool bGain) {
	if (m_hStateStack.empty()) return;
	m_bFocused = bGain;
	m_hStateStack.back()->AppFocus(bGain);
}

void SHR::cStateMgr::OS_Notify() {
	if (m_hStateStack.empty()) return;
	return m_hStateStack.back()->OS_Notify();
}

void SHR::cStateMgr::FileDraggedIn() {
	if (m_hStateStack.empty()) return;
	return m_hStateStack.back()->FileDraggedIn();
}

void SHR::cStateMgr::FileDraggedOut() {
	if (m_hStateStack.empty()) return;
	return m_hStateStack.back()->FileDraggedOut();
}

void SHR::cStateMgr::FileDropped() {
	if (m_hStateStack.empty()) return;
	return m_hStateStack.back()->FileDropped();
}

void SHR::cStateMgr::Push(cState *nState) {
	m_bPush = true;
	m_hNew = nState;
}

void SHR::cState::_replaceFor(cState *phState) {
	_hOwner->ReplaceFor(phState);
}

void SHR::cState::_ForceRender() {
	_hOwner->ForceRender();
}

void SHR::cStateMgr::ForceRender() {
	hge->Gfx_BeginScene();

	if (!m_hStateStack.back()->Opaque() && m_hStateStack.size() > 1) {
		m_hStateStack[m_hStateStack.size() - 2]->Render();
	}

	hge->Gfx_EndScene();
}

void SHR::cStateMgr::OnResize() {
    for (auto state : m_hStateStack) {
        state->OnResize();
    }
}
