#ifndef H_SHARED_STATEMGR
#define H_SHARED_STATEMGR

#include "hge.h"
#include "hgesprite.h"
#include "../WapMap/returncodes.h"
#include <vector>

namespace SHR {

    class cState;

    class cStateMgr {
    private:
        std::vector<cState *> m_hStateStack;
        bool m_bFocused;
        bool m_bPush, m_bPop, m_bReplace, m_bFlip;
        BiggestReturnCode m_ReturnCode;
        cState *m_hNew;

        template <class RC>
        bool Flip(ReturnCode<RC> code) {
            if (m_hStateStack.size() < 2) return false;
            if (m_bPop) m_bPop = false;
            m_bFlip = true;
            m_ReturnCode = code;
            return true;
        }

        template <class RC>
        void Pop(ReturnCode<RC> code) {
            if (m_bFlip) m_bFlip = false;
            m_bPop = true;
            m_ReturnCode = code;
        }

        void ReplaceFor(cState *phState);

        void ForceRender();

        bool bBuffering;
        HTARGET hBuffer;
        hgeSprite *sprBuffer;
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

        void FileDraggedIn();

        void FileDraggedOut();

        void FileDropped();

        void Push(cState *phState);

        cState *GetState() { return m_hStateStack.back(); };

        void ReloadShader();

        bool BufferingEnabled() { return bBuffering; };

        void EnableBuffering(bool b);

        void ManualRenderStart();

        void ManualRenderStop();

        void AppFocus(bool bGain);

        bool IsAppFocused() { return m_bFocused; };

        void OS_Notify();

        template <class RC>
        const ReturnCode<RC>& GetReturnCode() { return *(ReturnCode<RC>*)(&m_ReturnCode); }

        void OnResize();
    };


    class cState {
    private:
        cStateMgr* _hOwner;
        bool _bOnTop;

        friend class cStateMgr;

    public:
        virtual bool Opaque() { return false; };

        virtual void Init() {};

        virtual void Destroy() {};

        virtual bool Think() { return false; };

        virtual bool Render() { return false; };

        virtual void GainFocus(ReturnCode<void> code, bool bFlipped) {};

        virtual void PreRender() {};

        virtual bool PromptExit() { return true; };

        virtual void GfxRestore() {};

        virtual void FileDraggedIn() {};

        virtual void FileDraggedOut() {};

        virtual void FileDropped() {};

        virtual void AppFocus(bool bGain) {};

        virtual void OS_Notify() {};

        virtual void OnResize() {};

        template <class RC>
        bool _flipMe(ReturnCode<RC> code) {
            return _hOwner->Flip(code);
        }

        bool _flipMe(ReturnCode<void> code) {
            return _hOwner->Flip<void>(code);
        }

        template <class RC>
        void _popMe(ReturnCode<RC> code) {
            _hOwner->Pop(code);
        }

        void _popMe(ReturnCode<void> code) {
            _hOwner->Pop<void>(code);
        }

        void _replaceFor(cState* phState);

        bool _isOnTop() { return _bOnTop; };

        void _ForceRender();
    };
}

#endif
