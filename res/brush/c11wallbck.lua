gameID = wmCLAW
layerName = "ACTION"
levelID = 11

brushName_PL = "Œciana+t³o"
brushName = "Wall+bg"
brushIcon = 47

function cbBrushPlaced(plPlane, iX, iY)

if wmGetTile(plPlane, iX, iY) ~= 33
   then
	wmPlaceTile(plPlane, iX, iY, 33)
end
if wmGetTile(plPlane, iX, iY+1) ~= 33
   then
	wmPlaceTile(plPlane, iX, iY+1, 33)
end

if wmGetTile(plPlane, iX-1, iY+2) ~= 33 and
   wmGetTile(plPlane, iX-1, iY+2) ~= 52 and
   wmGetTile(plPlane, iX-1, iY+2) ~= 23 and
   wmGetTile(plPlane, iX-1, iY+2) ~= 25 and
   wmGetTile(plPlane, iX-1, iY+2) ~= 60 and
   wmGetTile(plPlane, iX-1, iY+2) ~= 11 and
   wmGetTile(plPlane, iX-1, iY+2) ~= 51 and
   wmGetTile(plPlane, iX-1, iY+2) ~= 53 and
   wmGetTile(plPlane, iX-1, iY+2) ~= 15 and
   wmGetTile(plPlane, iX-1, iY+2) ~= 53 and
   wmGetTile(plPlane, iX-1, iY+2) ~= 15 and
   wmGetTile(plPlane, iX-1, iY+2) ~= 27
   then
	wmPlaceTile(plPlane, iX-1, iY+2, 76)
end
if wmGetTile(plPlane, iX+1, iY+2) ~= 33 and
   wmGetTile(plPlane, iX+1, iY+2) ~= 52 and
   wmGetTile(plPlane, iX+1, iY+2) ~= 23 and
   wmGetTile(plPlane, iX+1, iY+2) ~= 25 and
   wmGetTile(plPlane, iX+1, iY+2) ~= 11 and
   wmGetTile(plPlane, iX+1, iY+2) ~= 51 and
   wmGetTile(plPlane, iX+1, iY+2) ~= 53 and
   wmGetTile(plPlane, iX+1, iY+2) ~= 15 and
   wmGetTile(plPlane, iX+1, iY+2) ~= 11 and
   wmGetTile(plPlane, iX+1, iY+2) ~= 53 and
   wmGetTile(plPlane, iX+1, iY+2) ~= 15 and
   wmGetTile(plPlane, iX+1, iY+2) ~= 31
   then
	wmPlaceTile(plPlane, iX+1, iY+2, 77)
end
if wmGetTile(plPlane, iX-1, iY+1) ~= 33 and
   wmGetTile(plPlane, iX-1, iY+1) ~= 52 and
   wmGetTile(plPlane, iX-1, iY+1) ~= 27
   then
	wmPlaceTile(plPlane, iX-1, iY+1, 23)
end

if wmGetTile(plPlane, iX+1, iY) ~= 33 and
   wmGetTile(plPlane, iX+1, iY) ~= 27 and
   wmGetTile(plPlane, iX+1, iY) ~= 11 and
   wmGetTile(plPlane, iX+1, iY) ~= 53 and
   wmGetTile(plPlane, iX+1, iY) ~= 15 and
   wmGetTile(plPlane, iX+1, iY) ~= 31
   then
	wmPlaceTile(plPlane, iX+1, iY, 25)
end
if wmGetTile(plPlane, iX-1, iY) ~= 33 and
   wmGetTile(plPlane, iX-1, iY) ~= 53 and
   wmGetTile(plPlane, iX-1, iY) ~= 15 and
   wmGetTile(plPlane, iX-1, iY) ~= 27
   then
	wmPlaceTile(plPlane, iX-1, iY, 23)
end
if wmGetTile(plPlane, iX+1, iY+1) ~= 33 and
   wmGetTile(plPlane, iX+1, iY+1) ~= 27 and
   wmGetTile(plPlane, iX+1, iY+1) ~= 11 and
   wmGetTile(plPlane, iX+1, iY+1) ~= 53 and
   wmGetTile(plPlane, iX+1, iY+1) ~= 15 and
   wmGetTile(plPlane, iX+1, iY+1) ~= 31
   then
	wmPlaceTile(plPlane, iX+1, iY+1, 25)
end
if wmGetTile(plPlane, iX-1, iY+1) ~= 33 and
   wmGetTile(plPlane, iX-1, iY+1) ~= 27 and
   wmGetTile(plPlane, iX-1, iY+1) ~= 53 and
   wmGetTile(plPlane, iX-1, iY+1) ~= 15 and
   wmGetTile(plPlane, iX-1, iY+1) ~= 53 and
   wmGetTile(plPlane, iX-1, iY+1) ~= 15 and
   wmGetTile(plPlane, iX-1, iY+1) ~= 31
   then
	wmPlaceTile(plPlane, iX-1, iY+1, 23)
end

if wmGetTile(plPlane, iX, iY-1) ~= 33 and
   wmGetTile(plPlane, iX, iY-1) ~= 23 and
   wmGetTile(plPlane, iX, iY-1) ~= 25 and
   wmGetTile(plPlane, iX, iY-2) == 60
   then
	wmPlaceTile(plPlane, iX, iY-1, 68)
	wmPlaceTile(plPlane, iX-1, iY-1, 31)
	wmPlaceTile(plPlane, iX, iY-2, 63)
else
if wmGetTile(plPlane, iX, iY-1) ~= 33 and
   wmGetTile(plPlane, iX, iY-1) ~= 23 and
   wmGetTile(plPlane, iX, iY-1) ~= 25 and
   wmGetTile(plPlane, iX, iY-2) == 48
   then
	wmPlaceTile(plPlane, iX, iY-1, 65)
	wmPlaceTile(plPlane, iX+1, iY-1, 27)
	wmPlaceTile(plPlane, iX, iY-2, 61)
else
if wmGetTile(plPlane, iX, iY-1) ~= 33 and
   wmGetTile(plPlane, iX, iY-1) ~= 23 and
   wmGetTile(plPlane, iX, iY-1) ~= 31 and
   wmGetTile(plPlane, iX, iY-1) ~= 25 and
   wmGetTile(plPlane, iX, iY-1) ~= 27 and
   wmGetTile(plPlane, iX, iY-1) ~= 68 and
   wmGetTile(plPlane, iX, iY-1) ~= 65
   then
	wmPlaceTile(plPlane, iX, iY-1, 42)
end
end
end

if wmGetTile(plPlane, iX-1, iY-1) ~= 33 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 23 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 25 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 68 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 65 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 42 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 27 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 53 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 15 and
   wmGetTile(plPlane, iX-1, iY-1) ~= 31
   then
	wmPlaceTile(plPlane, iX-1, iY-1, 41)
end
if wmGetTile(plPlane, iX+1, iY-1) ~= 33 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 23 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 25 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 68 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 65 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 42 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 27 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 11 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 53 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 15 and
   wmGetTile(plPlane, iX+1, iY-1) ~= 31
   then
	wmPlaceTile(plPlane, iX+1, iY-1, 46)
end

if wmGetTile(plPlane, iX-2, iY-1) ~= 33 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 42 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 41 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 60 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 48 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 23 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 68 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 65 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 25 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 27 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 61 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 63 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 53 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 15 and
   wmGetTile(plPlane, iX-2, iY-1) ~= 31
   then
	wmPlaceTile(plPlane, iX-2, iY-1, 40)
end
if wmGetTile(plPlane, iX+2, iY-1) ~= 33 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 42 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 46 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 60 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 48 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 25 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 68 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 65 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 23 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 27 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 61 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 63 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 51 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 11 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 53 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 15 and
   wmGetTile(plPlane, iX+2, iY-1) ~= 31
   then
	wmPlaceTile(plPlane, iX+2, iY-1, 47)
end

if wmGetTile(plPlane, iX+2, iY) ~= 33 and
   wmGetTile(plPlane, iX+2, iY) == 42 and
   wmGetTile(plPlane, iX+2, iY) ~= 46 and
   wmGetTile(plPlane, iX+2, iY) ~= 48 and
   wmGetTile(plPlane, iX+2, iY) ~= 25 and
   wmGetTile(plPlane, iX+2, iY) ~= 23
   then
	wmPlaceTile(plPlane, iX+2, iY, 68)
	wmPlaceTile(plPlane, iX+1, iY, 31)
	wmPlaceTile(plPlane, iX+2, iY-1, 63)
else
if wmGetTile(plPlane, iX+2, iY) ~= 33 and
   wmGetTile(plPlane, iX+2, iY) == 60 and
   wmGetTile(plPlane, iX+3, iY) == 42 and
   wmGetTile(plPlane, iX+2, iY) ~= 46 and
   wmGetTile(plPlane, iX+2, iY) ~= 48 and
   wmGetTile(plPlane, iX+2, iY) ~= 25 and
   wmGetTile(plPlane, iX+2, iY) ~= 23
   then
	wmPlaceTile(plPlane, iX+2, iY, 68)
	wmPlaceTile(plPlane, iX+1, iY, 31)
	wmPlaceTile(plPlane, iX+2, iY-1, 63)
else
if wmGetTile(plPlane, iX+2, iY) ~= 33 and
   wmGetTile(plPlane, iX+2, iY) ~= 42 and
   wmGetTile(plPlane, iX+2, iY) ~= 46 and
   wmGetTile(plPlane, iX+2, iY) ~= 48 and
   wmGetTile(plPlane, iX+2, iY) ~= 25 and
   wmGetTile(plPlane, iX+2, iY) ~= 68 and
   wmGetTile(plPlane, iX+2, iY) ~= 65 and
   wmGetTile(plPlane, iX+2, iY) ~= 61 and
   wmGetTile(plPlane, iX+2, iY) ~= 63 and
   wmGetTile(plPlane, iX+2, iY) ~= 27 and
   wmGetTile(plPlane, iX+2, iY) ~= 31 and
   wmGetTile(plPlane, iX+2, iY) ~= 51 and
   wmGetTile(plPlane, iX+2, iY) ~= 11 and
   wmGetTile(plPlane, iX+2, iY) ~= 53 and
   wmGetTile(plPlane, iX+2, iY) ~= 15 and
   wmGetTile(plPlane, iX+2, iY) ~= 23
   then
	wmPlaceTile(plPlane, iX+2, iY, 60)
end
end
end
if wmGetTile(plPlane, iX-2, iY) ~= 33 and
   wmGetTile(plPlane, iX-2, iY) == 42 and
   wmGetTile(plPlane, iX-2, iY) ~= 46 and
   wmGetTile(plPlane, iX-2, iY) ~= 48 and
   wmGetTile(plPlane, iX-2, iY) ~= 25 and
   wmGetTile(plPlane, iX-2, iY) ~= 23
   then
	wmPlaceTile(plPlane, iX-2, iY, 65)
	wmPlaceTile(plPlane, iX-1, iY, 27)
	wmPlaceTile(plPlane, iX-2, iY-1, 61)
else
if wmGetTile(plPlane, iX-2, iY) ~= 33 and
   wmGetTile(plPlane, iX-2, iY) == 48 and
   wmGetTile(plPlane, iX-3, iY) == 42 and
   wmGetTile(plPlane, iX-2, iY) ~= 46 and
   wmGetTile(plPlane, iX-2, iY) ~= 48 and
   wmGetTile(plPlane, iX-2, iY) ~= 25 and
   wmGetTile(plPlane, iX-2, iY) ~= 23
   then
	wmPlaceTile(plPlane, iX-2, iY, 65)
	wmPlaceTile(plPlane, iX-1, iY, 27)
	wmPlaceTile(plPlane, iX-2, iY-1, 63)
	
else
if wmGetTile(plPlane, iX-2, iY) ~= 33 and
   wmGetTile(plPlane, iX-2, iY) ~= 42 and
   wmGetTile(plPlane, iX-2, iY) ~= 46 and
   wmGetTile(plPlane, iX-2, iY) ~= 48 and
   wmGetTile(plPlane, iX-2, iY) ~= 25 and
   wmGetTile(plPlane, iX-2, iY) ~= 68 and
   wmGetTile(plPlane, iX-2, iY) ~= 65 and
   wmGetTile(plPlane, iX-2, iY) ~= 61 and
   wmGetTile(plPlane, iX-2, iY) ~= 63 and
   wmGetTile(plPlane, iX-2, iY) ~= 27 and
   wmGetTile(plPlane, iX-2, iY) ~= 31 and
   wmGetTile(plPlane, iX-2, iY) ~= 53 and
   wmGetTile(plPlane, iX-2, iY) ~= 15 and
   wmGetTile(plPlane, iX-2, iY) ~= 23
   then
	wmPlaceTile(plPlane, iX-2, iY, 48)
end
end
end

if wmGetTile(plPlane, iX-2, iY+1) ~= 33 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 42 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 46 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 48 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 25 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 60 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 27 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 31 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 65 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 68 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 61 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 63 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 53 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 15 and
   wmGetTile(plPlane, iX-2, iY+1) ~= 23
   then
	wmPlaceTile(plPlane, iX-2, iY+1, 72)
end
if wmGetTile(plPlane, iX+2, iY+1) ~= 33 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 42 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 46 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 48 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 25 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 60 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 27 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 31 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 65 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 68 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 61 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 63 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 51 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 11 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 53 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 15 and
   wmGetTile(plPlane, iX+2, iY+1) ~= 23
   then
	wmPlaceTile(plPlane, iX+2, iY+1, 74)
end

if wmGetTile(plPlane, iX-2, iY+2) ~= 33 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 42 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 46 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 48 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 25 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 60 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 72 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 74 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 52 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 27 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 31 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 65 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 68 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 61 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 63 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 76 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 77 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 11 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 51 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 53 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 15 and
   wmGetTile(plPlane, iX-2, iY+2) ~= 23
   then
	wmPlaceTile(plPlane, iX-2, iY+2, 75)
end
if wmGetTile(plPlane, iX+2, iY+2) ~= 33 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 42 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 46 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 48 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 25 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 60 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 72 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 74 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 52 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 27 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 31 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 65 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 68 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 61 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 63 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 76 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 77 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 11 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 51 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 53 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 15 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 51 and
   wmGetTile(plPlane, iX+2, iY+2) ~= 23
   then
	wmPlaceTile(plPlane, iX+2, iY+2, 78)
end

if wmGetTile(plPlane, iX, iY+2) ~= 33 and
   wmGetTile(plPlane, iX, iY+2) == 60
then
	wmPlaceTile(plPlane, iX, iY+2, 51)
	wmPlaceTile(plPlane, iX-1, iY+2, 11)
else
if wmGetTile(plPlane, iX, iY+2) ~= 33 and
   wmGetTile(plPlane, iX, iY+2) == 48
then
	wmPlaceTile(plPlane, iX, iY+2, 53)
	wmPlaceTile(plPlane, iX+1, iY+2, 15)
else
if wmGetTile(plPlane, iX, iY+2) ~= 33 and
   wmGetTile(plPlane, iX, iY+2) ~= 11 and
   wmGetTile(plPlane, iX, iY+2) ~= 51 and
   wmGetTile(plPlane, iX, iY+2) ~= 53 and
   wmGetTile(plPlane, iX, iY+2) ~= 15
   then
	wmPlaceTile(plPlane, iX, iY+2, 52)
end
end
end
if wmGetTile(plPlane, iX+3, iY) == 52 and
   wmGetTile(plPlane, iX+2, iY) == 60
then
	wmPlaceTile(plPlane, iX+2, iY, 51)
	wmPlaceTile(plPlane, iX+1, iY, 11)
end
if wmGetTile(plPlane, iX-3, iY) == 52 and
   wmGetTile(plPlane, iX-2, iY) == 48
then
	wmPlaceTile(plPlane, iX-2, iY, 53)
	wmPlaceTile(plPlane, iX-1, iY, 15)
end

end