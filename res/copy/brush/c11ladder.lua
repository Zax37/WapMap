gameID = wmCLAW
layerName = "ACTION"
levelID = 11

brushName_PL = "Drabina"
brushName = "Ladder"
brushIcon = 95
enableSettings = 1
cbTest = 0

function cbInitSettings()
 cbTest = wmAddSetting(wmCheckbox, "T³o")
end

function cbBrushPlaced(plPlane, iX, iY)
if wmGetSettingValue(cbTest)==1 then
 if wmGetTile(plPlane, iX, iY) == 98 then
  wmPlaceTile(plPlane, iX, iY, 96)
 end
 if wmGetTile(plPlane, iX, iY+1) == 94 then
  wmPlaceTile(plPlane, iX, iY+1, 96)
 end

 if wmGetTile(plPlane, iX, iY) ~= 96 then
  wmPlaceTile(plPlane, iX, iY, 94)
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 96 then
  wmPlaceTile(plPlane, iX, iY+1, 98)
 end 
 else
 if wmGetTile(plPlane, iX, iY) == 97 then
  wmPlaceTile(plPlane, iX, iY, 95)
 end
 if wmGetTile(plPlane, iX, iY+1) == 93 then
  wmPlaceTile(plPlane, iX, iY+1, 95)
 end

 if wmGetTile(plPlane, iX, iY) ~= 95 then
  wmPlaceTile(plPlane, iX, iY, 93)
 end
 
 if wmGetTile(plPlane, iX, iY+1) ~= 95 then
  wmPlaceTile(plPlane, iX, iY+1, 97)
 end
end
end