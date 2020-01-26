gameID=wmCLAW
layerName="ACTION"
levelID=4
brushName_PL="Podium"
brushName="Podium"
brushIcon=150

function cbBrushPlaced(plPlane, iX, iY)  
if wmGetTile(plPlane, iX-2, iY-1)~=134 and wmGetTile(plPlane, iX-2, iY-1)~=104 and wmGetTile(plPlane, iX-2, iY-1)~=137 and wmGetTile(plPlane, iX-2, iY-1)~=601 then
wmPlaceTile(plPlane, iX-2, iY-1, 133)
end
if wmGetTile(plPlane, iX-2, iY)~=141 then
wmPlaceTile(plPlane, iX-2, iY, 107)
end
if wmGetTile(plPlane, iX-2, iY-1)~=134 and wmGetTile(plPlane, iX-2, iY-1)~=104 and wmGetTile(plPlane, iX-2, iY-1)~=137 and wmGetTile(plPlane, iX-2, iY-1)~=601 then
wmPlaceTile(plPlane, iX-1, iY-1, 601)
end
wmPlaceTile(plPlane, iX-1, iY, 150)
wmPlaceTile(plPlane, iX-1, iY+1, 153)
if wmGetTile(plPlane, iX+1, iY-1)~=134 and wmGetTile(plPlane, iX+1, iY-1)~=104 and wmGetTile(plPlane, iX+1,iY-1)~=137 and wmGetTile(plPlane, iX+1, iY-1)~=601 then
wmPlaceTile(plPlane, iX, iY-1, 137)
end
wmPlaceTile(plPlane, iX, iY, 151)
wmPlaceTile(plPlane, iX, iY+1, 154)
if wmGetTile(plPlane, iX+1, iY-1)~=134 and wmGetTile(plPlane, iX+1, iY-1)~=104 and wmGetTile(plPlane, iX+1,iY-1)~=137 and wmGetTile(plPlane, iX+1, iY-1)~=601 then
wmPlaceTile(plPlane, iX+1, iY-1, 138)
end
if wmGetTile(plPlane, iX, iY-1)==101 then
wmPlaceTile(plPlane, iX, iY-1, 104)
end
if wmGetTile(plPlane, iX-1, iY-1)==600 then
wmPlaceTile(plPlane, iX-1, iY-1, 104)
end
if wmGetTile(plPlane, iX+1, iY)~=141 then
wmPlaceTile(plPlane, iX+1, iY, 112)
end
end