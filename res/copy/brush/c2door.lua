
gameID = wmCLAW
layerName = "ACTION"
levelID = 2

brushName_PL = "Drzwi"
brushName = "Door"
brushIcon = 401

function cbBrushPlaced(plPlane, iX, iY)
 wmPlaceTile(plPlane, iX, iY, 401)
 wmPlaceTile(plPlane, iX, iY+1, 402)
end

