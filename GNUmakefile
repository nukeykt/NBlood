#
# EDuke32 Makefile for GNU Make
#

### Global Profiles
ifeq ($(FURY),1)
    APPBASENAME := fury
    APPNAME := Ion Fury
    NETCODE := 0
    POLYMER := 0
    RETAIL_MENU := 1
    STANDALONE := 1
    USE_LIBVPX := 0
    SDL_STATIC := 1
endif

### Platform and Toolchain Configuration
include Common.mak

### File Extensions
asm := nasm
o := o

### Directories
source := source
obj := obj

### Functions
# Some of this still needs work--"getfiltered" takes a list of files and strips the paths off of them,
# while the other functions end up expanding the result later by adding the paths back.
# This is inefficient, but it was a better use of time than reworking all of this at the moment.
define parent
$(word 1,$(subst _, ,$1))
endef
define expandobjs
$$(addprefix $$($$(call parent,$1)_obj)/,$$(addsuffix .$$o,$$(basename $$($1_objs) $$($1_rsrc_objs) $$($1_gen_objs))))
endef
define expandsrcs
$(addprefix $($(call parent,$1)_src)/,$($1_objs)) $(addprefix $($(call parent,$1)_rsrc)/,$($1_rsrc_objs)) $(addprefix $($(call parent,$1)_obj)/,$($1_gen_objs))
endef
define expanddeps
$(strip $1 $(foreach j,$1,$(call $0,$($j_deps))))
endef
define getdeps
$(call expanddeps,$1_$2 $(common_$2_deps) engine)
endef
define getfiltered
$(filter-out $(strip $($1_excl)), $(subst $($1_src)/, ,$(wildcard $($1_src)/$2)))
endef

##### External Library Definitions

#### libxmp-lite

libxmplite := libxmp-lite

libxmplite_root := $(source)/$(libxmplite)
libxmplite_src := $(libxmplite_root)/src
libxmplite_inc := $(libxmplite_root)/include
libxmplite_obj := $(obj)/$(libxmplite)

libxmplite_excl :=
libxmplite_objs := $(call getfiltered,libxmplite,*.c)

libxmplite_cflags := -DHAVE_ROUND -DLIBXMP_CORE_PLAYER -DLIBXMP_NO_PROWIZARD -DLIBXMP_NO_DEPACKERS -DBUILDING_STATIC -I$(libxmplite_inc)/libxmp-lite -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare -Wno-cast-qual


#### PhysicsFS

physfs := physfs

physfs_root := $(source)/$(physfs)
physfs_src := $(physfs_root)/src
physfs_inc := $(physfs_root)/include
physfs_obj := $(obj)/$(physfs)

physfs_excl :=

ifeq ($(PLATFORM),WINDOWS)
    physfs_excl += physfs_platform_unix.c
else
    physfs_excl += physfs_platform_windows.c
endif

ifneq (0,$(USE_PHYSFS))
    physfs_objs := $(call getfiltered,physfs,*.c)
else
    physfs_objs :=
endif

ifeq ($(PLATFORM),APPLE)
    physfs_objs += physfs_platform_apple.m
endif

physfs_cflags :=


#### glad

glad := glad

glad_root := $(source)/$(glad)
glad_src := $(glad_root)/src
glad_inc := $(glad_root)/include
glad_obj := $(obj)/$(glad)

glad_excl :=

ifneq ($(RENDERTYPE),WIN)
    glad_excl += glad_wgl.c
endif

ifneq (0,$(USE_OPENGL))
    glad_objs := $(call getfiltered,glad,*.c)
else
    glad_objs :=
endif

glad_cflags :=


#### mimalloc

mimalloc := mimalloc

mimalloc_root := $(source)/$(mimalloc)
mimalloc_src := $(mimalloc_root)/src
mimalloc_inc := $(mimalloc_root)/include
mimalloc_obj := $(obj)/$(mimalloc)

mimalloc_excl := \
    alloc-override.c \
    arena-abandon.c \
    free.c \
    page-queue.c \
    static.c \

mimalloc_objs := $(call getfiltered,mimalloc,*.c)
mimalloc_objs += prim/prim.c

mimalloc_cflags := -D_WIN32_WINNT=0x0600 -DMI_USE_RTLGENRANDOM -DMI_SHOW_ERRORS -fexceptions -Wno-cast-qual -Wno-unknown-pragmas -Wno-array-bounds -Wno-null-dereference -Wno-missing-field-initializers

ifeq (,$(filter 1 2 3 4 5 6 7,$(GCC_MAJOR)))
    mimalloc_cflags += -Wno-class-memaccess
endif


#### imgui

imgui := imgui

imgui_root := $(source)/$(imgui)
imgui_src := $(imgui_root)/src
imgui_inc := $(imgui_root)/include
imgui_obj := $(obj)/$(imgui)

imgui_excl :=

ifneq ($(RENDERTYPE),SDL)
    imgui_excl += imgui_impl_sdl2.cpp
endif
ifneq ($(RENDERTYPE),WIN)
    imgui_excl += imgui_impl_win32.cpp
endif

imgui_objs := $(call getfiltered,imgui,*.cpp)

imgui_cflags := -I$(imgui_inc) -Wno-cast-qual -Wno-cast-function-type -Wno-null-dereference -Wno-stringop-overflow


#### Voidwrap

voidwrap := voidwrap

voidwrap_root := $(source)/$(voidwrap)
voidwrap_src := $(voidwrap_root)/src
voidwrap_inc := $(voidwrap_root)/include
voidwrap_obj := $(obj)/$(voidwrap)

voidwrap_excl :=

ifneq ($(PLATFORM),WINDOWS)
    voidwrap_excl += dllmain.cpp
