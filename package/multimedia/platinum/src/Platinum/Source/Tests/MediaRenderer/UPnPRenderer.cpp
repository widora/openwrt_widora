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
#include "Platinum.h"
#include "UPnPRenderer.h"
#include <stdlib.h>
#include <math.h>

NPT_SET_LOCAL_LOGGER("tq210.upnp.renderer")

/*----------------------------------------------------------------------
|   externals
+---------------------------------------------------------------------*/
extern NPT_UInt8 Platinum_120x120_jpg[16096];
extern NPT_UInt8 Platinum_120x120_png[26577];
extern NPT_UInt8 Platinum_48x48_jpg[3041];
extern NPT_UInt8 Platinum_48x48_png[4681];

/*----------------------------------------------------------------------
|   CUPnPRenderer::CUPnPRenderer
+---------------------------------------------------------------------*/
CUPnPRenderer::CUPnPRenderer(const char* friendly_name, bool show_ip /*= false*/,
                             const char* uuid /*= NULL*/, unsigned int port /*= 0*/)
    : PLT_MediaRenderer(friendly_name, show_ip, uuid, port),
	  last_duration(-1), last_position(-1)
{
	g_print("%s init.\n", __FUNCTION__);
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::~CUPnPRenderer
+---------------------------------------------------------------------*/
CUPnPRenderer::~CUPnPRenderer()
{
    g_print("%s: exit.\n", __FUNCTION__);
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::SetPlayer
+---------------------------------------------------------------------*/
void
CUPnPRenderer::SetPlayer(GstElement *player)
{
    //g_print("%s.\n", __FUNCTION__);
	m_player = player;

	PLT_Service *rct;
	if (NPT_FAILED(FindServiceByType("urn:schemas-upnp-org:service:RenderingControl:1", rct)))
    {
		g_print("%s: Error: no RenderingControl.\n", __FUNCTION__);
		return;
	}
	
	g_object_set(m_player, "mute", FALSE, NULL);
	rct->SetStateVariable("Mute", "0");
	
	double fraction = 0.1;
	g_print("%s: unMute. Init volume fraction to %f.\n", __FUNCTION__, fraction);
	g_object_set(m_player, "volume", fraction, NULL);
	rct->SetStateVariable("Volume", "50");
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::SetupServices
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::SetupServices()
{
	g_print("%s.\n", __FUNCTION__);

    NPT_CHECK(PLT_MediaRenderer::SetupServices());

	// update what we can play
    PLT_Service* service = NULL;
    NPT_CHECK_FATAL(FindServiceByType("urn:schemas-upnp-org:service:ConnectionManager:1", service));
    service->SetStateVariable("SinkProtocolInfo", "http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_PRO,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_SP_G726,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_FULL,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE,http-get:*:audio/L16;rate=44100;channels=1:DLNA.ORG_PN=LPCM,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_PRO,http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM,http-get:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L1_WMA,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMDRM_WMABASE,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_FULL,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAFULL,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPLL_BASE,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC_XAC3,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMDRM_WMVSPLL_BASE,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_BASE,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L5_SO_G726,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG,http-get:*:audio/mpeg:DLNA.ORG_PN=MP3,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL_XAC3,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAPRO,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG1,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L4_SO_G726,http-get:*:audio/L16;rate=48000;channels=2:DLNA.ORG_PN=LPCM,http-get:*:audio/mpeg:DLNA.ORG_PN=MP3X,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_MP3,http-get:*:video/x-ms-wmv:*,http-get:*:audio/x-ms-wma:*,http-get:*:audio/*:*,http-get:*:audio/mpeg:*,http-get:*:audio/x-mpeg:*,http-get:*:audio/x-scpls:*,http-get:*:audio/aiff:*,http-get:*:audio/x-aiff:*,http-get:*:audio/x-m4a:*,http-get:*:audio/m4a:*,http-get:*:audio/mp4:*,http-get:*:audio/aac:*,http-get:*:audio/ape:*,http-get:*:audio/flac:*,http-get:*:audio/x-flac:*,http-get:*:audio/ogg:*,http-get:*:audio/wav:*,http-get:*:audio/x-wav:*");
    
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::SetupIcons
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::SetupIcons()
{
	g_print("%s.\n", __FUNCTION__);

	/*NPT_String file_root = "etc/images";
    AddIcon(
        PLT_DeviceIcon("image/png", 256, 256, 8, "/icon256x256.png"),
        file_root);
    AddIcon(
        PLT_DeviceIcon("image/png", 120, 120, 8, "/icon120x120.png"),
        file_root);
    AddIcon(
        PLT_DeviceIcon("image/png", 48, 48, 8, "/icon48x48.png"),
        file_root);
    AddIcon(
        PLT_DeviceIcon("image/png", 32, 32, 8, "/icon32x32.png"),
        file_root);
    AddIcon(
        PLT_DeviceIcon("image/png", 16, 16, 8, "/icon16x16.png"),
        file_root);*/

	/*if (m_Icons.GetItemCount() == 0)*/ {
		AddIcon(
			PLT_DeviceIcon("image/jpeg", 120, 120, 24, "/images/platinum-120x120.jpg"),
			Platinum_120x120_jpg, sizeof(Platinum_120x120_jpg), false);
		AddIcon(
			PLT_DeviceIcon("image/jpeg", 48, 48, 24, "/images/platinum-48x48.jpg"),
			Platinum_48x48_jpg, sizeof(Platinum_48x48_jpg), false);
		AddIcon(
			PLT_DeviceIcon("image/png", 120, 120, 24, "/images/platinum-120x120.png"),
			Platinum_120x120_png, sizeof(Platinum_120x120_png), false);
		AddIcon(
			PLT_DeviceIcon("image/png", 48, 48, 24, "/images/platinum-48x48.png"),
			Platinum_48x48_png, sizeof(Platinum_48x48_png), false);
	}

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::ProcessHttpRequest
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::ProcessHttpGetRequest(NPT_HttpRequest&              request,
                                  const NPT_HttpRequestContext& context,
                                  NPT_HttpResponse&             response)
{
    g_print("%s.\n", __FUNCTION__);

    return PLT_MediaRenderer::ProcessHttpGetRequest(request, context, response);
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::change_transport_state
+---------------------------------------------------------------------*/
void
CUPnPRenderer::change_transport_state(enum transport_state new_state)
{
	PLT_Service *avt;
	FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", avt);

	const char *transport_states = NULL;
	const char *available_actions = NULL;
	switch (new_state) {
	case TRANSPORT_STOPPED:
		transport_states = "STOPPED";
		available_actions = "PLAY";
		break;
	case TRANSPORT_PLAYING:
		transport_states = "PLAYING";
		available_actions = "PAUSE,STOP,SEEK,X_DLNA_SeekTime";
		break;
	case TRANSPORT_PAUSED_PLAYBACK:
		transport_states = "PAUSED_PLAYBACK";
		available_actions = "PLAY,STOP";
		break;
	case TRANSPORT_TRANSITIONING:
		transport_states = "TRANSITIONING";
		break;
	case TRANSPORT_PAUSED_RECORDING:
		transport_states = "PAUSED_RECORDING";
		break;
	case TRANSPORT_RECORDING:
		transport_states = "RECORDING";
		break;
	case TRANSPORT_NO_MEDIA_PRESENT:
		transport_states = "NO_MEDIA_PRESENT";
		break;
	}

	avt->SetStateVariable("TransportState", transport_states);
	
	if (available_actions) {
		avt->SetStateVariable("CurrentTransportActions", available_actions);
	}
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::get_current_player_state
+---------------------------------------------------------------------*/
GstState
CUPnPRenderer::get_current_player_state()
{
	GstState state = GST_STATE_PLAYING;
	GstState pending = GST_STATE_NULL;
	gst_element_get_state(m_player, &state, &pending, 0);
	return state;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::print_upnp_time
+---------------------------------------------------------------------*/
void
CUPnPRenderer::print_upnp_time(char *result, size_t size, gint64 t)
{
	const int hour = t / (60*60);
	const int minute = (t % (60*60)) / 60;
	const int second = (t % (60*60)) % 60;
	snprintf(result, size, "%02d:%02d:%02d", hour, minute, second);
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::get_position
+---------------------------------------------------------------------*/
void
CUPnPRenderer::get_position()
{
	NPT_AutoLock lock(m_state);

	gint64 duration, position;

	//g_print("%s.\n", __FUNCTION__);

	PLT_Service *avt;
	if (NPT_FAILED(FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", avt)))
	{
		g_print("%s: Error: no AVTransport.\n", __FUNCTION__);
		return;
	}

	if (get_current_player_state() != GST_STATE_PLAYING)
	{
		return;
	}

	if (gst_element_query_position (m_player, GST_FORMAT_TIME, &position)
		&& gst_element_query_duration (m_player, GST_FORMAT_TIME, &duration)) 
	{
		char tbuf[32];
		duration = duration / GST_SECOND;
		position = position / GST_SECOND;
		if (duration != last_duration) {
			print_upnp_time(tbuf, sizeof(tbuf), duration);
			avt->SetStateVariable("CurrentTrackDuration", tbuf);
    		avt->SetStateVariable("CurrentMediaDuration", tbuf);
			g_print("%s: duration:%s.\n", __FUNCTION__, tbuf);
			last_duration = duration;
		}
		if (position != last_position) {
			if(position>duration)
			{
				g_print("%s: Error: position>duration!\n", __FUNCTION__);
				return;
			}
			print_upnp_time(tbuf, sizeof(tbuf), position);
			avt->SetStateVariable("RelativeTimePosition", tbuf);
    		avt->SetStateVariable("AbsoluteTimePosition", tbuf);
			//g_print("%s: position:%s.\n", __FUNCTION__, tbuf);
			last_position = position;
		}
	}
	else
	{
		g_print("%s: Error: Failed to get track.\n", __FUNCTION__);
	}

	NPT_String pre_state;
    avt->GetStateVariableValue("TransportState", pre_state);
	if(!pre_state.Compare("TRANSITIONING"))
	{
		g_print("%s: transitioning->play.\n", __FUNCTION__);
		change_transport_state(TRANSPORT_PLAYING);
	}
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::end_of_stream
+---------------------------------------------------------------------*/
void
CUPnPRenderer::end_of_stream()
{
	NPT_AutoLock lock(m_state);

	g_print("%s.\n", __FUNCTION__);

	PLT_Service *avt;
	FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", avt);
	NPT_String meta;
	avt->GetStateVariableValue("AVTransportURIMetaData", meta);

	if(meta.Find("qq.com") >= 0)
	{
		g_print("%s: qq.\n", __FUNCTION__);
		return;
	}
	else if(meta.Find("kugou.com") >= 0)
	{
		g_print("%s: kugou.\n", __FUNCTION__);
		return;
	}

	// stop
	if (gst_element_set_state(m_player, GST_STATE_READY) ==
		GST_STATE_CHANGE_FAILURE) {
		g_print("%s: Error: setting  GST_STATE_READY failed.\n", __FUNCTION__);
	}
	change_transport_state(TRANSPORT_STOPPED);
	// maybe something ?
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnPause
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnPause(PLT_ActionReference& action)
{
	NPT_AutoLock lock(m_state);

	(void)action;
	//g_print("%s.\n", __FUNCTION__);

	PLT_Service *avt;
	FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", avt);
	NPT_String pre_state;
    avt->GetStateVariableValue("TransportState", pre_state);

	if(!pre_state.Compare("PAUSED_PLAYBACK"))
	{
		g_print("%s: Warn: no change.\n", __FUNCTION__);
	}
	else if(!pre_state.Compare("PLAYING"))
	{
		// play->pause
		g_print("%s: play->pause.\n", __FUNCTION__);
		if (gst_element_set_state(m_player, GST_STATE_PAUSED) ==
			GST_STATE_CHANGE_FAILURE) {
			g_print("%s: Error: setting GST_STATE_PAUSED failed.\n", __FUNCTION__);
			return NPT_FAILURE;
		}
		else
		{
			change_transport_state(TRANSPORT_PAUSED_PLAYBACK);
		}
	}
	else
	{
		g_print("%s: Error: %s(701).\n", __FUNCTION__, pre_state.GetChars());
		return NPT_FAILURE;
	}
	
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnPlay
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnPlay(PLT_ActionReference& action)
{
	NPT_AutoLock lock(m_state);

	(void)action;
	//g_print("%s.\n", __FUNCTION__);

	PLT_Service *avt;
	FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", avt);
	NPT_String pre_state;
    avt->GetStateVariableValue("TransportState", pre_state);

	if(!pre_state.Compare("PLAYING"))
	{
		g_print("%s: Warn: no change.\n", __FUNCTION__);
		return NPT_SUCCESS;
	}
	else if(!pre_state.Compare("PAUSED_PLAYBACK"))
	{
		// pause->play
		g_print("%s: pause->play.\n", __FUNCTION__);
		// play
	}
	else if(!pre_state.Compare("STOPPED"))
	{
		// stop->play
		g_print("%s: stop->play.\n", __FUNCTION__);
		// get uri
		NPT_String uri;
		avt->GetStateVariableValue("AVTransportURI", uri);
		if(uri.IsEmpty())
		{
			g_print("%s: Error: uri(null).\n", __FUNCTION__);
			return NPT_FAILURE;
		}
		// set uri
		g_object_set(G_OBJECT(m_player), "uri", uri.GetChars(), NULL);
		// play
	}
	else
	{
		g_print("%s: Error: %s(701).\n", __FUNCTION__, pre_state.GetChars());
		return NPT_FAILURE;
	}

	// play
	g_print("%s: play.\n", __FUNCTION__);
	if (gst_element_set_state(m_player, GST_STATE_PLAYING) ==
	    GST_STATE_CHANGE_FAILURE) {
		g_print("%s: Error: setting  GST_STATE_PLAYING failed.\n", __FUNCTION__);
		return NPT_FAILURE;
	}
	else
	{
		change_transport_state(TRANSPORT_PLAYING);
	}
    
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnStop
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnStop(PLT_ActionReference& action)
{
	NPT_AutoLock lock(m_state);

	(void)action;
	//g_print("%s.\n", __FUNCTION__);

	PLT_Service *avt;
	FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", avt);
	NPT_String pre_state;
    avt->GetStateVariableValue("TransportState", pre_state);

	/*if(!pre_state.Compare("STOPPED"))
	{
		g_print("%s: Warn: no change.\n", __FUNCTION__);
	}
	else if(!pre_state.Compare("NO_MEDIA_PRESENT"))
	{
		g_print("%s: %s->stop.\n", __FUNCTION__, pre_state.GetChars());
		change_transport_state(TRANSPORT_STOPPED);
	}
	else*/
	{
		// stop
		g_print("%s: %s->stop.\n", __FUNCTION__, pre_state.GetChars());
		
		// stop
		if (gst_element_set_state(m_player, GST_STATE_READY) ==
			GST_STATE_CHANGE_FAILURE) {
			g_print("%s: Error: setting  GST_STATE_READY failed.\n", __FUNCTION__);
			return NPT_FAILURE;
		}

		last_duration = -1;
		last_position = -1;
		avt->SetStateVariable("RelativeTimePosition", "00:00:00"); 
		avt->SetStateVariable("AbsoluteTimePosition", "00:00:00");
		avt->SetStateVariable("CurrentTrackDuration", "00:00:00");
		avt->SetStateVariable("CurrentMediaDuration", "00:00:00");
		change_transport_state(TRANSPORT_STOPPED);
	}

	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::parse_upnp_time
+---------------------------------------------------------------------*/
gint64
CUPnPRenderer::parse_upnp_time(const char *time_string)
{
	int hour = 0;
	int minute = 0;
	int second = 0;
	sscanf(time_string, "%d:%02d:%02d", &hour, &minute, &second);
	const gint64 seconds = (hour * 3600 + minute * 60 + second);
	const gint64 one_sec_unit = GST_SECOND;
	return one_sec_unit * seconds;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnSeek
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnSeek(PLT_ActionReference& action)
{
	NPT_AutoLock lock(m_state);

	//g_print("%s.\n", __FUNCTION__);
	
	if (get_current_player_state() != GST_STATE_PLAYING)
	{
		g_print("%s: Error: is not playing.\n", __FUNCTION__);
		return NPT_ERROR_INVALID_STATE;
	}

	NPT_String unit, target;
	NPT_CHECK_SEVERE(action->GetArgumentValue("Unit", unit));
	NPT_CHECK_SEVERE(action->GetArgumentValue("Target", target));

	if (unit.Compare("REL_TIME")) {
		g_print("%s: Error: is not REL_TIME.\n", __FUNCTION__);
		return NPT_FAILURE;
	}
	g_print("%s: Target: %s.\n", __FUNCTION__, target.GetChars());
	
	// converts target
	gint64 position_nanos = parse_upnp_time(target.GetChars());
	g_print("%s: Time: %" GST_TIME_FORMAT ".\n", __FUNCTION__, GST_TIME_ARGS (position_nanos));
	if (!gst_element_seek(m_player, 1.0, GST_FORMAT_TIME,
			     GST_SEEK_FLAG_FLUSH,
			     GST_SEEK_TYPE_SET, position_nanos,
			     GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
		g_print("%s: Error: seek error.\n", __FUNCTION__);
		return NPT_FAILURE;
	}

	g_print("%s: play->transitioning.\n", __FUNCTION__);
	change_transport_state(TRANSPORT_TRANSITIONING);	// qplay

	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnSetAVTransportURI
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnSetAVTransportURI(PLT_ActionReference& action)
{
	NPT_AutoLock lock(m_state);

	//g_print("%s.\n", __FUNCTION__);
	
	NPT_String uri, meta;
	NPT_CHECK_SEVERE(action->GetArgumentValue("CurrentURI", uri));
	NPT_CHECK_SEVERE(action->GetArgumentValue("CurrentURIMetaData", meta));
	g_print("%s: CurrentURI: %s.\n", __FUNCTION__, uri.GetChars());
	g_print("%s: CurrentURIMetaData: %s.\n", __FUNCTION__, meta.GetChars());

	PLT_Service *avt;
	FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", avt);

	// update service state variables
	avt->SetStateVariable("AVTransportURI", uri);
	avt->SetStateVariable("AVTransportURIMetaData", meta);
	
	//NPT_CHECK_SEVERE(action->SetArgumentsOutFromStateVariable());
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnSetVolume
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnSetVolume(PLT_ActionReference& action)
{
	NPT_AutoLock lock(m_state);

	//g_print("%s.\n", __FUNCTION__);

	NPT_String volume;
    NPT_CHECK_SEVERE(action->GetArgumentValue("DesiredVolume", volume));
	g_print("%s: DesiredVolume: %s.\n", __FUNCTION__, volume.GetChars());

	int volume_level = atoi(volume.GetChars());  // range 0..100
	if (volume_level < 0) volume_level = 0;
	if (volume_level > 100) volume_level = 100;

	float decibel;
	if (volume_level < 50) {
		decibel = (-60.0) + ((-20.0) - (-60.0)) / 50 * volume_level;
	}
	else {
		decibel = (-20.0) + ((0.0) - (-20.0)) / 50 * (volume_level - 50);
	}

	double fraction = exp(decibel / 20 * log(10));
	g_print("%s: Set volume fraction to %f.\n", __FUNCTION__, fraction);
	g_object_set(m_player, "volume", fraction, NULL);

	PLT_Service *rct;
	FindServiceByType("urn:schemas-upnp-org:service:RenderingControl:1", rct);
	rct->SetStateVariable("Volume", volume);
	
	return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   CUPnPRenderer::OnSetMute
+---------------------------------------------------------------------*/
NPT_Result
CUPnPRenderer::OnSetMute(PLT_ActionReference& action)
{
	NPT_AutoLock lock(m_state);

	//g_print("%s.\n", __FUNCTION__);

	NPT_String mute;
    NPT_CHECK_SEVERE(action->GetArgumentValue("DesiredMute", mute));
	g_print("%s: DesiredMute: %s.\n", __FUNCTION__, mute.GetChars());
	
	if(!mute.Compare("1"))
	{
		g_object_set(m_player, "mute", TRUE, NULL);
	}
	else
	{
		g_object_set(m_player, "mute", FALSE, NULL);
	}

	PLT_Service *rct;
	FindServiceByType("urn:schemas-upnp-org:service:RenderingControl:1", rct);
	rct->SetStateVariable("Mute", mute);
	
	return NPT_SUCCESS;
}
