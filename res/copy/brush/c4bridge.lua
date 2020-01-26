--Footbridge brush by kijanek6 23.04.2011
gameID = wmCLAW
layerName = "ACTION"
levelID = 4

brushName_PL = "Kladka"
brushName = "Bridge"
brushIcon = 159

function placeLeftEnd(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 160 then
  wmPlaceTile(plPlane, iX, iY, 159)
 elseif wmGetTile(plPlane, iX, iY) ~= 159 then
  wmPlaceTile(plPlane, iX, iY, 155)
 end
end

function placeRightEnd(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 155 then
  wmPlaceTile(plPlane, iX, iY, 159)
 elseif wmGetTile(plPlane, iX, iY) ~= 159 then
  wmPlaceTile(plPlane, iX, iY, 160)
 end
end

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 159 and wmGetTile(plPlane, iX+1, iY) == 159 then
  return
 end
 if wmGetTile(plPlane, iX, iY) == 160 then
  wmPlaceTile(plPlane, iX, iY, 159)
  placeRightEnd(plPlane, iX+1, iY)
 elseif wmGetTile(plPlane, iX+1, iY) == 155 then
  wmPlaceTile(plPlane, iX+1, iY, 159)
  placeLeftEnd(plPlane, iX, iY)
 else
  placeLeftEnd(plPlane, iX, iY)
  placeRightEnd(plPlane, iX+1, iY)
 end
end
