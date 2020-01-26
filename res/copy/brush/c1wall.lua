--Zax37 was here 19.01.2013
gameID = wmCLAW
layerName = "ACTION"
levelID = 1

brushName_PL = "Sciana"
brushName = "Wall"
brushIcon = 12
enableSettings = 1

function cbInitSettings()
 if wmGetLangCode() == "PL" then
  cbOpt = wmAddSetting(wmCheckbox, "Okna")
 else
  cbOpt = wmAddSetting(wmCheckbox, "Windows")
 end  
end

function cbBrushPlaced(plPlane, iX, iY)
 if wmGetTile(plPlane, iX-1, iY-1) == 308 then
  wmPlaceTile(plPlane, iX-1, iY-1, 309)
 elseif wmGetTile(plPlane, iX-1, iY-1) == 920 then
  wmPlaceTile(plPlane, iX-1, iY-1, 302)
 elseif wmGetTile(plPlane, iX-1, iY-1) == 307 then
  wmPlaceTile(plPlane, iX-1, iY-1, 304)
  if wmGetTile(plPlane, iX-2, iY-1) ~= 309 then
   wmPlaceTile(plPlane, iX-2, iY-1, 304)
  end
 elseif wmGetTile(plPlane, iX-1, iY-1) ~= 12 and wmGetTile(plPlane, iX-1, iY-1) ~= 926 and
    wmGetTile(plPlane, iX-1, iY-1) ~= 302 and
    wmGetTile(plPlane, iX-1, iY-1) ~= 304 and
    wmGetTile(plPlane, iX-1, iY-1) ~= 920 and
    wmGetTile(plPlane, iX-1, iY-1) ~= 74 and
    wmGetTile(plPlane, iX-1, iY-1) ~= 309 then
  if wmGetTile(plPlane, iX-1, iY-1) ~= 305 then
   wmPlaceTile(plPlane, iX-1, iY-1, 303)
  else
   wmPlaceTile(plPlane, iX-1, iY-1, 304)
  end
 end

 if wmGetTile(plPlane, iX+1, iY-1) == 302 then
  wmPlaceTile(plPlane, iX+1, iY-1, 74)
 elseif wmGetTile(plPlane, iX+1, iY-1) == 303 then
  wmPlaceTile(plPlane, iX+1, iY-1, 304)
 elseif wmGetTile(plPlane, iX+1, iY-1) == 920 then
  wmPlaceTile(plPlane, iX+1, iY-1, 74)
 elseif wmGetTile(plPlane, iX+1, iY-1) ~= 308 and
    wmGetTile(plPlane, iX+1, iY-1) ~= 304 and
    wmGetTile(plPlane, iX+1, iY-1) ~= 305 and
    wmGetTile(plPlane, iX+1, iY-1) ~= 74 and
    wmGetTile(plPlane, iX+1, iY-1) ~= 309 and
    wmGetTile(plPlane, iX+1, iY-1) ~= 12 and
    wmGetTile(plPlane, iX+1, iY-1) ~= 926 then
  wmPlaceTile(plPlane, iX+1, iY-1, 307)
 end

 if wmGetTile(plPlane, iX, iY-1) == 302 or wmGetTile(plPlane, iX, iY-1) == 920 then
  wmPlaceTile(plPlane, iX, iY-1, 74)
 elseif wmGetTile(plPlane, iX, iY-1) == 305 then
  wmPlaceTile(plPlane, iX, iY-1, 304)
 elseif wmGetTile(plPlane, iX, iY-1) == 308 then
  wmPlaceTile(plPlane, iX, iY-1, 309)
 elseif wmGetTile(plPlane, iX, iY-1) ~= 12 and wmGetTile(plPlane, iX, iY-1) ~= 926 and
        wmGetTile(plPlane, iX, iY-1) ~= 74 and
        wmGetTile(plPlane, iX, iY-1) ~= 309 then
  if wmGetTile(plPlane, iX+1, iY-1) == 307 then
   wmPlaceTile(plPlane, iX, iY-1, 305)
  else
   wmPlaceTile(plPlane, iX, iY-1, 304)
  end
 end

 if wmGetTile(plPlane, iX-1, iY) == 308 or wmGetTile(plPlane, iX-1, iY) == 309 then
  wmPlaceTile(plPlane, iX-1, iY, 12)
 elseif wmGetTile(plPlane, iX-1, iY) == 307 then
  wmPlaceTile(plPlane, iX-1, iY, 74)
 elseif wmGetTile(plPlane, iX-1, iY) == 304 or wmGetTile(plPlane, iX-1, iY) == 305 then
  wmPlaceTile(plPlane, iX-1, iY, 74)
 elseif wmGetTile(plPlane, iX-1, iY) ~= 12 and wmGetTile(plPlane, iX-1, iY) ~= 926 and wmGetTile(plPlane, iX-1, iY) ~= 74 then
  if wmGetTile(plPlane, iX-1, iY+1) == 302 or
     wmGetTile(plPlane, iX-1, iY+1) == 74 or
     wmGetTile(plPlane, iX-1, iY+1) == 321 or
     wmGetTile(plPlane, iX-1, iY+1) == 406 or
     wmGetTile(plPlane, iX-1, iY+1) == 920 then
   wmPlaceTile(plPlane, iX-1, iY, 302)
  else
   wmPlaceTile(plPlane, iX-1, iY, 920)
  end
 end

 wmPlaceTile(plPlane, iX, iY, 12+(wmRandInt(0,1)*wmGetSettingValue(cbOpt)*914))

 if wmGetTile(plPlane, iX+1, iY) == 74 or wmGetTile(plPlane, iX+1, iY) == 302 or wmGetTile(plPlane, iX+1, iY) == 920 then
  wmPlaceTile(plPlane, iX+1, iY, 12)
 elseif wmGetTile(plPlane, iX+1, iY) == 303 then
  if wmGetTile(plPlane, iX+1, iY-1) == 74 then
   wmPlaceTile(plPlane, iX+1, iY, 12)
   wmPlaceTile(plPlane, iX+2, iY, 309)
  else 
   wmPlaceTile(plPlane, iX+1, iY, 309)
  end
 elseif wmGetTile(plPlane, iX+1, iY) == 304 or wmGetTile(plPlane, iX+1, iY) == 305 then
  wmPlaceTile(plPlane, iX+1, iY, 309)
 elseif wmGetTile(plPlane, iX+1, iY) ~= 12 and wmGetTile(plPlane, iX+1, iY) ~= 926 and wmGetTile(plPlane, iX+1, iY) ~= 309 then
  if wmGetTile(plPlane, iX+1, iY-1) == 74 then
   wmPlaceTile(plPlane, iX+1, iY, 12)
  else 
   wmPlaceTile(plPlane, iX+1, iY, 308)
  end
 end
end