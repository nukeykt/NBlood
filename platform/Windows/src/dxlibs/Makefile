# ensure dinput.h can be found before running

%.o: %.c
	$(PREFIX)gcc -I../../include -Wall -Wextra -c $< -o $@

lib%.a: %.def %.o
	$(PREFIX)dlltool -k --output-lib $@ -d $*.def
	$(PREFIX)ar r $@ $*.o
	$(PREFIX)ranlib $@

lib%.a: %.def
	$(PREFIX)dlltool -k --output-lib $@ -d $<



all : libdsound.a libdxguid.a libdxguid_sdl.a
	@ls -l $^

full : libd3d8.a libd3dx8d.a libd3dxof.a libddraw.a libdinput.a libdinput8.a libdplayx.a \
libdpnaddr.a libdpnet.a libdpnlobby.a libdpvoice.a libdsetup.a \
libdsound.a libdxguid.a libdxguid_sdl.a
	@ls -l $^
	@echo move the created libraries to the lib dir.
	@echo happy coding !!



libdxguid_sdl.a : dxguid_sdl.o
	$(PREFIX)ar -r libdxguid_sdl.a dxguid_sdl.o
	$(PREFIX)ranlib libdxguid_sdl.a

libdxguid.a : dxguid.o
	$(PREFIX)ar -r libdxguid.a dxguid.o
	$(PREFIX)ranlib libdxguid.a

libdinput_custom.a : dinput.o
	$(PREFIX)ar r libdinput_custom.a dinput.o
	$(PREFIX)ranlib libdinput_custom.a

libdinput8.a : dinput8.def dinput.o
	$(PREFIX)dlltool -k --output-lib libdinput8.a -d dinput8.def
	$(PREFIX)ar r libdinput8.a dinput.o
	$(PREFIX)ranlib libdinput8.a



clean:
	-rm -f *.a *.o
