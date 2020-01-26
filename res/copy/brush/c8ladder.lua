gameID = wmCLAW
layerName = "ACTION"
levelID = 8

brushName_PL = "Drabina"
brushName = "Ladder"
brushIcon = 83

function cbBrushPlaced(plPlane, iX, iY)
if wmGetTile(plPlane, iX, iY) == 157 then
 wmPlaceTile(plPlane, iX, iY, 536)
 if wmGetTile(plPlane, iX-1, iY) ~= 536 and wmGetTile(plPlane, iX-1, iY+1) ~= 544 then
  wmPlaceTile(plPlane, iX-1, iY+1, 542)
 end
 if wmGetTile(plPlane, iX+1, iY) ~= 536 and wmGetTile(plPlane, iX+1, iY+1) ~= 542 then
  wmPlaceTile(plPlane, iX+1, iY+1, 544)
 end
elseif wmGetTile(plPlane, iX, iY) == 121 or wmGetTile(plPlane, iX, iY) == 189 or wmGetTile(plPlane, iX, iY) == 560 or wmGetTile(plPlane, iX, iY) == 544 or wmGetTile(plPlane, iX, iY) == 542 or wmGetTile(plPlane, iX, iY) == 179 or wmGetTile(plPlane, iX, iY) == 175 or wmGetTile(plPlane, iX, iY) == 543 then
 if wmGetTile(plPlane, iX, iY) == 179 then
  wmPlaceTile(plPlane, iX, iY, 175)
 end
 if wmGetTile(plPlane, iX, iY+1) == 543 then
  wmPlaceTile(plPlane, iX, iY+1, 175)
 end

 if wmGetTile(plPlane, iX, iY) ~= 175 then
  wmPlaceTile(plPlane, iX, iY, 543)
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 175 then
  wmPlaceTile(plPlane, iX, iY+1, 179)
 end
elseif wmGetTile(plPlane, iX, iY) ~= 157 then
 if wmGetTile(plPlane, iX, iY) == 85 then
  wmPlaceTile(plPlane, iX, iY, 83)
 end
 if wmGetTile(plPlane, iX, iY+1) == 589 then
  wmPlaceTile(plPlane, iX, iY+1, 83)
 end

 if wmGetTile(plPlane, iX, iY) ~= 83 then
  wmPlaceTile(plPlane, iX, iY, 589)
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 83 then
  wmPlaceTile(plPlane, iX, iY+1, 85)
 end
end
end
