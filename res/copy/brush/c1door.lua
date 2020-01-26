--Zax37 was here 19.01.2013
gameID = wmCLAW
layerName = "ACTION"
levelID = 1

brushName_PL = "Drzwi"
brushName = "Door"
brushIcon = 401
enableSettings = 1

ddType = 0

function cbInitSettings()
 if wmGetLangCode() == "PL" then
  ddType = wmAddSetting(wmDropDown, "Typ")
  wmAddDropDownOption(ddType, "Auto")
  wmAddDropDownOption(ddType, "Lewe")
  wmAddDropDownOption(ddType, "Prawe")
 else
  ddType = wmAddSetting(wmDropDown, "Type")
  wmAddDropDownOption(ddType, "Auto")
  wmAddDropDownOption(ddType, "Left")
  wmAddDropDownOption(ddType, "Right")
 end
end

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetSettingValue(ddType) == 0 then
  if wmGetTile(plPlane, iX, iY+1) == 309 and wmGetTile(plPlane, iX+1, iY) == wmEMPTY and wmGetTile(plPlane, iX, iY) == 308 then
   wmPlaceTile(plPlane, iX, iY, 401)
   wmPlaceTile(plPlane, iX+1, iY, 402)
   wmPlaceTile(plPlane, iX, iY+1, 403)
   wmPlaceTile(plPlane, iX+1, iY+1, 404)
  elseif wmGetTile(plPlane, iX, iY+1) == 74 and wmGetTile(plPlane, iX-1, iY) == wmEMPTY and wmGetTile(plPlane, iX, iY) == 302 then
   wmPlaceTile(plPlane, iX-1, iY, 405)
   wmPlaceTile(plPlane, iX, iY, 406)
   wmPlaceTile(plPlane, iX-1, iY+1, 407)
   wmPlaceTile(plPlane, iX, iY+1, 408)
  elseif wmGetTile(plPlane, iX, iY) == 309 and wmGetTile(plPlane, iX+1, iY-1) == wmEMPTY and wmGetTile(plPlane, iX, iY-1) == 308 then
   wmPlaceTile(plPlane, iX, iY-1, 401)
   wmPlaceTile(plPlane, iX+1, iY-1, 402)
   wmPlaceTile(plPlane, iX, iY, 403)
   wmPlaceTile(plPlane, iX+1, iY, 404)
  elseif wmGetTile(plPlane, iX, iY) == 74 and wmGetTile(plPlane, iX-1, iY-1) == wmEMPTY and wmGetTile(plPlane, iX, iY-1) == 302 then
   wmPlaceTile(plPlane, iX-1, iY-1, 405)
   wmPlaceTile(plPlane, iX, iY-1, 406)
   wmPlaceTile(plPlane, iX-1, iY, 407)
   wmPlaceTile(plPlane, iX, iY, 408)
  end
 elseif wmGetSettingValue(ddType) == 1 then
  wmPlaceTile(plPlane, iX, iY, 401)
  wmPlaceTile(plPlane, iX+1, iY, 402)
  wmPlaceTile(plPlane, iX, iY+1, 403)
  wmPlaceTile(plPlane, iX+1, iY+1, 404)
 else
  wmPlaceTile(plPlane, iX-1, iY, 405)
  wmPlaceTile(plPlane, iX, iY, 406)
  wmPlaceTile(plPlane, iX-1, iY+1, 407)
  wmPlaceTile(plPlane, iX, iY+1, 408)
 end
end