endif

voidwrap_objs := $(call getfiltered,voidwrap,*.cpp)

ifeq ($(IMPLICIT_ARCH),x86_64)
    ifeq ($(PLATFORM),WINDOWS)
        voidwrap_lib := voidwrap_steam_x64.dll
        steamworks_lib := win64/steam_api64.dll
    else
        voidwrap_lib := libvoidwrap_steam.so
        steamworks_lib := linux64/libsteam_api.so
    endif
else
    ifeq ($(PLATFORM),WINDOWS)
        voidwrap_lib := voidwrap_steam_x86.dll
        steamworks_lib := steam_api.dll
    else
        voidwrap_lib := libvoidwrap_steam.so
        steamworks_lib := linux32/libsteam_api.so
    endif
endif

voidwrap_cflags := -I$(voidwrap_root)/sdk/public/steam -fPIC -fvisibility=hidden -Wno-invalid-offsetof


#### libsmackerdec

libsmackerdec := libsmackerdec

libsmackerdec_objs := \
    BitReader.cpp \
    FileStream.cpp \
    HuffmanVLC.cpp \
    LogError.cpp \
    SmackerDecoder.cpp \

libsmackerdec_root := $(source)/$(libsmackerdec)
libsmackerdec_src := $(libsmackerdec_root)/src
libsmackerdec_inc := $(libsmackerdec_root)/include
libsmackerdec_obj := $(obj)/$(libsmackerdec)

libsmackerdec_cflags :=


#### hmpplay

hmpplay := hmpplay

hmpplay_objs := \
    hmpplay.cpp \
    fmmidi3.cpp \

hmpplay_root := $(source)/$(hmpplay)
hmpplay_src := $(hmpplay_root)/src
hmpplay_inc := $(hmpplay_root)/include
hmpplay_obj := $(obj)/$(hmpplay)

hmpplay_cflags :=


##### Component Definitions

#### EBacktrace

ifndef ebacktrace_dll
    ebacktrace_dll := ebacktrace1.dll
    ifeq ($(findstring x86_64,$(COMPILERTARGET)),x86_64)
        ebacktrace_dll := ebacktrace1-64.dll
    endif
endif


#### BUILD Engine

engine := build

engine_root := $(source)/$(engine)
engine_src := $(engine_root)/src
engine_inc := $(engine_root)/include
engine_obj := $(obj)/$(engine)

engine_cflags :=

engine_deps :=

ifneq (1,$(SDL_TARGET))
    engine_deps += imgui
endif

ifneq (0,$(USE_PHYSFS))
    engine_deps += physfs
endif

ifneq (0,$(USE_MIMALLOC))
    engine_deps += mimalloc
endif

engine_editor_objs := \
    build.cpp \
    config.cpp \

engine_tools_objs := \
    colmatch.cpp \
    compat.cpp \
    crc32.cpp \
    klzw.cpp \
    kplib.cpp \
    loguru.cpp \
    lz4.cpp \
    pragmas.cpp \
    smmalloc.cpp \
    smmalloc_generic.cpp \
    smmalloc_tls.cpp \
    vfs.cpp \

engine_excl := \
    a-c.cpp \
    sdlayer12.cpp \
    sdlkeytrans.cpp \
    startgtk.editor.cpp \
    startwin.editor.cpp \
    $(engine_editor_objs) \

ifeq (1,$(USE_OPENGL))
    engine_deps += glad
else
    engine_excl += \
        glbuild.cpp \
        glsurface.cpp \
        mdsprite.cpp \
        polymer.cpp \
        polymost.cpp \
        tilepacker.cpp \
        voxmodel.cpp \

endif

ifeq ($(PLATFORM),WINDOWS)
    ifeq ($(STARTUP_WINDOW),1)
        engine_editor_objs += startwin.editor.cpp
    endif
else
    engine_excl += winbits.cpp
endif

ifeq ($(PLATFORM),WII)
    LINKERFLAGS += -Wl,-wrap,c_default_exceptionhandler
else
    engine_excl += wiibits.cpp
endif

ifneq ($(RENDERTYPE),SDL)
    engine_excl += sdlayer.cpp
endif
ifneq ($(RENDERTYPE),WIN)
    engine_excl += winlayer.cpp rawinput.cpp
endif

ifeq (1,$(HAVE_GTK2))
    ifeq ($(STARTUP_WINDOW),1)
        engine_editor_objs += startgtk.editor.cpp
    endif
else
    engine_excl += gtkbits.cpp dynamicgtk.cpp
endif

ifeq ($(USE_LIBVPX),0)
    engine_excl += animvpx.cpp
endif

engine_objs := \
    $(call getfiltered,engine,*.c) \
    $(call getfiltered,engine,*.cpp) \
    polymost1Frag.glsl \
    polymost1Vert.glsl \

ifeq (0,$(NOASM))
  engine_objs += a.nasm
else
  engine_objs += a-c.cpp
endif

ifeq ($(PLATFORM),DARWIN)
    engine_objs += osxbits.mm
    engine_tools_objs += osxbits.mm
    ifeq ($(STARTUP_WINDOW),1)
        engine_editor_objs += startosx.editor.mm
    endif
    ifeq ($(SDL_TARGET),1)
        ifneq ($(SDL_FRAMEWORK),0)
            engine_objs += SDLMain.mm
        endif
    endif
endif


#### mact

mact := mact

mact_root := $(source)/$(mact)
mact_src := $(mact_root)/src
mact_inc := $(mact_root)/include
mact_obj := $(obj)/$(mact)

mact_excl :=
mact_objs := $(call getfiltered,mact,*.cpp)

mact_cflags :=

