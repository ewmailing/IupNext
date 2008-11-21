
.PHONY: all iup iupcd iupcontrols iupgl iup_pplot iupim ledc iupview iuplua3 iuplua5 iupconsole
#all: iup iupcd iupcontrols iupgl iup_pplot iupim ledc iupview iuplua3 iuplua5 iupconsole
all: iup iupcd iupcontrols iupgl iup_pplot iupim ledc iupview iuplua5 iupconsole

iup:
	@$(MAKE) --no-print-directory -C ./src/
iupcd:
	@$(MAKE) --no-print-directory -C ./srccd/
iupcontrols:
	@$(MAKE) --no-print-directory -C ./srccontrols/
iup_pplot:
	@$(MAKE) --no-print-directory -C ./srcpplot/
iupgl:
	@$(MAKE) --no-print-directory -C ./srcgl/
iupim:
	@$(MAKE) --no-print-directory -C ./srcim/
iupimglib:
	@$(MAKE) --no-print-directory -C ./srcimglib/
iuplua3:
	@$(MAKE) --no-print-directory -C ./srclua3/
iuplua5:
	@$(MAKE) --no-print-directory -C ./srclua5/
iupconsole:
	@$(MAKE) --no-print-directory -C ./srcconsole/
ledc:
	@$(MAKE) --no-print-directory -C ./srcledc/
iupview: iupcontrols iup
	@$(MAKE) --no-print-directory -C ./srcview/
