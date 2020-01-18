#ifndef SHR_PBAR_H
#define SHR_PBAR_H

#include <string>

#include "guichan/graphics.hpp"
#include "guichan/platform.hpp"
#include "guichan/widget.hpp"
#include "../cGUIparts.h"

using namespace gcn;

namespace SHR
{
    class GCN_CORE_DECLSPEC ProgressBar: public gcn::Widget
    {
    public:

        ProgressBar(guiParts * phGfx);
        virtual void draw(Graphics* graphics);

        void setEnd(int i){ iEnd = i; };
        void setProgress(int i){ iProgress = i; };
        int GetProgress(){ return iProgress; };
        float getPercentageProgress();
        void setOpacity(char op){ cOpacity = op; };
    protected:
        int iProgress, iEnd;
        guiParts * hGfx;
        char cOpacity;
    };
}

#endif // end GCN_ProgressBar_HPP
