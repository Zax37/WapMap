#ifndef H_WIN_ABOUT
#define H_WIN_ABOUT

#include "window.h"

class winAbout : public cWindow {
private:
    WIDG::Viewport *vp;
public:
    winAbout();

    virtual void Draw(int piCode);

    virtual void Think();
};

#endif // H_WIN_TILEBROWSER
