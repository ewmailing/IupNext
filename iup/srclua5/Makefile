ifeq ($(OS), Windows_NT)
  TECMAKE_CMD = $(MAKE) --no-print-directory -f ../tecmakewin.mak TEC_UNAME=$(TEC_UNAME)
  WINLIBS = iupluaole
else
  TECMAKE_CMD = $(MAKE) --no-print-directory -f ../tecmake.mak
endif

.PHONY: do_all iuplua iupluacd iupluacontrols iupluagl iupluaglcontrols iuplua_plot iuplua_pplot iuplua_mglplot iuplua_scintilla iupluaim iupluaimglib iupluatuio iupluaole iupluaweb
do_all: iuplua iupluacd iupluacontrols iupluagl iupluaglcontrols iuplua_plot iuplua_pplot iuplua_mglplot iuplua_scintilla iupluaim iupluaimglib iupluatuio $(WINLIBS)

iuplua:
	@$(TECMAKE_CMD) USE_LUA51=Yes 
	@$(TECMAKE_CMD) USE_LUA52=Yes 
	@$(TECMAKE_CMD) USE_LUA53=Yes 

iupluacd:
	@$(TECMAKE_CMD) MF=iupcd USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iupcd USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iupcd USE_LUA53=Yes

iupluacontrols:
	@$(TECMAKE_CMD) MF=iupcontrols USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iupcontrols USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iupcontrols USE_LUA53=Yes

iupluagl:
	@$(TECMAKE_CMD) MF=iupgl USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iupgl USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iupgl USE_LUA53=Yes

iupluaglcontrols:
	@$(TECMAKE_CMD) MF=iupglcontrols USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iupglcontrols USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iupglcontrols USE_LUA53=Yes

iuplua_plot:
	@$(TECMAKE_CMD) MF=iup_plot USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iup_plot USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iup_plot USE_LUA53=Yes

iuplua_pplot:
	@$(TECMAKE_CMD) MF=iup_pplot USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iup_pplot USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iup_pplot USE_LUA53=Yes

iuplua_mglplot:
	@$(TECMAKE_CMD) MF=iup_mglplot USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iup_mglplot USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iup_mglplot USE_LUA53=Yes

iuplua_scintilla:
	@$(TECMAKE_CMD) MF=iup_scintilla USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iup_scintilla USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iup_scintilla USE_LUA53=Yes

iupluaim:
	@$(TECMAKE_CMD) MF=iupim USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iupim USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iupim USE_LUA53=Yes

iupluaimglib:
	@$(TECMAKE_CMD) MF=iupimglib USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iupimglib USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iupimglib USE_LUA53=Yes
  
iupluatuio:
	@$(TECMAKE_CMD) MF=iuptuio USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iuptuio USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iuptuio USE_LUA53=Yes
  
iupluaole:
	@$(TECMAKE_CMD) MF=iupole USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iupole USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iupole USE_LUA53=Yes
  
iupluaweb:
	@$(TECMAKE_CMD) MF=iupweb USE_LUA51=Yes
	@$(TECMAKE_CMD) MF=iupweb USE_LUA52=Yes
	@$(TECMAKE_CMD) MF=iupweb USE_LUA53=Yes
