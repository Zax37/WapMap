gameID = wmCLAW
layerName = "ACTION"
levelID = 14

brushName_PL = "Drabina"
brushName = "Ladder"
brushIcon = 58

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 59 then
  wmPlaceTile(plPlane, iX, iY, 58)
 end
 if wmGetTile(plPlane, iX, iY+1) == 65 then
  wmPlaceTile(plPlane, iX, iY+1, 58)
 end

 if wmGetTile(plPlane, iX, iY) ~= 58 then
  wmPlaceTile(plPlane, iX, iY, 65)
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 58 then
  wmPlaceTile(plPlane, iX, iY+1, 59)
 end
end