#### AudioLib

audiolib := audiolib

audiolib_root := $(source)/$(audiolib)
audiolib_src := $(audiolib_root)/src
audiolib_inc := $(audiolib_root)/include
audiolib_obj := $(obj)/$(audiolib)

audiolib_deps :=

audiolib_excl := \
    music_external.cpp \

ifneq ($(PLATFORM),WINDOWS)
    audiolib_excl += driver_directsound.cpp driver_winmm.cpp
endif
ifneq ($(SUBPLATFORM),LINUX)
    audiolib_excl += driver_alsa.cpp
endif
ifneq ($(RENDERTYPE),SDL)
    audiolib_excl += driver_sdl.cpp
endif

audiolib_objs := $(call getfiltered,audiolib,*.cpp)

audiolib_cflags :=

ifneq (0,$(HAVE_XMP))
    audiolib_cflags += -I$(libxmplite_inc)
    audiolib_deps += libxmplite
endif


#### Tools

tools := tools

tools_objs := \
    compat_tools.cpp \

tools_root := $(source)/$(tools)
tools_src := $(tools_root)/src
tools_obj := $(obj)/$(tools)

tools_cflags := $(engine_cflags) -I$(engine_src)

tools_deps := engine_tools

ifneq (0,$(USE_MIMALLOC))
    tools_deps += mimalloc
endif

tools_targets := \
    arttool \
    bsuite \
    cacheinfo \
    generateicon \
    givedepth \
    ivfrate \
    kextract \
    kgroup \
    kmd2tool \
    map2stl \
    md2tool \
    mkpalette \
    transpal \
    unpackssi \
    wad2art \
    wad2map \

ifeq ($(PLATFORM),WINDOWS)
    tools_targets += enumdisplay getdxdidf
endif
ifeq ($(RENDERTYPE),SDL)
    tools_targets += makesdlkeytrans
endif


#### KenBuild (Test Game)

kenbuild := kenbuild

kenbuild_root := $(source)/$(kenbuild)
kenbuild_src := $(kenbuild_root)/src
kenbuild_rsrc := $(kenbuild_root)/rsrc
kenbuild_obj := $(obj)/$(kenbuild)

kenbuild_cflags := -I$(kenbuild_src)

kenbuild_game := ekenbuild
kenbuild_editor := ekenbuild-editor

kenbuild_game_deps := audiolib

kenbuild_game_proper := EKenBuild
kenbuild_editor_proper := EKenBuild-Editor

kenbuild_game_objs := \
    common.cpp \
    config.cpp \
    kdmeng.cpp \
    game.cpp \

kenbuild_editor_objs := \
    bstub.cpp \
    common.cpp \

kenbuild_game_rsrc_objs :=
kenbuild_editor_rsrc_objs :=
kenbuild_game_gen_objs :=
kenbuild_editor_rsrc_objs :=

ifeq (11,$(HAVE_GTK2)$(STARTUP_WINDOW))
    kenbuild_game_objs += startgtk.game.cpp
    kenbuild_game_gen_objs += game_banner.c
    kenbuild_editor_gen_objs += build_banner.c
endif
ifeq ($(RENDERTYPE),SDL)
    kenbuild_game_rsrc_objs += game_icon.c
    kenbuild_editor_rsrc_objs += build_icon.c
endif
ifeq ($(PLATFORM),WINDOWS)
    kenbuild_game_objs += startwin.game.cpp
    kenbuild_game_rsrc_objs += gameres.rc
    kenbuild_editor_rsrc_objs += buildres.rc
endif

ifeq ($(PLATFORM),DARWIN)
    ifeq ($(STARTUP_WINDOW),1)
        kenbuild_game_objs += StartupWinController.game.mm
    endif
endif


#### TekWar

tekwar := tekwar

tekwar_root := $(source)/$(tekwar)
tekwar_src := $(tekwar_root)/src
tekwar_rsrc := $(tekwar_root)/rsrc
tekwar_obj := $(obj)/$(tekwar)

tekwar_cflags := -I$(tekwar_src)

tekwar_game := etekwar
tekwar_editor := etekwar-editor

tekwar_game_proper := ETekWar
tekwar_editor_proper := ETekWar Editor

tekwar_game_deps := audiolib mact libsmackerdec hmpplay

tekwar_game_objs := \
    b5compat.cpp \
    common.cpp \
    config.cpp \
    grpscan.cpp \
    osdcmds.cpp \
    tekcdr.cpp \
    tekchng.cpp \
    tekgame.cpp \
    tekgun.cpp \
    tekldsv.cpp \
    tekmap.cpp \
    tekmsc.cpp \
    tekprep.cpp \
    teksmk.cpp \
    teksnd.cpp \
    tekspr.cpp \
    tekstat.cpp \
    tektag.cpp \
    tektxt.cpp \
    tekver.cpp \

tekwar_editor_objs := \
    bstub.cpp \

tekwar_game_rsrc_objs :=
tekwar_editor_rsrc_objs :=
tekwar_game_gen_objs :=
tekwar_editor_rsrc_objs :=

ifeq (11,$(HAVE_GTK2)$(STARTUP_WINDOW))
    tekwar_game_objs += startgtk.game.cpp
    tekwar_game_gen_objs += game_banner.c
    tekwar_editor_gen_objs += build_banner.c
endif
ifeq ($(RENDERTYPE),SDL)
    tekwar_game_rsrc_objs += game_icon.c
    tekwar_editor_rsrc_objs += game_icon.c
endif
ifeq ($(PLATFORM),WINDOWS)
    tekwar_game_objs += startwin.game.cpp
    tekwar_game_rsrc_objs += gameres.rc
    tekwar_editor_rsrc_objs += buildres.rc
