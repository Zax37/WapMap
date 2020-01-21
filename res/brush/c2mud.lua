--Mud brush by Zax37 01.10.2010
gameID = wmCLAW
layerName = "ACTION"
levelID = 2

brushName_PL = "Bloto"
brushName = "Mud"
brushIcon = 314

function cbBrushPlaced(plPlane, iX, iY)
if wmGetTile(plPlane, iX, iY) == 32 then
 if wmGetTile(plPlane, iX-1, iY) == 315 then
  hObjects = wmGetObjectsInArea(iX*64-33, iY*64+152, iX*64-31, iY*64+154)
   if hObjects ~= nil then
    for i=0, table.getn(hObjects) do
     if wmGetObjectLogic(hObjects[i])=='GooCoverup' and wmGetObjectImageSet(hObjects[i])=='LEVEL_GOORIGHT' then
      wmDeleteObject(hObjects[i])
      wmCreateObject('GooCoverup', 'LEVEL_GOOCENTER', iX*64-32, iY*64+153, 8000)
     end
    end
  wmPlaceTile(plPlane, iX-1, iY, 314)
  wmPlaceTile(plPlane, iX, iY, 315)
  wmPlaceTile(plPlane, iX, iY+1, 48)
  wmPlaceTile(plPlane, iX, iY+2, 107)
  wmCreateObject('GooCoverup', 'LEVEL_GOORIGHT', iX*64+32, iY*64+153, 8000)
  randbub = wmRandInt(0,3)
  if randbub==1 then wmCreateObject('BehindAniCandy', 'LEVEL_SINGLEBUBBLE', iX*64+wmRandInt(0,16)-wmRandInt(0,12), iY*64+46+wmRandInt(0,4))
  elseif randbub==0 then wmCreateObject('BehindAniCandy', 'LEVEL_TRIPLEBUBBLE', iX*64+wmRandInt(0,20)-wmRandInt(0,12), iY*64+46+wmRandInt(0,4)) end
 end
 elseif wmGetTile(plPlane, iX+1, iY) == 313 then
  hObjects = wmGetObjectsInArea(iX*64+63, iY*64+152, iX*64+65, iY*64+154)
   if hObjects ~= nil then
    for i=0, table.getn(hObjects) do
     if wmGetObjectLogic(hObjects[i])=='GooCoverup' and wmGetObjectImageSet(hObjects[i])=='LEVEL_GOOLEFT' then
      wmDeleteObject(hObjects[i])
      wmCreateObject('GooCoverup', 'LEVEL_GOOCENTER', iX*64+96, iY*64+153, 8000)
     end
    end
  end
  wmPlaceTile(plPlane, iX+1, iY, 314)
  wmPlaceTile(plPlane, iX, iY, 313)
  wmPlaceTile(plPlane, iX, iY+1, 48)
  wmPlaceTile(plPlane, iX, iY+2, 107)
  wmCreateObject('GooCoverup', 'LEVEL_GOOLEFT', iX*64+32, iY*64+153, 8000)
  randbub = wmRandInt(0,3)
  if randbub==1 then wmCreateObject('BehindAniCandy', 'LEVEL_SINGLEBUBBLE', iX*64+32+wmRandInt(0,16)-wmRandInt(0,12), iY*64+46+wmRandInt(0,4))
  elseif randbub==0 then wmCreateObject('BehindAniCandy', 'LEVEL_TRIPLEBUBBLE', iX*64+42+wmRandInt(0,30)-wmRandInt(0,12), iY*64+46+wmRandInt(0,4)) end
 elseif wmGetTile(plPlane, iX, iY) == 32 and wmGetTile(plPlane, iX-1, iY) == 32 and wmGetTile(plPlane, iX+1, iY) == 32 then
  wmPlaceTile(plPlane, iX-1, iY, 313)
  wmPlaceTile(plPlane, iX, iY, 314)
  wmPlaceTile(plPlane, iX+1, iY, 315)
  wmPlaceTile(plPlane, iX, iY+1, 48)
  wmPlaceTile(plPlane, iX, iY+2, 107)
  wmPlaceTile(plPlane, iX-1, iY+1, 48)
  wmPlaceTile(plPlane, iX-1, iY+2, 107)
  wmPlaceTile(plPlane, iX+1, iY+1, 48)
  wmPlaceTile(plPlane, iX+1, iY+2, 107)
  wmCreateObject('GooCoverup', 'LEVEL_GOOLEFT', iX*64-32, iY*64+153, 8000)
  wmCreateObject('GooCoverup', 'LEVEL_GOOCENTER', iX*64+32, iY*64+153, 8000)
  wmCreateObject('GooCoverup', 'LEVEL_GOORIGHT', iX*64+96, iY*64+153, 8000)
  randbub = wmRandInt(0,3)
  if randbub==1 then wmCreateObject('BehindAniCandy', 'LEVEL_SINGLEBUBBLE', iX*64+wmRandInt(0,16)-wmRandInt(0,12), iY*64+46+wmRandInt(0,4))
  elseif randbub==0 then wmCreateObject('BehindAniCandy', 'LEVEL_TRIPLEBUBBLE', iX*64+wmRandInt(0,40)-wmRandInt(0,12), iY*64+46+wmRandInt(0,4)) end
  elseif randbub==2 then wmCreateObject('BehindAniCandy', 'LEVEL_TRIPLEBUBBLE', iX*64-20-wmRandInt(0,40)-wmRandInt(0,12), iY*64+46+wmRandInt(0,4)) end
 elseif wmGetTile(plPlane, iX, iY) == 32 and wmGetTile(plPlane, iX-1, iY) == 32 then
  wmPlaceTile(plPlane, iX-1, iY, 313)
  wmPlaceTile(plPlane, iX, iY, 315)
  wmPlaceTile(plPlane, iX, iY+1, 48)
  wmPlaceTile(plPlane, iX, iY+2, 107)
  wmPlaceTile(plPlane, iX-1, iY+1, 48)
  wmPlaceTile(plPlane, iX-1, iY+2, 107)
  wmCreateObject('GooCoverup', 'LEVEL_GOOLEFT', iX*64-32, iY*64+153, 8000)
  wmCreateObject('GooCoverup', 'LEVEL_GOORIGHT', iX*64+32, iY*64+153, 8000)
 elseif wmGetTile(plPlane, iX, iY) == 32 and wmGetTile(plPlane, iX+1, iY) == 32 then
  wmPlaceTile(plPlane, iX, iY, 313)
  wmPlaceTile(plPlane, iX+1, iY, 315)
  wmPlaceTile(plPlane, iX, iY+1, 48)
  wmPlaceTile(plPlane, iX, iY+2, 107)
  wmPlaceTile(plPlane, iX+1, iY+1, 48)
  wmPlaceTile(plPlane, iX+1, iY+2, 107)
  wmCreateObject('GooCoverup', 'LEVEL_GOOLEFT', iX*64+32, iY*64+153, 8000)
  wmCreateObject('GooCoverup', 'LEVEL_GOORIGHT', iX*64+96, iY*64+153, 8000)
 end

