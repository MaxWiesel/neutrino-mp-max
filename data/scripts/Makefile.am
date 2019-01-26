installdir = $(bindir)

install_DATA = \
	backup.sh \
	restore.sh \
	wakeup.sh \
	install.sh 

if BOXTYPE_ARMBOX
install_DATA += \
	restore_flash.sh
endif

install-data-hook:
	chmod 0755 $(DESTDIR)$(bindir)/backup.sh
	chmod 0755 $(DESTDIR)$(bindir)/restore.sh
	chmod 0755 $(DESTDIR)$(bindir)/wakeup.sh
	chmod 0755 $(DESTDIR)$(bindir)/install.sh
if BOXTYPE_ARMBOX
	chmod 0755 $(DESTDIR)$(bindir)/restore_flash.sh
endif
