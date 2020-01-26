gameID = wmCLAW
layerName = "ACTION"
levelID = 10

brushName_PL = "Drabina"
brushName = "Ladder"
brushIcon = 154

function cbBrushPlaced(plPlane, iX, iY)
if wmGetTile(plPlane, iX, iY) == 18  or wmGetTile(plPlane, iX, iY) == 19 or wmGetTile(plPlane, iX, iY) == 158 or wmGetTile(plPlane, iX, iY) == 157 or wmGetTile(plPlane, iX, iY) == 156 then
 if wmGetTile(plPlane, iX, iY) == 158 then
  wmPlaceTile(plPlane, iX, iY, 157)
 end
 if wmGetTile(plPlane, iX, iY+1) == 156 then
  wmPlaceTile(plPlane, iX, iY+1, 157)
 end

 if wmGetTile(plPlane, iX, iY) ~= 157 then
  wmPlaceTile(plPlane, iX, iY, 156)
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 157 then
  wmPlaceTile(plPlane, iX, iY+1, 158)
 end
else
 if wmGetTile(plPlane, iX, iY) == 155 then
  wmPlaceTile(plPlane, iX, iY, 154)
 end
 if wmGetTile(plPlane, iX, iY+1) == 153 then
  wmPlaceTile(plPlane, iX, iY+1, 154)
 end

 if wmGetTile(plPlane, iX, iY) ~= 154 then
  wmPlaceTile(plPlane, iX, iY, 153)
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 154 then
  wmPlaceTile(plPlane, iX, iY+1, 155)
 end
end
end
