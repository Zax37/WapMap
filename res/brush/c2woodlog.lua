--Woodlog brush by Zax37 01.10.2010
gameID = wmCLAW
layerName = "ACTION"
levelID = 2

brushName_PL = "Bala"
brushName = "Wood log"
brushIcon = 14

function GetRandCenterLog()
 return 17
end

function cbBrushPlaced(plPlane, iX, iY)
 bPlacedUpper = 0
 --³¹czenie
 if ((wmGetTile(plPlane, iX, iY-1) == 23 or wmGetTile(plPlane, iX, iY) == 23) or (wmGetTile(plPlane, iX, iY-1) == 20 or wmGetTile(plPlane, iX, iY) == 20) ) and ((wmGetTile(plPlane, iX, iY+1) == 15 or wmGetTile(plPlane, iX, iY) == 15) or (wmGetTile(plPlane, iX, iY+1) == 18 or wmGetTile(plPlane, iX, iY) == 18)) then
  wmPlaceTile(plPlane, iX, iY, 14)
   if(wmGetTile(plPlane, iX, iY-2) == 15) then
    wmPlaceTile(plPlane, iX, iY-1, 18)
   end

   if(wmGetTile(plPlane, iX, iY-3) == 15) then
    wmPlaceTile(plPlane, iX, iY-2, 18)
   end

   if(wmGetTile(plPlane, iX, iY+2) == 23) then
    wmPlaceTile(plPlane, iX, iY+1, 20)
   end

   if(wmGetTile(plPlane, iX, iY+3) == 23) then
    wmPlaceTile(plPlane, iX, iY+2, 20)
   end

   if(wmGetTile(plPlane, iX, iY+1) == 15 or wmGetTile(plPlane, iX, iY+1) == 18) then
    wmPlaceTile(plPlane, iX, iY+1, 14)
   end

   if(wmGetTile(plPlane, iX, iY-1) == 23 or wmGetTile(plPlane, iX, iY-1) == 20) then
    wmPlaceTile(plPlane, iX, iY-1, 14)
   end
 end
 if (((wmGetTile(plPlane, iX, iY-1) == 14) or (wmGetTile(plPlane, iX, iY-1) == 18)) and ((wmGetTile(plPlane, iX, iY+1) == 14) or (wmGetTile(plPlane, iX, iY+1) == 20)) and (wmGetTile(plPlane, iX, iY) ~= 14)) then
  wmPlaceTile(plPlane, iX, iY, 14)
 --rozszerzanie w górê
 elseif wmGetTile(plPlane, iX, iY) == 15 then
  wmPlaceTile(plPlane, iX, iY, 18)
  wmPlaceTile(plPlane, iX, iY-1, 15)
   if wmGetTile(plPlane, iX, iY+1) == 23 then
    wmPlaceTile(plPlane, iX, iY+1, 20)
    wmPlaceTile(plPlane, iX, iY+2, 23)
   elseif wmGetTile(plPlane, iX, iY+1) == 18 then
    wmPlaceTile(plPlane, iX, iY+1, 14)
   end
   if wmGetTile(plPlane, iX, iY+4) == 18 then

    wmPlaceTile(plPlane, iX, iY+2, 14)
    wmPlaceTile(plPlane, iX, iY+3, 14)
    wmPlaceTile(plPlane, iX, iY+4, 14)
   end
 --rozszerzanie w górê2
 elseif wmGetTile(plPlane, iX, iY+1) == 15 then
  wmPlaceTile(plPlane, iX, iY+1, 18)
  wmPlaceTile(plPlane, iX, iY, 15)
   if wmGetTile(plPlane, iX, iY+2) == 23 then
    wmPlaceTile(plPlane, iX, iY+2, 20)
    wmPlaceTile(plPlane, iX, iY+3, 23)
   elseif wmGetTile(plPlane, iX, iY+2) == 18 then
    wmPlaceTile(plPlane, iX, iY+2, 14)
   end
   if wmGetTile(plPlane, iX, iY+4) == 18 then

    wmPlaceTile(plPlane, iX, iY+2, 14)
    wmPlaceTile(plPlane, iX, iY+3, 14)
    wmPlaceTile(plPlane, iX, iY+4, 14)
   end
 --rozszerzanie w dó³
 elseif wmGetTile(plPlane, iX, iY) == 23 then
  if wmGetTile(plPlane, iX, iY+1) ~= 14 and wmGetTile(plPlane, iX, iY+1) ~= 17 then
   wmPlaceTile(plPlane, iX, iY, 20)
   wmPlaceTile(plPlane, iX, iY+1, 23)
  else
   wmPlaceTile(plPlane, iX, iY, 14)
   if wmGetTile(plPlane, iX, iY+1) == 18 then
    wmPlaceTile(plPlane, iX, iY+1, 14)
   end
  end

 if wmGetTile(plPlane, iX, iY-1) == 15 then
    wmPlaceTile(plPlane, iX, iY-1, 18)
    wmPlaceTile(plPlane, iX, iY-2, 15)
   elseif wmGetTile(plPlane, iX, iY-1) == 20 then
    wmPlaceTile(plPlane, iX, iY-1, 14)
   end

 elseif wmGetTile(plPlane, iX, iY+1) == 29 or wmGetTile(plPlane, iX, iY+1) == 35 or wmGetTile(plPlane, iX, iY+1) == 58 then
  wmPlaceTile(plPlane, iX, iY+1, 3)
  wmPlaceTile(plPlane, iX, iY, 1)
 
 elseif wmGetTile(plPlane, iX, iY) ~= 14 and wmGetTile(plPlane, iX, iY) ~= 18 and wmGetTile(plPlane, iX, iY) ~= 20 then
  wmPlaceTile(plPlane, iX, iY, 15)
  wmPlaceTile(plPlane, iX, iY+1, 23)
 end
 if (wmGetTile(plPlane, iX, iY-1) == 15) and (wmGetTile(plPlane, iX, iY) == 14) then
  wmPlaceTile(plPlane, iX, iY-1, 18)
 end
 if (wmGetTile(plPlane, iX, iY+1) == 23) and (wmGetTile(plPlane, iX, iY) == 14) then
  wmPlaceTile(plPlane, iX, iY+1, 20)
 end

 --extra fix!!!! juhu
 if (wmGetTile(plPlane, iX, iY-1) == 18 and wmGetTile(plPlane, iX, iY-2) ~= 15) then
  wmPlaceTile(plPlane, iX, iY-2, 15)
 end
 if (wmGetTile(plPlane, iX, iY-2) == 20 and wmGetTile(plPlane, iX, iY-1) == 14) then
  wmPlaceTile(plPlane, iX, iY-2, 14)
 end
 if (wmGetTile(plPlane, iX, iY+2) == 18 and wmGetTile(plPlane, iX, iY-1) == 14) then
  wmPlaceTile(plPlane, iX, iY+2, 14)
 end
 if (wmGetTile(plPlane, iX, iY+1) == 18 and wmGetTile(plPlane, iX, iY+2) == 18) then
  wmPlaceTile(plPlane, iX, iY+1, 14)
 end
 if (wmGetTile(plPlane, iX, iY+2) == 18 and wmGetTile(plPlane, iX, iY+1) == 14) then
  wmPlaceTile(plPlane, iX, iY+2, 14)
 end
 if (wmGetTile(plPlane, iX, iY+2) == 23 and wmGetTile(plPlane, iX, iY+3) == 23) then
  wmPlaceTile(plPlane, iX, iY+1, 14)
  wmPlaceTile(plPlane, iX, iY+2, 20)
 end
 if (wmGetTile(plPlane, iX, iY+3) == 23 and wmGetTile(plPlane, iX, iY+4) == 23) then
  wmPlaceTile(plPlane, iX, iY+2, 14)
  wmPlaceTile(plPlane, iX, iY+3, 20)
 end
 if (wmGetTile(plPlane, iX, iY-2) == 15 and wmGetTile(plPlane, iX, iY-3) == 15) then
  wmPlaceTile(plPlane, iX, iY-1, 14)
  wmPlaceTile(plPlane, iX, iY-2, 18)
 end
 if (wmGetTile(plPlane, iX, iY+1) == 20 and wmGetTile(plPlane, iX, iY+2) == 15) then
  wmPlaceTile(plPlane, iX, iY+1, 14)
  wmPlaceTile(plPlane, iX, iY+2, 14)
  wmPlaceTile(plPlane, iX, iY+3, 14)
 end
 if (wmGetTile(plPlane, iX, iY+1) == 20 and wmGetTile(plPlane, iX, iY+2) == 20) then
  wmPlaceTile(plPlane, iX, iY+1, 14)
 end
 if (wmGetTile(plPlane, iX, iY+1) == 20 and wmGetTile(plPlane, iX, iY+2) ~= 23) then
  wmPlaceTile(plPlane, iX, iY+2, 23)
 end
 if (wmGetTile(plPlane, iX, iY-1) == 18 and wmGetTile(plPlane, iX, iY-2) == 15 and wmGetTile(plPlane, iX, iY-3) == 20) then
  wmPlaceTile(plPlane, iX, iY-1, 14)
  wmPlaceTile(plPlane, iX, iY-2, 14)
  wmPlaceTile(plPlane, iX, iY-3, 14)
 end
 if (wmGetTile(plPlane, iX, iY+2) == 23 and wmGetTile(plPlane, iX, iY+3) == 18) then
  wmPlaceTile(plPlane, iX, iY+1, 14)
  wmPlaceTile(plPlane, iX, iY+2, 14)
  wmPlaceTile(plPlane, iX, iY+3, 14)
 end
 if (wmGetTile(plPlane, iX, iY+4) == 14 and wmGetTile(plPlane, iX, iY+3) == 23) then
  wmPlaceTile(plPlane, iX, iY+2, 14)
  wmPlaceTile(plPlane, iX, iY+3, 14)
 end
end


