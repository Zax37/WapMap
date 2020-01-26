gameID = wmCLAW
layerName = "ACTION"
levelID = 13
brushName_PL = "K³ody"
brushName = "Logs"
brushIcon = 231

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX-1, iY)==234 and wmGetTile(plPlane, iX, iY)==229 then
  wmPlaceTile(plPlane, iX-1, iY,231)
  wmPlaceTile(plPlane, iX, iY,231)
 elseif wmGetTile(plPlane, iX, iY)==234 and wmGetTile(plPlane, iX+1, iY)==229 then
  wmPlaceTile(plPlane, iX, iY,231)
  wmPlaceTile(plPlane, iX+1, iY,231)
 elseif wmGetTile(plPlane, iX, iY)==229 then
  wmPlaceTile(plPlane, iX-1, iY,229)
  wmPlaceTile(plPlane, iX, iY,231)
 elseif wmGetTile(plPlane, iX, iY)==234 then
  wmPlaceTile(plPlane, iX+1, iY,234)
  wmPlaceTile(plPlane, iX, iY,231)
 elseif wmGetTile(plPlane, iX-1, iY)==234 then
  wmPlaceTile(plPlane, iX, iY,234)
  wmPlaceTile(plPlane, iX-1, iY,231)
 elseif wmGetTile(plPlane, iX, iY)~=231 then 
  wmPlaceTile(plPlane, iX-1, iY,229)
  wmPlaceTile(plPlane, iX, iY,234) 
end
end