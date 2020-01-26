gameID = wmCLAW
layerName = "ACTION"
levelID = 1

brushName_PL = "Kolumna"
brushName = "Column"
brushIcon = 934

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 304 then
  wmPlaceTile(plPlane, iX, iY, 935)
  if wmGetTile(plPlane, iX, iY-1) == wmEMPTY then
   wmPlaceTile(plPlane, iX, iY-1, 933)
  end
 elseif wmGetTile(plPlane, iX, iY) == 933 then
  wmPlaceTile(plPlane, iX, iY-1, 933)
  wmPlaceTile(plPlane, iX, iY, 934)
 end
end

