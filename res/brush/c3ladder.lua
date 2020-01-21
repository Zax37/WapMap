--brush by Zax37 20.04.2011
gameID = wmCLAW
layerName = "ACTION"
levelID = 3

brushName_PL = "Drabina"
brushName = "Ladder"
brushIcon = 667

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 670 then
  wmPlaceTile(plPlane, iX, iY, 669)
 end

 if wmGetTile(plPlane, iX, iY+1) == 668 then
  wmPlaceTile(plPlane, iX, iY+1, 669)
 end

 if wmGetTile(plPlane, iX, iY) ~= 669 then
  if wmGetTile(plPlane, iX, iY) == 655 then wmPlaceTile(plPlane, iX, iY, 667)
  else wmPlaceTile(plPlane, iX, iY, 668) end
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 669 then
  wmPlaceTile(plPlane, iX, iY+1, 670)
 end
end
