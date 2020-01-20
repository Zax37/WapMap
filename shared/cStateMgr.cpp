#define NOMINMAX
#include "cStateMgr.h"
#ifndef NULL
#define NULL 0
#endif

#include "../WapMap/globals.h"

#include "hge.h"
extern HGE * hge;

SHR::cStateMgr::cStateMgr()
{
	m_bPush = m_bPop = m_bReplace = 0;
	m_iRet = 0;
	m_bFocused = 1;
}

void SHR::cStateMgr::EnableBuffering(bool b)
{
	if (bBuffering == b) return;
	if (b) {
		hBuffer = hge->Target_Create(hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT), 0);
		sprBuffer = new hgeSprite(0, 0, 0, hge->System_GetState(HGE_SCREENWIDTH), hge->System_GetState(HGE_SCREENHEIGHT));
	}
	else {
		hge->Target_Free(hBuffer);
		delete sprBuffer;
	}
	bBuffering = b;
}

void SHR::cStateMgr::ReloadShader()
{
	if (!BufferingEnabled()) EnableBuffering(1);
	FILE * f = fopen("test.fx", "rb");
	if (f != NULL) {
		fclose(f);
		//myShader = hge->Shader_Create("test.fx", 0);
		if (myShader == NULL) {
			GV->Console->Print("~r~Failed to load shader. HGE printout:");
			GV->Console->Print(hge->System_GetErrorMessage());
		}
		else {
			GV->Console->Print("~g~Shader loaded.");
		}
		//HSHTECH tech = hge->Shader_GetTechnique(myShader, "std");
		//if( tech == NULL )
		// GV->Console->Print("~r~Failed to load shader technique.");
		//hge->Shader_SetTechnique(myShader, tech);
	}
}

void SHR::cStateMgr::Init()
{
	bBuffering = 0;
	hBuffer = NULL;
	sprBuffer = NULL;
	myShader = NULL;
}

SHR::cStateMgr::~cStateMgr()
{
	for (int i = 0; i < m_hStateStack.size(); i++) {
		m_hStateStack[i]->Destroy();
		delete m_hStateStack[i];
	}
	EnableBuffering(0);
}

void SHR::cStateMgr::ReplaceFor(cState * phState)
{
	m_bReplace = 1;
	m_hNew = phState;
}

bool SHR::cStateMgr::Think()
{
	bool bActionsDone = 0;
	if (m_bReplace || m_bPop || m_bPush || m_bFlip)
		bActionsDone = 1;

	while (bActionsDone) {
		bActionsDone = 0;
		if (m_bReplace) {
			m_bPop = m_bPush = m_bReplace = 0;
			m_hStateStack.back()->Destroy();
			delete m_hStateStack.back();
			m_hStateStack.pop_back();
			m_hStateStack.push_back(m_hNew);
			m_hNew->_hOwner = this;
			m_hNew->_bOnTop = 1;
			m_hNew->Init();
			bActionsDone = 1;
		}
		if (m_bPop) {
			m_bPop = 0;
			m_hStateStack.back()->Destroy();
			delete m_hStateStack.back();
			m_hStateStack.pop_back();
			if (m_hStateStack.size() != 0) {
				m_hStateStack.back()->_bOnTop = 1;
				m_hStateStack.back()->GainFocus(m_iRet, 0);
			}
			bActionsDone = 1;
		}
		if (m_bPush) {
			if (m_hStateStack.size() > 0)
				m_hStateStack.back()->_bOnTop = 0;
			m_hStateStack.push_back(m_hNew);
			m_hNew->_hOwner = this;
			m_hNew->_bOnTop = 1;
			m_bPush = 0;
			m_hNew->Init();
			bActionsDone = 1;
		}
		if (m_bFlip) {
			m_bFlip = 0;
			if (m_hStateStack.size() > 1) {
				cState * oldon = m_hStateStack.back();
				m_hStateStack.pop_back();
				cState * oldunder = m_hStateStack.back();
				m_hStateStack.pop_back();
				m_hStateStack.push_back(oldon);
				m_hStateStack.push_back(oldunder);
				oldunder->_bOnTop = 1;
				oldon->_bOnTop = 0;
				oldunder->GainFocus(m_iRet, 1);
				bActionsDone = 1;
			}
		}
	}

	if (m_hStateStack.size() == 0) {
		return 0;
	}
	return m_hStateStack.back()->Think();
}

