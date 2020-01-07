game.exe : game.obj engine.obj cache1d.obj multi.obj a.obj kdmeng.obj k.obj
	wlink f game f engine f cache1d f multi f a f kdmeng f k
build.exe : build.obj engine.obj cache1d.obj a.obj bstub.obj
	wlink f build f engine f cache1d f a f bstub

game.obj : game.c build.h names.h pragmas.h
	wcc386 game /5r /s /orb
build.obj : build.c build.h pragmas.h
	wcc386 build /5r /s /orb
bstub.obj : bstub.c build.h
	wcc386 bstub /5r /s /orb
cache1d.obj : cache1d.c pragmas.h
	wcc386 cache1d /5r /s /orb
engine.obj : engine.c build.h pragmas.h ves2.h
	wcc386 engine /5r /s /orb
a.obj : a.asm
	wasm a.asm
multi.obj : multi.c
	wcc386 multi /4r /s /or
kdmeng.obj : kdmeng.c makefile.
	wcc386 kdmeng /4r /s /or /d2
k.obj : k.asm makefile.
	wasm k.asm
