#ifndef H_WINDOW
#define H_WINDOW

#include "../wViewport.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../globals.h"

extern HGE* hge;

class cWindow : public WIDG::VpCallback, public gcn::ActionListener {
protected:
    SHR::Win myWin;
public:
    cWindow(const char* name) : myWin(&GV->gcnParts, name) {
        myWin.setVisible(false);
        myWin.setClose(true);
        myWin.addActionListener(this);
    }

    cWindow(const char* name, int w, int h) : cWindow(name) {
        myWin.setDimension(gcn::Rectangle(0, 0, w, h));
    }

    ~cWindow() override = default;

    virtual void Think() {};

    virtual void OnDocumentChange() {};

    virtual void Open() {
        myWin.setPosition((hge->System_GetState(HGE_SCREENWIDTH) - myWin.getWidth()) / 2,
                          (hge->System_GetState(HGE_SCREENHEIGHT) - myWin.getHeight()) / 2);
        myWin.setVisible(true);
        myWin.getParent()->moveToTop(&myWin);
    };

    virtual void Close() {
        myWin.setVisible(false);
    };

    SHR::Win *GetWindow() { return &myWin; };

    void action(const gcn::ActionEvent &) override {};
};

#endif // H_WINDOW
