
.PHONY:	all clean install

all: inline_image.so

inline_image.so: inline_image.c
	gcc inline_image.c `pkg-config --cflags --libs purple glib-2.0` -g -O2 -fPIC -pipe -o inline_image.so -shared

clean:
	rm -f inline_image.so

install:
	cp inline_image.so ~/.purple/plugins/
