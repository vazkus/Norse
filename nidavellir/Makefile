include Makefile.common

all : preinstall build install

preinstall: 
	mkdir -p $(NORSE_INCDIR)
	cp $(HREIDMAR)/chconf.h  $(NORSE_INCDIR)
	cp $(HREIDMAR)/mcuconf.h $(NORSE_INCDIR)
	cp $(HREIDMAR)/halconf.h $(NORSE_INCDIR)

SUBDIRS = Hreidmar

.PHONY: subdirs $(SUBDIRS)

build :
	-for d in $(SUBDIRS); do (cd $$d; $(MAKE) ); done

install :
	-for d in $(SUBDIRS); do (cd $$d; $(MAKE) install ); done

clean :
	-for d in $(SUBDIRS); do (cd $$d; $(MAKE) clean ); done
