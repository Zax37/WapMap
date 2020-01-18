#include "../editing_ww.h"
#include "../../globals.h"
#include "../../conf.h"
#include "../../../shared/commonFunc.h"
#include "../../langID.h"
#include <cmath>
#include "../../databanks/tiles.h"
#include "../../cBrush.h"
extern HGE * hge;

bool State::EditingWW::TileThink(bool pbConsumed)
{
    float mx, my;
    hge->Input_GetMousePos(&mx, &my);
    if( iActiveTool == EWW_TOOL_WRITEID ){
     vTileGhosting.clear();

     if( conWriteID->isVisible() &&
        (hge->Input_KeyDown(HGEK_LBUTTON) || hge->Input_KeyDown(HGEK_RBUTTON)) &&
        conMain->getWidgetAt(mx, my) != conWriteID ){
      conWriteID->setShow(0);
     }else if( iTileWriteIDx != -1 && iTileWriteIDy != -1 ){
      TileGhost tg;
      tg.x = iTileWriteIDx;
      tg.y = iTileWriteIDy;
      tg.pl = GetActivePlane();
      tg.id = EWW_TILE_PIPETTE;
      vTileGhosting.push_back(tg);
      int posx = iTileWriteIDx*GetActivePlane()->GetTileWidth(),
          posy = iTileWriteIDy*GetActivePlane()->GetTileHeight();
      posx = Wrd2ScrX(GetActivePlane(), posx);
      posy = Wrd2ScrY(GetActivePlane(), posy);
      int tileh = GetActivePlane()->GetTileHeight()*fZoom;
      int tilew = (GetActivePlane()->GetTileWidth()*fZoom-48.0f)/2.0f;
      conWriteID->setPosition(posx+tilew, posy+tileh/2-10);
     }
    }

    if( iActiveTool == EWW_TOOL_NONE ){
       if( vPort->GetWidget()->isMouseOver() && !pbConsumed && hge->Input_KeyDown(HGEK_LBUTTON) && !bObjDragSelection ){
         bObjDragSelection = 1;
         iObjDragOrigX = Scr2WrdX(GetActivePlane(), mx);
         iObjDragOrigY = Scr2WrdY(GetActivePlane(), my);
         iTileSelectX1 = iTileSelectY1 = iTileSelectX2 = iTileSelectY2 = -1;
       }else if( hge->Input_GetKeyState(HGEK_LBUTTON) && vPort->GetWidget()->isMouseOver() && !pbConsumed ){
        /*int actx = Scr2WrdX(GetActivePlane(), mx),
            acty = Scr2WrdY(GetActivePlane(), my);
        int x = std::min(iObjDragOrigX, actx),
            y = std::min(iObjDragOrigY, acty),
            w = std::max(iObjDragOrigX, actx)-x,
            h = std::max(iObjDragOrigY, acty)-y;*/
        vPort->MarkToRedraw(1);
       }else if( bObjDragSelection ){
        bObjDragSelection = 0;
        int secx = Scr2WrdX(GetActivePlane(), mx);
        int secy = Scr2WrdY(GetActivePlane(), my);
        if( secx != iObjDragOrigX && secy != iObjDragOrigY ){
         iTileSelectX1 = std::min(iObjDragOrigX, secx)/GetActivePlane()->GetTileWidth();
         iTileSelectY1 = std::min(iObjDragOrigY, secy)/GetActivePlane()->GetTileHeight();
         iTileSelectX2 = std::max(iObjDragOrigX, secx)/GetActivePlane()->GetTileWidth();
         iTileSelectY2 = std::max(iObjDragOrigY, secy)/GetActivePlane()->GetTileHeight();
        }else{
         iTileSelectX1 = iTileSelectY1 = iTileSelectX2 = iTileSelectY2 = -1;
         vPort->MarkToRedraw(1);
        }
       }

       if( tilContext->isVisible() && (hge->Input_KeyDown(HGEK_RBUTTON) || hge->Input_KeyDown(HGEK_LBUTTON)) ){
        if( !(mx > tilContext->getX() &&
            mx < tilContext->getX()+tilContext->getWidth() &&
            my > tilContext->getY() &&
            my < tilContext->getY()+tilContext->getHeight()) ){
         tilContext->setVisible(0);
         return 0;
         //printf("close\n");
        }
       }

       if( hge->Input_KeyDown(HGEK_RBUTTON) || hge->Input_KeyUp(HGEK_RBUTTON) ){
        if( !tilContext->isVisible() ){
         if( bOpenObjContext && hge->Input_KeyUp(HGEK_RBUTTON) && fObjContextX == fCamX && fObjContextY == fCamY &&
             conMain->getWidgetAt(mx,my) == vPort->GetWidget() ){
          tilContext->setVisible(1);
          bOpenObjContext = 0;
         }else if( hge->Input_KeyDown(HGEK_RBUTTON) ){
          int tx, ty;
          tx = int(Scr2WrdX(GetActivePlane(), mx)/GetActivePlane()->GetTileWidth());
          ty = int(Scr2WrdY(GetActivePlane(), my)/GetActivePlane()->GetTileHeight());
          if( tx >= iTileSelectX1 && tx <= iTileSelectX2 && ty >= iTileSelectY1 && ty <= iTileSelectY2 ){
           tilContext->SetModel(conmodTilesSelected);
           bOpenObjContext = 1;
          }else if( MDI->GetActiveDoc()->hTileClipboard != NULL && !strcmp(MDI->GetActiveDoc()->hTileClipboardImageSet, GetActivePlane()->GetImageSet(0)) ){
           tilContext->SetModel(conmodTilesPaste);
           bOpenObjContext = 1;
          }
          if( bOpenObjContext ){
           fObjContextX = fCamX;
           fObjContextY = fCamY;
          }
         }
         //objContext->setVisible(1);
         tilContext->setPosition(mx, my);
         fObjContextX = fCamX;
         fObjContextY = fCamY;
         tilContext->adjustSize();
        }
       }
       if( tilContext->isVisible() ){
        if( (fObjContextX-fCamX) != 0 || (fObjContextY-fCamY) != 0 ){
         tilContext->setX(tilContext->getX()+(fObjContextX-fCamX));
         tilContext->setY(tilContext->getY()+(fObjContextY-fCamY));
         fObjContextX = fCamX;
         fObjContextY = fCamY;
        }
       }

     if( MDI->GetActiveDoc()->hTileClipboard != NULL && !strcmp(MDI->GetActiveDoc()->hTileClipboardImageSet, GetActivePlane()->GetImageSet(0)) ){
      if( hge->Input_GetKeyState(HGEK_TAB) ){
       int tx = Scr2WrdX(GetActivePlane(), mx)/GetActivePlane()->GetTileWidth(), ty = Scr2WrdY(GetActivePlane(), my)/GetActivePlane()->GetTileHeight();
       if( vTileGhosting.size() == 0 || vTileGhosting.size() != 0 && (tx != vTileGhosting[0].x || ty != vTileGhosting[0].y) ){
        vTileGhosting.clear();
        for(int y=0;y<MDI->GetActiveDoc()->iTileCBh;y++)
         for(int x=0;x<MDI->GetActiveDoc()->iTileCBw;x++){
          TileGhost tg;
          tg.x = tx+x;
          tg.y = ty+y;
          tg.pl = GetActivePlane();
          if( MDI->GetActiveDoc()->hTileClipboard[x][y].IsFilled() )
           tg.id = EWW_TILE_FILL;
          else if( MDI->GetActiveDoc()->hTileClipboard[x][y].IsInvisible() )
           tg.id = EWW_TILE_NONE;
          else
           tg.id = MDI->GetActiveDoc()->hTileClipboard[x][y].GetID();
          vTileGhosting.push_back(tg);
         }
        vPort->MarkToRedraw(1);
       }
      }else if( !vTileGhosting.empty() ){
       vTileGhosting.clear();
       vPort->MarkToRedraw(1);
      }
     }
    }

    if( iActiveTool == EWW_TOOL_PENCIL || iActiveTool == EWW_TOOL_FILL || iActiveTool == EWW_TOOL_BRUSH || iActiveTool == EWW_TOOL_WRITEID ){
     bool bVPfocused = (conMain->getWidgetAt(mx,my) == vPort->GetWidget() && !pbConsumed);
     int tgSize = vTileGhosting.size();
     if( wintpFillColor->isVisible() != (iTilePicked==EWW_TILE_FILL) )
      wintpFillColor->setVisible((iTilePicked==EWW_TILE_FILL));
     if( (iActiveTool == EWW_TOOL_BRUSH && !bVPfocused) )
      vTileGhosting.clear();
     if( bVPfocused ){
      int hx = Scr2WrdX(GetActivePlane(), mx)/GetActivePlane()->GetTileWidth();
      int hy = Scr2WrdY(GetActivePlane(), my)/GetActivePlane()->GetTileHeight();

      if( hx >= GetActivePlane()->GetPlaneWidth() )
       if( (GetActivePlane()->GetFlags() & WWD::Flag_p_XWrapping) == 0 )
        hx = -1;
       else
        hx = hx%GetActivePlane()->GetPlaneWidth();
      if( hy >= GetActivePlane()->GetPlaneHeight() )
       if( (GetActivePlane()->GetFlags() & WWD::Flag_p_YWrapping) == 0 )
        hy = -1;
       else
        hy = hy%GetActivePlane()->GetPlaneHeight();

      if( iActiveTool == EWW_TOOL_BRUSH && (hx != lastbrushx || hy != lastbrushy) )
       vTileGhosting.clear();

      //lastbrushx = hx;
      //lastbrushy = hy;
      iPipetteTileHL = EWW_TILE_NONE;
      if( hx != -1 && hy != -1 ){
       if(  (iActiveTool == EWW_TOOL_PENCIL && (/*iTilePicked >= 0 || iTilePicked == EWW_TILE_ERASE || iTilePicked == EWW_TILE_FILL ||*/ iTilePicked == EWW_TILE_PIPETTE))
          || iActiveTool == EWW_TOOL_WRITEID ){
         TileGhost tg;
         tg.x = hx;
         tg.y = hy;
         tg.pl = GetActivePlane();
         if( iActiveTool == EWW_TOOL_WRITEID )
          tg.id = EWW_TILE_PIPETTE;
         else
          tg.id = iTilePicked >= 0 ? hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTileByIterator(iTilePicked)->GetID()
                                   : iTilePicked;
         vTileGhosting.push_back(tg);
         if( iTilePicked == EWW_TILE_PIPETTE || iActiveTool == EWW_TOOL_WRITEID ){
          if( GetActivePlane()->GetTile(hx, hy)->IsFilled() )
           iPipetteTileHL = EWW_TILE_FILL;
          else if( GetActivePlane()->GetTile(hx, hy)->IsInvisible() )
           iPipetteTileHL = EWW_TILE_ERASE;
          else{
           cTileImageSet * set = hTileset->GetSet(GetActivePlane()->GetImageSet(0));
           for(int z=0;z<set->GetTilesCount();z++)
            if( set->GetTileByIterator(z)->GetID() == GetActivePlane()->GetTile(hx, hy)->GetID() ){
             iPipetteTileHL = z;
             break;
            }
          }
         }
       }else if( iActiveTool == EWW_TOOL_BRUSH ){
        if( (hx != lastbrushx || hy != lastbrushy) && iTilePicked >= 0 ){
         cBrush * brush = hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetBrushByIterator(iTilePicked);
         brush->ApplyGhosting(GetActivePlane(), hx, hy, &vTileGhosting);
        }
       }

       if( iActiveTool == EWW_TOOL_PENCIL && (iTilePicked >= 0 || iTilePicked == EWW_TILE_ERASE || iTilePicked == EWW_TILE_FILL) ){
        bool bPlacing = 0;
        if( iTileDrawMode == EWW_DRAW_RECT || iTileDrawMode == EWW_DRAW_LINE || iTileDrawMode == EWW_DRAW_ELLIPSE ){
         if( iTileDrawStartX == -1 && iTileDrawStartY == -1 && hge->Input_KeyDown(HGEK_LBUTTON) ){
          iTileDrawStartX = hx;
          iTileDrawStartY = hy;
         }
         bPlacing = !hge->Input_GetKeyState(HGEK_LBUTTON);
        }else{
         bPlacing = (hge->Input_KeyDown(HGEK_LBUTTON) || ((hx!=iLastBrushPlacedX||hy!=iLastBrushPlacedY)&&hge->Input_GetKeyState(HGEK_LBUTTON)));
        }

        if( hx != iLastBrushPlacedX || hy != iLastBrushPlacedY ){
         vTileGhosting.clear();

         if( iTileDrawMode == EWW_DRAW_POINT || iTileDrawMode == EWW_DRAW_SPRAY ){
          int pointsize = (iTileDrawMode == EWW_DRAW_POINT ? slitpiPointSize->getValue()
                                                           : slitpiSpraySize->getValue());
          if( pointsize == 1 ){
           TileGhost tg;
           tg.x = hx;
           tg.y = hy;
           tg.pl = GetActivePlane();
           tg.id = iTilePicked >= 0 ? hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTileByIterator(iTilePicked)->GetID()
                                   : iTilePicked;
           vTileGhosting.push_back(tg);
          }else{
           pointsize--;
           for(int y=hy-pointsize;y<=hy+pointsize;y++)
            for(int x=hx-pointsize;x<=hx+pointsize;x++){
             float dist = sqrt(pow(x-hx, 2)+pow(y-hy, 2));
             //printf("testing %d,%d [%d,%d] %f/%d\n", )
             if( dist < (pointsize+1) ){
              TileGhost tg;
              tg.x = x;
              tg.y = y;
              tg.pl = GetActivePlane();
              tg.id = iTilePicked >= 0 ? hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTileByIterator(iTilePicked)->GetID()
                                       : iTilePicked;
              vTileGhosting.push_back(tg);
             }
            }
           if( iTileDrawMode == EWW_DRAW_SPRAY ){
            float density = slitpiSprayDensity->getValue()/100.0f;
            int clearnum = float(vTileGhosting.size())*(1.0f-density);
            for(int i=0;i<clearnum;i++){
             int idx = hge->Random_Int(0, vTileGhosting.size()-1);
             vTileGhosting.erase(vTileGhosting.begin()+idx);
            }
           }
          }

         }else if( iTileDrawStartX != -1 && iTileDrawStartY != -1 ){
          int x1 = (iTileDrawStartX > hx ? hx : iTileDrawStartX),
              y1 = (iTileDrawStartY > hy ? hy : iTileDrawStartY),
              x2 = (iTileDrawStartX > hx ? iTileDrawStartX : hx),
              y2 = (iTileDrawStartY > hy ? iTileDrawStartY : hy);

          if( iTileDrawMode == EWW_DRAW_LINE ){
           x1 = iTileDrawStartX; y1 = iTileDrawStartY;
           x2 = hx; y2 = hy;
           int dx = x2-x1;
           int dy = y2-y1;
           int incXH, incXL;
           if(dx >= 0) incXH = incXL = 1;
           else { dx = -dx; incXH = incXL = -1; }
           int incYH, incYL;
           if(dy >= 0) incYH = incYL = 1;
           else { dy = -dy; incYH = incYL = -1; }
           int longD, shortD;
           if(dx >= dy) { longD = dx;  shortD = dy;  incYL = 0; }
           else		 { longD = dy;  shortD = dx;  incXL = 0; }
	       int d = 2*shortD - longD;
	       int incDL = 2*shortD;
	       int incDH = 2*shortD - 2*longD;
	       for(int i=0; i<=longD; i++){
            TileGhost tg;
            tg.x = x1;
            tg.y = y1;
            tg.pl = GetActivePlane();
            tg.id = iTilePicked >= 0 ? hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTileByIterator(iTilePicked)->GetID()
                                     : iTilePicked;
            vTileGhosting.push_back(tg);
		    if(d >= 0)	{ x1 += incXH; y1 += incYH; d += incDH; }
		    else		{ x1 += incXL; y1 += incYL; d += incDL; }
	       }
          }else if( iTileDrawMode == EWW_DRAW_RECT ){
           for(int y=y1;y<=y2;y++)
            for(int x=x1;x<=x2;x++){
             if( cbtpiRectFilled->isSelected() || !cbtpiRectFilled->isSelected() && (x==x1||x==x2||y==y1||y==y2) ){
              TileGhost tg;
              tg.x = x;
              tg.y = y;
              tg.pl = GetActivePlane();
              tg.id = iTilePicked >= 0 ? hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTileByIterator(iTilePicked)->GetID()
                                       : iTilePicked;
              vTileGhosting.push_back(tg);
             }
            }
          }else if( iTileDrawMode == EWW_DRAW_ELLIPSE && abs(x1-x2) > 2 && abs(y1-y2) > 2 ){
           int radiusX = (x2-x1)/2,
               radiusY = (y2-y1)/2;
           int centerX = x1+radiusX,
               centerY = y1+radiusY;
           int x, y, changeX, changeY, e, twoASquare, twoBSquare, stoppingX, stoppingY;

           twoASquare = 2 * radiusX * radiusX;
           twoBSquare = 2 * radiusY * radiusY;
           x = radiusX;
           y = 0;
           changeX = radiusY * radiusY * (1 - 2 * radiusX);
           changeY = radiusX * radiusX;
           e = 0;
           stoppingX = twoBSquare * radiusX;
           stoppingY = 0;

           TileGhost tg;
           tg.pl = GetActivePlane();
           tg.id = iTilePicked >= 0 ? hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTileByIterator(iTilePicked)->GetID()
                                    : iTilePicked;
           while (stoppingX >= stoppingY){
            tg.x = centerX+x; tg.y = centerY+y;
            vTileGhosting.push_back(tg);
            tg.x = centerX-x; tg.y = centerY-y;
            vTileGhosting.push_back(tg);
            tg.x = centerX+x; tg.y = centerY-y;
            vTileGhosting.push_back(tg);
            tg.x = centerX-x; tg.y = centerY+y;
            vTileGhosting.push_back(tg);
            if( cbtpiEllipseFilled->isSelected() ){
             for(int z=centerX-x; z<centerX+x; z++){
              tg.x = z; tg.y = centerY-y;
              vTileGhosting.push_back(tg);
              tg.y = centerY+y;
              vTileGhosting.push_back(tg);
             }
            }

            y++;
            stoppingY += twoASquare;
            e += changeY;
            changeY += twoASquare;
            if ( (2*e + changeX) > 0){
             x--;
             stoppingX -= twoBSquare;
             e += changeX;
             changeX += twoBSquare;
            }
           }

           x = 0;
           y = radiusY;
           changeX = radiusY * radiusY;
           changeY = radiusX * radiusX * (1 - 2 * radiusY);
           e = 0;
           stoppingX = 0;
           stoppingY = twoASquare * radiusY;
           while (stoppingX <= stoppingY){
            tg.x = centerX+x; tg.y = centerY+y;
            vTileGhosting.push_back(tg);
            tg.x = centerX-x; tg.y = centerY-y;
            vTileGhosting.push_back(tg);
            tg.x = centerX+x; tg.y = centerY-y;
            vTileGhosting.push_back(tg);
            tg.x = centerX-x; tg.y = centerY+y;
            vTileGhosting.push_back(tg);
            if( cbtpiEllipseFilled->isSelected() ){
             for(int z=centerX-x; z<centerX+x; z++){
              tg.x = z; tg.y = centerY-y;
              vTileGhosting.push_back(tg);
              tg.y = centerY+y;
              vTileGhosting.push_back(tg);
             }
            }
            x++;
            stoppingX += twoBSquare;
            e += changeX;
            changeX += twoBSquare;
            if ( (2*e + changeY) > 0){
             y--;
             stoppingY -= twoASquare;
             e += changeY;
             changeY += twoASquare;
            }
           }
          }
         }
         iLastBrushPlacedX = hx;
         iLastBrushPlacedY = hy;
        }

        if( bPlacing ){
         bool chng = 0;
         for(size_t t=0;t<vTileGhosting.size();t++){
          WWD::Tile * tl = vTileGhosting.at(t).pl->GetTile(vTileGhosting.at(t).x, vTileGhosting.at(t).y);
          if( vTileGhosting.at(t).id == EWW_TILE_ERASE ){
           if( !tl->IsInvisible() ){
            tl->SetInvisible(1);
            chng = 1;
           }
          }else if( vTileGhosting.at(t).id == EWW_TILE_FILL ){
           if( !tl->IsFilled() ){
            tl->SetFilled(1);
            chng = 1;
           }
          }else{
           if( tl->GetID() != vTileGhosting.at(t).id ){
            tl->SetID(vTileGhosting.at(t).id);
            chng = 1;
           }
          }
         }
         if( chng ){
          MarkUnsaved();
          hPlaneData[GetActivePlaneID()]->bUpdateBuffer = 1;
         }
         vTileGhosting.clear();
         if( iTileDrawMode == EWW_DRAW_RECT || iTileDrawMode == EWW_DRAW_LINE || iTileDrawMode == EWW_DRAW_ELLIPSE ){
          iTileDrawStartX = iTileDrawStartY = -1;
         }
        }

       }

       if( iActiveTool == EWW_TOOL_WRITEID && (hge->Input_KeyDown(HGEK_LBUTTON) ||
                                               hge->Input_KeyDown(HGEK_ENTER) && !conWriteID->isVisible() ) ){
         iTileWriteIDx = hx;
         iTileWriteIDy = hy;
         int posx = hx*GetActivePlane()->GetTileWidth(),
             posy = hy*GetActivePlane()->GetTileHeight();
         posx = Wrd2ScrX(GetActivePlane(), posx);
         posy = Wrd2ScrY(GetActivePlane(), posy);
         int tileh = GetActivePlane()->GetTileHeight()*fZoom;
         int tilew = (GetActivePlane()->GetTileWidth()*fZoom-48.0f)/2.0f;
         char tmp[25];
         if( !GetActivePlane()->GetTile(hx, hy)->IsFilled() && !GetActivePlane()->GetTile(hx, hy)->IsInvisible() )
          sprintf(tmp, "%d", GetActivePlane()->GetTile(hx, hy)->GetID());
         else
          tmp[0] = '\0';
         tfWriteID->setText(std::string(tmp));
         conWriteID->setVisible(0);
         conWriteID->setShow(1);
         tfWriteID->requestFocus();
         tfWriteID->setCaretPosition(tfWriteID->getText().length());
         conWriteID->setPosition(posx+tilew, posy+tileh/2-10);
       }

       if( hge->Input_GetKeyState(HGEK_LBUTTON) ){
        if( iActiveTool == EWW_TOOL_PENCIL && iTilePicked == EWW_TILE_PIPETTE ||
                  iActiveTool == EWW_TOOL_FILL && iTilePicked == EWW_TILE_PIPETTE ){
         if( GetActivePlane()->GetTile(hx, hy)->IsFilled() )
          iTilePicked = EWW_TILE_FILL;
         else if( GetActivePlane()->GetTile(hx, hy)->IsInvisible() )
          iTilePicked = EWW_TILE_ERASE;
         else{
          cTileImageSet * set = hTileset->GetSet(GetActivePlane()->GetImageSet(0));
          for(int z=0;z<set->GetTilesCount();z++)
           if( set->GetTileByIterator(z)->GetID() == GetActivePlane()->GetTile(hx, hy)->GetID() ){
            iTilePicked = z;
            break;
           }
         }
         RebuildTilePicker();
        /*}else if( iActiveTool == EWW_TOOL_PENCIL ){
         WWD::Tile * t = GetActivePlane()->GetTile(hx, hy);
         if( iTilePicked >= 0 ){
          int toid = hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTileByIterator(iTilePicked)->m_iID;
          if( t->GetID() != toid ){
           t->SetID(hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTileByIterator(iTilePicked)->m_iID);
           MarkUnsaved();
           hPlaneData[GetActivePlaneID()]->bUpdateBuffer = 1;
          }
         }else if( iTilePicked == EWW_TILE_ERASE ){
          if( !t->IsInvisible() ){
           t->SetInvisible(1);
           MarkUnsaved();
           hPlaneData[GetActivePlaneID()]->bUpdateBuffer = 1;
          }
         }else if( iTilePicked == EWW_TILE_FILL ){
          if( !t->IsFilled() ){
           t->SetFilled(1);
           MarkUnsaved();
           hPlaneData[GetActivePlaneID()]->bUpdateBuffer = 1;
          }
         }*/
        }else if( iActiveTool == EWW_TOOL_FILL ){
         bFloodFillBuf = new bool*[GetActivePlane()->GetPlaneWidth()];
         for(int x=0;x<GetActivePlane()->GetPlaneWidth();x++){
          bFloodFillBuf[x] = new bool[GetActivePlane()->GetPlaneHeight()];
          for(int y=0;y<GetActivePlane()->GetPlaneHeight();y++)
           bFloodFillBuf[x][y] = 0;
         }

         int base = 0;
         if( GetActivePlane()->GetTile(hx, hy)->IsInvisible() )
          base = EWW_TILE_ERASE;
         else if( GetActivePlane()->GetTile(hx, hy)->IsFilled() )
          base = EWW_TILE_FILL;
         else
          base = GetActivePlane()->GetTile(hx, hy)->GetID();

         FloodFill(base, hx, hy, iTilePicked);
         hPlaneData[GetActivePlaneID()]->bUpdateBuffer = 1;

         for(int x=0;x<GetActivePlane()->GetPlaneWidth();x++){
          delete [] bFloodFillBuf[x];
          bFloodFillBuf[x] = 0;
         }
         delete [] bFloodFillBuf;
         vPort->MarkToRedraw(1);
        }else if( iActiveTool == EWW_TOOL_BRUSH  && iTilePicked != EWW_TILE_NONE ){
         if( iLastBrushPlacedX != hx || iLastBrushPlacedY != hy ){
          vTileGhosting.clear();
          cBrush * brush = hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetBrushByIterator(iTilePicked);
          if( brush->Apply(GetActivePlane(), hx, hy) ){
           MarkUnsaved();
           vPort->MarkToRedraw(1);
          }
          iLastBrushPlacedX = hx;
          iLastBrushPlacedY = hy;
         }
        }
       }

       if( iActiveTool == EWW_TOOL_BRUSH && hx != -1 && hy != -1 ){
        lastbrushx = hx;
        lastbrushy = hy;
       }
      }

      if( hx != iTilePreviewX || hy != iTilePreviewY ){
       vPort->MarkToRedraw(1);
       iTilePreviewX = hx;
       iTilePreviewY = hy;
      }
     }else{
      if( iActiveTool == EWW_TOOL_PENCIL && iTileDrawStartX != -1 && iTileDrawStartY != -1 ){
       iTileDrawStartX = iTileDrawStartY = -1;
       vTileGhosting.clear();
      }
     }
     if( tgSize != vTileGhosting.size() ){
      vPort->MarkToRedraw(1);
     }
    }
    return 0;
}

