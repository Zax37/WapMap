--Woodlogdouble brush by Zax37 01.10.2010
--ja pierdole kto to pisa³ oO Zax37 19.01.2013
gameID = wmCLAW
layerName = "ACTION"
levelID = 2

brushName_PL = "Dwie bale"
brushName = "Double wood log"
brushIcon = 113

function cbBrushPlaced(plPlane, iX, iY)
zaxer = 0

 if wmGetTile(plPlane, iX, iY) == 3 and wmGetTile(plPlane, iX, iY-1) == 1 then
  wmPlaceTile(plPlane, iX, iY, 32)
  wmPlaceTile(plPlane, iX, iY-1, 113) 
 end
 if wmGetTile(plPlane, iX, iY+1) == 3 and wmGetTile(plPlane, iX, iY) == 1 then
  wmPlaceTile(plPlane, iX, iY+1, 32)
  wmPlaceTile(plPlane, iX, iY, 113) 
 end

 if wmGetTile(plPlane, iX, iY) == 12 and (wmGetTile(plPlane, iX, iY+1) == 113 or wmGetTile(plPlane, iX, iY+1) == 28 or wmGetTile(plPlane, iX, iY+1) == 35 or wmGetTile(plPlane, iX, iY+1) == 58) then
  wmPlaceTile(plPlane, iX, iY, 38)
  if wmGetTile(plPlane, iX, iY+1) == 58 or wmGetTile(plPlane, iX, iY+1) == 35 then wmPlaceTile(plPlane, iX, iY+1, 32)
  elseif wmGetTile(plPlane, iX, iY+1) == 28 then wmPlaceTile(plPlane, iX, iY+1, 46)
  else wmPlaceTile(plPlane, iX, iY+1, 48)
  end
 elseif wmGetTile(plPlane, iX, iY) == 37 and (wmGetTile(plPlane, iX, iY+1) == 113 or wmGetTile(plPlane, iX, iY+1) == 28 or wmGetTile(plPlane, iX, iY+1) == 35 or wmGetTile(plPlane, iX, iY+1) == 58) then
  wmPlaceTile(plPlane, iX, iY, 53)
  if wmGetTile(plPlane, iX, iY+1) == 58 or wmGetTile(plPlane, iX, iY+1) == 35 then wmPlaceTile(plPlane, iX, iY+1, 32)
  elseif wmGetTile(plPlane, iX, iY+1) == 28 then wmPlaceTile(plPlane, iX, iY+1, 46)
  else wmPlaceTile(plPlane, iX, iY+1, 48)
  end
 elseif wmGetTile(plPlane, iX, iY) == 312 and (wmGetTile(plPlane, iX, iY+1) == 113 or wmGetTile(plPlane, iX, iY+1) == 28 or wmGetTile(plPlane, iX, iY+1) == 35 or wmGetTile(plPlane, iX, iY+1) == 58) then
  wmPlaceTile(plPlane, iX, iY, 311)
  if wmGetTile(plPlane, iX, iY+1) == 58 or wmGetTile(plPlane, iX, iY+1) == 35 then wmPlaceTile(plPlane, iX, iY+1, 32)
  elseif wmGetTile(plPlane, iX, iY+1) == 28 then wmPlaceTile(plPlane, iX, iY+1, 46)
  else wmPlaceTile(plPlane, iX, iY+1, 48)
  end
 elseif wmGetTile(plPlane, iX, iY) == 52 and (wmGetTile(plPlane, iX, iY+1) == 113 or wmGetTile(plPlane, iX, iY+1) == 28 or wmGetTile(plPlane, iX, iY+1) == 35 or wmGetTile(plPlane, iX, iY+1) == 58) then
  wmPlaceTile(plPlane, iX, iY, 48)
  if wmGetTile(plPlane, iX, iY+1) == 58 or wmGetTile(plPlane, iX, iY+1) == 35 then wmPlaceTile(plPlane, iX, iY+1, 32)
  elseif wmGetTile(plPlane, iX, iY+1) == 28 then wmPlaceTile(plPlane, iX, iY+1, 46)
  else wmPlaceTile(plPlane, iX, iY+1, 48)
  end
 end

 if wmGetTile(plPlane, iX, iY-1) == 12 and (wmGetTile(plPlane, iX, iY) == 113 or wmGetTile(plPlane, iX, iY) == 28 or wmGetTile(plPlane, iX, iY) == 35 or wmGetTile(plPlane, iX, iY) == 58) then
  wmPlaceTile(plPlane, iX, iY-1, 38)
  if wmGetTile(plPlane, iX, iY) == 58 or wmGetTile(plPlane, iX, iY) == 35 then wmPlaceTile(plPlane, iX, iY, 32)
  elseif wmGetTile(plPlane, iX, iY) == 28 then wmPlaceTile(plPlane, iX, iY, 46)
  else wmPlaceTile(plPlane, iX, iY, 48)
  end
 elseif wmGetTile(plPlane, iX, iY-1) == 37 and (wmGetTile(plPlane, iX, iY) == 113 or wmGetTile(plPlane, iX, iY) == 28 or wmGetTile(plPlane, iX, iY) == 35 or wmGetTile(plPlane, iX, iY) == 58) then
  wmPlaceTile(plPlane, iX, iY-1, 53)
  if wmGetTile(plPlane, iX, iY) == 58 or wmGetTile(plPlane, iX, iY) == 35 then wmPlaceTile(plPlane, iX, iY, 32)
  elseif wmGetTile(plPlane, iX, iY) == 28 then wmPlaceTile(plPlane, iX, iY, 46)
  else wmPlaceTile(plPlane, iX, iY, 48)
  end
 elseif wmGetTile(plPlane, iX, iY-1) == 312 and (wmGetTile(plPlane, iX, iY) == 113 or wmGetTile(plPlane, iX, iY) == 28 or wmGetTile(plPlane, iX, iY) == 35 or wmGetTile(plPlane, iX, iY) == 58) then
  wmPlaceTile(plPlane, iX, iY-1, 311)
  if wmGetTile(plPlane, iX, iY) == 58 or wmGetTile(plPlane, iX, iY) == 35 then wmPlaceTile(plPlane, iX, iY, 32)
  elseif wmGetTile(plPlane, iX, iY) == 28 then wmPlaceTile(plPlane, iX, iY, 46)
  else wmPlaceTile(plPlane, iX, iY, 48)
  end
 elseif wmGetTile(plPlane, iX, iY-1) == 52 and (wmGetTile(plPlane, iX, iY) == 113 or wmGetTile(plPlane, iX, iY) == 28 or wmGetTile(plPlane, iX, iY) == 35 or wmGetTile(plPlane, iX, iY) == 58) then
  wmPlaceTile(plPlane, iX, iY-1, 48)
  if wmGetTile(plPlane, iX, iY) == 58 or wmGetTile(plPlane, iX, iY) == 35 then wmPlaceTile(plPlane, iX, iY, 32)
  elseif wmGetTile(plPlane, iX, iY) == 28 then wmPlaceTile(plPlane, iX, iY, 46)
  else wmPlaceTile(plPlane, iX, iY, 48)
  end
 end

 if wmGetTile(plPlane, iX, iY) == 29 then wmPlaceTile(plPlane, iX, iY, 35) end

 if wmGetTile(plPlane, iX, iY) == 52 and wmGetTile(plPlane, iX, iY+1) == 113 then
 wmPlaceTile(plPlane, iX, iY, 48)
 wmPlaceTile(plPlane, iX, iY+1, 48)
 end
if wmGetTile(plPlane, iX, iY) ~= 29 and wmGetTile(plPlane, iX, iY) ~= 12 and wmGetTile(plPlane, iX, iY) ~= 37 and wmGetTile(plPlane, iX, iY) ~= 312 and wmGetTile(plPlane, iX, iY) ~= 32 and wmGetTile(plPlane, iX, iY) ~= 37 and wmGetTile(plPlane, iX, iY) ~= 38 and wmGetTile(plPlane, iX, iY) ~= 39 and wmGetTile(plPlane, iX, iY) ~= 46 and wmGetTile(plPlane, iX, iY) ~= 35 and wmGetTile(plPlane, iX, iY) ~= 58 and wmGetTile(plPlane, iX, iY) ~= 53 and wmGetTile(plPlane, iX, iY) ~= 310 and wmGetTile(plPlane, iX, iY) ~= 311 and wmGetTile(plPlane, iX, iY) ~= 28 then


if wmGetTile(plPlane, iX, iY) ~= 48 then
 if wmGetTile(plPlane, iX, iY-1) == 48 then
  wmPlaceTile(plPlane, iX, iY, 52)

 elseif wmGetTile(plPlane, iX, iY+1) == 35 then
  wmPlaceTile(plPlane, iX, iY+1, 32)
  wmPlaceTile(plPlane, iX, iY, 113)
  zaxer = 1
 elseif (wmGetTile(plPlane, iX, iY-1) == 48 and wmGetTile(plPlane, iX, iY+1) == 48 and wmGetTile(plPlane, iX, iY) ~= 48) then
  wmPlaceTile(plPlane, iX, iY, 48)
  zaxer = 1
 elseif (wmGetTile(plPlane, iX, iY) == 52 and wmGetTile(plPlane, iX, iY+1) == 113) then
  wmPlaceTile(plPlane, iX, iY, 48)
  wmPlaceTile(plPlane, iX, iY+1, 48)
  zaxer = 1
 elseif (wmGetTile(plPlane, iX, iY-1) == 52 and wmGetTile(plPlane, iX, iY) == 113) then
  wmPlaceTile(plPlane, iX, iY-1, 48)
  wmPlaceTile(plPlane, iX, iY, 48)
  zaxer = 1
 elseif (wmGetTile(plPlane, iX, iY-1) == 52) then
  wmPlaceTile(plPlane, iX, iY-1, 48)
  wmPlaceTile(plPlane, iX, iY, 52)
  zaxer = 1
 elseif wmGetTile(plPlane, iX, iY+1) == 48 then
  wmPlaceTile(plPlane, iX, iY, 113)
  zaxer = 1
 elseif wmGetTile(plPlane, iX, iY+1) == 113 then
  wmPlaceTile(plPlane, iX, iY+1, 48)
  wmPlaceTile(plPlane, iX, iY, 113)
  zaxer = 1
 elseif wmGetTile(plPlane, iX, iY+1) == 28 then
  wmPlaceTile(plPlane, iX, iY+1, 46)
  if wmGetTile(plPlane, iX, iY) == 52 then
   wmPlaceTile(plPlane, iX, iY, 48)   
  else
   wmPlaceTile(plPlane, iX, iY, 113)
  end
  zaxer = 1
 elseif wmGetTile(plPlane, iX, iY) == 312 and wmGetTile(plPlane, iX, iY+1) == 113 then
  wmPlaceTile(plPlane, iX, iY+1, 48)
  wmPlaceTile(plPlane, iX, iY, 311)
  zaxer = 1
 elseif wmGetTile(plPlane, iX, iY) == 113 then
  wmPlaceTile(plPlane, iX, iY, 48)
  wmPlaceTile(plPlane, iX, iY-1, 113)
 elseif wmGetTile(plPlane, iX, iY-1) == 12 then
  wmPlaceTile(plPlane, iX, iY-1, 38)
  wmPlaceTile(plPlane, iX, iY, 52)
 elseif wmGetTile(plPlane, iX, iY-1) == 37 then
  wmPlaceTile(plPlane, iX, iY-1, 53)
  wmPlaceTile(plPlane, iX, iY, 52)
 elseif wmGetTile(plPlane, iX, iY-1) == 312 then
  wmPlaceTile(plPlane, iX, iY-1, 311)
  wmPlaceTile(plPlane, iX, iY, 52)
 elseif wmGetTile(plPlane, iX, iY+1) == 29 then
  wmPlaceTile(plPlane, iX, iY+1, 35)
 elseif wmGetTile(plPlane, iX, iY-1) ~= 38 and wmGetTile(plPlane, iX, iY-1) ~= 53 and wmGetTile(plPlane, iX, iY-1) ~= 39 then
  wmPlaceTile(plPlane, iX, iY, 52)
  wmPlaceTile(plPlane, iX, iY-1, 113)
 end
end
end
 if zaxer == 0 and (wmGetTile(plPlane, iX, iY-1) == 48 or wmGetTile(plPlane, iX, iY-1) == 113 or wmGetTile(plPlane, iX, iY-1) == 52) and (wmGetTile(plPlane, iX, iY+1) == 48 or wmGetTile(plPlane, iX, iY+1) == 113 or wmGetTile(plPlane, iX, iY+1) == 52) then
 wmPlaceTile(plPlane, iX, iY, 48)
 end
 if wmGetTile(plPlane, iX, iY) == 52 and wmGetTile(plPlane, iX, iY+1) == 113 then
 wmPlaceTile(plPlane, iX, iY, 48)
 wmPlaceTile(plPlane, iX, iY+1, 48)
 end
end


