
.PHONY: do_all iup iupgtk iupmot
do_all: iup

iup:
	@$(MAKE) --no-print-directory -f ../tecmake_compact.mak 

iupgtk:
	@$(MAKE) --no-print-directory -f ../tecmake_compact.mak USE_GTK=Yes

iupmot:
	@$(MAKE) --no-print-directory -f ../tecmake_compact.mak USE_MOTIF=Yes
