CC = g++ -Wall -pedantic -ansi -I /usr/include/glib-2.0/ -I /usr/lib/x86_64-linux-gnu/glib-2.0/include/ -I /usr/include/gtk-2.0/ -I /usr/include/cairo/ -I /usr/lib/x86_64-linux-gnu/gtk-2.0/include/ -I /usr/include/gdk-pixbuf-2.0/ -I /usr/include/pango-1.0/ -I /usr/include/libglade-2.0/


default: palnet.o gtkmain.o palbubble.o palcallback.o gtkasset.o gtkbubble.o gtkbigpage.o palclient.o
	$(CC) gtkmain.o palnet.o palcallback.o gtkbubble.o palbubble.o palclient.o gtkasset.o gtkbigpage.o -o linpal `curl-config --libs` `pkg-config --libs gtk+-2.0 libglade-2.0 glib-2.0 gthread-2.0`

palnet.o: palnet.cpp palnet.h palcallback.h
	$(CC) -c palnet.cpp 

palcallback.o: palcallback.h palcallback.cpp
	$(CC) -c palcallback.cpp 

palclient.o: palclient.h palclient.cpp
	$(CC) -c palclient.cpp

gtkasset.o: gtkasset.h gtkasset.cpp 
	$(CC) -c gtkasset.cpp `pkg-config --cflags gtk+-2.0 glib-2.0 gthread-2.0 libglade-2.0`

gtkbubble.o: gtkbubble.cpp gtkbubble.h
	$(CC) -c gtkbubble.cpp `pkg-config --cflags gtk+-2.0 glib-2.0 gthread-2.0 libglade-2.0`

palbubble.o: palbubble.cpp palbubble.h
	$(CC) -c palbubble.cpp `pkg-config --cflags gtk+-2.0 glib-2.0 gthread-2.0 libglade-2.0`
	
gtkbigpage.o: gtkbigpage.h gtkbigpage.cpp
	$(CC) -c gtkbigpage.cpp -o gtkbigpage.o `pkg-config --cflags gtk+-2.0 glib-2.0 gthread-2.0 libglade-2.0`

gtkmain.o: gtkmain.cpp palcallback.h gtkmain.h
	$(CC) -c gtkmain.cpp `curl-config --cflags` `pkg-config --cflags gtk+-2.0 libglade-2.0 glib-2.0 gthread-2.0`

clean:
	rm *.o
