#ifndef H_C_INTERFACESHEET
#define H_C_INTERFACESHEET

#include <hge.h>
#include <hgesprite.h>
#include "cAppMenu.h"

struct cInterfaceSheet {
    hgeSprite *sprMainBackground;
    hgeSprite *sprAppBar[LAY_APP_BUTTONS_COUNT + 1]; //3 controls - minimize, maximize, exit + restore variant of maximize
    hgeSprite *sprBreadcrumb[4][5]; //4 states: disabled, normal, focused, selected; 5 parts - left, center, left V, right V, right
    hgeSprite *sprScrollbar[2][3][3]; //2 orientations: hor and ver, 2 elements: background, scroll and focused scroll; 3 parts - left, center, right / top, middle, bottom
    hgeSprite *sprSlider[2][4]; //2 types - circle and pointer; 4 states: normal, selected, focus, disabled
    hgeSprite *sprSliderBG[3]; //3 parts - left, center, right
    hgeSprite *sprControl[2][6]; //2 types: radio and tick; 5 elements - sign on, sign off, normal, focus, disabled, focus border
    hgeSprite *sprTextField[2][9]; //2 states: normal and focus, 8 parts: left-top, top, top-right, left, right, left-bottom, bottom, right-bottom + divisor
    hgeSprite *sprDropDownBarArrow;
    hgeSprite *sprButton[4][9]; //4 states: disabled, normal, focused, selected; 9 parts
};

#endif // H_C_INTERFACESHEET