endif


#### Duke Nukem 3D

duke3d := duke3d

duke3d_game_ldflags :=
duke3d_editor_ldflags :=

duke3d_game_stripflags :=
duke3d_editor_stripflags :=

duke3d_root := $(source)/$(duke3d)
duke3d_src := $(duke3d_root)/src
duke3d_rsrc := $(duke3d_root)/rsrc
duke3d_obj := $(obj)/$(duke3d)

ifneq (,$(APPBASENAME))
    ifeq ($(PLATFORM),WINDOWS)
        duke3d_rsrc := $(duke3d_root)/rsrc/$(APPBASENAME)
    endif
    duke3d_obj := $(obj)/$(APPBASENAME)
endif

duke3d_cflags :=

common_editor_deps := duke3d_common_editor engine_editor

duke3d_game_deps := audiolib mact
duke3d_editor_deps := audiolib

duke3d_game := eduke32
duke3d_editor := mapster32

ifneq (,$(APPBASENAME))
    duke3d_game := $(APPBASENAME)
endif

duke3d_game_proper := EDuke32
duke3d_editor_proper := Mapster32

duke3d_common_editor_objs := \
    m32common.cpp \
    m32def.cpp \
    m32exec.cpp \
    m32vars.cpp \

duke3d_editor_objs := \
    astub.cpp \
    common.cpp \
    grpscan.cpp \
    sounds_mapster32.cpp \

duke3d_excl := \
    in_android.cpp \
    m32structures.cpp \
    mdump.cpp \
    startgtk.game.cpp \
    startwin.game.cpp \
    $(duke3d_common_editor_objs) \
    $(duke3d_editor_objs) \

duke3d_game_objs := $(call getfiltered,duke3d,*.cpp) \
    common.cpp \
    grpscan.cpp \

duke3d_game_rsrc_objs :=
duke3d_editor_rsrc_objs :=
duke3d_game_gen_objs :=
duke3d_editor_gen_objs :=

duke3d_game_miscdeps :=
duke3d_editor_miscdeps :=
duke3d_game_orderonlydeps :=
duke3d_editor_orderonlydeps :=

ifeq ($(SUBPLATFORM),LINUX)
    LIBS += -lFLAC -lasound
endif

ifeq ($(PLATFORM),BSD)
    LIBS += -lFLAC -lexecinfo
endif

ifeq ($(PLATFORM),DARWIN)
    LIBS += -lFLAC \
            -Wl,-framework,Cocoa -Wl,-framework,Carbon \
            -Wl,-framework,CoreMIDI -Wl,-framework,AudioUnit \
            -Wl,-framework,AudioToolbox -Wl,-framework,IOKit
    ifneq (00,$(DARWIN9)$(DARWIN10))
        LIBS += -Wl,-framework,QuickTime
    endif

    ifeq ($(STARTUP_WINDOW),1)
        duke3d_game_objs += GrpFile.game.mm GameListSource.game.mm startosx.game.mm
    endif
endif

ifeq ($(PLATFORM),WINDOWS)
    LIBS += -lFLAC -ldsound
    duke3d_game_objs += winbits.cpp
    duke3d_game_rsrc_objs += gameres.rc
    duke3d_editor_rsrc_objs += buildres.rc
    ifeq ($(STARTUP_WINDOW),1)
        duke3d_game_objs += startwin.game.cpp
    endif
endif

ifeq (11,$(HAVE_GTK2)$(STARTUP_WINDOW))
    duke3d_game_objs += startgtk.game.cpp
    duke3d_game_gen_objs += game_banner.c
    duke3d_editor_gen_objs += build_banner.c
endif
ifeq ($(RENDERTYPE),SDL)
    duke3d_game_rsrc_objs += game_icon.c
    duke3d_editor_rsrc_objs += build_icon.c
endif


#### Blood

blood := blood

blood_game_ldflags :=

blood_game_stripflags :=

blood_root := $(source)/$(blood)
blood_src := $(blood_root)/src
blood_rsrc := $(blood_root)/rsrc
blood_obj := $(obj)/$(blood)

blood_cflags := -I$(blood_src)

blood_game_deps := audiolib mact libsmackerdec

ifneq (0,$(NETCODE))
    blood_game_deps += enet
endif

blood_game := nblood

ifneq (,$(APPBASENAME))
    blood_game := $(APPBASENAME)
endif

blood_game_proper := NBlood

blood_game_objs := \
	blood.cpp \
	actor.cpp \
	ai.cpp \
	aibat.cpp \
	aibeast.cpp \
	aiboneel.cpp \
	aiburn.cpp \
	aicaleb.cpp \
	aicerber.cpp \
	aicult.cpp \
	aigarg.cpp \
	aighost.cpp \
	aigilbst.cpp \
	aihand.cpp \
	aihound.cpp \
	aiinnoc.cpp \
	aipod.cpp \
	airat.cpp \
	aispid.cpp \
	aitchern.cpp \
	aizomba.cpp \
	aizombf.cpp \
	asound.cpp \
	barf.cpp \
	callback.cpp \
	choke.cpp \
	common.cpp \
	config.cpp \
	controls.cpp \
	credits.cpp \
	db.cpp \
	demo.cpp \
	dude.cpp \
	endgame.cpp \
	eventq.cpp \
	fire.cpp \
	fx.cpp \
	gamemenu.cpp \
	gameutil.cpp \
	getopt.cpp \
	gfx.cpp \
	gib.cpp \
	globals.cpp \
	gui.cpp \
	inifile.cpp \
	iob.cpp \
	levels.cpp \
	loadsave.cpp \
	map2d.cpp \
	menu.cpp \
	messages.cpp \
	mirrors.cpp \
	misc.cpp \
	network.cpp \
	osdcmd.cpp \
	player.cpp \
	qav.cpp \
	qheap.cpp \
	replace.cpp \
	resource.cpp \
	screen.cpp \
	sectorfx.cpp \
	seq.cpp \
	sfx.cpp \
	sound.cpp \
	tile.cpp \
	trig.cpp \
	triggers.cpp \
	view.cpp \
	warp.cpp \
	weapon.cpp \

