// SDL 1.2 compatibility.

#include "build.h"
#include "mutex.h"

#include <SDL/SDL_events.h>

#define SURFACE_FLAGS	(SDL_SWSURFACE|SDL_HWPALETTE|SDL_HWACCEL|SDL_RESIZABLE)

#ifdef _WIN32
#include "winbits.h"

HWND win_gethwnd(void)
{
    struct SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    if (SDL_GetWMInfo(&wmInfo) != 1)
    {
        LOG_F(ERROR, "Couldn't get window handle");
        return 0;
    }

    return wmInfo.window;
}
#endif

int32_t videoSetVsync(int32_t newSync)
{
    if (vsync_renderlayer == newSync)
        return newSync;

    vsync_renderlayer = newSync;

    videoResetMode();
    if (videoSetGameMode(fullscreen, xres, yres, bpp, upscalefactor))
        LOG_F(ERROR, "Failed to set video mode!");

    return newSync;
}

int32_t sdlayer_checkversion(void)
{
    const SDL_version *linked = SDL_Linked_Version();
    SDL_version compiled;

    SDL_VERSION(&compiled);

    int constexpr bufsiz = 128;
    auto buf = (char*)Balloca(bufsiz);
    buf[0] = 0;

    if (!Bmemcmp(&compiled, &linked, sizeof(SDL_version)))
        Bsnprintf(buf, bufsiz, "Initializing SDL %d.%d.%d", compiled.major, compiled.minor, compiled.patch);
    else
        Bsnprintf(buf, bufsiz, "Initializing SDL %d.%d.%d (built against version %d.%d.%d)",
            linked->major, linked->minor, linked->patch, compiled.major, compiled.minor, compiled.patch);

    LOG_F(INFO, "%s", buf);

    if (SDL_VERSIONNUM(linked->major, linked->minor, linked->patch) < SDL_REQUIREDVERSION)
    {
        /*reject running under SDL versions older than what is stated in sdl_inc.h */
        LOG_F(ERROR, "You need SDL %d.%d.%d or newer to run %s.",SDL_MIN_X,SDL_MIN_Y,SDL_MIN_Z,apptitle);
        return -1;
    }

    return 0;
}

//
// initsystem() -- init SDL systems
//
int32_t initsystem(void)
{
    // TODO: Refactor the init sequence such that we don't need to duplicate this across both sdlayer objects.
#if defined NOSDLPARACHUTE
    const int sdlinitflags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;
#else
    const int sdlinitflags = SDL_INIT_VIDEO;
#endif

    mutex_init(&m_initprintf);

#ifdef _WIN32
    windowsPlatformInit();
#endif
    sysReadCPUID();

    if (sdlayer_checkversion())
        return -1;

    if (SDL_Init(sdlinitflags))
    {
        LOG_F(ERROR, "SDL initialization failed: %s.", SDL_GetError());
        LOG_F(WARNING, "Non-interactive mode enabled; this is probably not what you want.");
        VLOG_F(LOG_GFX, "Video output disabled.");

        novideo = 1;
#ifdef USE_OPENGL
        nogl = 1;
#endif
    }

    frameplace = 0;
    lockcount = 0;


    if (!novideo)
    {
        char drvname[32];

#ifdef USE_OPENGL
        if (SDL_GL_LoadLibrary(0))
        {
            LOG_F(ERROR, "Failed loading OpenGL driver: %s; all OpenGL modes are unavailable.", SDL_GetError());
            nogl = 1;
        }
#endif
        if (SDL_VideoDriverName(drvname, 32))
            LOG_F(INFO, "Using '%s' video driver.", drvname);

        wm_setapptitle(apptitle);
    }

#if defined GEKKO
    SDL_ShowCursor(SDL_DISABLE);
#endif

    return 0;
}

