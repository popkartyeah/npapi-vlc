/*****************************************************************************
 * vlc.cpp: support for NPRuntime API for Netscape Script-able plugins
 *                 FYI: http://www.mozilla.org/projects/plugins/npruntime.html
 *****************************************************************************
 * Copyright (C) 2005 the VideoLAN team
 *
 * Authors: Damien Fouilleul <Damien.Fouilleul@laposte.net>
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

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <vlc/vlc.h>

/* Mozilla stuff */
#ifdef HAVE_MOZILLA_CONFIG_H
#   include <mozilla-config.h>
#endif

#include "npovlc.h"
#include "vlcplugin.h"

/*
** implementation of vlc root object
*/

const NPUTF8 * const VlcNPObject::propertyNames[] =
{
    /* no properties */
};

const int VlcNPObject::propertyCount = sizeof(VlcNPObject::propertyNames)/sizeof(NPUTF8 *);

const NPUTF8 * const VlcNPObject::methodNames[] =
{
    "play",
    "pause",
    "stop",
    "fullscreen",
    "set_volume",
    "get_volume",
    "mute",
    "get_int_variable",
    "set_int_variable",
    "get_bool_variable",
    "set_bool_variable",
    "get_str_variable",
    "set_str_variable",
    "clear_playlist",
    "add_item",
    "next",
    "previous",
    "isplaying",
    "get_length",
    "get_position",
    "get_time",
    "seek",
};

enum VlcNPObjectMethodIds
{
    ID_play = 0,
    ID_pause,
    ID_stop,
    ID_fullscreen,
    ID_set_volume,
    ID_get_volume,
    ID_mute,
    ID_get_int_variable,
    ID_set_int_variable,
    ID_get_bool_variable,
    ID_set_bool_variable,
    ID_get_str_variable,
    ID_set_str_variable,
    ID_clear_playlist,
    ID_add_item,
    ID_next,
    ID_previous,
    ID_isplaying,
    ID_get_length,
    ID_get_position,
    ID_get_time,
    ID_seek,
};

const int VlcNPObject::methodCount = sizeof(VlcNPObject::methodNames)/sizeof(NPUTF8 *);