ifeq ($(NOONE_EXTENSIONS),1)
    blood_game_objs += nnextsif.cpp
    blood_game_objs += nnexts.cpp
    blood_game_objs += nnextstr.cpp
    blood_game_objs += nnextcitem.cpp
    blood_game_objs += nnextcdud.cpp
    blood_game_objs += aicdud.cpp
endif

blood_game_rsrc_objs :=
blood_game_gen_objs :=

blood_game_miscdeps :=
blood_game_orderonlydeps :=

ifeq ($(PLATFORM),DARWIN)
    ifeq ($(STARTUP_WINDOW),1)
        blood_game_objs += startosx.game.mm
    endif
endif

ifeq ($(PLATFORM),WINDOWS)
    blood_game_objs += winbits.cpp
    blood_game_rsrc_objs += gameres.rc
    ifeq ($(STARTUP_WINDOW),1)
        blood_game_objs += startwin.game.cpp
    endif
endif

ifeq (11,$(HAVE_GTK2)$(STARTUP_WINDOW))
    blood_game_objs += startgtk.game.cpp
    blood_game_gen_objs += game_banner.c
endif
ifeq ($(RENDERTYPE),SDL)
    blood_game_rsrc_objs += game_icon.c
endif


#### Redneck Rampage

rr := rr

rr_game_ldflags :=
rr_editor_ldflags :=

rr_game_stripflags :=
rr_editor_stripflags :=

rr_root := $(source)/$(rr)
rr_src := $(rr_root)/src
rr_rsrc := $(rr_root)/rsrc
rr_obj := $(obj)/$(rr)

rr_cflags := -I$(rr_src)

common_editor_deps := rr_common_editor engine_editor

rr_game_deps := audiolib mact
rr_editor_deps := audiolib

ifneq (0,$(NETCODE))
    rr_game_deps += enet
endif

rr_game := rednukem
rr_editor := rrmapster32

ifneq (,$(APPBASENAME))
    rr_game := $(APPBASENAME)
endif

rr_game_proper := Rednukem
rr_editor_proper := RRMapster32

rr_common_editor_objs := \
    m32common.cpp \
    m32def.cpp \
    m32exec.cpp \
    m32vars.cpp \

rr_game_objs := \
    game.cpp \
    global.cpp \
    actors.cpp \
    gamedef.cpp \
    gameexec.cpp \
    gamevars.cpp \
    player.cpp \
    premap.cpp \
    sector.cpp \
    anim.cpp \
    common.cpp \
    config.cpp \
    demo.cpp \
    input.cpp \
    menus.cpp \
    namesdyn.cpp \
    net.cpp \
    savegame.cpp \
    rts.cpp \
    osdfuncs.cpp \
    osdcmds.cpp \
    grpscan.cpp \
    sounds.cpp \
    soundsdyn.cpp \
    cheats.cpp \
    sbar.cpp \
    screentext.cpp \
    screens.cpp \
    cmdline.cpp \
    rrdh.cpp \
    filestream.cpp \
    playmve.cpp \

rr_editor_objs := \
    astub.cpp \
    common.cpp \
    grpscan.cpp \
    sounds_mapster32.cpp \

rr_game_rsrc_objs :=
rr_editor_rsrc_objs :=
rr_game_gen_objs :=
rr_editor_gen_objs :=

rr_game_miscdeps :=
rr_editor_miscdeps :=
rr_game_orderonlydeps :=
rr_editor_orderonlydeps :=

ifeq ($(PLATFORM),DARWIN)
    ifeq ($(STARTUP_WINDOW),1)
        rr_game_objs += GrpFile.game.mm GameListSource.game.mm startosx.game.mm
    endif
endif

ifeq ($(PLATFORM),WINDOWS)
    rr_game_objs += winbits.cpp
    rr_game_rsrc_objs += gameres.rc
    rr_editor_rsrc_objs += buildres.rc
    ifeq ($(STARTUP_WINDOW),1)
        rr_game_objs += startwin.game.cpp
    endif
endif

ifeq (11,$(HAVE_GTK2)$(STARTUP_WINDOW))
    rr_game_objs += startgtk.game.cpp
    rr_game_gen_objs += game_banner.c
    rr_editor_gen_objs += build_banner.c
endif
ifeq ($(RENDERTYPE),SDL)
    rr_game_rsrc_objs += game_icon.c
    rr_editor_rsrc_objs += build_icon.c
endif

n64 := n64
n64_src := $(rr_src)/$(n64)
n64_obj := $(rr_obj)/$(n64)
n64_objs := \
    reality.cpp \
    reality_music.cpp \
    reality_player.cpp \
    reality_render.cpp \
    reality_sbar.cpp \
    reality_screens.cpp \
    reality_sound.cpp \
    reality_util.cpp \

n64_cflags :=

rr_game_deps += n64


#### Shadow Warrior

sw := sw

sw_root := $(source)/$(sw)
sw_src := $(sw_root)/src
sw_rsrc := $(sw_root)/rsrc
sw_obj := $(obj)/$(sw)

sw_cflags :=

sw_game_deps := audiolib mact
sw_editor_deps := audiolib

sw_game := voidsw
sw_editor := wangulator

