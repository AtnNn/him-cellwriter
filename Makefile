LIBS = gtk+-2.0 glib-2.0 gconf-2.0 hildon-1 hildon-input-method-ui-3.0 hildon-input-method-framework-3.0

build:
	LIBS='$(LIBS)' make -C src

clean:
	make -C src clean

install: src/him_cellwriter.so default_profile
	mkdir -p $(prefix)/lib/hildon-input-method/
	install src/him_cellwriter.so $(prefix)/lib/hildon-input-method/
	mkdir -p $(prefix)/share/him_cellwriter
	install default_profile $(prefix)/share/him_cellwriter/profile

configure:
	@echo '#!/bin/sh' > configure
	@echo 'echo Looking for $(LIBS) && echo "Found!" && echo Run make to build' >> configure
	@echo 'pkg-config --print-errors $(LIBS)' >> configure
	@chmod +x configure

distclean:
