--brush by Zax37 20.04.2011
gameID = wmCLAW
layerName = "ACTION"
levelID = 3

brushName_PL = "Drzewo"
brushName = "Tree"
brushIcon = 653

function checkGroup(plPlane, Xx, Yy)
 if wmGetTile(plPlane, Xx, Yy)==644 or wmGetTile(plPlane, Xx, Yy)==647 or wmGetTile(plPlane, Xx, Yy)==652 or wmGetTile(plPlane, Xx, Yy)==653 or wmGetTile(plPlane, Xx, Yy)==654 or wmGetTile(plPlane, Xx, Yy)==657 or wmGetTile(plPlane, Xx, Yy)==659 or wmGetTile(plPlane, Xx, Yy)==661 or wmGetTile(plPlane, Xx, Yy)==662 or wmGetTile(plPlane, Xx, Yy)==663 or wmGetTile(plPlane, Xx, Yy)==664 or wmGetTile(plPlane, Xx, Yy)==665 or wmGetTile(plPlane, Xx, Yy)==666 then return 1
 else return 0
 end
end

function cbBrushPlaced(plPlane, iX, iY)
if wmGetTile(plPlane, iX-1, iY)==657 and wmGetTile(plPlane, iX-1, iY-1)==657 then
 wmPlaceTile(plPlane, iX-1, iY, 661)
 wmPlaceTile(plPlane, iX-1, iY-1, 659)
 wmPlaceTile(plPlane, iX, iY, 662)
 wmPlaceTile(plPlane, iX, iY-1, 660)
 wmPlaceTile(plPlane, iX+1, iY, 656)
 wmPlaceTile(plPlane, iX+1, iY-1, 649)
elseif wmGetTile(plPlane, iX+1, iY)==657 and wmGetTile(plPlane, iX+1, iY-1)==657 then
 wmPlaceTile(plPlane, iX+1, iY, 665)
 wmPlaceTile(plPlane, iX+1, iY-1, 663)
 wmPlaceTile(plPlane, iX, iY, 664)
 wmPlaceTile(plPlane, iX, iY-1, 660)
 wmPlaceTile(plPlane, iX-1, iY, 650)
 wmPlaceTile(plPlane, iX-1, iY-1, 645)
else 
 if wmGetTile(plPlane, iX, iY)==644 then
  wmPlaceTile(plPlane, iX, iY, 647)
 elseif checkGroup(plPlane, iX, iY)==0 then
  wmPlaceTile(plPlane, iX, iY, 666)
 end
  wmPlaceTile(plPlane, iX, iY-1, 657)
  wmPlaceTile(plPlane, iX-2, iY-1, 0)
  wmPlaceTile(plPlane, iX-1, iY-1, 0)
  wmPlaceTile(plPlane, iX+1, iY-1, 0)
  wmPlaceTile(plPlane, iX+2, iY-1, 0)
  wmPlaceTile(plPlane, iX-2, iY-2, 650)
  wmPlaceTile(plPlane, iX-1, iY-2, 652)
  wmPlaceTile(plPlane, iX, iY-2, 653)
  wmPlaceTile(plPlane, iX+1, iY-2, 654)
  wmPlaceTile(plPlane, iX+2, iY-2, 656)
  wmPlaceTile(plPlane, iX-2, iY-3, 645)
  wmPlaceTile(plPlane, iX-1, iY-3, 646)
 if checkGroup(plPlane, iX, iY-3)==0 then
  wmPlaceTile(plPlane, iX, iY-3, 644)
 else wmPlaceTile(plPlane, iX, iY-3, 647)
 end
  wmPlaceTile(plPlane, iX+1, iY-3, 648)
  wmPlaceTile(plPlane, iX+2, iY-3, 649)
end
end