sw_game_proper := VoidSW
sw_editor_proper := Wangulator

sw_editor_objs := \
    bldscript.cpp \
    brooms.cpp \
    colormap.cpp \
    common.cpp \
    grpscan.cpp \
    jbhlp.cpp \
    jnstub.cpp \

sw_excl := \
    startgtk.game.cpp \
    startwin.game.cpp \
    $(sw_editor_objs) \

sw_game_objs := $(call getfiltered,sw,*.cpp) \
    colormap.cpp \
    common.cpp \
    grpscan.cpp \

sw_game_rsrc_objs :=
sw_editor_rsrc_objs :=
sw_game_gen_objs :=
sw_editor_gen_objs :=

ifeq (11,$(HAVE_GTK2)$(STARTUP_WINDOW))
    sw_game_objs += startgtk.game.cpp
    sw_game_gen_objs += game_banner.c
    sw_editor_gen_objs += build_banner.c
endif
ifeq ($(RENDERTYPE),SDL)
    sw_game_rsrc_objs += game_icon.c
    sw_editor_rsrc_objs += game_icon.c
endif
ifeq ($(PLATFORM),WINDOWS)
    sw_game_objs += startwin.game.cpp
    sw_game_rsrc_objs += gameres.rc
    sw_editor_rsrc_objs += buildres.rc
endif
ifeq ($(PLATFORM),DARWIN)
    ifeq ($(STARTUP_WINDOW),1)
        sw_game_objs += GrpFile.game.mm GameListSource.game.mm StartupWinController.game.mm
    endif
endif


#### Exhumed

exhumed := exhumed

exhumed_root := $(source)/$(exhumed)
exhumed_src := $(exhumed_root)/src
exhumed_rsrc := $(exhumed_root)/rsrc
exhumed_obj := $(obj)/$(exhumed)

exhumed_cflags := -I$(exhumed_src)

exhumed_game_deps := duke3d_common_midi audiolib mact
exhumed_editor_deps := audiolib

exhumed_game := pcexhumed
exhumed_editor := pcexhumed_editor

exhumed_game_proper := PCExhumed
exhumed_editor_proper := PCExhumed Editor

exhumed_game_objs := \
    aistuff.cpp \
    anims.cpp \
    anubis.cpp \
    bubbles.cpp \
    bullet.cpp \
    cd.cpp \
    common.cpp \
    config.cpp \
    enginesubs.cpp \
    exhumed.cpp \
    exscript.cpp \
    fish.cpp \
    grenade.cpp \
    grpscan.cpp \
    gun.cpp \
    init.cpp \
    input.cpp \
    items.cpp \
    lavadude.cpp \
    light.cpp \
    lighting.cpp \
    lion.cpp \
    map.cpp \
    memorystream.cpp \
    menu.cpp \
    mono.cpp \
    move.cpp \
    movie.cpp \
    mummy.cpp \
    network.cpp \
    object.cpp \
    osdcmds.cpp \
    player.cpp \
    queen.cpp \
    ra.cpp \
    ramses.cpp \
    random.cpp \
    rat.cpp \
    record.cpp \
    rex.cpp \
    roach.cpp \
    runlist.cpp \
    save.cpp \
    scorp.cpp \
    sequence.cpp \
    serial.cpp \
    set.cpp \
    snake.cpp \
    sound.cpp \
    spider.cpp \
    status.cpp \
    stream.cpp \
    switch.cpp \
    text2.cpp \
    timer.cpp \
    trigdat.cpp \
    version.cpp \
    view.cpp \
    wasp.cpp \

exhumed_editor_objs :=

exhumed_game_rsrc_objs :=
exhumed_editor_rsrc_objs :=
exhumed_game_gen_objs :=
exhumed_editor_gen_objs :=

ifeq (11,$(HAVE_GTK2)$(STARTUP_WINDOW))
    exhumed_game_objs += startgtk.game.cpp
    exhumed_game_gen_objs += game_banner.c
    exhumed_editor_gen_objs += build_banner.c
endif
ifeq ($(RENDERTYPE),SDL)
    exhumed_game_rsrc_objs += game_icon.c
    exhumed_editor_rsrc_objs += game_icon.c
endif
ifeq ($(PLATFORM),WINDOWS)
    exhumed_game_objs += startwin.game.cpp
    exhumed_game_rsrc_objs += gameres.rc
    exhumed_editor_rsrc_objs += buildres.rc
endif
ifeq ($(PLATFORM),DARWIN)
    ifeq ($(STARTUP_WINDOW),1)
        exhumed_game_objs += GrpFile.game.mm GameListSource.game.mm startosx.game.mm
    endif
endif


#### Witchaven

witchaven := witchaven

witchaven_root := $(source)/$(witchaven)
witchaven_src := $(witchaven_root)/src
witchaven_rsrc := $(witchaven_root)/rsrc
witchaven_obj := $(obj)/$(witchaven)

witchaven_cflags := -I$(witchaven_src)

witchaven_game_deps := duke3d_common_midi audiolib mact hmpplay
witchaven_editor_deps := audiolib

witchaven_game := ewitchaven
witchaven_editor := ewitchaven_editor

witchaven_game_proper := EWitchaven
witchaven_editor_proper := EWitchaven Editor

witchaven_game_objs := \
    animation.cpp \
    common.cpp \
    config.cpp \
    effects.cpp \
    enginesubs.cpp \
    grpscan.cpp \
    input.cpp \
    menu.cpp \
    network.cpp \
    objects.cpp \
    osdcmds.cpp \
    player.cpp \
    sound.cpp \
    tags.cpp \
    view.cpp \
    witchaven.cpp \

witchaven_editor_objs :=

