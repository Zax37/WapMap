--Ground brush by Zax37 02.10.2010
gameID = wmCLAW
layerName = "ACTION"
levelID = 3

brushName_PL = "Ziemia"
brushName = "Ground"
brushIcon = 630
enableSettings = 1

function cbInitSettings()
 if wmGetLangCode() == "PL" then
  cbGrass = wmAddSetting(wmCheckbox, "Trawa")
 else cbGrass = wmAddSetting(wmCheckbox, "Grass")
 end
end

function t(wha)
 if wha=="tl" then
  if wmGetSettingValue(cbGrass)==1 then return 603
  else return 620
  end
 elseif wha=="tl2" then
  if wmGetSettingValue(cbGrass)==1 then return 604
  else return 621
  end
 elseif wha=="tm" then
  if wmGetSettingValue(cbGrass)==1 then return 605+wmRandInt(0,1)
  else return 622
  end
 elseif wha=="tr" then
  if wmGetSettingValue(cbGrass)==1 then return 607
  else return 623
  end
 elseif wha=="ml" then
  if wmGetSettingValue(cbGrass)==1 then return 612
  else return 624
  end
 elseif wha=="ml2" then return 625
 elseif wha=="mm" then return 630
 elseif wha=="mr" then
  if wmGetSettingValue(cbGrass)==1 then return 611
  else return 631
  end
 elseif wha=="bl" then return 632
 elseif wha=="bl2" then return 633
 elseif wha=="bm" then return 634
 elseif wha=="br" then return 635
 elseif wha=="ctl" then return 617
 elseif wha=="ctr" then return 618
 elseif wha=="ctr2" then return 619 
 elseif wha=="cbl" then
  if wmGetSettingValue(cbGrass)==1 then return 626
  else return 638 
  end
 elseif wha=="cbr" then return 642
 elseif wha=="cbr2" then return 643
 end
end

function ins(t)
 return((t>=603 and t<=643) or t==666 or t==685 or (t>=689 and t<=696) or  t>=701)
end

function tl(t)
 return(t==603 or t==620)
end

function tl2(t)
 return(t==604 or t==621)
end 

function tm(t)
 return(t==605 or t==606 or t==622 or (t>=639 and t<=641) or t==666 or t==692 or t==704 or t==692 or t==704) 
end

function tr(t)
 return(t==607 or t==623 or t==627) 
end

function ml(t)
 return(t==608 or t==609 or t==612 or t==624 or t==628 or t==689 or t==701) 
end

function ml2(t)
 return(t==613 or t==625 or t==629 or t==690 or t==702) 
end

function mr(t)
 return(t==611 or t==615 or t==631 or t==685 or (t>=710 and t<=712)) 
end

function mm(t)
 return(t==630)
end

function bl(t)
 return(t==632) 
end

function bl2(t)
 return(t==633) 
end

function bm(t)
 return(t==634) 
end

function br(t)
 return(t==635) 
end

function ctl(t)
 return(t==617) 
end

function ctr(t)
 return(t==618) 
end

function ctr2(t)
 return(t==619) 
end

function cbl(t)
 return(t==626 or t==638 or t==691 or t==713) 
end

function cbr(t)
 return(t==642 or t==692 or t==704)
end

function cbr2(t)
 return(t==643 or t==696 or t==709)
end

function placetl(plPlane, iX, iY)

 if tr(wmGetTile(plPlane, iX-2, iY-1)) then
  wmPlaceTile(plPlane, iX-2, iY-1, t("tm"))
 elseif ctl(wmGetTile(plPlane, iX-2, iY-1)) then
  if cbr2(wmGetTile(plPlane, iX-3, iY-1)) or mm(wmGetTile(plPlane, iX-3, iY-1)) or br(wmGetTile(plPlane, iX-1, iY-1)) or bm(wmGetTile(plPlane, iX-1, iY-1)) then
   wmPlaceTile(plPlane, iX-2, iY-1, t("mm"))
  else wmPlaceTile(plPlane, iX-2, iY-1, t("cbr2"))
  end
 elseif  mr(wmGetTile(plPlane, iX-2, iY-1)) then
  wmPlaceTile(plPlane, iX-2, iY-1, t("cbl"))
 elseif bl(wmGetTile(plPlane, iX-2, iY-1)) then
  wmPlaceTile(plPlane, iX-2, iY-1, t("ml"))
 elseif bl2(wmGetTile(plPlane, iX-2, iY-1)) or bm(wmGetTile(plPlane, iX-2, iY-1)) then
  wmPlaceTile(plPlane, iX-2, iY-1, t("ctr"))
 elseif br(wmGetTile(plPlane, iX-2, iY-1)) then
  placetm(plPlane, iX-2, iY-1)
  placetr(plPlane, iX-2, iY-1)
  wmPlaceTile(plPlane, iX-2, iY-1, t("ctr"))
 elseif not ins(wmGetTile(plPlane, iX-2, iY-1)) then
  wmPlaceTile(plPlane, iX-2, iY-1, t("tl"))
 end

end

function placetl2(plPlane, iX, iY)

if ctr(wmGetTile(plPlane, iX-2, iY-1)) and not tr(wmGetTile(plPlane, iX-1, iY-2)) and not ctl(wmGetTile(plPlane, iX-1, iY-2)) then
  wmPlaceTile(plPlane, iX-1, iY-1, t("ctr2"))
