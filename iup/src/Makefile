
.PHONY: do_all iup iupgtk iupmot
do_all: iup

iup:
	@$(MAKE) --no-print-directory -f ../tecmake.mak 

iupgtk:
	@$(MAKE) --no-print-directory -f ../tecmake.mak USE_GTK=Yes

iupmot:
	@$(MAKE) --no-print-directory -f ../tecmake.mak USE_MOTIF=Yes
