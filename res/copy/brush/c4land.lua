gameID=wmCLAW
layerName="ACTION"
levelID=4
brushName_PL="Ziemia"
brushName="Ground"
brushIcon=104

function cbBrushPlaced(plPlane, iX, iY)  
if wmGetTile(plPlane, iX, iY)==600 and wmGetTile(plPlane, iX+1, iY)==101 then
wmPlaceTile(plPlane, iX, iY, 104)
wmPlaceTile(plPlane, iX+1, iY, 104)
wmPlaceTile(plPlane, iX, iY+1, 141)
wmPlaceTile(plPlane, iX+1, iY+1, 141)
elseif wmGetTile(plPlane, iX, iY)==101 and wmGetTile(plPlane, iX-1, iY)==600 then
wmPlaceTile(plPlane, iX, iY, 104)
wmPlaceTile(plPlane, iX-1, iY, 104)
wmPlaceTile(plPlane, iX, iY+1, 141)
wmPlaceTile(plPlane, iX-1, iY+1, 141)
elseif wmGetTile(plPlane, iX, iY)==138 then
wmPlaceTile(plPlane, iX, iY, 600)
wmPlaceTile(plPlane, iX-1, iY, 104)
elseif wmGetTile(plPlane, iX,iY)==133 then
wmPlaceTile(plPlane, iX, iY, 101)
wmPlaceTile(plPlane, iX+1, iY, 104)
elseif wmGetTile(plPlane, iX, iY)==600 then
wmPlaceTile(plPlane, iX, iY, 104)
wmPlaceTile(plPlane, iX, iY+1, 141)
wmPlaceTile(plPlane, iX+1, iY, 600)
wmPlaceTile(plPlane, iX+1, iY+1, 144)
elseif wmGetTile(plPlane, iX, iY)==101 then
wmPlaceTile(plPlane, iX-1, iY, 101)
wmPlaceTile(plPlane, iX-1, iY+1, 139)
wmPlaceTile(plPlane, iX, iY, 134)
wmPlaceTile(plPlane, iX, iY+1, 141)
elseif wmGetTile(plPlane, iX, iY)~=104 and wmGetTile(plPlane, iX, iY)~=134 and wmGetTile(plPlane, iX, iY)~=137 and wmGetTile(plPlane, iX, iY)~=601 then
wmPlaceTile(plPlane, iX-1, iY, 101)
wmPlaceTile(plPlane, iX-1, iY+1, 139)
wmPlaceTile(plPlane, iX, iY, 104)
wmPlaceTile(plPlane, iX, iY+1, 141)
wmPlaceTile(plPlane, iX+1, iY, 600)
wmPlaceTile(plPlane, iX+1, iY+1, 144)
end
end