gameID = wmCLAW
layerName = "ACTION"
levelID = 2

brushName_PL = "Blanki"
brushName = "Battlements"
brushIcon = 72

function cbBrushPlaced(plPlane, iX, iY)
wmPlaceTile(plPlane, iX, iY, 77)
wmPlaceTile(plPlane, iX, iY-1, 72)
wmPlaceTile(plPlane, iX, iY-2, 68)

if wmGetTile(plPlane, iX-1, iY) == 77 and
   wmGetTile(plPlane, iX+1, iY) == 77 then
  wmPlaceTile(plPlane, iX, iY, 77)
  wmPlaceTile(plPlane, iX, iY-1, 72)
  wmPlaceTile(plPlane, iX, iY-2, 68)
else
if wmGetTile(plPlane, iX-1, iY) == 77 and
   wmGetTile(plPlane, iX+2, iY) == 77 then
  wmPlaceTile(plPlane, iX, iY, 77)
  wmPlaceTile(plPlane, iX, iY-1, 72)
  wmPlaceTile(plPlane, iX, iY-2, 68)
else
if wmGetTile(plPlane, iX-2, iY) == 77 and
   wmGetTile(plPlane, iX+1, iY) == 77 then
  wmPlaceTile(plPlane, iX, iY, 77)
  wmPlaceTile(plPlane, iX, iY-1, 72)
  wmPlaceTile(plPlane, iX, iY-2, 68)
else
if wmGetTile(plPlane, iX-2, iY) == 77 and
   wmGetTile(plPlane, iX+2, iY) == 77 then
  wmPlaceTile(plPlane, iX, iY, 77)
  wmPlaceTile(plPlane, iX, iY-1, 72)
  wmPlaceTile(plPlane, iX, iY-2, 68)
else
if wmGetTile(plPlane, iX-3, iY) == 77 and
   wmGetTile(plPlane, iX+1, iY) == 77 then
  wmPlaceTile(plPlane, iX, iY, 77)
  wmPlaceTile(plPlane, iX, iY-1, 72)
  wmPlaceTile(plPlane, iX, iY-2, 68)
else
if wmGetTile(plPlane, iX-1, iY) == 77 and
   wmGetTile(plPlane, iX+3, iY) == 77 then
  wmPlaceTile(plPlane, iX, iY, 77)
  wmPlaceTile(plPlane, iX, iY-1, 72)
  wmPlaceTile(plPlane, iX, iY-2, 68)
else
  if wmGetTile(plPlane, iX-1, iY) ~= 77 then
    wmPlaceTile(plPlane, iX-1, iY, 76)
    wmPlaceTile(plPlane, iX-1, iY-1, 71)
    wmPlaceTile(plPlane, iX-1, iY-2, 68)
    wmPlaceTile(plPlane, iX-2, iY, 75)
    wmPlaceTile(plPlane, iX-2, iY-1, 70)
    wmPlaceTile(plPlane, iX-2, iY-2, 68)
  end
  if wmGetTile(plPlane, iX+1, iY) ~= 77 then
    wmPlaceTile(plPlane, iX+1, iY, 78)
    wmPlaceTile(plPlane, iX+1, iY-1, 73)
    wmPlaceTile(plPlane, iX+1, iY-2, 68)
    wmPlaceTile(plPlane, iX+2, iY, 79)
    wmPlaceTile(plPlane, iX+2, iY-1, 74)
    wmPlaceTile(plPlane, iX+2, iY-2, 68)
  end
end
end
end
end
end
end
end