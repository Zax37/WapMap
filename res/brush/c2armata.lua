gameID=wmCLAW
layerName="ACTION"
levelID=2
brushName_PL="Armata"
brushName="Cannon"
brushIcon=085

function cbBrushPlaced(plPlane, iX, iY) 
 if (wmGetTile(plPlane, iX, iY-1) == 082 and wmGetTile(plPlane, iX, iY) == 082 and wmGetTile(plPlane, iX, iY+1) == 082) or (wmGetTile(plPlane, iX, iY-1) == 097 and wmGetTile(plPlane, iX, iY) == 097 and wmGetTile(plPlane, iX, iY+1) == 097) then
  wmPlaceTile(plPlane, iX, iY-1, 086)
  wmPlaceTile(plPlane, iX, iY, 090)
  wmPlaceTile(plPlane, iX, iY+1, 094)
  wmPlaceTile(plPlane, iX+1, iY-1, 087)
  wmPlaceTile(plPlane, iX+1, iY, 091)
  wmPlaceTile(plPlane, iX+1, iY+1, 095)
 elseif (wmGetTile(plPlane, iX, iY-1) == 080 and wmGetTile(plPlane, iX, iY) == 080 and wmGetTile(plPlane, iX, iY+1) == 080) or (wmGetTile(plPlane, iX, iY-1) == 096 and wmGetTile(plPlane, iX, iY) == 096 and wmGetTile(plPlane, iX, iY+1) == 096) then
  wmPlaceTile(plPlane, iX-1, iY-1, 083)
  wmPlaceTile(plPlane, iX-1, iY, 088)
  wmPlaceTile(plPlane, iX-1, iY+1, 092)
  wmPlaceTile(plPlane, iX, iY-1, 084)
  wmPlaceTile(plPlane, iX, iY, 089)
  wmPlaceTile(plPlane, iX, iY+1, 093)
 elseif wmGetTile(plPlane, iX, iY) == 081 or wmGetTile(plPlane, iX, iY) == 105 or wmGetTile(plPlane, iX, iY) == 77 then
  wmPlaceTile(plPlane, iX, iY, 085)
 end
end