witchaven_game_rsrc_objs :=
witchaven_editor_rsrc_objs :=
witchaven_game_gen_objs :=
witchaven_editor_gen_objs :=

ifeq (11,$(HAVE_GTK2)$(STARTUP_WINDOW))
    witchaven_game_objs += startgtk.game.cpp
    witchaven_game_gen_objs += game_banner.c
    witchaven_editor_gen_objs += build_banner.c
endif
ifeq ($(RENDERTYPE),SDL)
    witchaven_game_rsrc_objs += game_icon.c
    witchaven_editor_rsrc_objs += game_icon.c
endif
ifeq ($(PLATFORM),WINDOWS)
    witchaven_game_objs += startwin.game.cpp
    witchaven_game_rsrc_objs += gameres.rc
    witchaven_editor_rsrc_objs += buildres.rc
endif


#### Includes

COMPILERFLAGS += \
    -MP -MMD \
    -I$(engine_inc) \
    -I$(mact_inc) \
    -I$(audiolib_inc) \
    -I$(glad_inc) \
    -I$(voidwrap_inc) \
    -I$(imgui_inc) \
    -I$(libsmackerdec_inc) \
    -I$(hmpplay_inc) \

ifneq (0,$(USE_MIMALLOC))
    COMPILERFLAGS += -I$(mimalloc_inc)
endif

ifneq (0,$(USE_PHYSFS))
    COMPILERFLAGS += -I$(physfs_inc) -DUSE_PHYSFS
endif

ifneq (0,$(MICROPROFILE))
  COMPILERFLAGS += -DMICROPROFILE_ENABLED=1
endif

##### Recipes

games := \
    duke3d \
    kenbuild \
    blood \
    rr \
    sw \
    exhumed \
    witchaven \
	tekwar \

libraries := \
    audiolib \
    engine \
    glad \
    imgui \
    libxmplite \
    mact \
    voidwrap \
    libsmackerdec \
    hmpplay \
    n64 \

ifneq (0,$(USE_MIMALLOC))
    libraries += mimalloc
endif

ifneq (0,$(USE_PHYSFS))
    libraries += physfs
endif

components := \
    $(games) \
    $(libraries) \
    tools \

roles := \
    game \
#    editor \


ifeq ($(PRETTY_OUTPUT),1)
.SILENT:
endif
.PHONY: \
    $(addprefix clean,$(games) test utils tools) \
    $(engine_obj)/rev.$o \
    all \
    clang-tools \
    clean \
    printtools \
    printutils \
    rev \
    start \
    veryclean \

.SUFFIXES:
.SECONDEXPANSION:


#### Targets

all: \
    blood \
    rr \
    exhumed \

start:
	$(BUILD_STARTED)

tools: $(addsuffix $(EXESUFFIX),$(tools_targets)) | start
	@$(call LL,$^)

$(games): $$(foreach i,$(roles),$$($$@_$$i)$(EXESUFFIX)) | start
	@$(call LL,$^)

ebacktrace: $(ebacktrace_dll) | start
	@$(call LL,$^)

voidwrap: $(voidwrap_lib) | start
	@$(call LL,$^)

ifeq ($(PLATFORM),WII)
ifneq ($(ELF2DOL),)
%$(DOLSUFFIX): %$(EXESUFFIX)
endif
endif


define BUILDRULE

$$($1_$2)$$(EXESUFFIX): $$(foreach i,$(call getdeps,$1,$2),$$(call expandobjs,$$i)) $$($1_$2_miscdeps) | $$($1_$2_orderonlydeps)
	$$(LINK_STATUS)
	$$(call MKDIR,"$$(obj)/$$($1_$2)_dump")
	$$(RECIPE_IF) $$(LINKER) $$(call LF,$$(obj)/$$($1_$2)_dump) -o $$@ $$^ $$(GUI_LIBS) $$($1_$2_ldflags) $$(LIBDIRS) $$(LIBS) $$(RECIPE_RESULT_LINK)
ifeq ($$(PLATFORM),WII)
ifneq ($$(ELF2DOL),)
	$$(ELF2DOL) $$@ $$($1_$2)$$(DOLSUFFIX)
endif
endif
ifneq ($$(STRIP),)
	$$(STRIP) $$@ $$($1_$2_stripflags)
endif
ifeq ($$(PLATFORM),DARWIN)
	cp -RPf "platform/Apple/bundles/$$($1_$2_proper).app" "./"
	$(call MKDIR,"$$($1_$2_proper).app/Contents/MacOS")
	cp -f "$$($1_$2)$$(EXESUFFIX)" "$$($1_$2_proper).app/Contents/MacOS/"
endif
	$$(call RMDIR,"$$(obj)/$$($1_$2)_dump")

endef

$(foreach i,$(games),$(foreach j,$(roles),$(eval $(call BUILDRULE,$i,$j))))


#### Rules

$(ebacktrace_dll): platform/Windows/src/backtrace.c
	$(COMPILE_STATUS)
	$(RECIPE_IF) $(CC) $(CONLYFLAGS) -O2 -ggdb -shared -Wall -Wextra -static-libgcc -I$(engine_inc) -o $@ $^ -lbfd -liberty -limagehlp $(RECIPE_RESULT_COMPILE)

libklzw$(DLLSUFFIX): $(engine_src)/klzw.cpp
	$(COMPILE_STATUS)
	$(RECIPE_IF) $(COMPILER_C) -shared -fPIC $< -o $@ $(RECIPE_RESULT_COMPILE)

