gameID = wmCLAW
layerName = "ACTION"
levelID = 2

brushName_PL = "Baszta"
brushName = "Tower"
brushIcon = 105

function cbBrushPlaced(plPlane, iX, iY)
wmPlaceTile(plPlane, iX, iY, 105)
  if wmGetTile(plPlane, iX-1, iY) == 105 and
     wmGetTile(plPlane, iX+1, iY) == 105 then
    wmPlaceTile(plPlane, iX, iY, 105)
  else
    if wmGetTile(plPlane, iX+1, iY) ~= 105 and
       wmGetTile(plPlane, iX-1, iY) ~= 105 then
      wmPlaceTile(plPlane, iX-1, iY, 96)
      wmPlaceTile(plPlane, iX, iY, 105)
      wmPlaceTile(plPlane, iX+1, iY, 97)
    else
      if wmGetTile(plPlane, iX+1, iY) == 105 then
        wmPlaceTile(plPlane, iX-1, iY, 96)
        wmPlaceTile(plPlane, iX, iY, 105)
      end
      if wmGetTile(plPlane, iX-1, iY) == 105 then
        wmPlaceTile(plPlane, iX+1, iY, 97)
        wmPlaceTile(plPlane, iX, iY, 105)
       end
     end
  end
if wmGetTile(plPlane, iX-1, iY-1) ~= 105 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 97 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 96 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 82 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 81 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 80 then
  wmPlaceTile(plPlane, iX-1, iY-1, 80)
end

if wmGetTile(plPlane, iX, iY-1) ~= 105 then

  wmPlaceTile(plPlane, iX, iY-1, 81)
end
if wmGetTile(plPlane, iX+1, iY-1) ~= 105 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 97 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 80 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 81 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 82 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 96 then
  wmPlaceTile(plPlane, iX+1, iY-1, 82)
end

end