void SHR::cStateMgr::ManualRenderStart()
{
	if (bBuffering)
		hge->Gfx_BeginScene(hBuffer);
	else
		hge->Gfx_BeginScene();
}

void SHR::cStateMgr::ManualRenderStop()
{
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

bool SHR::cStateMgr::Render()
{
	if (m_hStateStack.size() == 0) {
		return 0;
	}

	if (!m_hStateStack.back()->Opaque() && m_hStateStack.size() > 1) {
		m_hStateStack[m_hStateStack.size() - 2]->PreRender();
	}
	m_hStateStack.back()->PreRender();

	ManualRenderStart();

	bool ret = 0;
	if (!m_hStateStack.back()->Opaque() && m_hStateStack.size() > 1) {
		ret = m_hStateStack[m_hStateStack.size() - 2]->Render();
	}
	ret = std::max(m_hStateStack.back()->Render(), ret);

	ManualRenderStop();
	return 0;
}

bool SHR::cStateMgr::ExitFunc()
{
	if (m_hStateStack.size() == 0) {
		return 1;
	}
	return m_hStateStack.back()->PromptExit();
}

void SHR::cStateMgr::GfxRestore()
{
	if (m_hStateStack.size() == 0) return;
	if (!m_hStateStack.back()->Opaque() && m_hStateStack.size() > 1) {
		m_hStateStack[m_hStateStack.size() - 2]->GfxRestore();
	}
	return m_hStateStack.back()->GfxRestore();
}

void SHR::cStateMgr::AppFocus(bool bGain)
{
	if (m_hStateStack.size() == 0) return;
	m_bFocused = bGain;
	m_hStateStack.back()->AppFocus(bGain);
}

void SHR::cStateMgr::OS_Notify()
{
	if (m_hStateStack.size() == 0)
		return;
	return m_hStateStack.back()->OS_Notify();
}

void SHR::cStateMgr::FileDropped()
{
	if (m_hStateStack.size() == 0)
		return;
	return m_hStateStack.back()->FileDropped();
}

void SHR::cStateMgr::Push(cState * nState)
{
	m_bPush = 1;
	m_hNew = nState;
}

void SHR::cStateMgr::Pop(int iReturnCode)
{
	if (m_bFlip) m_bFlip = 0;
	m_bPop = 1;
	m_iRet = iReturnCode;
}

bool SHR::cStateMgr::Flip(int iReturnCode)
{
	if (m_hStateStack.size() < 2) return 0;
	if (m_bPop) m_bPop = 0;
	m_bFlip = 1;
	m_iRet = iReturnCode;
	return 1;
}

bool SHR::cState::_flipMe(int returnCode)
{
	return _hOwner->Flip(returnCode);
}

void SHR::cState::_popMe(int returnCode)
{
	_hOwner->Pop(returnCode);
}

void SHR::cState::_replaceFor(cState * phState)
{
	_hOwner->ReplaceFor(phState);
}

void SHR::cState::_ForceRender()
{
	_hOwner->ForceRender();
}

void SHR::cStateMgr::ForceRender()
{
	hge->Gfx_BeginScene();

	bool ret = 0;
	if (!m_hStateStack.back()->Opaque() && m_hStateStack.size() > 1) {
		ret = m_hStateStack[m_hStateStack.size() - 2]->Render();
	}
	ret = std::max(m_hStateStack.back()->Render(), ret);

	hge->Gfx_EndScene();
}