#ifdef GEKKO
static const char *joynames[3][15] = {
        {
            "Left Stick X", "Left Stick Y", "Right Stick X", "Right Stick Y", "Axis 5", "Axis 6", "Axis 7", "Axis 8",
            "Axis 9", "Axis 10", "Axis 11", "Axis 12", "Axis 13", "Axis 14", "Axis 15",
        },
        {
            "Button A", "Button B", "Button 1", "Button 2", "Button -", "Button +", "Button HOME", "Button Z", "Button C",
            "Button X", "Button Y", "Trigger L", "Trigger R", "Trigger ZL", "Trigger ZR",
        },
        {
            "D-Pad Up", "D-Pad Right", "D-Pad Down", "D-Pad Left", "Hat 5", "Hat 6", "Hat 7", "Hat 8", "Hat 9", "Hat 10",
            "Hat 11", "Hat 12", "Hat 13", "Hat 14", "Hat 15",
        }
};
const char *joyGetName(int32_t what, int32_t num)
{
    switch (what)
    {
        case 0:	// axis
            if ((unsigned)num > (unsigned)joystick.numAxes) return NULL;
            return joynames[0][num];

        case 1: // button
            if ((unsigned)num > (unsigned)joystick.numButtons) return NULL;
            return joynames[1][num];

        case 2: // hat
            if ((unsigned)num > (unsigned)joystick.numHats) return NULL;
            return joynames[2][num];

        default:
            return NULL;
    }
}
#endif

//
// grabmouse_low() -- show/hide mouse cursor, lower level (doesn't check state).
//                    furthermore return 0 if successful.
//

static inline char grabmouse_low(char a)
{
#if !defined GEKKO
    SDL_ShowCursor(a ? SDL_DISABLE : SDL_ENABLE);
    return (SDL_WM_GrabInput(a ? SDL_GRAB_ON : SDL_GRAB_OFF) != (a ? SDL_GRAB_ON : SDL_GRAB_OFF));
#else
    UNREFERENCED_PARAMETER(a);
    return 0;
#endif
}

void videoGetModes(int display)
{
    UNREFERENCED_PARAMETER(display);
    
    int32_t i, maxx = 0, maxy = 0;
    int32_t j;
    static int32_t cdepths[] = { 8,
#ifdef USE_OPENGL
                                 16, 24, 32,
#endif
                                 0 };
    SDL_Rect **modes;
    SDL_PixelFormat pf;

    pf.palette = NULL;
    pf.BitsPerPixel = 8;
    pf.BytesPerPixel = 1;

    if (modeschecked || novideo)
        return;

    validmodecnt = 0;
    //    initprintf("Detecting video modes:\n");

    // do fullscreen modes first
    for (j = 0; cdepths[j]; j++)
    {
#ifdef USE_OPENGL
        if (nogl && cdepths[j] > 8)
            continue;
#endif
        pf.BitsPerPixel = cdepths[j];
        pf.BytesPerPixel = cdepths[j] >> 3;

        // We convert paletted contents to non-paletted
        modes = SDL_ListModes((cdepths[j] == 8) ? NULL : &pf, SURFACE_FLAGS | SDL_FULLSCREEN);

        if (modes == (SDL_Rect **)0)
        {
            if (cdepths[j] > 8)
                cdepths[j] = -1;
            continue;
        }

        if (modes == (SDL_Rect **)-1)
        {
            for (i = 0; g_defaultVideoModes[i].x; i++)
                SDL_ADDMODE(g_defaultVideoModes[i].x, g_defaultVideoModes[i].y, cdepths[j], 1);
        }
        else
        {
            for (i = 0; modes[i]; i++)
            {
                if (!SDL_CHECKMODE(modes[i]->w, modes[i]->h))
                    continue;

                SDL_ADDMODE(modes[i]->w, modes[i]->h, cdepths[j], 1);

                if ((modes[i]->w > maxx) || (modes[i]->h > maxy))
                {
                    maxx = modes[i]->w;
                    maxy = modes[i]->h;
                }
            }
        }
    }

    SDL_CHECKFSMODES(maxx, maxy);

    // add windowed modes next
    for (j = 0; cdepths[j]; j++)
    {
#ifdef USE_OPENGL
        if (nogl && cdepths[j] > 8)
            continue;
#endif
        if (cdepths[j] < 0)
            continue;

        for (i = 0; g_defaultVideoModes[i].x; i++)
        {
            auto &mode = g_defaultVideoModes[i];
            if (mode.x > maxx || mode.y > maxy || !SDL_CHECKMODE(mode.x, mode.y))
                continue;

            SDL_ADDMODE(mode.x, mode.y, cdepths[j], 0);
        }
    }

    qsort((void *)validmode, validmodecnt, sizeof(struct validmode_t), &sortmodes);

    modeschecked = 1;
}

//
// setvideomode() -- set SDL video mode
//
int32_t videoSetMode(int32_t x, int32_t y, int32_t c, int32_t fs)
{
    int32_t regrab = 0, ret;
#ifdef USE_OPENGL
    static int32_t ovsync = 1;
#endif

    ret = setvideomode_sdlcommon(&x, &y, c, fs, &regrab);
    if (ret != 1)
    {
        if (ret == 0)
        {
            if (setvideomode_sdlcommonpost(x, y, c, fs, regrab))
                return -1;
        }
        return ret;
    }

    VLOG_F(LOG_GFX, "Setting video mode %dx%d (%d-bpp %s).", x, y, c, ((fs & 1) ? "fullscreen" : "windowed"));

    // restore gamma before we change video modes if it was changed
    if (nogl && sdl_surface && gammabrightness)
    {
        SDL_SetGammaRamp(sysgamma[0], sysgamma[1], sysgamma[2]);
        gammabrightness = 0;  // redetect on next mode switch
    }
    else if (sdl_surface)
        return setvideomode_sdlcommonpost(x, y, c, fs, regrab);
    
    // deinit
    destroy_window_resources();


#ifdef USE_OPENGL
    if (c > 8 || !nogl)
    {
        int32_t i;

        if (nogl)
            return -1;
# ifdef _WIN32
        windowsDwmSetupComposition(false);
# endif
        struct glattribs
        {
            SDL_GLattr attr;
            int32_t value;
        } sdlayer_gl_attributes [] =
        {
            { SDL_GL_DOUBLEBUFFER, 1 },
            { SDL_GL_STENCIL_SIZE, 1 },
            { SDL_GL_ACCELERATED_VISUAL, 1 },
            { SDL_GL_SWAP_CONTROL, vsync_renderlayer },
        };

        SDL_GL_ATTRIBUTES(i, sdlayer_gl_attributes);

        /* HACK: changing SDL GL attribs only works before surface creation,
            so we have to create a new surface in a different format first
            to force the surface we WANT to be recreated instead of reused. */
        if (vsync_renderlayer != ovsync)
        {
            if (sdl_surface)
            {
                SDL_FreeSurface(sdl_surface);
                sdl_surface =
                SDL_SetVideoMode(1, 1, 8, SDL_NOFRAME | SURFACE_FLAGS | ((fs & 1) ? SDL_FULLSCREEN : 0));
                SDL_FreeSurface(sdl_surface);
            }
            ovsync = vsync_renderlayer;
        }
        sdl_surface = SDL_SetVideoMode(x, y, c, SDL_RESIZABLE | SDL_OPENGL | ((fs & 1) ? SDL_FULLSCREEN : 0));
        if (!sdl_surface)
        {
            LOG_F(ERROR, "Unable to set video mode: SDL_SetVideoMode failed: %s.", SDL_GetError());
            nogl = 1;
            destroy_window_resources();
            return -1;
        }

        gladLoadGLLoader(SDL_GL_GetProcAddress);
    }
    else
#endif  // defined USE_OPENGL
    {
        // We convert paletted contents to non-paletted
        sdl_surface = SDL_SetVideoMode(x, y, 0, SURFACE_FLAGS | ((fs & 1) ? SDL_FULLSCREEN : 0));

        if (!sdl_surface)
        {
            LOG_F(ERROR, "Unable to set video mode: SDL_SetVideoMode failed: %s.", SDL_GetError());
            return -1;
        }
    }

    setvideomode_sdlcommonpost(x, y, c, fs, regrab);

    return 0;
}

