gameID = wmCLAW
layerName = "ACTION"
levelID = 1


brushName_PL = "Platforma"
brushName = "Platform"
brushIcon = 332

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 334 then
  wmPlaceTile(plPlane, iX, iY, 332)
 end

 if wmGetTile(plPlane, iX+1, iY) == 331 then
  wmPlaceTile(plPlane, iX+1, iY, 332)
 end

 if wmGetTile(plPlane, iX, iY) ~= 332 then
  wmPlaceTile(plPlane, iX, iY, 331)
 end
 if wmGetTile(plPlane, iX+1, iY) ~= 332 then
  wmPlaceTile(plPlane, iX+1, iY, 334)
 end
end