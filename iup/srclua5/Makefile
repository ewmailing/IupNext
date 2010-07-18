
.PHONY: do_all iuplua iupluacd iupluacontrols iuplua_pplot iupluagl iupluaim iupluaimglib iupluatuio
do_all: iuplua iupluacd iupluacontrols iuplua_pplot iupluagl iupluaim iupluaimglib iupluatuio

iuplua:
	@$(MAKE) --no-print-directory -f ../tecmake.mak 

iupluacd:
	@$(MAKE) --no-print-directory -f ../tecmake.mak MF=iupcd

iupluacontrols:
	@$(MAKE) --no-print-directory -f ../tecmake.mak MF=iupcontrols

iuplua_pplot:
	@$(MAKE) --no-print-directory -f ../tecmake.mak MF=iup_pplot

iupluagl:
	@$(MAKE) --no-print-directory -f ../tecmake.mak MF=iupgl

iupluaim:
	@$(MAKE) --no-print-directory -f ../tecmake.mak MF=iupim

iupluaimglib:
	@$(MAKE) --no-print-directory -f ../tecmake.mak MF=iupimglib
  
iupluatuio:
	@$(MAKE) --no-print-directory -f ../tecmake.mak MF=iuptuio