void State::EditingWW::FloodFill(int base, int x, int y, int tile)
{
    if( x < 0 || y < 0 ) return;
    if( GetActivePlane()->GetTile(x, y) == NULL ) return;
    x = GetActivePlane()->ClampX(x);
    y = GetActivePlane()->ClampY(y);
    if( bFloodFillBuf[x][y] ) return;

    if( GetActivePlane()->GetTile(x, y)->IsInvisible() && base != EWW_TILE_ERASE ||
        GetActivePlane()->GetTile(x, y)->IsFilled() && base != EWW_TILE_FILL ||
        GetActivePlane()->GetTile(x, y)->GetID() != -1 &&
        GetActivePlane()->GetTile(x, y)->GetID() != base )
     return;

    WWD::Tile * t = GetActivePlane()->GetTile(x, y);
    if( tile == EWW_TILE_ERASE && !t->IsInvisible() ){
     t->SetInvisible(1);
     MarkUnsaved();
    }else if( tile == EWW_TILE_FILL && !t->IsFilled() ){
     t->SetFilled(1);
     MarkUnsaved();
    }else if( tile >= 0 && t->GetID() != hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTileByIterator(tile)->GetID() ){
     t->SetID(hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetTileByIterator(tile)->GetID());
     MarkUnsaved();
    }
    bFloodFillBuf[x][y] = 1;

    FloodFill(base, x, y+1, tile);
    FloodFill(base, x, y-1, tile);
    FloodFill(base, x+1, y, tile);
    FloodFill(base, x-1, y, tile);
}

