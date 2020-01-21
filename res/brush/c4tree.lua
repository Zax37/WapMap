gameID=wmCLAW
layerName="ACTION"
levelID=4
brushName_PL="Drzewo"
brushName="Tree"
brushIcon=184

function cbBrushPlaced(plPlane, iX, iY)  
if wmGetTile(plPlane, iX,iY)==104 or wmGetTile(plPlane, iX,iY)==134 or wmGetTile(plPlane, iX,iY)==137 or wmGetTile(plPlane, iX,iY)==601 then
wmPlaceTile(plPlane, iX, iY, 185)
elseif wmGetTile(plPlane, iX,iY)~=185 then wmPlaceTile(plPlane, iX, iY, 184)
end
end