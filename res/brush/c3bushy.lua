--brush by Zax37 20.04.2011
gameID = wmCLAW
layerName = "ACTION"
levelID = 3

brushName_PL = "Pagórek"
brushName = "Hill"
brushIcon = 601

function checkGroup(plPlane, Xx, Yy)
if wmGetTile(plPlane, Xx, Yy)==10 or wmGetTile(plPlane, Xx, Yy)==11 or wmGetTile(plPlane, Xx, Yy)==14 or wmGetTile(plPlane, Xx, Yy)==15 or wmGetTile(plPlane, Xx, Yy)==601 or wmGetTile(plPlane, Xx, Yy)==602 or wmGetTile(plPlane, Xx, Yy)==8 or wmGetTile(plPlane, Xx, Yy)==9 then return 1
else return 0
end
end

function cbBrushPlaced(plPlane, iX, iY)
if checkGroup(plPlane, iX, iY)==0 and checkGroup(plPlane, iX-1, iY)==0 and checkGroup(plPlane, iX+1, iY)==0 then
 wmPlaceTile(plPlane, iX, iY, 601+wmRandInt(0,1))

elseif wmGetTile(plPlane, iX, iY)==wmEMPTY and wmGetTile(plPlane, iX-1, iY)==9 and wmGetTile(plPlane, iX+1, iY)==8 then
 wmPlaceTile(plPlane, iX-1, iY, 601+wmRandInt(0,1))
 wmPlaceTile(plPlane, iX, iY, 601+wmRandInt(0,1))
 wmPlaceTile(plPlane, iX+1, iY, 601+wmRandInt(0,1))

elseif wmGetTile(plPlane, iX-1, iY)==9 or (checkGroup(plPlane, iX-1, iY)==1 and wmGetTile(plPlane, iX, iY)==wmEMPTY) then
 if checkGroup(plPlane, iX-2, iY)==1 then
 wmPlaceTile(plPlane, iX-1, iY, 601+wmRandInt(0,1))
 else wmPlaceTile(plPlane, iX-1, iY, 8)
 end
 if checkGroup(plPlane, iX+1, iY)==0 then
 wmPlaceTile(plPlane, iX, iY, 9)
 else wmPlaceTile(plPlane, iX, iY, 601+wmRandInt(0,1))
 end

elseif wmGetTile(plPlane, iX+1, iY)==8 or (checkGroup(plPlane, iX+1, iY)==1 and wmGetTile(plPlane, iX, iY)==wmEMPTY) then
 if checkGroup(plPlane, iX+2, iY)==1 then
 wmPlaceTile(plPlane, iX+1, iY, 601+wmRandInt(0,1))
 else wmPlaceTile(plPlane, iX+1, iY, 9)
 end
 if checkGroup(plPlane, iX-1, iY)==0 then
 wmPlaceTile(plPlane, iX, iY, 8)
 else wmPlaceTile(plPlane, iX, iY, 601+wmRandInt(0,1))
 end

end
end
