#include "cPieChart.h"

#include <algorithm>
#include <SFML/Graphics/Image.hpp>

extern HGE * hge;

namespace SHR
{
    bool cPieChartComparison(cPieChartEl one, cPieChartEl two)
    {
        return (two.iPart < one.iPart);
    }

    cPieChart::cPieChart(int depth, bool darksh)
    {
        hSpr = NULL;
        hTex = NULL;
        iDepthHeight = depth;
        bDarkShadow = darksh;
        dwBg = 0x00000000;
    }

    cPieChart::~cPieChart()
    {
        Cleanup();
        for(int i=0;i<vElements.size();i++)
         delete [] vElements[i].szDesc;
    }

    void cPieChart::Cleanup()
    {
        if( hSpr != NULL ){
         delete hSpr;
         hge->Texture_Free(hTex);
         hSpr = NULL;
         hTex = NULL;
        }
    }

    void cPieChart::Render(int iW, int iH)
    {
        Cleanup();
        std::sort(vElements.begin(), vElements.end(), cPieChartComparison);
        if( iH  == -1 ) iH = iW/2;
        sf::Image hImgGD = sf::Image();
        hImgGD.create(iW, iH, sf::Color::Transparent);
        /*int gdcolTransparent = gdImageColorAllocateAlpha(hImgGD, GETR(dwBg), GETG(dwBg), GETB(dwBg), std::min(127, int(GETA((255-dwBg)/2))));
        gdImageAlphaBlending(hImgGD, 0);
        gdImageFilledRectangle(hImgGD, 0, 0, iW, iH, gdcolTransparent);*/

        /*const DWORD defcolors[] = {0xFF003366, 0xFFCCD6E0, 0xFF7F99B2, 0xFFF7EFC6,
                                   0xFFC6BE8C, 0xFFCC6600, 0xFF990000, 0xFF520000,
                                   0xFFBFBFC1, 0xFF808080};*/
        const DWORD defcolors[] = {0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00,
                                   0xFFFF00FF, 0xFF00FFFF, 0xFFFF7700, 0xFFFF0077,
                                   0xFF77FF00, 0xFF7700FF};
        //3d height = 16 (shadow_height)
        //shadow_dark - bool

        int iOverallCount = 0;
        for(size_t i=0;i<vElements.size();i++)
         iOverallCount += vElements[i].iPart;
        //int iColors[vElements.size()], iShadowColors[vElements.size()];
        for(size_t i=0;i<vElements.size();i++){
         vElements[i].fPercentage = float(vElements[i].iPart)*100.0f/float(iOverallCount);
         if( vElements[i].iColor == 0x00000000 )
          vElements[i].iColor = defcolors[(i%10)];
         DWORD base = vElements[i].iColor;
         int r = GETR(base), g = GETG(base), b = GETB(base);
         //iColors[i] = gdImageColorAllocate(hImgGD, r, g, b);
         if( bDarkShadow ){
		  (r > 99) ? r -= 100 : r = 0;
		  (g > 99) ? g -= 100 : g = 0;
		  (b > 99) ? b -= 100 : b = 0;
         }else{
		  (r < 220) ? r += 35 : r = 255;
		  (g < 220) ? g += 35 : g = 255;
		  (b < 220) ? b += 35 : b = 255;
         }
         //iShadowColors[i] = gdImageColorAllocate(hImgGD, r, g, b);
        }
        int iDiagWidth = iW-2, iDiagHeight = iH - iDepthHeight-2;
        int iCenterX = iW/2, iCenterY = iDiagHeight/2;

        for(int i=0;i<iDepthHeight;i++){
         int y = iCenterY+iDepthHeight-i;
         int anglestart = 270;
         for(size_t el=0;el<vElements.size();el++){
          //gdImageSetAntiAliased(hImgGD, iShadowColors[el]);

          int myangle = int(360.0f * (vElements[el].fPercentage/100.0f))%360;
          int endangle = int(anglestart+myangle)%360;

          //gdImageFilledArc(hImgGD, iCenterX, y, iDiagWidth, iDiagHeight, anglestart, endangle, gdAntiAliased, gdPie);
          //anglestart = (anglestart+myangle)%360;
         }
        }

        int anglestart = 270;
        for(size_t i=0;i<vElements.size();i++){
         //gdImageSetAntiAliased(hImgGD, iColors[i]);

         int myangle = int(360.0f * (vElements[i].fPercentage/100.0f))%360;
         int endangle = int(anglestart+myangle)%360;

         //gdImageFilledArc(hImgGD, iCenterX, iCenterY, iDiagWidth, iDiagHeight, anglestart, endangle, gdAntiAliased, gdPie);

         anglestart = (anglestart+myangle)%360;
        }

        hTex = hge->Texture_Create(iW, iH);
        DWORD * dest = hge->Texture_Lock(hTex, 0);
        /*for(int y=0;y<iH;y++)
         for(int x=0;x<iW;x++){
          int gdcol = gdImageGetPixel(hImgGD, x, y);
           dest[y*iW+x] = ARGB(255-(gdImageAlpha(hImgGD, gdcol)*2),
                               gdImageRed(hImgGD, gdcol),
                               gdImageGreen(hImgGD, gdcol),
                               gdImageBlue(hImgGD, gdcol));
         }*/
        hge->Texture_Unlock(hTex);

        //gdImageDestroy(hImgGD);
        hSpr = new hgeSprite(hTex, 0, 0, iW, iH);
    }

    void cPieChart::AddPart(const char * szDesc, int iPart, DWORD iColor)
    {
        cPieChartEl n;
        n.szDesc = new char[strlen(szDesc)+1];
        strcpy(n.szDesc, szDesc);
        n.iPart = iPart;
        n.iColor = iColor;
        n.fPercentage = -1;
        vElements.push_back(n);
    }
}
