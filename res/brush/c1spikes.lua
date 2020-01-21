gameID = wmCLAW
layerName = "ACTION"
levelID = 1


brushName_PL = "Kolce"
brushName = "Spikes"
brushIcon = 323

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 309 then
  if wmRandInt(0, 1) == 0 then
   wmPlaceTile(plPlane, iX, iY, 322)
  else
   wmPlaceTile(plPlane, iX, iY, 328)
  end
  if wmGetTile(plPlane, iX, iY-1) == 308 then
   wmPlaceTile(plPlane, iX, iY-1, 319)
  end
 elseif wmGetTile(plPlane, iX, iY) == 74 then
  wmPlaceTile(plPlane, iX, iY, 925)
  if wmGetTile(plPlane, iX, iY-1) == 302 then
   wmPlaceTile(plPlane, iX, iY-1, 321)
  end
 else
  if wmGetTile(plPlane, iX-1, iY) == 322 or
     wmGetTile(plPlane, iX-1, iY) == 328 then
   wmPlaceTile(plPlane, iX, iY, 323)
   wmPlaceTile(plPlane, iX, iY-1, 320)
  else
   wmPlaceTile(plPlane, iX, iY, 327)
   wmPlaceTile(plPlane, iX, iY-1, 326)
  end
 end
end