SOURCES = him_cellwriter.c recognize.c main.c window.c stroke.c options.c wordfreq.c averages.c cellwidget.c preprocess.c singleinstance.c keyevent.c 
HEADERS = keys.h config.h common.h

him_cellwriter.so: $(SOURCES) $(HEADERS)
	gcc `pkg-config $(LIBS) --libs --cflags` $(SOURCES) -shared -o $@

clean:
	rm him_cellwriter.so || true
