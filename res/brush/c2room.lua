gameID=wmCLAW
layerName="ACTION"
levelID=2
brushName_PL="Pokoj"
brushName = "Room"
brushIcon=507

function ins(t)
 return(t==097 or t==506 or t==500 or t==505 or t==507 or t==096 or t==508)
end

function tl(t)
 return(t==97)
end

function ml(t)
 return(t==97)
end

function bl(t)
 return(t==506)
end

function tm(t)
 return(t==500)
end

function mm(t)
 return(t==505)
end

function bm(t)
 return(t==507)
end

function tr(t)
 return(t==96)
end

function mr(t)
 return(t==96)
end

function br(t)
 return(t==508)
end

function t(wha)
 if wha=='tl' or wha=='ml' then return 97 end
 if wha=='tr' or wha=='mr' then return 96 end
 if wha=='bl' then return 506 end
 if wha=='bm' then return 507 end
 if wha=='br' then return 508 end
 if wha=='mm' then return 505 end
 if wha=='tm' then return 500 end
end

function cbBrushPlaced(plPlane, iX, iY) 

if tm(wmGetTile(plPlane, iX, iY)) or bm(wmGetTile(plPlane, iX, iY)) then wmPlaceTile(plPlane, iX, iY, t('mm')) end
if tl(wmGetTile(plPlane, iX-1, iY)) or bl(wmGetTile(plPlane, iX, iY)) then wmPlaceTile(plPlane, iX-1, iY, t('ml')) end
if bl(wmGetTile(plPlane, iX-1, iY)) then wmPlaceTile(plPlane, iX-1, iY, t('ml')) end
if br(wmGetTile(plPlane, iX+1, iY)) then wmPlaceTile(plPlane, iX+1, iY, t('mr')) end
if tm(wmGetTile(plPlane, iX, iY+1)) then wmPlaceTile(plPlane, iX, iY+1, t('mm')) end

if tl(wmGetTile(plPlane, iX, iY-1)) or tr(wmGetTile(plPlane, iX, iY-1)) then wmPlaceTile(plPlane, iX, iY-1, t('tm')) end
if ml(wmGetTile(plPlane, iX, iY)) or mr(wmGetTile(plPlane, iX, iY)) then wmPlaceTile(plPlane, iX, iY, t('mm')) end
if ml(wmGetTile(plPlane, iX, iY+1)) or mr(wmGetTile(plPlane, iX, iY+1)) then wmPlaceTile(plPlane, iX, iY+1, t('mm')) end

if (tl(wmGetTile(plPlane, iX-1, iY-1)) or tr(wmGetTile(plPlane, iX-1, iY-1))) and tm(wmGetTile(plPlane, iX-2, iY-1)) then wmPlaceTile(plPlane, iX-1, iY-1, t('tm')) end
if (ml(wmGetTile(plPlane, iX-1, iY-1)) or mr(wmGetTile(plPlane, iX-1, iY-1))) and mm(wmGetTile(plPlane, iX-2, iY-1)) then wmPlaceTile(plPlane, iX-1, iY-1, t('mm')) end
if (ml(wmGetTile(plPlane, iX-1, iY)) or mr(wmGetTile(plPlane, iX-1, iY))) and mm(wmGetTile(plPlane, iX-2, iY)) then wmPlaceTile(plPlane, iX-1, iY, t('mm')) end
if (ml(wmGetTile(plPlane, iX-1, iY+1)) or mr(wmGetTile(plPlane, iX-1, iY+1))) and mm(wmGetTile(plPlane, iX-2, iY+1)) then wmPlaceTile(plPlane, iX-1, iY+1, t('mm')) end
if (bl(wmGetTile(plPlane, iX-1, iY+1)) or br(wmGetTile(plPlane, iX-1, iY+1))) and bm(wmGetTile(plPlane, iX-2, iY+1)) then wmPlaceTile(plPlane, iX-1, iY+1, t('bm')) end

if bl(wmGetTile(plPlane, iX, iY+1)) or br(wmGetTile(plPlane, iX, iY+1)) then wmPlaceTile(plPlane, iX, iY+1, t('bm')) end

if (bl(wmGetTile(plPlane, iX-1, iY)) or br(wmGetTile(plPlane, iX-1, iY)) or bm(wmGetTile(plPlane, iX-1, iY))) then wmPlaceTile(plPlane, iX-1, iY, t('mm')) end
if (bl(wmGetTile(plPlane, iX+1, iY)) or br(wmGetTile(plPlane, iX+1, iY)) or bm(wmGetTile(plPlane, iX+1, iY))) then wmPlaceTile(plPlane, iX+1, iY, t('mm')) end

if not ins(wmGetTile(plPlane, iX-1, iY-1)) then wmPlaceTile(plPlane, iX-1, iY-1, t('tl')) end
if not ins(wmGetTile(plPlane, iX, iY-1)) then wmPlaceTile(plPlane, iX, iY-1, t('tm')) end
if not ins(wmGetTile(plPlane, iX+1, iY-1)) then wmPlaceTile(plPlane, iX+1, iY-1, t('tr')) end

if not ins(wmGetTile(plPlane, iX-1, iY)) then wmPlaceTile(plPlane, iX-1, iY, t('ml')) end
wmPlaceTile(plPlane, iX, iY, t('mm'))
if not ins(wmGetTile(plPlane, iX+1, iY)) then wmPlaceTile(plPlane, iX+1, iY, t('mr')) end

if not ins(wmGetTile(plPlane, iX-1, iY+1)) then wmPlaceTile(plPlane, iX-1, iY+1, t('bl')) end
if not ins(wmGetTile(plPlane, iX, iY+1)) then wmPlaceTile(plPlane, iX, iY+1, t('bm')) end
if not ins(wmGetTile(plPlane, iX+1, iY+1)) then wmPlaceTile(plPlane, iX+1, iY+1, t('br')) end

end