# to debug the tools link phase, make a copy of this rule explicitly replacing % with the name of a tool, such as kextract
%$(EXESUFFIX): $(tools_obj)/%.$o $(foreach i,tools $(tools_deps),$(call expandobjs,$i))
	$(LINK_STATUS)
	$(call MKDIR,"$(tools_obj)/$*")
	$(RECIPE_IF) $(LINKER) $(call LF,$(tools_obj)/$*) -o $@ $^ $(LIBDIRS) $(LIBS) $(RECIPE_RESULT_LINK)
ifneq ($(STRIP),)
	$(STRIP) $@
endif
	$(call RMDIR,"$(tools_obj)/$*")


### Voidwrap

$(voidwrap_lib): $(foreach i,$(voidwrap),$(call expandobjs,$i))
	$(LINK_STATUS)
	$(RECIPE_IF) $(LINKER) $(call LF,$(voidwrap_obj)) -shared -Wl,-soname,$@ -o $@ $^ $(LIBDIRS) $(voidwrap_root)/sdk/redistributable_bin/$(steamworks_lib) $(RECIPE_RESULT_LINK)
ifneq ($(STRIP),)
	$(STRIP) $@
endif


### Main Rules

define OBJECTRULES

include $(wildcard $($1_obj)/*.d)

$$($1_obj)/%.$$o: $$($1_src)/%.nasm | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(RECIPE_IF) $$(AS) $$(ASFLAGS) $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.yasm | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(RECIPE_IF) $$(AS) $$(ASFLAGS) $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.c | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_C) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.cpp | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_CXX) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.m | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_OBJC) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.mm | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_OBJCXX) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_obj)/%.c | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_C) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_src)/%.glsl | $$($1_obj)
	@echo Creating $$($1_obj)/$$(<F).cpp from $$<
	@$$(call RAW_ECHO,extern char const *$$(basename $$(<F));) > $$($1_obj)/$$(<F).cpp
	@$$(call RAW_ECHO,char const *$$(basename $$(<F)) = R"shader$$(paren_open)) >> $$($1_obj)/$$(<F).cpp
	@$$(call CAT,$$<) >> $$($1_obj)/$$(<F).cpp
	@$$(call RAW_ECHO,$$(paren_close)shader";) >> $$($1_obj)/$$(<F).cpp
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_CXX) $$($1_cflags) -c $$($1_obj)/$$(<F).cpp -o $$@ $$(RECIPE_RESULT_COMPILE)

## Cosmetic stuff

$$($1_obj)/%.$$o: $$($1_rsrc)/%.rc | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(RC) -i $$< -o $$@ --include-dir=$$(engine_inc) --include-dir=$$($1_src) --include-dir=$$($1_rsrc) -DPOLYMER=$$(POLYMER) $(REVFLAG) $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%.$$o: $$($1_rsrc)/%.c | $$($1_obj)
	$$(COMPILE_STATUS)
	$$(call MKDIR,$$(dir $$@))
	$$(RECIPE_IF) $$(COMPILER_C) $$($1_cflags) -c $$< -o $$@ $$(RECIPE_RESULT_COMPILE)

$$($1_obj)/%_banner.c: $$($1_rsrc)/%.bmp | $$($1_obj)
	echo "#include \"gtkpixdata_shim.h\"" > $$@
	gdk-pixbuf-csource --extern --struct --raw --name=startbanner_pixdata $$^ | sed 's/load_inc//' >> $$@

endef

$(foreach i,$(components),$(eval $(call OBJECTRULES,$i)))


### Other special cases

# Comment out the following rule to debug a-c.o
$(engine_obj)/a-c.$o: $(engine_src)/a-c.cpp | $(engine_obj)
	$(COMPILE_STATUS)
	$(RECIPE_IF) $(subst -O$(OPTLEVEL),-O2,$(subst $(ASAN_FLAGS),,$(COMPILER_CXX))) $(engine_cflags) -c $< -o $@ $(RECIPE_RESULT_COMPILE)

$(engine_obj)/rev.$o: $(engine_src)/rev.cpp | $(engine_obj)
	$(COMPILE_STATUS)
	$(RECIPE_IF) $(COMPILER_CXX) $(engine_cflags) $(REVFLAG) -c $< -o $@ $(RECIPE_RESULT_COMPILE)


### Directories

$(foreach i,$(components),$($i_obj)):
	-$(call MKDIR,$@)

### Phonies

clang-tools: $(filter %.c %.cpp,$(foreach i,$(call getdeps,duke3d,game),$(call expandsrcs,$i)))
	echo $^ -- -x c++ $(CXXONLYFLAGS) $(COMPILERFLAGS) $(CWARNS) $(foreach i,$(components),$($i_cflags))

$(addprefix clean,$(games)):
	-$(call RM,$(foreach i,$(roles),$($(subst clean,,$@)_$i)$(EXESUFFIX)))
	-$(call RMDIR,$($(subst clean,,$@)_obj))
ifeq ($(PLATFORM),DARWIN)
	-$(call RMDIR,$(foreach i,$(roles),"$($(subst clean,,$@)_$i_proper).app"))
endif

cleantools:
	-$(call RM,$(addsuffix $(EXESUFFIX),$($(subst clean,,$@)_targets)))
	-$(call RMDIR,$($(subst clean,,$@)_obj))

clean: cleanduke3d cleansw cleanblood cleanrr cleanexhumed cleanwitchaven cleantekwar cleantools
	-$(call RMDIR,$(obj))
	-$(call RM,$(ebacktrace_dll))
	-$(call RM,$(voidwrap_lib))

printtools:
	echo "$(addsuffix $(EXESUFFIX),$(tools_targets))"

rev: $(engine_obj)/rev.$o


### Compatibility

cleantest: cleankenbuild
cleanutils: cleantools
printutils: printtools
test: kenbuild
utils: tools
veryclean: clean
