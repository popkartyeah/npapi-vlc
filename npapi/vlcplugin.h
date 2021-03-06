/*****************************************************************************
 * vlcplugin.h: a VLC plugin for Mozilla
 *****************************************************************************
 * Copyright (C) 2002-2009 the VideoLAN team
 * $Id$
 *
 * Authors: Samuel Hocevar <sam@zoy.org>
 *          Damien Fouilleul <damienf@videolan.org>
 *          Jean-Paul Saman <jpsaman@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/*******************************************************************************
 * Instance state information about the plugin.
 ******************************************************************************/
#ifndef __VLCPLUGIN_H__
#define __VLCPLUGIN_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vlc/vlc.h>

// Setup XP_MACOSX, XP_UNIX, XP_WIN
#if defined(_WIN32)
#define XP_WIN 1
#elif defined(__APPLE__)
#define XP_MACOSX 1
#else
#define XP_UNIX 1
#define MOZ_X11 1
#endif

#if !defined(XP_MACOSX) && !defined(XP_UNIX) && !defined(XP_WIN)
#define XP_UNIX 1
#elif defined(XP_MACOSX)
#undef XP_UNIX
#endif

#ifdef XP_WIN
    /* Windows stuff */
#   include <windows.h>
#   include <winbase.h>
#endif

#ifdef XP_MACOSX
    /* Mac OS X stuff */
#   include <Quickdraw.h>
#endif

#ifdef XP_UNIX
#   include <pthread.h>
    /* X11 stuff */
#   include <X11/Xlib.h>
#   include <X11/Intrinsic.h>
#   include <X11/StringDefs.h>
#   include <X11/X.h>

#   ifndef __APPLE__
#       include <X11/xpm.h>
#   endif
#endif

#ifndef __MAX
#   define __MAX(a, b)   ( ((a) > (b)) ? (a) : (b) )
#endif
#ifndef __MIN
#   define __MIN(a, b)   ( ((a) < (b)) ? (a) : (b) )
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

//on windows, to avoid including <npapi.h> 
//from Microsoft SDK (rather then from Mozilla SDK),
//#include it indirectly via <npfunctions.h>
#include <npfunctions.h>

#include <vector>
#include <assert.h>

#include "control/nporuntime.h"

#if (((NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR) < 20)
    typedef uint16 NPuint16_t;
    typedef int16 NPint16_t;
    typedef int32 NPint32_t;
#else
    typedef uint16_t NPuint16_t;
    typedef int16_t NPint16_t;
    typedef int32_t NPint32_t;
#endif

typedef struct {
#if defined(XP_UNIX)
    pthread_mutex_t mutex;
#elif defined(XP_WIN)
    CRITICAL_SECTION cs;
#else
#warning "locking not implemented in this platform"
#endif
} plugin_lock_t;

typedef enum vlc_toolbar_clicked_e {
    clicked_Unknown = 0,
    clicked_Play,
    clicked_Pause,
    clicked_Stop,
    clicked_timeline,
    clicked_Time,
    clicked_Fullscreen,
    clicked_Mute,
    clicked_Unmute
} vlc_toolbar_clicked_t;

typedef struct {
    const char *name;                      /* event name */
    const libvlc_event_type_t libvlc_type; /* libvlc event type */
    libvlc_callback_t libvlc_callback;     /* libvlc callback function */
} vlcplugin_event_t;

class EventObj
{
private:

    class Listener
    {
    public:
        Listener(vlcplugin_event_t *event, NPObject *p_object, bool b_bubble):
            _event(event), _listener(p_object), _bubble(b_bubble)
            {
                assert(event);
                assert(p_object);
            }
        Listener(): _event(NULL), _listener(NULL), _bubble(false) { }
        ~Listener()
            {
            }
        const libvlc_event_type_t event_type() const { return _event->libvlc_type; }
        NPObject *listener() const { return _listener; }
        bool bubble() const { return _bubble; }
    private:
        vlcplugin_event_t *_event;
        NPObject *_listener;
        bool _bubble;
    };

    class VLCEvent
    {
    public:
        VLCEvent(libvlc_event_type_t libvlc_event_type, NPVariant *npparams, uint32_t npcount):
            _libvlc_event_type(libvlc_event_type), _npparams(npparams), _npcount(npcount)
            {
            }
        VLCEvent(): _libvlc_event_type(0), _npparams(NULL), _npcount(0) { }
        ~VLCEvent()
            {
            }
        const libvlc_event_type_t event_type() { return _libvlc_event_type; }
        NPVariant *params() const { return _npparams; }
        const uint32_t count() { return _npcount; }
    private:
        libvlc_event_type_t _libvlc_event_type;
        NPVariant *_npparams;
        uint32_t _npcount;
    };
    libvlc_event_manager_t *_em; /* libvlc media_player event manager */
public:
    EventObj(): _em(NULL)  { /* deferred to init() */ }
    bool init();
    ~EventObj();

    void deliver(NPP browser);
    void callback(const libvlc_event_t *event, NPVariant *npparams, uint32_t count);
    bool insert(const NPString &name, NPObject *listener, bool bubble);
    bool remove(const NPString &name, NPObject *listener, bool bubble);
    void unhook_manager(void *);
    void hook_manager(libvlc_event_manager_t *, void *);
private:
    vlcplugin_event_t *find_event(const char *s) const;
    const char *find_name(const libvlc_event_t *event);
    typedef std::vector<Listener> lr_l;
    typedef std::vector<VLCEvent> ev_l;
    lr_l _llist; /* list of registered listeners with 'addEventListener' method */
    ev_l _elist; /* scheduled events list for delivery to browser */

    plugin_lock_t lock;
};

#ifdef XP_WIN
HMODULE DllGetModule();
#include "../common/win32_fullscreen.h"
#endif

class VlcPlugin
{
public:
    VlcPlugin( NPP, NPuint16_t );
    virtual ~VlcPlugin();

    NPError             init(int argc, char* const argn[], char* const argv[]);
    libvlc_instance_t*  getVLC()
                            { return libvlc_instance; };
    libvlc_media_player_t* getMD()
    {
        if( !libvlc_media_player )
        {
             libvlc_printerr("no mediaplayer");
        }
        return libvlc_media_player;
    }
    NPP                 getBrowser()
                            { return p_browser; };
    char*               getAbsoluteURL(const char *url);
    NPWindow&           getWindow()
                            { return npwindow; };
    void                setWindow(const NPWindow &window);

    NPClass*            getScriptClass()
                            { return p_scriptClass; };

#if defined(XP_WIN)
    WNDPROC             getWindowProc()
                            { return pf_wndproc; };
    void                setWindowProc(WNDPROC wndproc)
                            { pf_wndproc = wndproc; };
#endif

#if defined(XP_UNIX)
    int                 setSize(unsigned width, unsigned height);
    Window              getVideoWindow()
                            { return npvideo; };
    void                setVideoWindow(Window window)
                            { npvideo = window; };
    Window              getControlWindow()
                            { return npcontrol; };
    void                setControlWindow(Window window)
                            { npcontrol = window; };

    void                showToolbar();
    void                hideToolbar();
    void                redrawToolbar();
    void                getToolbarSize(unsigned int *width, unsigned int *height)
                            { *width = i_tb_width; *height = i_tb_height; };
    int                 setToolbarSize(unsigned int width, unsigned int height)
                            { i_tb_width = width; i_tb_height = height; return 1; };
    vlc_toolbar_clicked_t getToolbarButtonClicked( int i_xpos, int i_ypos );
#endif

    NPuint16_t  i_npmode; /* either NP_EMBED or NP_FULL */

    /* plugin properties */
    int      b_stream;
    int      b_autoplay;
    int      b_toolbar;
    char *   psz_text;
    char *   psz_target;

    void playlist_play()
    {
        if( playlist_isplaying() )
            playlist_stop();
        if( libvlc_media_player||playlist_select(0) )
            libvlc_media_player_play(libvlc_media_player);
    }
    void playlist_play_item(int idx)
    {
        if( playlist_select(idx) )
            libvlc_media_player_play(libvlc_media_player);
    }
    void playlist_stop()
    {
        if( libvlc_media_player )
            libvlc_media_player_stop(libvlc_media_player);
    }
    void playlist_next()
    {
        if( playlist_select(playlist_index+1) )
            libvlc_media_player_play(libvlc_media_player);
    }
    void playlist_prev()
    {
        if( playlist_select(playlist_index-1) )
            libvlc_media_player_play(libvlc_media_player);
    }
    void playlist_pause()
    {
        if( libvlc_media_player )
            libvlc_media_player_pause(libvlc_media_player);
    }
    int playlist_isplaying()
    {
        int is_playing = 0;
        if( libvlc_media_player )
            is_playing = libvlc_media_player_is_playing(
                                libvlc_media_player );
        return is_playing;
    }

    int playlist_add( const char * );
    int playlist_add_extended_untrusted( const char *, const char *, int,
                                const char ** );
    int playlist_delete_item( int );
    void playlist_clear();
    int  playlist_count();

    void toggle_fullscreen();
    void set_fullscreen( int );
    int  get_fullscreen();

    bool  player_has_vout();


    static bool canUseEventListener();

    EventObj events;
    static void event_callback(const libvlc_event_t *, NPVariant *, uint32_t, void *);
private:
    bool playlist_select(int);
    void set_player_window();

    /* VLC reference */
    int                 playlist_index;
    libvlc_instance_t   *libvlc_instance;
    libvlc_media_list_t *libvlc_media_list;
    libvlc_media_player_t *libvlc_media_player;
    NPClass             *p_scriptClass;

    /* browser reference */
    NPP     p_browser;
    char*   psz_baseURL;

    /* display settings */
    NPWindow  npwindow;
#if defined(XP_WIN)
    WNDPROC   pf_wndproc;
    VLCWindowsManager _WindowsManager;
#endif
#if defined(XP_UNIX)
    unsigned int     i_width, i_height;
    unsigned int     i_tb_width, i_tb_height;
    Window           npvideo, npcontrol;

    XImage *p_btnPlay;
    XImage *p_btnPause;
    XImage *p_btnStop;
    XImage *p_timeline;
    XImage *p_btnTime;
    XImage *p_btnFullscreen;
    XImage *p_btnMute;
    XImage *p_btnUnmute;

    int i_last_position;
#endif

    static void eventAsync(void *);
};

/*******************************************************************************
 * Plugin properties.
 ******************************************************************************/
#define PLUGIN_NAME         "VLC Web Plugin"
#define PLUGIN_DESCRIPTION \
    "Version %s, copyright 1996-2011 VideoLAN and Authors" \
    "<br /><a href=\"http://www.videolan.org/vlc/\">http://www.videolan.org/vlc/</a>"

#endif
