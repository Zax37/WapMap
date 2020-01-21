--Window brush by kijanek6/KijediStudio 30.09.2010

gameID = wmCLAW
layerName = "ACTION"
levelID = 2

brushName_PL = "Okno"
brushName = "Window"
brushIcon = 501

function cbBrushPlaced(plPlane, iX, iY)
 wmPlaceTile(plPlane, iX, iY, 501)
 wmPlaceTile(plPlane, iX+1, iY, 502)
 wmPlaceTile(plPlane, iX, iY+1, 503)
 wmPlaceTile(plPlane, iX+1, iY+1, 504)
end

