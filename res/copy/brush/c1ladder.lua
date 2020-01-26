gameID = wmCLAW
layerName = "ACTION"
levelID = 1

brushName_PL = "Drabina"
brushName = "Ladder"
brushIcon = 311

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 312 then
  wmPlaceTile(plPlane, iX, iY, 311)
 end
 if wmGetTile(plPlane, iX, iY+1) == 310 then
  wmPlaceTile(plPlane, iX, iY+1, 311)
 end

 if wmGetTile(plPlane, iX, iY) ~= 311 then
  wmPlaceTile(plPlane, iX, iY, 310)
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 311 then
  wmPlaceTile(plPlane, iX, iY+1, 312)
 end
end
