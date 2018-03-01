/*
 *      Copyright (C) 2012-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _UPNP_RENDERER_H_
#define _UPNP_RENDERER_H_


#include "PltMediaRenderer.h"
#include <gst/gst.h>
#include <glib.h>


enum transport_state {
	TRANSPORT_STOPPED,
	TRANSPORT_PLAYING,
	TRANSPORT_TRANSITIONING,	/* optional */
	TRANSPORT_PAUSED_PLAYBACK,	/* optional */
	TRANSPORT_PAUSED_RECORDING,	/* optional */
	TRANSPORT_RECORDING,	/* optional */
	TRANSPORT_NO_MEDIA_PRESENT	/* optional */
};


class CUPnPRenderer : public PLT_MediaRenderer
{
public:
    CUPnPRenderer(const char*  friendly_name,
                  bool         show_ip = false,
                  const char*  uuid = NULL,
                  unsigned int port = 0);

    virtual ~CUPnPRenderer();

	void SetPlayer(GstElement *player);

	// Http server handler
    virtual NPT_Result ProcessHttpGetRequest(NPT_HttpRequest&              request,
                                             const NPT_HttpRequestContext& context,
                                             NPT_HttpResponse&             response);
	
	void get_position();
	void end_of_stream();
	
    // AVTransport methods
    virtual NPT_Result OnPause(PLT_ActionReference& action);
    virtual NPT_Result OnPlay(PLT_ActionReference& action);
    virtual NPT_Result OnStop(PLT_ActionReference& action);
    virtual NPT_Result OnSeek(PLT_ActionReference& action);
    virtual NPT_Result OnSetAVTransportURI(PLT_ActionReference& action);

    // RenderingControl methods
    virtual NPT_Result OnSetVolume(PLT_ActionReference& action);
    virtual NPT_Result OnSetMute(PLT_ActionReference& action);

private:
    NPT_Result SetupServices();
    NPT_Result SetupIcons();
    GstState get_current_player_state();
	void change_transport_state(enum transport_state new_state);
	void print_upnp_time(char *result, size_t size, gint64 t);
	gint64 parse_upnp_time(const char *time_string);

private:    
    NPT_Mutex m_state;
    GstElement *m_player;
	gint64 last_duration;
	gint64 last_position;
};
#endif /* _UPNP_RENDERER_H_ */
