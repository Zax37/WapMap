gameID = wmCLAW
layerName = "ACTION"
levelID = 7

brushName_PL = "Drabina"
brushName = "Ladder"
brushIcon = 114

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 43 then
  wmPlaceTile(plPlane, iX, iY, 110)
 elseif wmGetTile(plPlane, iX, iY) == 49 then
  wmPlaceTile(plPlane, iX, iY, 112)
  wmPlaceTile(plPlane, iX, iY+1, 115)
 elseif wmGetTile(plPlane, iX, iY) == 309 or wmGetTile(plPlane, iX, iY) == 119 or wmGetTile(plPlane, iX, iY) == 120 or wmGetTile(plPlane, iX, iY) == 121 then
 if wmGetTile(plPlane, iX, iY) == 121 then
  wmPlaceTile(plPlane, iX, iY, 120)
 end
 if wmGetTile(plPlane, iX, iY+1) == 119 then
  wmPlaceTile(plPlane, iX, iY+1, 120)
 end

 if wmGetTile(plPlane, iX, iY) ~= 120 then
  wmPlaceTile(plPlane, iX, iY, 119)
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 120 then
  wmPlaceTile(plPlane, iX, iY+1, 121)
 end
 else
 if wmGetTile(plPlane, iX, iY) == 115 then
  wmPlaceTile(plPlane, iX, iY, 114)
 end
 if wmGetTile(plPlane, iX, iY+1) == 113 then
  wmPlaceTile(plPlane, iX, iY+1, 114)
 end

 if wmGetTile(plPlane, iX, iY) ~= 114 then
  wmPlaceTile(plPlane, iX, iY, 113)
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 114 then
  wmPlaceTile(plPlane, iX, iY+1, 115)
 end
end
end
