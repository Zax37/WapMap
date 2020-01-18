#include "cColorPicker.h"
#include "globals.h"
#include "../shared/commonFunc.h"

extern HGE * hge;

cColorPicker::cColorPicker(SHR::Contener * dest, int x, int y, DWORD dwInitColor, bool bWithAlpha)
{
    winPicker = new SHR::Win(&GV->gcnParts, "");
    winPicker->setDimension(gcn::Rectangle(0, 0, 250, 131));
    winPicker->setMovable(0);
    winPicker->setShadow(0);
    bUseAlpha = bWithAlpha;
    if( bUseAlpha )
     winPicker->setHeight(156);

    labR = new SHR::Lab("R:");
    labR->adjustSize();
    winPicker->add(labR, 5, 13);
    labG = new SHR::Lab("G:");
    labG->adjustSize();
    winPicker->add(labG, 5, 38);
    labB = new SHR::Lab("B:");
    labB->adjustSize();
    winPicker->add(labB, 5, 63);

    sliR = new SHR::Slider(0, 255);
    sliR->setDimension(gcn::Rectangle(0, 0, 158, 11));
    sliR->addActionListener(this);
    sliR->setStyle(SHR::Slider::BALL);
    winPicker->add(sliR, 25, 16);
    sliG = new SHR::Slider(0, 255);
    sliG->setDimension(gcn::Rectangle(0, 0, 158, 11));
    sliG->addActionListener(this);
    sliG->setStyle(SHR::Slider::BALL);
    winPicker->add(sliG, 25, 41);
    sliB = new SHR::Slider(0, 255);
    sliB->setDimension(gcn::Rectangle(0, 0, 158, 11));
    sliB->addActionListener(this);
    sliB->setStyle(SHR::Slider::BALL);
    winPicker->add(sliB, 25, 66);

    tfR = new SHR::TextField("");
    tfR->setWidth(50);
    tfR->adjustHeight();
    tfR->SetNumerical(1);
    tfR->addActionListener(this);
    winPicker->add(tfR, 190, 11);
    tfG = new SHR::TextField("");
    tfG->setWidth(50);
    tfG->adjustHeight();
    tfG->SetNumerical(1);
    tfG->addActionListener(this);
    winPicker->add(tfG, 190, 36);
    tfB = new SHR::TextField("");
    tfB->setWidth(50);
    tfB->adjustHeight();
    tfB->SetNumerical(1);
    tfB->addActionListener(this);
    winPicker->add(tfB, 190, 61);

    int offy = bUseAlpha ? 25 : 0;

    if( bUseAlpha ){
     labA = new SHR::Lab("A:");
     labA->adjustSize();
     winPicker->add(labA, 5, 88);
     sliA = new SHR::Slider(0, 255);
     sliA->setDimension(gcn::Rectangle(0, 0, 158, 11));
     sliA->addActionListener(this);
     winPicker->add(sliA, 25, 91);

     tfA = new SHR::TextField("");
     tfA->setWidth(50);
     tfA->adjustHeight();
     tfA->SetNumerical(1);
     tfA->addActionListener(this);
     winPicker->add(tfA, 190, 86);
    }else{
     labA = NULL;
    }

    labHex = new SHR::Lab("#");
    labHex->adjustSize();
    winPicker->add(labHex, 5, 88+offy);

    tfHex = new SHR::TextField("");
    tfHex->setWidth(75);
    tfHex->adjustHeight();
    tfHex->addActionListener(this);
    winPicker->add(tfHex, 25, 86+offy);

    vp = new WIDG::Viewport(this, 0);
    winPicker->add(vp);

    dest->add(winPicker, x, y);

    SetColor(dwInitColor);
}

cColorPicker::~cColorPicker()
{
    delete labR; delete labG; delete labB;
    if( bUseAlpha ){
     delete labA;
     delete sliA;
     delete tfA;
    }
    delete tfR; delete tfG; delete tfB;
    delete sliR; delete sliG; delete sliB;
    delete tfHex;
    delete labHex;
    delete winPicker;
}

void cColorPicker::SetColor(DWORD n)
{
    int r = GETR(n), g = GETG(n), b = GETB(n), a = GETA(n);
    char tmp[12];
    if( bUseAlpha ){
     sprintf(tmp, "%d", a);
     tfA->setText(tmp);
     sliA->setValue(a);
    }
    sliR->setValue(r);
    sprintf(tmp, "%d", r);
    tfR->setText(tmp);
    sliG->setValue(g);
    sprintf(tmp, "%d", g);
    tfG->setText(tmp);
    sliB->setValue(b);
    sprintf(tmp, "%d", b);
    tfB->setText(tmp);
    dwColor = n;

    if( !bUseAlpha ){
     n = SETA(n, 0);
     sprintf(tmp, "%06X", n);
    }else
     sprintf(tmp, "%08X", n);

    tfHex->setText(tmp);
}


void cColorPicker::Draw(int iCode)
{
    int dx, dy;
    winPicker->getAbsolutePosition(dx, dy);
    hgeQuad q;
    if( bUseAlpha )
     SHR::SetQuad(&q, dwColor, dx+105, dy+127, dx+242, dy+150);
    else
     SHR::SetQuad(&q, dwColor, dx+105, dy+102, dx+242, dy+125);
    q.blend = BLEND_DEFAULT;
    q.tex = 0;
    hge->Gfx_RenderQuad(&q);
}

void cColorPicker::action(const gcn::ActionEvent &actionEvent)
{
    if( actionEvent.getSource() == sliR ){
     SetColor(SETR(dwColor, int(sliR->getValue())));
    }else if( actionEvent.getSource() == tfR ){
     SetColor(SETR(dwColor, atoi(tfR->getText().c_str())));
    }else if( actionEvent.getSource() == sliG ){
     SetColor(SETG(dwColor, int(sliG->getValue())));
    }else if( actionEvent.getSource() == tfG ){
     SetColor(SETG(dwColor, atoi(tfG->getText().c_str())));
    }else if( actionEvent.getSource() == sliB ){
     SetColor(SETB(dwColor, int(sliB->getValue())));
    }else if( actionEvent.getSource() == tfB ){
     SetColor(SETB(dwColor, atoi(tfB->getText().c_str())));
    }else if( bUseAlpha && actionEvent.getSource() == sliA ){
     SetColor(SETA(dwColor, int(sliA->getValue())));
    }else if( bUseAlpha && actionEvent.getSource() == tfA ){
     SetColor(SETA(dwColor, atoi(tfA->getText().c_str())));
    }else if( actionEvent.getSource() == tfHex ){
     int len = strlen(tfHex->getText().c_str());
     if( bUseAlpha && len != 8 || !bUseAlpha && len != 6 ) return;
     DWORD n = 0;
     sscanf(tfHex->getText().c_str(), "%X", &n);
     if( !bUseAlpha )
      n = SETA(n, 255);
     SetColor(n);
    }
}