elseif tr(wmGetTile(plPlane, iX-1, iY-2)) and ctr(wmGetTile(plPlane, iX-2, iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("cbl"))
elseif ml(wmGetTile(plPlane, iX-1, iY-1)) or bl(wmGetTile(plPlane, iX-1, iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("cbr"))
elseif ml(wmGetTile(plPlane, iX-2, iY-1)) then
  wmPlaceTile(plPlane, iX-1, iY-1, t("ml2"))
elseif ml2(wmGetTile(plPlane, iX-1, iY-1)) and not ml(wmGetTile(plPlane, iX-2, iY-1)) then
  wmPlaceTile(plPlane, iX-1, iY-1, t("mm"))
elseif mr(wmGetTile(plPlane, iX-1, iY-1)) then
  wmPlaceTile(plPlane, iX-1, iY-1, t("cbl"))
elseif tr(wmGetTile(plPlane, iX-1, iY-1)) then
  wmPlaceTile(plPlane, iX-1, iY-1, t("tm"))
elseif ctr(wmGetTile(plPlane, iX-1, iY-1)) and mm(wmGetTile(plPlane, iX-1, iY-2)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("mm"))
elseif ctr(wmGetTile(plPlane, iX-1, iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("ctr2"))
elseif bm(wmGetTile(plPlane, iX-1, iY-1)) and ctr(wmGetTile(plPlane, iX-2, iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("ctr2"))
elseif bm(wmGetTile(plPlane, iX-1, iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("mm"))
elseif br(wmGetTile(plPlane, iX-1, iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("cbl"))
elseif tl(wmGetTile(plPlane, iX-1, iY-1)) and cbl(wmGetTile(plPlane, iX-2, iY-1)) or tm(wmGetTile(plPlane, iX-2, iY-1)) and not cbr(wmGetTile(plPlane, iX-1, iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("tm"))
elseif tl(wmGetTile(plPlane, iX-1, iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("tl2"))
elseif ml(wmGetTile(plPlane, iX-2, iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("ml2"))
elseif ctr(wmGetTile(plPlane, iX-2, iY-1)) then
 if br(wmGetTile(plPlane, iX, iY-1)) then
  wmPlaceTile(plPlane, iX-1, iY-1, t("ctr2"))
 elseif mm(wmGetTile(plPlane, iX, iY-1)) then
  wmPlaceTile(plPlane, iX-1, iY-1, t("ctr2"))
 elseif not ctr2(wmGetTile(plPlane, iX-1, iY-1)) then
  wmPlaceTile(plPlane, iX-1, iY-1, t("cbl"))
 end
elseif ctl(wmGetTile(plPlane, iX-1, iY-1)) or ctr(wmGetTile(plPlane, iX-1, iY-1)) or ctr2(wmGetTile(plPlane, iX-1, iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("mm"))
elseif not tl(wmGetTile(plPlane,iX-2,iY-1)) and not mm(wmGetTile(plPlane,iX-1,iY-1)) and not cbl(wmGetTile(plPlane,iX-1,iY-1)) and not cbr(wmGetTile(plPlane,iX-1,iY-1)) and not cbr2(wmGetTile(plPlane,iX-1,iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("tm"))
elseif not ins(wmGetTile(plPlane, iX-1, iY-1)) then
 wmPlaceTile(plPlane, iX-1, iY-1, t("tl2"))
end

end

function placetm(plPlane, iX, iY)

if br(wmGetTile(plPlane, iX, iY-1)) or mr(wmGetTile(plPlane, iX, iY-1)) then
 wmPlaceTile(plPlane, iX, iY-1, t("cbl"))
elseif ml2(wmGetTile(plPlane, iX, iY-1)) and ctr2(wmGetTile(plPlane, iX-1, iY-1)) then
 wmPlaceTile(plPlane, iX, iY-1, t("mm"))
elseif ctl(wmGetTile(plPlane, iX, iY-1)) or ctr2(wmGetTile(plPlane, iX, iY-1)) then
 wmPlaceTile(plPlane, iX, iY-1, t("mm"))
elseif tr(wmGetTile(plPlane, iX, iY-1)) then
 wmPlaceTile(plPlane, iX, iY-1, t("tm"))
elseif tl(wmGetTile(plPlane, iX, iY-1)) or tl2(wmGetTile(plPlane, iX, iY-1)) then
 wmPlaceTile(plPlane, iX, iY-1, t("tm"))
elseif ctr(wmGetTile(plPlane, iX, iY-1)) then
 wmPlaceTile(plPlane, iX, iY-1, t("mm"))
 wmPlaceTile(plPlane, iX+1, iY-1, t("mm"))
elseif bl(wmGetTile(plPlane, iX, iY-1)) or ml(wmGetTile(plPlane, iX, iY-1)) then
 wmPlaceTile(plPlane, iX, iY-1, t("cbr"))
elseif cbr(wmGetTile(plPlane, iX-1, iY-1)) then
 wmPlaceTile(plPlane, iX, iY-1, t("cbr2"))
elseif bm(wmGetTile(plPlane, iX, iY-1)) then
 wmPlaceTile(plPlane, iX, iY-1, t("mm"))
elseif not ins(wmGetTile(plPlane, iX, iY-1)) then
 wmPlaceTile(plPlane, iX, iY-1, t("tm"))
end

end

function placetr(plPlane, iX, iY)

if br(wmGetTile(plPlane, iX+1, iY-1)) then
 wmPlaceTile(plPlane, iX+1, iY-1, t("mr"))
elseif wmGetTile(plPlane, iX+1, iY-1)==710 then
 wmPlaceTile(plPlane, iX+1, iY-1, t("mr")) 
elseif ml(wmGetTile(plPlane, iX+1, iY-1)) then
 wmPlaceTile(plPlane, iX+1, iY-1, t("cbr"))
 wmPlaceTile(plPlane, iX+2, iY-1, t("cbr2"))
elseif ctr(wmGetTile(plPlane, iX+1, iY-1)) then
 wmPlaceTile(plPlane, iX+1, iY-1, t("mm"))
 wmPlaceTile(plPlane, iX+2, iY-1, t("mm"))
elseif ctr2(wmGetTile(plPlane, iX+1, iY-1)) then
 wmPlaceTile(plPlane, iX+1, iY-1, t("mm"))
elseif bm(wmGetTile(plPlane, iX+1, iY-1)) or bl2(wmGetTile(plPlane, iX+1, iY-1)) then
 wmPlaceTile(plPlane, iX+1, iY-1, t("ctl"))
elseif tl(wmGetTile(plPlane, iX+1, iY-1)) or tl2(wmGetTile(plPlane, iX+1, iY-1)) then
  wmPlaceTile(plPlane, iX+1, iY-1, t("tm"))
  if tl2(wmGetTile(plPlane, iX+2, iY-1)) then
   wmPlaceTile(plPlane, iX+2, iY-1, t("tm"))
  end
elseif bl(wmGetTile(plPlane, iX+1, iY-1)) then
 wmPlaceTile(plPlane, iX, iY-1, t("cbr"))
 wmPlaceTile(plPlane, iX+1, iY, t("ctl"))
 wmPlaceTile(plPlane, iX+1, iY-1, t("cbr2"))
  placetl(plPlane, iX+2, iY-1)
  placetl2(plPlane, iX+2, iY-1)
  placetr(plPlane, iX+1, iY-1)
  placetr(plPlane, iX+1, iY)
  placebr(plPlane, iX+1, iY-1)
elseif ml2(wmGetTile(plPlane, iX+1, iY-1)) and ctr2(wmGetTile(plPlane, iX, iY-1)) then
 wmPlaceTile(plPlane, iX+1, iY-1, t("mm"))
elseif ml2(wmGetTile(plPlane, iX+1, iY-1)) then
 wmPlaceTile(plPlane, iX+1, iY-1, t("cbr2"))
elseif not ins(wmGetTile(plPlane, iX+1, iY-1)) then 
 wmPlaceTile(plPlane, iX+1, iY-1, t("tr"))
end

end

function placeml(plPlane, iX, iY)

if br(wmGetTile(plPlane, iX-2, iY)) or bm(wmGetTile(plPlane, iX-2, iY)) or bl2(wmGetTile(plPlane, iX-2, iY)) then
 wmPlaceTile(plPlane, iX-2, iY, t("ctr"))
 wmPlaceTile(plPlane, iX-1, iY, t("ctr2"))
elseif bl(wmGetTile(plPlane, iX-2, iY)) or tl(wmGetTile(plPlane, iX-2, iY)) then
 wmPlaceTile(plPlane, iX-2, iY, t("ml"))
elseif tl2(wmGetTile(plPlane, iX-2, iY)) or tm(wmGetTile(plPlane, iX-2, iY)) then
 wmPlaceTile(plPlane, iX-2, iY, t("cbr"))
elseif tr(wmGetTile(plPlane, iX-2, iY)) then
 wmPlaceTile(plPlane, iX-2, iY, t("cbr"))
 wmPlaceTile(plPlane, iX-1, iY, t("cbr2")) 
elseif ctl(wmGetTile(plPlane, iX-2, iY)) or cbl(wmGetTile(plPlane, iX-2, iY)) then
 wmPlaceTile(plPlane, iX-2, iY, t("mm"))
elseif mr(wmGetTile(plPlane, iX-2, iY)) then
 wmPlaceTile(plPlane, iX-2, iY, t("mm"))
 wmPlaceTile(plPlane, iX-1, iY, t("mm"))
elseif not ins(wmGetTile(plPlane, iX-2, iY)) then 
 wmPlaceTile(plPlane, iX-2, iY, t("ml"))
end

end

function placeml2(plPlane, iX, iY)

if mr(wmGetTile(plPlane, iX-1, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("mm"))
elseif cbr(wmGetTile(plPlane, iX-1, iY)) and ml(wmGetTile(plPlane, iX-2, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("ml2"))
elseif mm(wmGetTile(plPlane, iX-2, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("mm"))
elseif bm(wmGetTile(plPlane, iX-1, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("mm"))
elseif ctr(wmGetTile(plPlane, iX-1, iY)) and ml(wmGetTile(plPlane, iX-2, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("ml2"))
elseif bl(wmGetTile(plPlane, iX-1, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("ml2"))
elseif cbr(wmGetTile(plPlane, iX-1, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("cbr2"))
elseif tl(wmGetTile(plPlane, iX-1, iY)) or tl2(wmGetTile(plPlane, iX-1, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("ml2")) 
elseif tm(wmGetTile(plPlane, iX-1, iY)) or tr(wmGetTile(plPlane, iX-1, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("cbr2"))
elseif bl2(wmGetTile(plPlane, iX-1, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("ml2"))
elseif ml(wmGetTile(plPlane, iX-1, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("ml2"))
 wmPlaceTile(plPlane, iX, iY, t("mm"))
elseif ctl(wmGetTile(plPlane, iX-1, iY)) or br(wmGetTile(plPlane, iX-1, iY)) or cbl(wmGetTile(plPlane, iX-1, iY)) or cbr(wmGetTile(plPlane, iX-1, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("mm"))
elseif not ins(wmGetTile(plPlane, iX-1, iY)) then
 wmPlaceTile(plPlane, iX-1, iY, t("ml2"))
end

end

function placemm(plPlane, iX, iY)

if ml(wmGetTile(plPlane, iX, iY)) then
 wmPlaceTile(plPlane, iX, iY, t("mm"))
 wmPlaceTile(plPlane, iX+1, iY, t("mm"))
else
 wmPlaceTile(plPlane, iX, iY, t("mm"))
end

end

function placemr(plPlane, iX, iY)

if ml(wmGetTile(plPlane, iX+1, iY)) then
 wmPlaceTile(plPlane, iX+1, iY, t("mm"))
 wmPlaceTile(plPlane, iX+2, iY, t("mm"))

elseif tr(wmGetTile(plPlane, iX+1, iY)) then
 wmPlaceTile(plPlane, iX+1, iY, t("mr"))
elseif bm(wmGetTile(plPlane, iX+1, iY)) or bl2(wmGetTile(plPlane, iX+1, iY)) then
 wmPlaceTile(plPlane, iX+1, iY, t("ctl"))
elseif bl(wmGetTile(plPlane, iX+1, iY)) then
 placebm(plPlane, iX+2, iY-1)
 wmPlaceTile(plPlane, iX+1, iY, t("ctl"))
elseif tm(wmGetTile(plPlane, iX+1, iY)) or tl2(wmGetTile(plPlane, iX+1, iY)) then
 wmPlaceTile(plPlane, iX+1, iY, t("cbl"))
elseif cbr(wmGetTile(plPlane, iX+1, iY)) then
 wmPlaceTile(plPlane, iX+1, iY, t("mm"))
 wmPlaceTile(plPlane, iX+2, iY, t("mm"))
elseif cbr2(wmGetTile(plPlane, iX+1, iY)) then
 wmPlaceTile(plPlane, iX+1, iY, t("mm"))
elseif tl(wmGetTile(plPlane, iX+1, iY)) then
 wmPlaceTile(plPlane, iX+1, iY, t("cbl"))
 wmPlaceTile(plPlane, iX+2, iY, t("tm")) 
elseif ctr(wmGetTile(plPlane, iX+1, iY)) then
 wmPlaceTile(plPlane, iX+1, iY, t("mm"))
 wmPlaceTile(plPlane, iX+2, iY, t("mm"))
elseif ctr2(wmGetTile(plPlane, iX+1, iY)) then
 wmPlaceTile(plPlane, iX+1, iY, t("mm"))
elseif br(wmGetTile(plPlane, iX+1, iY)) then
 wmPlaceTile(plPlane, iX+1, iY, t("mr"))
elseif not ins(wmGetTile(plPlane, iX+1, iY)) then
 wmPlaceTile(plPlane, iX+1, iY, t("mr"))
end

end

function placebl(plPlane, iX, iY)

if tl(wmGetTile(plPlane, iX-2, iY+1)) then
 wmPlaceTile(plPlane, iX-2, iY+1, t("ml"))
 wmPlaceTile(plPlane, iX-1, iY+1, t("ml2"))
elseif tr(wmGetTile(plPlane, iX-2, iY+1)) then
 wmPlaceTile(plPlane, iX-2, iY+1, t("ctl"))
 wmPlaceTile(plPlane, iX-2, iY, t("cbr"))
 wmPlaceTile(plPlane, iX-1, iY, t("cbr2"))
 placetl(plPlane, iX-1, iY+1)
 placeml(plPlane, iX-1, iY+1)
elseif cbl(wmGetTile(plPlane, iX-2, iY+1)) then
 wmPlaceTile(plPlane, iX-2, iY+1, t("mm"))
elseif mr(wmGetTile(plPlane, iX-2, iY+1)) then
 wmPlaceTile(plPlane, iX-2, iY+1, t("ctl"))
elseif br(wmGetTile(plPlane, iX-2, iY+1)) then
 wmPlaceTile(plPlane, iX-2, iY+1, t("bm"))
elseif tl2(wmGetTile(plPlane, iX-2, iY+1)) or tm(wmGetTile(plPlane, iX-2, iY+1)) then
 wmPlaceTile(plPlane, iX-2, iY+1, t("cbr"))
 wmPlaceTile(plPlane, iX-1, iY+1, t("cbr2"))
elseif not ins(wmGetTile(plPlane, iX-2, iY+1)) then
 wmPlaceTile(plPlane, iX-2, iY+1, t("bl"))
end

end

function placebl2(plPlane, iX, iY)

if tl(wmGetTile(plPlane, iX-1, iY+1)) then
 wmPlaceTile(plPlane, iX-1, iY+1, t("ctr"))
elseif ml(wmGetTile(plPlane, iX-1, iY+1)) then
 wmPlaceTile(plPlane, iX-1, iY+1, t("ctr")) 
 wmPlaceTile(plPlane, iX, iY+1, t("ctr2")) 
elseif cbr(wmGetTile(plPlane, iX-1, iY+1)) or cbl(wmGetTile(plPlane, iX-1, iY+1)) then
 wmPlaceTile(plPlane, iX-1, iY+1, t("mm"))
elseif br(wmGetTile(plPlane, iX-1, iY+1)) or ctl(wmGetTile(plPlane, iX-2, iY+1)) and not ctr(wmGetTile(plPlane, iX-1, iY+1)) then
 wmPlaceTile(plPlane, iX-1, iY+1, t("bm"))
elseif bl(wmGetTile(plPlane, iX-1, iY+1)) then
 if bm(wmGetTile(plPlane, iX-2, iY+1)) then
  wmPlaceTile(plPlane, iX-1, iY+1, t("bm"))
 else
  wmPlaceTile(plPlane, iX-1, iY+1, t("bl2"))
 end
elseif mr(wmGetTile(plPlane, iX-1, iY+1)) or tr(wmGetTile(plPlane, iX-1, iY+1)) then
 wmPlaceTile(plPlane, iX-1, iY+1, t("ctl")) 
elseif ml(wmGetTile(plPlane, iX-1, iY+1)) then
 wmPlaceTile(plPlane, iX-1, iY+1, t("ctr")) 
 wmPlaceTile(plPlane, iX, iY+1, t("ctr2")) 
elseif tm(wmGetTile(plPlane, iX-1, iY+1)) then
 wmPlaceTile(plPlane, iX-1, iY+1, t("mm")) 
elseif bm(wmGetTile(plPlane, iX-2, iY+1)) and not ctr(wmGetTile(plPlane, iX-1, iY+1)) then
 wmPlaceTile(plPlane, iX-1, iY+1, t("bm")) 
elseif not ins(wmGetTile(plPlane, iX-1, iY+1)) then
 wmPlaceTile(plPlane, iX-1, iY+1, t("bl2"))
end

end

function placebm(plPlane, iX, iY)

if mr(wmGetTile(plPlane, iX, iY+1)) or tr(wmGetTile(plPlane, iX, iY+1)) then
 wmPlaceTile(plPlane, iX, iY+1, t("ctl"))
elseif ml(wmGetTile(plPlane, iX, iY+1)) then
 wmPlaceTile(plPlane, iX, iY+1, t("ctr")) 
 wmPlaceTile(plPlane, iX+1, iY+1, t("ctr2")) 
elseif cbl(wmGetTile(plPlane, iX, iY+1)) then
 wmPlaceTile(plPlane, iX, iY+1, t("mm")) 
elseif cbr(wmGetTile(plPlane, iX, iY+1)) then
 wmPlaceTile(plPlane, iX, iY+1, t("mm")) 
 wmPlaceTile(plPlane, iX+1, iY+1, t("mm")) 
elseif cbr2(wmGetTile(plPlane, iX, iY+1)) then
 wmPlaceTile(plPlane, iX, iY+1, t("mm")) 
elseif bl(wmGetTile(plPlane, iX, iY+1)) or bl2(wmGetTile(plPlane, iX, iY+1)) then
 wmPlaceTile(plPlane, iX, iY+1, t("bm"))
elseif tl(wmGetTile(plPlane, iX, iY+1)) then
 wmPlaceTile(plPlane, iX, iY+1, t("ctr"))
elseif tl2(wmGetTile(plPlane, iX, iY+1)) then
 wmPlaceTile(plPlane, iX, iY+1, t("ctr2")) 
elseif tm(wmGetTile(plPlane, iX, iY+1)) then
 wmPlaceTile(plPlane, iX, iY+1, t("mm"))
elseif br(wmGetTile(plPlane, iX, iY+1)) then
 wmPlaceTile(plPlane, iX, iY+1, t("bm"))
elseif not ins(wmGetTile(plPlane, iX, iY+1)) then
 wmPlaceTile(plPlane, iX, iY+1, t("bm"))
end

end

function placebr(plPlane, iX, iY)

if bl(wmGetTile(plPlane, iX+1, iY+1)) then
 wmPlaceTile(plPlane, iX+1, iY+1, t("bm"))
 placebm(plPlane, iX+2, iY)
elseif tl(wmGetTile(plPlane, iX+1, iY+1)) then
 wmPlaceTile(plPlane, iX+1, iY+1, t("ctr"))
 wmPlaceTile(plPlane, iX+1, iY, t("cbl"))
 placetr(plPlane, iX+1, iY+1)
 placemr(plPlane, iX+1, iY+1)
elseif cbr(wmGetTile(plPlane, iX+1, iY+1)) then
 wmPlaceTile(plPlane, iX+1, iY+1, t("mm"))
 wmPlaceTile(plPlane, iX+2, iY+1, t("mm"))
elseif ml(wmGetTile(plPlane, iX+1, iY+1)) then
 wmPlaceTile(plPlane, iX+1, iY+1, t("ctr")) 
 wmPlaceTile(plPlane, iX+2, iY+1, t("ctr2")) 
elseif tm(wmGetTile(plPlane, iX+1, iY+1)) or tl2(wmGetTile(plPlane, iX+1, iY+1)) then
 wmPlaceTile(plPlane, iX+1, iY+1, t("cbl"))
elseif tr(wmGetTile(plPlane, iX+1, iY+1)) then
 wmPlaceTile(plPlane, iX+1, iY+1, t("mr"))
elseif bl2(wmGetTile(plPlane, iX+1, iY+1)) then
 wmPlaceTile(plPlane, iX+1, iY+1, t("bm"))
if mr(wmGetTile(plPlane, iX+1, iY)) and wmGetSettingValue(cbGrass)==1 then wmPlaceTile(plPlane, iX+1, iY, 710)
end
elseif not ins(wmGetTile(plPlane, iX+1, iY+1)) then
 if wmGetSettingValue(cbGrass)==1 then wmPlaceTile(plPlane, iX+1, iY, 710)
 end
 wmPlaceTile(plPlane, iX+1, iY+1, t("br"))
end

end


function cbBrushPlaced(plPlane, iX, iY)

 placetl(plPlane, iX, iY)
 placetl2(plPlane, iX, iY)
 placetm(plPlane, iX, iY)
 placetr(plPlane, iX, iY)
 placeml(plPlane, iX, iY)
 placeml2(plPlane, iX, iY)
 placemm(plPlane, iX, iY)
 placemr(plPlane, iX, iY)
 placebl(plPlane, iX, iY)
 placebl2(plPlane, iX, iY)
 placebm(plPlane, iX, iY)
 placebr(plPlane, iX, iY)

end

