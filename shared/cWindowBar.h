#ifndef H_SHARED_WINBAR
#define H_SHARED_WINBAR

namespace SHR
{
 class cWinBar {
  private:
   hgeSprite * sprTitle;
  public:
   cWinBar();
   ~cWinBar();
   void Think();
   void Render();
 }
}
#endif
