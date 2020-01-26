--Platform brush by Zax37 01.10.2010
--ojapierdole challange accepted Zax37 19.01.2013
gameID = wmCLAW
layerName = "ACTION"
levelID = 2

brushName_PL = "Platforma"
brushName = "Platform"
brushIcon = 38

function ins(plPlane, iX, iY)
 if wmGetTile(plPlane, iX, iY) == 3 or
 wmGetTile(plPlane, iX, iY) == 3 or
 wmGetTile(plPlane, iX, iY) == 9 or
 wmGetTile(plPlane, iX, iY) == 12 or
 wmGetTile(plPlane, iX, iY) == 28 or
 wmGetTile(plPlane, iX, iY) == 29 or
 wmGetTile(plPlane, iX, iY) == 32 or
 wmGetTile(plPlane, iX, iY) == 35 or
 wmGetTile(plPlane, iX, iY) == 37 or
 wmGetTile(plPlane, iX, iY) == 38 or
 wmGetTile(plPlane, iX, iY) == 39 or
 wmGetTile(plPlane, iX, iY) == 46 or
 wmGetTile(plPlane, iX, iY) == 53 or
 wmGetTile(plPlane, iX, iY) == 58 or
 wmGetTile(plPlane, iX, iY) >= 310 and
 wmGetTile(plPlane, iX, iY) <= 315 then
 return true
 else return false
 end
end

function cbBrushPlaced(plPlane, iX, iY)

--tworzenie na logach, empty i drabinach
if not ins(plPlane, iX, iY) and not ins(plPlane, iX-1, iY) and not ins(plPlane, iX+1, iY)
and not ins(plPlane, iX, iY+1) and not ins(plPlane, iX-1, iY+1) and not ins(plPlane, iX+1, iY+1) then
--if (wmGetTile(plPlane, iX, iY) == 48 or wmGetTile(plPlane, iX, iY) == 16 or wmGetTile(plPlane, iX, iY) == 22 or wmGetTile(plPlane, iX, iY) == 27 or wmGetTile(plPlane, iX, iY) == wmEMPTY or wmGetTile(plPlane, iX, iY) == 52 or wmGetTile(plPlane, iX, iY) == 113) and wmGetTile(plPlane, iX-1, iY) ~= 310 and wmGetTile(plPlane, iX+1, iY) ~= 28 and wmGetTile(plPlane, iX+1, iY) ~= 46 then

if wmGetTile(plPlane, iX-1, iY) ~= 14 and wmGetTile(plPlane, iX-1, iY) ~= 97 and wmGetTile(plPlane, iX-1, iY) ~= 17 and wmGetTile(plPlane, iX-1, iY) ~= 18 then
 if wmGetTile(plPlane, iX-1, iY) == 48 or wmGetTile(plPlane, iX-1, iY) == 52 then
    wmPlaceTile(plPlane, iX-1, iY, 46)
 else
    wmPlaceTile(plPlane, iX-1, iY, 28)
 end
 if wmGetTile(plPlane, iX-1, iY+1) == 48 or wmGetTile(plPlane, iX-1, iY+1) == 113 then
    wmPlaceTile(plPlane, iX-1, iY+1, 53)
 else
    wmPlaceTile(plPlane, iX-1, iY+1, 37)
 end
