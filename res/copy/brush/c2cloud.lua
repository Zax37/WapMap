gameID=wmCLAW
layerName="ACTION"
levelID=2
brushName_PL="Chmurka"
brushName="Cloud"
brushIcon=322

function cbBrushPlaced(plPlane, iX, iY)  
 wmPlaceTile(plPlane, iX, iY, 322)
 wmPlaceTile(plPlane, iX+1, iY, 323)
end