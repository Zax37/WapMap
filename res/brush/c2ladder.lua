
gameID = wmCLAW
layerName = "ACTION"
levelID = 2

brushName_PL = "Drabina"
brushName = "Ladder"
brushIcon = 27

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 22 then
  wmPlaceTile(plPlane, iX, iY, 27)
 end

 if wmGetTile(plPlane, iX, iY+1) == 16 then
  wmPlaceTile(plPlane, iX, iY+1, 27)
 end

 if wmGetTile(plPlane, iX, iY) ~= 27 then
  wmPlaceTile(plPlane, iX, iY, 16)
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 27 then
  wmPlaceTile(plPlane, iX, iY+1, 22)
 end
end