end

 if wmGetTile(plPlane, iX, iY) == 48 or wmGetTile(plPlane, iX, iY) == 52 then
    wmPlaceTile(plPlane, iX, iY, 32)
 elseif wmGetTile(plPlane, iX, iY) == 113 then
    wmPlaceTile(plPlane, iX, iY, 35)
 else wmPlaceTile(plPlane, iX, iY, 29)
 end
 if wmGetTile(plPlane, iX, iY+1) == 48 or wmGetTile(plPlane, iX, iY+1) == 113 then
    wmPlaceTile(plPlane, iX, iY+1, 38)
 else wmPlaceTile(plPlane, iX, iY+1, 12)
 end
 if wmGetTile(plPlane, iX+1, iY) ~= 14 and wmGetTile(plPlane, iX+1, iY) ~= 96 and wmGetTile(plPlane, iX+1, iY) ~= 17 and wmGetTile(plPlane, iX+1, iY) ~= 18 then
  wmPlaceTile(plPlane, iX+1, iY, 310)
  if wmGetTile(plPlane, iX+1, iY-1) == 48 or wmGetTile(plPlane, iX+1, iY-1) == 52 then
     wmPlaceTile(plPlane, iX+1, iY-1, 48)
  elseif wmGetTile(plPlane, iX+1, iY-1) == 12 then wmPlaceTile(plPlane, iX+1, iY-1, 39)
  elseif wmGetTile(plPlane, iX+1, iY-1) == 37 then wmPlaceTile(plPlane, iX+1, iY-1, 53)
  elseif wmGetTile(plPlane, iX+1, iY-1) == 52 then wmPlaceTile(plPlane, iX+1, iY-1, 48)
  elseif wmGetTile(plPlane, iX+1, iY-1) == 312 then wmPlaceTile(plPlane, iX+1, iY-1, 311)
  elseif wmGetTile(plPlane, iX+1, iY-1) ~= 53 and wmGetTile(plPlane, iX+1, iY-1) ~= 38 and wmGetTile(plPlane, iX+1, iY-1) ~= 39 and wmGetTile(plPlane, iX+1, iY-1) ~= 311 then
     wmPlaceTile(plPlane, iX+1, iY-1, 113)
  end

 if wmGetTile(plPlane, iX+1, iY+1) == 48 or wmGetTile(plPlane, iX+1, iY+1) == 113 then
    wmPlaceTile(plPlane, iX+1, iY+1, 311)
 else wmPlaceTile(plPlane, iX+1, iY+1, 312)
 end

end
end

--wydluz w lewo
if wmGetTile(plPlane, iX, iY) == 46 or wmGetTile(plPlane, iX, iY) == 28 then

if (not ins(plPlane, iX-1, iY) and not ins(plPlane, iX-1, iY+1)) or (wmGetTile(plPlane, iX-1, iY) == 310 and (wmGetTile(plPlane, iX-1, iY+1) == 311 or wmGetTile(plPlane, iX-1, iY+1) == 312)) then
 if wmGetTile(plPlane, iX, iY) == 46 then
    wmPlaceTile(plPlane, iX, iY, 32)
 else wmPlaceTile(plPlane, iX, iY, 35)
 end
 if wmGetTile(plPlane, iX, iY+1) == 53 then
    wmPlaceTile(plPlane, iX, iY+1, 38)
 else
    wmPlaceTile(plPlane, iX, iY+1, 12)
 end
end

 if wmGetTile(plPlane, iX-1, iY) == 310 then
    wmPlaceTile(plPlane, iX-1, iY, 32)
    if wmGetTile(plPlane, iX-1, iY+1) == 312 then
     wmPlaceTile(plPlane, iX-1, iY+1, 12)
    else wmPlaceTile(plPlane, iX-1, iY+1, 38)
    end
 elseif not ins(plPlane,iX-1,iY) and not ins(plPlane,iX-1,iY+1) then
 if wmGetTile(plPlane, iX-1, iY) == 48 or wmGetTile(plPlane, iX-1, iY) == 52 then
    wmPlaceTile(plPlane, iX-1, iY, 46)
 elseif wmGetTile(plPlane, iX-1, iY) ~= 14 and wmGetTile(plPlane, iX-1, iY) ~= 17 and wmGetTile(plPlane, iX-1, iY) ~= 97 then
    wmPlaceTile(plPlane, iX-1, iY, 28)
 end
 if (wmGetTile(plPlane, iX-1, iY) ~= 14 and wmGetTile(plPlane, iX-1, iY) ~= 17 and wmGetTile(plPlane, iX-1, iY) ~= 97) and wmGetTile(plPlane, iX-1, iY+1) == 48 or wmGetTile(plPlane, iX-1, iY+1) == 113 then
    wmPlaceTile(plPlane, iX-1, iY+1, 53)
 elseif (wmGetTile(plPlane, iX-1, iY) ~= 14 and wmGetTile(plPlane, iX-1, iY) ~= 17 and wmGetTile(plPlane, iX-1, iY) ~= 97) and wmGetTile(plPlane, iX-1, iY+1) ~= 38 and wmGetTile(plPlane, iX-1, iY+1) ~= 12 then
    wmPlaceTile(plPlane, iX-1, iY+1, 37)
 end
