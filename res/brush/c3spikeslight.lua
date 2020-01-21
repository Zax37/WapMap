--brush by Zax37 20.04.2011
gameID = wmCLAW
layerName = "ACTION"
levelID = 3

brushName_PL = "Kolce1"
brushName = "Spikes1"
brushIcon = 686

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY)==638 or wmGetTile(plPlane, iX, iY)==626 then
  wmPlaceTile(plPlane, iX, iY, 691)
  wmPlaceTile(plPlane, iX, iY-1, 685)
 elseif (wmGetTile(plPlane, iX, iY)>=639 and wmGetTile(plPlane, iX, iY)<=641) or wmGetTile(plPlane, iX, iY)==622 then
  wmPlaceTile(plPlane, iX, iY, 692)
  wmPlaceTile(plPlane, iX, iY-1, 686)
 elseif wmGetTile(plPlane, iX, iY)==642 then
  wmPlaceTile(plPlane, iX, iY, 692)
  wmPlaceTile(plPlane, iX, iY-1, 689)
 elseif wmGetTile(plPlane, iX, iY)==643 then
  wmPlaceTile(plPlane, iX, iY, 696)
  wmPlaceTile(plPlane, iX, iY-1, 690)
 end 
end
