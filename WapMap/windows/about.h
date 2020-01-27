#ifndef H_WIN_ABOUT
#define H_WIN_ABOUT

#include "window.h"

class winAbout : public cWindow {
private:
    WIDG::Viewport *vp;
public:
    winAbout();

    ~winAbout();

    virtual void Draw(int piCode);

    virtual void Think();

    virtual void Open();

    virtual void Close();
};

#endif // H_WIN_TILEBROWSER