end
end
--wydluz w prawo
if wmGetTile(plPlane, iX, iY) == 310 then

if (not ins(plPlane, iX+1, iY) and not ins(plPlane, iX+1, iY+1)) or ((wmGetTile(plPlane, iX+1, iY) == 28 or wmGetTile(plPlane, iX+1, iY) == 46) and (wmGetTile(plPlane, iX+1, iY+1) == 37 or wmGetTile(plPlane, iX+1, iY+1) == 53)) then
 if wmGetTile(plPlane, iX, iY) == 310 then
    wmPlaceTile(plPlane, iX, iY, 32)
 else wmPlaceTile(plPlane, iX, iY, 35)
 end
 if wmGetTile(plPlane, iX, iY+1) == 311 then
    wmPlaceTile(plPlane, iX, iY+1, 38)
 else
    wmPlaceTile(plPlane, iX, iY+1, 12)
 end
end
if wmGetTile(plPlane, iX+1, iY) ~= 14 and wmGetTile(plPlane, iX+1, iY) ~= 17 and wmGetTile(plPlane, iX+1, iY) ~= 96 then
 if wmGetTile(plPlane, iX+1, iY) == 46 or wmGetTile(plPlane, iX+1, iY) == 28 then
  if wmGetTile(plPlane, iX+1, iY) == 28 then wmPlaceTile(plPlane, iX+1, iY, 35)
  else wmPlaceTile(plPlane, iX+1, iY, 32)
  end
  if wmGetTile(plPlane, iX+1, iY+1) == 37 then wmPlaceTile(plPlane, iX+1, iY+1, 12)
  else wmPlaceTile(plPlane, iX+1, iY+1, 39)
  end
 elseif not ins(plPlane,iX+1,iY) and not ins(plPlane,iX+1,iY+1) then
 if wmGetTile(plPlane, iX+1, iY-1) ~= 48 and wmGetTile(plPlane, iX+1, iY-1) ~= 113 then
    wmPlaceTile(plPlane, iX+1, iY, 310)
    if wmGetTile(plPlane, iX+1, iY-1) == 52 then
    wmPlaceTile(plPlane, iX+1, iY-1, 48)
    elseif wmGetTile(plPlane, iX+1, iY-1) == 12 then wmPlaceTile(plPlane, iX+1, iY-1, 39)
    elseif wmGetTile(plPlane, iX+1, iY-1) == 37 then wmPlaceTile(plPlane, iX+1, iY-1, 53)
    elseif wmGetTile(plPlane, iX+1, iY-1) == 52 then wmPlaceTile(plPlane, iX+1, iY-1, 48)
    elseif wmGetTile(plPlane, iX+1, iY-1) == 312 then wmPlaceTile(plPlane, iX+1, iY-1, 311)
    elseif wmGetTile(plPlane, iX+1, iY-1) ~= 53 and wmGetTile(plPlane, iX+1, iY-1) ~= 38 and wmGetTile(plPlane, iX+1, iY-1) ~= 39 and wmGetTile(plPlane, iX+1, iY-1) ~= 311 then
    wmPlaceTile(plPlane, iX+1, iY-1, 113)
    end
 else
    wmPlaceTile(plPlane, iX+1, iY, 310)
 end
 if wmGetTile(plPlane, iX+1, iY+1) == 48 or wmGetTile(plPlane, iX+1, iY+1) == 113 then
    wmPlaceTile(plPlane, iX+1, iY+1, 311)
 elseif wmGetTile(plPlane, iX+1, iY+1) ~= 39 and wmGetTile(plPlane, iX+1, iY+1) ~= 12 then
    wmPlaceTile(plPlane, iX+1, iY+1, 312)
 end
end

end
end

end