void State::EditingWW::RebuildTilePicker()
{
    if( !btpiFixedPos ){
     winTilePicker->setY(vPort->GetY());
     winTilePicker->setHeight(vPort->GetHeight());
     btpiFixedPos = 1;
     buttpiReloadBrush->setY(winTilePicker->getHeight()-50);
     labtpiReloadBrush->setY(winTilePicker->getHeight()-50+6);
     cbtpiShowTileID->setY(winTilePicker->getHeight()-60);
     cbtpiShowProperties->setY(winTilePicker->getHeight()-40);
    }
    buttpiModePencil->setHighlight(iActiveTool==EWW_TOOL_PENCIL||iActiveTool==EWW_TOOL_FILL);
    buttpiModeBrush->setHighlight(iActiveTool==EWW_TOOL_BRUSH);

    buttpiPoint->setHighlight(iActiveTool != EWW_TOOL_FILL   && iTilePicked != EWW_TILE_PIPETTE && iTileDrawMode==EWW_DRAW_POINT);
    buttpiLine->setHighlight(iActiveTool != EWW_TOOL_FILL    && iTilePicked != EWW_TILE_PIPETTE && iTileDrawMode==EWW_DRAW_LINE);
    buttpiRect->setHighlight(iActiveTool != EWW_TOOL_FILL    && iTilePicked != EWW_TILE_PIPETTE && iTileDrawMode==EWW_DRAW_RECT);
    buttpiEllipse->setHighlight(iActiveTool != EWW_TOOL_FILL && iTilePicked != EWW_TILE_PIPETTE && iTileDrawMode==EWW_DRAW_ELLIPSE);
    buttpiSpray->setHighlight(iActiveTool != EWW_TOOL_FILL && iTilePicked != EWW_TILE_PIPETTE && iTileDrawMode==EWW_DRAW_SPRAY);

    buttpiFlood->setVisible(iActiveTool!=EWW_TOOL_BRUSH);
    buttpiSpray->setVisible(iActiveTool!=EWW_TOOL_BRUSH);
    buttpiFlood->setHighlight(iTilePicked != EWW_TILE_PIPETTE && iActiveTool==EWW_TOOL_FILL);
    buttpiReloadBrush->setVisible(iActiveTool==EWW_TOOL_BRUSH);
    labtpiReloadBrush->setVisible(iActiveTool==EWW_TOOL_BRUSH);

    labtpiToolOptions->setVisible(iActiveTool==EWW_TOOL_PENCIL);
    labtpiPointSize->setVisible(iActiveTool==EWW_TOOL_PENCIL&&iTileDrawMode==EWW_DRAW_POINT);
    slitpiPointSize->setVisible(iActiveTool==EWW_TOOL_PENCIL&&iTileDrawMode==EWW_DRAW_POINT);
    labtpiLineThickness->setVisible(iActiveTool==EWW_TOOL_PENCIL&&iTileDrawMode==EWW_DRAW_LINE);
    slitpiLineThickness->setVisible(iActiveTool==EWW_TOOL_PENCIL&&iTileDrawMode==EWW_DRAW_LINE);
    cbtpiRectFilled->setVisible(iActiveTool==EWW_TOOL_PENCIL&&iTileDrawMode==EWW_DRAW_RECT);
    cbtpiEllipseFilled->setVisible(iActiveTool==EWW_TOOL_PENCIL&&iTileDrawMode==EWW_DRAW_ELLIPSE);
    labtpiSpraySize->setVisible(iActiveTool==EWW_TOOL_PENCIL&&iTileDrawMode==EWW_DRAW_SPRAY);
    labtpiSprayDensity->setVisible(iActiveTool==EWW_TOOL_PENCIL&&iTileDrawMode==EWW_DRAW_SPRAY);
    slitpiSpraySize->setVisible(iActiveTool==EWW_TOOL_PENCIL&&iTileDrawMode==EWW_DRAW_SPRAY);
    slitpiSprayDensity->setVisible(iActiveTool==EWW_TOOL_PENCIL&&iTileDrawMode==EWW_DRAW_SPRAY);

    if( iActiveTool == EWW_TOOL_BRUSH ){
     iTilePickerOffUp = 5;
     iTilePickerOffDown = 60;
     cBrush * br = hTileset->GetSet(GetActivePlane()->GetImageSet(0))->GetBrushByIterator(iTilePicked);
     if( iTilePicked >= 0 && br->HasSettings() )
      iTilePickerOffDown += 10+br->GetSettingsHeight();
    }else{
     iTilePickerOffUp = 40;
     iTilePickerOffDown = 70;
    }
    iTilePickerOffUp += 15+40+32*2;

    cbtpiShowTileID->setVisible(iActiveTool!=EWW_TOOL_BRUSH);
    cbtpiShowProperties->setVisible(iActiveTool!=EWW_TOOL_BRUSH);
    slitpiPicker->setY(iTilePickerOffUp);
    slitpiPicker->setHeight(winTilePicker->getHeight()-iTilePickerOffUp-iTilePickerOffDown);
    RefreshTilePickerSlider();
}

