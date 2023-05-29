#ifndef H_STATE_LOADMAP
#define H_STATE_LOADMAP

#include "../../shared/cStateMgr.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wProgressBar.h"
#include "../../shared/cWWD.h"
#include "../../shared/cProgressInfo.h"
#include "guichan.hpp"
#include "../cDataController.h"
#include "../cParallelLoop.h"
#include "../cMDI.h"

#include <future>

struct membuf : std::streambuf
{
    membuf(char* begin, char* end)
    {
        this->setg(begin, begin, end);
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in) override
    {
        if (dir == std::ios_base::cur)
            gbump(off);
        else if (dir == std::ios_base::end)
            setg(eback(), egptr() + off, egptr());
        else if (dir == std::ios_base::beg)
            setg(eback(), eback() + off, egptr());
        return gptr() - eback();
    }

    pos_type seekpos(pos_type sp, std::ios_base::openmode which) override
    {
        return seekoff(sp - pos_type(off_type(0)), std::ios_base::beg, which);
    }
};

namespace State {
    class LoadMap : public SHR::cState, public cParallelCallback {
    private:
        char *alt_ptr;
        uint32_t alt_size;
        bool alt_planes;
        int alt_width, alt_height;
        char alt_name[64], alt_author[64];
        bool wasRendered = false;

    public:
        LoadMap(WWD::Parser* parser);

        LoadMap(const char *pszFilename);

        LoadMap(char *ptr, uint32_t size, bool addPlanes, int mw, int mh, const char* name, const char* author);

        ~LoadMap();

        virtual bool Opaque();

        virtual void Init();

        virtual void Destroy();

        virtual bool Think();

        virtual bool Render();

        gcn::Gui *gui;
        SHR::Win *winLoad;
        SHR::ProgressBar *barWhole, *barAction;
        SHR::Lab *labWhole, *labAction, *labDesc;

        char *szDir, *szFilename, *szFilepath;

        std::future<DocumentData*> mapLoading;
        DocumentData* MapLoadTask();

        //int iGlobalProgress, iDetailedProgress, iDetailed

        //sMapData * md;
        //friend class LoadMapActionListener;
    };
};

#endif
