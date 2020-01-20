#ifndef H_SHARED_STATEMGR
#define H_SHARED_STATEMGR

#include "hge.h"
#include "hgeSprite.h"

#include <vector>

namespace SHR {

	class cStateMgr;

	class cState {
	private:
		cStateMgr * _hOwner;
		bool _bOnTop;
		friend class cStateMgr;
	public:
		virtual bool Opaque() { return 0; };
		virtual void Init() { };
		virtual void Destroy() { };
		virtual bool Think() { return 0; };
		virtual bool Render() { return 0; };
		virtual void GainFocus(int iReturnCode, bool bFlipped) { };
		virtual void PreRender() { };
		virtual bool PromptExit() { return  1; };
		virtual void GfxRestore() {};
		virtual void FileDropped() {};
		virtual void AppFocus(bool bGain) {};
		virtual void OS_Notify() {};

		bool _flipMe(int returnCode);
		void _popMe(int returnCode);
		void _replaceFor(cState * phState);
		bool _isOnTop() { return _bOnTop; };
		void _ForceRender();
	};

	class cStateMgr {
	private:
		std::vector<cState*> m_hStateStack;
		bool m_bFocused;
		bool m_bPush, m_bPop, m_bReplace, m_bFlip;
		int m_iRet;
		cState * m_hNew;
		bool Flip(int iReturnCode);
		void Pop(int iReturnCode);
		void ReplaceFor(cState * phState);
		void ForceRender();

		bool bBuffering;
		HTARGET hBuffer;
		hgeSprite * sprBuffer;
		HSHADER myShader;
		friend class cState;
	public:
		void Init();
		cStateMgr();
		~cStateMgr();
		bool Think();
		bool Render();
		bool ExitFunc();
		void GfxRestore();
		void FileDropped();
		void Push(cState * phState);
		cState * GetState() { return m_hStateStack.back(); };
		void ReloadShader();
		bool BufferingEnabled() { return bBuffering; };
		void EnableBuffering(bool b);
		void ManualRenderStart();
		void ManualRenderStop();
		void AppFocus(bool bGain);
		bool IsAppFocused() { return m_bFocused; };
		void OS_Notify();
	};
}

#endif
