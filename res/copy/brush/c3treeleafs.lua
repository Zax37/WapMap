--brush by Zax37 20.04.2011
gameID = wmCLAW
layerName = "ACTION"
levelID = 3

brushName_PL = "Liœcie"
brushName = "Leafs"
brushIcon = 648

function cbBrushPlaced(plPlane, iX, iY)
if wmGetTile(plPlane, iX, iY)==649 then
 wmPlaceTile(plPlane, iX, iY, 648)
 wmPlaceTile(plPlane, iX+1, iY, 649)
 wmPlaceTile(plPlane, iX, iY+1, 655)
 wmPlaceTile(plPlane, iX+1, iY+1, 656)
elseif wmGetTile(plPlane, iX, iY)==645 then
 wmPlaceTile(plPlane, iX, iY, 648)
 wmPlaceTile(plPlane, iX-1, iY, 645)
 wmPlaceTile(plPlane, iX, iY+1, 651)
 wmPlaceTile(plPlane, iX-1, iY+1, 650)
else
 wmPlaceTile(plPlane, iX, iY, 648)
 wmPlaceTile(plPlane, iX+1, iY, 649)
 wmPlaceTile(plPlane, iX-1, iY, 645)
 wmPlaceTile(plPlane, iX, iY+1, 655)
 wmPlaceTile(plPlane, iX+1, iY+1, 656)
 wmPlaceTile(plPlane, iX-1, iY+1, 650)
end
end