void State::EditingWW::RefreshTilePickerSlider()
{
    cTileImageSet * set = hTileset->GetSet(GetActivePlane()->GetImageSet(0));
    int scale = 0;
    if( !set ) scale = 0;
    else{
     if( iActiveTool == EWW_TOOL_BRUSH )
      scale = set->GetBrushesCount()*57-slitpiPicker->getHeight();
     else
      scale = int((set->GetTilesCount()+2)/4+1)*57-slitpiPicker->getHeight()+10;
    }

    slitpiPicker->setEnabled(scale>0);
    if( scale > 0 )
     slitpiPicker->setScaleEnd(scale);

    if( !winTilePicker->isVisible() ){
     slitpiPicker->setValue(slitpiPicker->getScaleEnd());
    }
}

void State::EditingWW::HandleBrushSwitch(int itOld, int itNew)
{
    bool changed = 0;
    int oOff = 0, nOff = 0;
    cTileImageSet * set = hTileset->GetSet(GetActivePlane()->GetImageSet(0));
    if( itOld >= 0 && set->GetBrushByIterator(itOld)->HasSettings() ){
     oOff = set->GetBrushByIterator(itOld)->GetSettingsHeight()+10;
     set->GetBrushByIterator(itOld)->RemoveSettingsFromContainer(winTilePicker);
     changed = 1;
    }
    if( itNew >= 0 && set->GetBrushByIterator(itNew)->HasSettings() ){
     nOff = set->GetBrushByIterator(itNew)->GetSettingsHeight()+10;
     set->GetBrushByIterator(itNew)->AddSettingsToContainer(winTilePicker, 5,
                                                            winTilePicker->getHeight()-60-set->GetBrushByIterator(itNew)->GetSettingsHeight());
     changed = 1;
    }
    if( changed ){
     RebuildTilePicker();
     if( nOff != oOff )
      slitpiPicker->setValue(slitpiPicker->getValue()+(nOff-oOff));
    }
}