if wmGetTile(plPlane, iX-1, iY) == 315 and wmGetTile(plPlane, iX, iY) == 313 then
 wmPlaceTile(plPlane, iX-1, iY, 314)
 wmPlaceTile(plPlane, iX, iY, 314)
  wmPlaceTile(plPlane, iX, iY+1, 48)
  wmPlaceTile(plPlane, iX, iY+2, 107)
  wmPlaceTile(plPlane, iX-1, iY+1, 48)
  wmPlaceTile(plPlane, iX-1, iY+2, 107)
  hObjects = wmGetObjectsInArea(iX*64-33, iY*64+152, iX*64+33, iY*64+154)
   if hObjects ~= nil then
    for i=0, table.getn(hObjects) do
     if wmGetObjectLogic(hObjects[i])=='GooCoverup' and (wmGetObjectImageSet(hObjects[i])=='LEVEL_GOORIGHT' or wmGetObjectImageSet(hObjects[i])=='LEVEL_GOOLEFT') then
      wmCreateObject('GooCoverup', 'LEVEL_GOOCENTER', wmGetObjectParam(hObjects[i], wmobjPosX), wmGetObjectParam(hObjects[i], wmobjPosY), 8000)
      wmDeleteObject(hObjects[i])
     end
    end
   end
elseif wmGetTile(plPlane, iX+1, iY) == 313 and wmGetTile(plPlane, iX, iY) == 315 then
 wmPlaceTile(plPlane, iX, iY, 314)
 wmPlaceTile(plPlane, iX+1, iY, 314)
  wmPlaceTile(plPlane, iX, iY+1, 48)
  wmPlaceTile(plPlane, iX, iY+2, 107)
  wmPlaceTile(plPlane, iX+1, iY+1, 48)
  wmPlaceTile(plPlane, iX+1, iY+2, 107)
  hObjects = wmGetObjectsInArea(iX*64+31, iY*64+152, iX*64+66, iY*64+154)
   if hObjects ~= nil then
    for i=0, table.getn(hObjects) do
     if wmGetObjectLogic(hObjects[i])=='GooCoverup' and (wmGetObjectImageSet(hObjects[i])=='LEVEL_GOORIGHT' or wmGetObjectImageSet(hObjects[i])=='LEVEL_GOOLEFT') then
      wmCreateObject('GooCoverup', 'LEVEL_GOOCENTER', wmGetObjectParam(hObjects[i], wmobjPosX), wmGetObjectParam(hObjects[i], wmobjPosY), 8000)
      wmDeleteObject(hObjects[i])
     end
    end
   end
--elseif wmGetTile(plPlane, iX+2, iY) == 313 and wmGetTile(plPlane, iX+1, iY) == 315 then
-- wmPlaceTile(plPlane, iX+1, iY, 314)
-- wmPlaceTile(plPlane, iX+2, iY, 314)
end
end
