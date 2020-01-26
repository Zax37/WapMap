--brush by Zax37 20.04.2011
gameID = wmCLAW
layerName = "ACTION"
levelID = 3

brushName_PL = "Kolce2"
brushName = "Spikes2"
brushIcon = 698

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY)==638 or wmGetTile(plPlane, iX, iY)==626 then
  wmPlaceTile(plPlane, iX, iY, 713)
  wmPlaceTile(plPlane, iX, iY-1, 712)
 elseif (wmGetTile(plPlane, iX, iY)>=639 and wmGetTile(plPlane, iX, iY)<=641) or wmGetTile(plPlane, iX, iY)==622 then
  wmPlaceTile(plPlane, iX, iY, 704)
  wmPlaceTile(plPlane, iX, iY-1, 698)
 elseif wmGetTile(plPlane, iX, iY)==642 then
  wmPlaceTile(plPlane, iX, iY, 704)
  wmPlaceTile(plPlane, iX, iY-1, 701)
 elseif wmGetTile(plPlane, iX, iY)==643 then
  wmPlaceTile(plPlane, iX, iY, 709)
  wmPlaceTile(plPlane, iX, iY-1, 702)
 end 
end