RuntimeNPObject::InvokeResult VlcNPObject::invoke(int index, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    VlcPlugin *p_plugin = reinterpret_cast<VlcPlugin *>(_instance->pdata);
    if( p_plugin )
    {
        libvlc_exception_t ex;
        libvlc_exception_init(&ex);

        switch( index )
        {
            case ID_play:
                if( argCount == 0 )
                {
                    libvlc_playlist_play(p_plugin->getVLC(), -1, 0, NULL, &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_pause:
                if( argCount == 0 )
                {
                    libvlc_playlist_pause(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_stop:
                if( argCount == 0 )
                {
                    libvlc_playlist_stop(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_fullscreen:
                if( argCount == 0 )
                {
                    libvlc_input_t *p_input = libvlc_playlist_get_input(p_plugin->getVLC(), &ex);
                    if( p_input )
                    {
                        libvlc_toggle_fullscreen(p_input, &ex);
                        libvlc_input_free(p_input);
                        if( libvlc_exception_raised(&ex) )
                        {
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                            return INVOKERESULT_GENERIC_ERROR;
                        }
                        else
                        {
                            VOID_TO_NPVARIANT(*result);
                            return INVOKERESULT_NO_ERROR;
                        }
                    }
                    else
                    {
                        /* cannot get input, probably not playing */
                        if( libvlc_exception_raised(&ex) )
                        {
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                        }
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_set_volume:
                if( (argCount == 1) && isNumberValue(args[0]) )
                {
                    libvlc_audio_set_volume(p_plugin->getVLC(), numberValue(args[0]), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_get_volume:
                if( argCount == 0 )
                {
                    int val = libvlc_audio_get_volume(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        INT32_TO_NPVARIANT(val, *result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_mute:
                if( argCount == 0 )
                {
                    libvlc_audio_toggle_mute(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_get_int_variable:
                if( (argCount == 1) && NPVARIANT_IS_STRING(args[0]) )
                {
                    const NPString &name = NPVARIANT_TO_STRING(args[0]);
                    NPUTF8 *s = new NPUTF8[name.utf8length+1];
                    if( s )
                    {
                        int vlc_id = libvlc_get_vlc_id(p_plugin->getVLC());
                        vlc_value_t val;
                        strncpy(s, name.utf8characters, name.utf8length);
                        s[name.utf8length] = '\0';
                        if( VLC_SUCCESS == VLC_VariableGet(vlc_id, s, &val) )
                        {
                            delete s;
                            INT32_TO_NPVARIANT(val.i_int, *result);
                            return INVOKERESULT_NO_ERROR;
                        }
                        else
                        {
                            delete s;
                            return INVOKERESULT_INVALID_ARGS;
                        }
                    }
                    else
                        return INVOKERESULT_OUT_OF_MEMORY;
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_set_int_variable:
                if( (argCount == 2)
                    && NPVARIANT_IS_STRING(args[0])
                    && isNumberValue(args[1]) )
                {
                    const NPString &name = NPVARIANT_TO_STRING(args[0]);
                    NPUTF8 *s = new NPUTF8[name.utf8length+1];
                    if( s )
                    {
                        int vlc_id = libvlc_get_vlc_id(p_plugin->getVLC());
                        vlc_value_t val;
                        strncpy(s, name.utf8characters, name.utf8length);
                        s[name.utf8length] = '\0';
                        val.i_int = numberValue(args[1]);
                        if( VLC_SUCCESS == VLC_VariableSet(vlc_id, s, val) )
                        {
                            delete s;
                            VOID_TO_NPVARIANT(*result);
                            return INVOKERESULT_NO_ERROR;
                        }
                        else
                        {
                            delete s;
                            return INVOKERESULT_INVALID_ARGS;
                        }
                    }
                    else
                        return INVOKERESULT_OUT_OF_MEMORY;
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_get_bool_variable:
                if( (argCount == 1) && NPVARIANT_IS_STRING(args[0]) )
                {
                    const NPString &name = NPVARIANT_TO_STRING(args[0]);
                    NPUTF8 *s = new NPUTF8[name.utf8length+1];
                    if( s )
                    {
                        int vlc_id = libvlc_get_vlc_id(p_plugin->getVLC());
                        vlc_value_t val;
                        strncpy(s, name.utf8characters, name.utf8length);
                        s[name.utf8length] = '\0';
                        if( VLC_SUCCESS == VLC_VariableGet(vlc_id, s, &val) )
                        {
                            delete s;
                            BOOLEAN_TO_NPVARIANT(val.b_bool, *result);
                            return INVOKERESULT_NO_ERROR;
                        }
                        else
                        {
                            delete s;
                            return INVOKERESULT_INVALID_ARGS;
                        }
                    }
                    else
                        return INVOKERESULT_OUT_OF_MEMORY;
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_set_bool_variable:
                if( (argCount == 2)
                    && NPVARIANT_IS_STRING(args[0])
                    && NPVARIANT_IS_BOOLEAN(args[1]) )
                {
                    const NPString &name = NPVARIANT_TO_STRING(args[0]);
                    NPUTF8 *s = new NPUTF8[name.utf8length+1];
                    if( s )
                    {
                        int vlc_id = libvlc_get_vlc_id(p_plugin->getVLC());
                        vlc_value_t val;
                        strncpy(s, name.utf8characters, name.utf8length);
                        s[name.utf8length] = '\0';
                        val.b_bool = NPVARIANT_TO_BOOLEAN(args[1]);
                        if( VLC_SUCCESS == VLC_VariableSet(vlc_id, s, val) )
                        {
                            delete s;
                            VOID_TO_NPVARIANT(*result);
                            return INVOKERESULT_NO_ERROR;
                        }
                        else
                        {
                            delete s;
                            return INVOKERESULT_INVALID_ARGS;
                        }
                    }
                    else
                        return INVOKERESULT_OUT_OF_MEMORY;
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_get_str_variable:
                if( (argCount == 1) && NPVARIANT_IS_STRING(args[0]) )
                {
                    const NPString &name = NPVARIANT_TO_STRING(args[0]);
                    NPUTF8 *s = new NPUTF8[name.utf8length+1];
                    if( s )
                    {
                        int vlc_id = libvlc_get_vlc_id(p_plugin->getVLC());
                        vlc_value_t val;
                        strncpy(s, name.utf8characters, name.utf8length);
                        s[name.utf8length] = '\0';
                        if( VLC_SUCCESS == VLC_VariableGet(vlc_id, s, &val) )
                        {
                            delete s;
                            if( val.psz_string )
                            {
                                int len = strlen(val.psz_string);
                                NPUTF8 *retval = (NPUTF8 *)NPN_MemAlloc(len);
                                if( retval )
                                {
                                    memcpy(retval, val.psz_string, len);
                                    STRINGN_TO_NPVARIANT(retval, len, *result);
                                    free(val.psz_string);
                                    return INVOKERESULT_NO_ERROR;
                                }
                                else
                                {
                                    return INVOKERESULT_OUT_OF_MEMORY;
                                }
                            }
                            else
                            {
                                /* null string */
                                STRINGN_TO_NPVARIANT(NULL, 0, *result);
                                return INVOKERESULT_NO_ERROR;
                            }
                        }
                        else
                        {
                            delete s;
                            return INVOKERESULT_INVALID_ARGS;
                        }
                    }
                    else
                        return INVOKERESULT_OUT_OF_MEMORY;
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_set_str_variable:
                if( (argCount == 2)
                    && NPVARIANT_IS_STRING(args[0])
                    && NPVARIANT_IS_STRING(args[1]) )
                {
                    const NPString &name = NPVARIANT_TO_STRING(args[0]);
                    NPUTF8 *s = new NPUTF8[name.utf8length+1];
                    if( s )
                    {
                        int vlc_id = libvlc_get_vlc_id(p_plugin->getVLC());
                        vlc_value_t val;
                        strncpy(s, name.utf8characters, name.utf8length);
                        s[name.utf8length] = '\0';
                        const NPString &v = NPVARIANT_TO_STRING(args[1]);
                        val.psz_string = new NPUTF8[v.utf8length+1];
                        if( val.psz_string )
                        {
                            strncpy(val.psz_string, v.utf8characters, v.utf8length);
                            val.psz_string[v.utf8length] = '\0';
                            if( VLC_SUCCESS == VLC_VariableSet(vlc_id, s, val) )
                            {
                                delete s;
                                delete val.psz_string;
                                VOID_TO_NPVARIANT(*result);
                                return INVOKERESULT_NO_ERROR;
                            }
                            else
                            {
                                delete s;
                                delete val.psz_string;
                                return INVOKERESULT_INVALID_ARGS;
                            }
                        }
                        else
                        {
                            delete s;
                            return INVOKERESULT_OUT_OF_MEMORY;
                        }
                    }
                    else
                        return INVOKERESULT_OUT_OF_MEMORY;
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_clear_playlist:
                if( argCount == 0 )
                {
                    libvlc_playlist_clear(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_add_item:
                if( (argCount == 1) && NPVARIANT_IS_STRING(args[0]) )
                {
                    const NPString &name = NPVARIANT_TO_STRING(args[0]);
                    NPUTF8 *s = new NPUTF8[name.utf8length+1];
                    if( s )
                    {
                        strncpy(s, name.utf8characters, name.utf8length);
                        s[name.utf8length] = '\0';

                        char *url = p_plugin->getAbsoluteURL(s);
                        delete s;
                        if( ! url )
                            // what happened ?
                            return INVOKERESULT_GENERIC_ERROR;
                             
                        int item = libvlc_playlist_add(p_plugin->getVLC(), url, NULL, &ex);
                        free(url);
                        if( libvlc_exception_raised(&ex) )
                        {
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                            return INVOKERESULT_GENERIC_ERROR;
                        }
                        else
                        {
                            INT32_TO_NPVARIANT(item, *result);
                            return INVOKERESULT_NO_ERROR;
                        }
                    }
                    else
                        return INVOKERESULT_OUT_OF_MEMORY;
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_next:
                if( argCount == 0 )
                {
                    libvlc_playlist_next(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_previous:
                if( argCount == 0 )
                {
                    libvlc_playlist_prev(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_isplaying:
                if( argCount == 0 )
                {
                    int isplaying = libvlc_playlist_isplaying(p_plugin->getVLC(), &ex);
                    if( libvlc_exception_raised(&ex) )
                    {
                        NPN_SetException(this, libvlc_exception_get_message(&ex));
                        libvlc_exception_clear(&ex);
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                    else
                    {
                        BOOLEAN_TO_NPVARIANT(isplaying, *result);
                        return INVOKERESULT_NO_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_get_length:
                if( argCount == 0 )
                {
                    libvlc_input_t *p_input = libvlc_playlist_get_input(p_plugin->getVLC(), &ex);
                    if( p_input )
                    {
                        vlc_int64_t val = libvlc_input_get_length(p_input, &ex);
                        libvlc_input_free(p_input);
                        if( libvlc_exception_raised(&ex) )
                        {
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                            return INVOKERESULT_GENERIC_ERROR;
                        }
                        else
                        {
                            INT32_TO_NPVARIANT((uint32_t)(val/1000LL), *result);
                            return INVOKERESULT_NO_ERROR;
                        }
                    }
                    else
                    {
                        /* cannot get input, probably not playing */
                        if( libvlc_exception_raised(&ex) )
                        {
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                        }
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_get_position:
                if( argCount == 0 )
                {
                    libvlc_input_t *p_input = libvlc_playlist_get_input(p_plugin->getVLC(), &ex);
                    if( p_input )
                    {
                        float val = libvlc_input_get_position(p_input, &ex);
                        libvlc_input_free(p_input);
                        if( libvlc_exception_raised(&ex) )
                        {
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                            return INVOKERESULT_GENERIC_ERROR;
                        }
                        else
                        {
                            DOUBLE_TO_NPVARIANT((double)val, *result);
                            return INVOKERESULT_NO_ERROR;
                        }
                    }
                    else
                    {
                        /* cannot get input, probably not playing */
                        if( libvlc_exception_raised(&ex) )
                        {
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                        }
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_get_time:
                if( argCount == 0 )
                {
                    libvlc_input_t *p_input = libvlc_playlist_get_input(p_plugin->getVLC(), &ex);
                    if( p_input )
                    {
                        vlc_int64_t val = libvlc_input_get_time(p_input, &ex);
                        libvlc_input_free(p_input);
                        if( libvlc_exception_raised(&ex) )
                        {
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                            return INVOKERESULT_GENERIC_ERROR;
                        }
                        else
                        {
                            DOUBLE_TO_NPVARIANT((uint32_t)(val/1000LL), *result);
                            return INVOKERESULT_NO_ERROR;
                        }
                    }
                    else
                    {
                        /* cannot get input, probably not playing */
                        if( libvlc_exception_raised(&ex) )
                        {
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                        }
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            case ID_seek:
                if( (argCount == 2)
                  && isNumberValue(args[0])
                  && NPVARIANT_IS_BOOLEAN(args[1]) )
                {
                    libvlc_input_t *p_input = libvlc_playlist_get_input(p_plugin->getVLC(), &ex);
                    if( p_input )
                    {
                        vlc_int64_t pos = numberValue(args[0])*1000LL;
                        if( NPVARIANT_TO_BOOLEAN(args[1]) )
                        {
                            /* relative seek */
                            vlc_int64_t from = libvlc_input_get_time(p_input, &ex);
                            if( libvlc_exception_raised(&ex) )
                            {
                                libvlc_input_free(p_input);
                                NPN_SetException(this, libvlc_exception_get_message(&ex));
                                libvlc_exception_clear(&ex);
                                return INVOKERESULT_GENERIC_ERROR;
                            }
                            pos += from;
                        }
                        /* jump to time */
                        libvlc_input_set_time(p_input, pos, &ex);
                        libvlc_input_free(p_input);
                        if( libvlc_exception_raised(&ex) )
                        {
                            libvlc_input_free(p_input);
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                            return INVOKERESULT_GENERIC_ERROR;
                        }
                        VOID_TO_NPVARIANT(*result);
                        return INVOKERESULT_NO_ERROR;
                    }
                    else
                    {
                        /* cannot get input, probably not playing */
                        if( libvlc_exception_raised(&ex) )
                        {
                            NPN_SetException(this, libvlc_exception_get_message(&ex));
                            libvlc_exception_clear(&ex);
                        }
                        return INVOKERESULT_GENERIC_ERROR;
                    }
                }
                return INVOKERESULT_NO_SUCH_METHOD;
            default:
                return INVOKERESULT_NO_SUCH_METHOD;
        }
    }
    return INVOKERESULT_GENERIC_ERROR;
}
