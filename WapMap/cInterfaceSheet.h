#ifndef H_C_INTERFACESHEET
#define H_C_INTERFACESHEET

#include <hge.h>
#include <hgeSprite.h>

struct cInterfaceSheet
{
 hgeSprite * sprMainFrameLU, * sprMainFrameU, * sprMainFrameRU,
                                              * sprMainFrameR,
           * sprMainFrameLD, * sprMainFrameD, * sprMainFrameRD;
 hgeSprite * sprMainBackground;
 hgeSprite * sprMiniLogo, * sprAppBar[2]; //2 controls - minimalise and exit
 hgeSprite * sprBreadcrumb[4][5]; //4 states: disabled, normal, focused, selected; 5 parts - left, center, left V, right V, right
 hgeSprite * sprScrollbar[2][3][3]; //2 orientations: hor and ver, 2 elements: background, scroll and focused scroll; 3 parts - left, center, right / top, middle, bottom
 hgeSprite * sprSlider[2][4]; //2 types - circle and pointer; 4 states: normal, selected, focus, disabled
 hgeSprite * sprSliderBG[3]; //3 parts - left, center, right
 hgeSprite * sprControl[2][5]; //2 types: radio and tick; 5 elements - sign on, sign off, normal, focus, disabled
 hgeSprite * sprMainShadeBar;
 hgeSprite * sprTextField[2][8]; //2 states: normal and focus, 8 parts: left-top, top, top-right, left, right, left-bottom, bottom, right-bottom
 hgeSprite * sprWindow[4]; //four corners clockwise
 hgeSprite * sprDropDown[2][2][3]; //two edges: top and bottom; 2 states: normal and focus; 3 parts left to right
 hgeSprite * sprDropDownBar[3], //l to r
           * sprDropDownButtonFocused, //same as sprDropDownBar[2] but focused
           * sprDropDownBarArrow;
 hgeSprite * sprButton[4][9]; //4 states: disabled, normal, focused, selected; 9 parts
};

#endif // H_C_INTERFACESHEET