// SDL 1.2 specific event handling
int32_t handleevents_pollsdl(void)
{
    int32_t code, rv = 0, j;
    SDL_Event ev;

    while (SDL_PollEvent(&ev))
    {
        switch (ev.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                code = keytranslation[ev.key.keysym.sym];
#ifdef KEY_PRINT_DEBUG
                printf("keytranslation[%d] = %s (%d)  %s\n", ev.key.keysym.sym, g_keyNameTable[code], code,
                       ev.key.type == SDL_KEYDOWN ? "DOWN" : "UP");
#endif
                if (code != OSD_OSDKey() && ev.key.keysym.unicode != 0 && ev.key.type == SDL_KEYDOWN &&
                    (ev.key.keysym.unicode & 0xff80) == 0 && !keyBufferFull())
                {
                    if (OSD_HandleChar(ev.key.keysym.unicode & 0x7f))
                        keyBufferInsert(ev.key.keysym.unicode & 0x7f);
                }

                // hook in the osd
                if ((j = OSD_HandleScanCode(code, (ev.key.type == SDL_KEYDOWN))) <= 0)
                {
                    if (j == -1)  // osdkey
                    {
                        for (j = 0; j < NUMKEYS; ++j)
                        {
                            if (keyGetState(j))
                            {
                                if (keypresscallback)
                                    keypresscallback(j, 0);
                            }
                            keySetState(j, 0);
                        }
                    }
                    break;
                }

                if (ev.key.type == SDL_KEYDOWN)
                {
                    if (!keyGetState(code))
                    {
                        if (keypresscallback)
                            keypresscallback(code, 1);
                    }
                    keySetState(code, 1);
                }
                else
                {
#ifdef __linux
                    if (code == 0x59)  // pause
                        break;
#endif
                    keySetState(code, 0);
                    if (keypresscallback)
                        keypresscallback(code, 0);
                }
                break;

            case SDL_ACTIVEEVENT:
                if (ev.active.state & SDL_APPINPUTFOCUS)
                {
                    appactive = ev.active.gain;
                    if (g_mouseGrabbed && g_mouseEnabled)
                        grabmouse_low(!!appactive);
#ifdef _WIN32
                    windowsHandleFocusChange(appactive);
#endif
                    rv = -1;

                    if (ev.active.state & SDL_APPMOUSEFOCUS)
                        g_mouseInsideWindow = ev.active.gain;
                }
                break;

            case SDL_VIDEORESIZE:
                if (fullscreen) break;
                sdl_resize = { ev.resize.w & ~1, ev.resize.h & ~1 };
                break;

            // SDL_MOUSEMOTION needs to fall through to default... this is just GEKKO processing!
            case SDL_MOUSEMOTION:
#ifdef GEKKO
                // check if it's a wiimote pointer pretending to be a mouse
                if (ev.motion.state & SDL_BUTTON_X2MASK)
                {
                    // the absolute values are used to draw the crosshair
                    g_mouseAbs.x = ev.motion.x;
                    g_mouseAbs.y = ev.motion.y;
                    // hack: reduce the scale of the "relative" motions
                    // to make it act more like a real mouse
                    ev.motion.xrel /= 16;
                    ev.motion.yrel /= 12;
                }
#endif
            fallthrough__;
            default: // OSD_Printf("Got event (%d)\n", ev.type); break;
                rv = handleevents_sdlcommon(&ev);
                break;
        }
    }

    return rv;
}

// from SDL HG, modified
int32_t SDL_WaitEventTimeout(SDL_Event *event, int32_t timeout)
{
    uint32_t expiration = 0;

    if (timeout > 0)
        expiration = SDL_GetTicks() + timeout;

    for (;;)
    {
        SDL_PumpEvents();
        switch (SDL_PeepEvents(event, 1, SDL_GETEVENT, ~0))  // SDL_FIRSTEVENT, SDL_LASTEVENT)) {
        {
            case -1: return 0;
            case 1: return 1;
            case 0:
                if (timeout == 0)
                {
                    /* Polling and no events, just return */
                    return 0;
                }
                if (timeout > 0 && ((int32_t)(SDL_GetTicks() - expiration) >= 0))
                {
                    /* Timeout expired and no events */
                    return 0;
                }
                SDL_Delay(10);
                break;
        }
    }
}
