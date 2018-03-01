/*****************************************************************
|
|   Platinum - AV Media Renderer Device
|
| Copyright (c) 2004-2010, Plutinosoft, LLC.
| All rights reserved.
| http://www.plutinosoft.com
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| OEMs, ISVs, VARs and other distributors that combine and 
| distribute commercially licensed software with Platinum software
| and do not wish to distribute the source code for the commercially
| licensed software under version 2, or (at your option) any later
| version, of the GNU General Public License (the "GPL") must enter
| into a commercial license agreement with Plutinosoft, LLC.
| licensing@plutinosoft.com
| 
| This program is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with this program; see the file LICENSE.txt. If not, write to
| the Free Software Foundation, Inc., 
| 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
| http://www.gnu.org/licenses/gpl-2.0.html
|
****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"
#include "PltMediaRenderer.h"
#include "PltService.h"

#include <iostream>
using std::cout;
using std::endl;


NPT_SET_LOCAL_LOGGER("platinum.media.renderer")

/*----------------------------------------------------------------------
|   external references
+---------------------------------------------------------------------*/
extern NPT_UInt8 RDR_ConnectionManagerSCPD[];
extern NPT_UInt8 RDR_AVTransportSCPD[];
extern NPT_UInt8 RDR_RenderingControlSCPD[];

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::PLT_MediaRenderer
+---------------------------------------------------------------------*/
PLT_MediaRenderer::PLT_MediaRenderer(const char*  friendly_name, 
                                     bool         show_ip     /* = false */, 
                                     const char*  uuid        /* = NULL */, 
                                     unsigned int port        /* = 0 */,
                                     bool         port_rebind /* = false */) :	
    PLT_DeviceHost("/", 
                   uuid, 
                   "urn:schemas-upnp-org:device:MediaRenderer:1", 
                   friendly_name, 
                   show_ip, 
                   port, 
                   port_rebind),
    m_Delegate(NULL)
{
	//cout << "plt PLT_MediaRenderer" << endl;
	
    m_ModelDescription = "Plutinosoft AV Media Renderer Device";
    m_ModelName        = "AV Renderer Device";
    m_ModelURL         = "http://www.plutinosoft.com/platinum";
    m_DlnaDoc          = "DMR-1.50";
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::~PLT_MediaRenderer
+---------------------------------------------------------------------*/
PLT_MediaRenderer::~PLT_MediaRenderer()
{
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::SetupServices
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::SetupServices()
{
    NPT_Reference<PLT_Service> service;

	//cout << "plt SetupServices" << endl;

    {
        /* AVTransport */
        service = new PLT_Service(
            this,
            "urn:schemas-upnp-org:service:AVTransport:1", 
            "urn:upnp-org:serviceId:AVTransport",
            "AVTransport",
            "urn:schemas-upnp-org:metadata-1-0/AVT/");
        NPT_CHECK_FATAL(service->SetSCPDXML((const char*) RDR_AVTransportSCPD));
        NPT_CHECK_FATAL(AddService(service.AsPointer()));

        service->SetStateVariableRate("LastChange", NPT_TimeInterval(0.2f));
        service->SetStateVariable("A_ARG_TYPE_InstanceID", "0"); 

        // GetCurrentTransportActions
        service->SetStateVariable("CurrentTransportActions", "Play,Pause,Stop,Seek,Next,Previous");

        // GetDeviceCapabilities
        service->SetStateVariable("PossiblePlaybackStorageMedia", "NONE,NETWORK,HDD,CD-DA,UNKNOWN");
        service->SetStateVariable("PossibleRecordStorageMedia", "NOT_IMPLEMENTED");
        service->SetStateVariable("PossibleRecordQualityModes", "NOT_IMPLEMENTED");

        // GetMediaInfo
        service->SetStateVariable("NumberOfTracks", "0");
        service->SetStateVariable("CurrentMediaDuration", "00:00:00");
        service->SetStateVariable("AVTransportURI", "");
        service->SetStateVariable("AVTransportURIMetadata", "");;
        service->SetStateVariable("NextAVTransportURI", "NOT_IMPLEMENTED");
        service->SetStateVariable("NextAVTransportURIMetadata", "NOT_IMPLEMENTED");
        service->SetStateVariable("PlaybackStorageMedium", "NONE");
        service->SetStateVariable("RecordStorageMedium", "NOT_IMPLEMENTED");
		service->SetStateVariable("RecordMediumWriteStatus", "NOT_IMPLEMENTED");

        // GetPositionInfo
        service->SetStateVariable("CurrentTrack", "0");
        service->SetStateVariable("CurrentTrackDuration", "00:00:00");
        service->SetStateVariable("CurrentTrackMetadata", "");
        service->SetStateVariable("CurrentTrackURI", "");
        service->SetStateVariable("RelativeTimePosition", "00:00:00"); 
        service->SetStateVariable("AbsoluteTimePosition", "00:00:00");
        service->SetStateVariable("RelativeCounterPosition", "2147483647"); // means NOT_IMPLEMENTED
        service->SetStateVariable("AbsoluteCounterPosition", "2147483647"); // means NOT_IMPLEMENTED

        // disable indirect eventing for certain state variables
        PLT_StateVariable* var;
        var = service->FindStateVariable("RelativeTimePosition");
        if (var) var->DisableIndirectEventing();
        var = service->FindStateVariable("AbsoluteTimePosition");
        if (var) var->DisableIndirectEventing();
        var = service->FindStateVariable("RelativeCounterPosition");
        if (var) var->DisableIndirectEventing();
        var = service->FindStateVariable("AbsoluteCounterPosition");
        if (var) var->DisableIndirectEventing();

        // GetTransportInfo
        service->SetStateVariable("TransportState", "NO_MEDIA_PRESENT");
        service->SetStateVariable("TransportStatus", "OK");
        service->SetStateVariable("TransportPlaySpeed", "1");

        // GetTransportSettings
        service->SetStateVariable("CurrentPlayMode", "NORMAL");
        service->SetStateVariable("CurrentRecordQualityMode", "NOT_IMPLEMENTED");
        
        service.Detach();
        service = NULL;
    }

    {
        /* ConnectionManager */
        service = new PLT_Service(
            this,
            "urn:schemas-upnp-org:service:ConnectionManager:1", 
            "urn:upnp-org:serviceId:ConnectionManager",
            "ConnectionManager");
        NPT_CHECK_FATAL(service->SetSCPDXML((const char*) RDR_ConnectionManagerSCPD));
        NPT_CHECK_FATAL(AddService(service.AsPointer()));

        service->SetStateVariable("CurrentConnectionIDs", "0");

        // put all supported mime types here instead
        service->SetStateVariable("SinkProtocolInfo", "http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_PRO,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_SP_G726,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_FULL,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE,http-get:*:audio/L16;rate=44100;channels=1:DLNA.ORG_PN=LPCM,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_PRO,http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM,http-get:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L1_WMA,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMDRM_WMABASE,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_FULL,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAFULL,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPLL_BASE,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC_XAC3,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMDRM_WMVSPLL_BASE,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_BASE,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L5_SO_G726,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG,http-get:*:audio/mpeg:DLNA.ORG_PN=MP3,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL_XAC3,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAPRO,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG1,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L4_SO_G726,http-get:*:audio/L16;rate=48000;channels=2:DLNA.ORG_PN=LPCM,http-get:*:audio/mpeg:DLNA.ORG_PN=MP3X,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_MP3,http-get:*:video/x-ms-wmv:*");
        service->SetStateVariable("SourceProtocolInfo", "");
        
        service.Detach();
        service = NULL;
    }

    {
        /* RenderingControl */
        service = new PLT_Service(
            this,
            "urn:schemas-upnp-org:service:RenderingControl:1", 
            "urn:upnp-org:serviceId:RenderingControl",
            "RenderingControl",
            "urn:schemas-upnp-org:metadata-1-0/RCS/");
        NPT_CHECK_FATAL(service->SetSCPDXML((const char*) RDR_RenderingControlSCPD));
        NPT_CHECK_FATAL(AddService(service.AsPointer()));

        service->SetStateVariableRate("LastChange", NPT_TimeInterval(0.2f));

        service->SetStateVariable("Mute", "0");
        service->SetStateVariableExtraAttribute("Mute", "Channel", "Master");
        service->SetStateVariable("Volume", "100");
        service->SetStateVariableExtraAttribute("Volume", "Channel", "Master");
        service->SetStateVariable("VolumeDB", "0");
        service->SetStateVariableExtraAttribute("VolumeDB", "Channel", "Master");

        service->SetStateVariable("PresetNameList", "FactoryDefaults");
        
        service.Detach();
        service = NULL;
    }

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnAction
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnAction(PLT_ActionReference&          action, 
                            const PLT_HttpRequestContext& context)
{
	NPT_AutoLock lock(m_state);

    NPT_COMPILER_UNUSED(context);

	//cout << "plt OnAction" << endl;

    /* parse the action name */
    NPT_String name = action->GetActionDesc().GetName();

	//cout << "plt name :" << name.GetChars() << endl;

    // since all actions take an instance ID and we only support 1 instance
    // verify that the Instance ID is 0 and return an error here now if not
    NPT_String serviceType = action->GetActionDesc().GetService()->GetServiceType();
    if (serviceType.Compare("urn:schemas-upnp-org:service:AVTransport:1", true) == 0) {
        if (NPT_FAILED(action->VerifyArgumentValue("InstanceID", "0"))) {
			cout << "718" << endl;
            action->SetError(718, "Not valid InstanceID");
            return NPT_FAILURE;
        }
    }
	else if (serviceType.Compare("urn:schemas-upnp-org:service:RenderingControl:1", true) == 0) {
		if (NPT_FAILED(action->VerifyArgumentValue("InstanceID", "0"))) {
			cout << "702" << endl;
			action->SetError(702, "Not valid InstanceID");
			return NPT_FAILURE;
		}
	}

	/* Is it a ConnectionManager Service Action ? */
	if (name.Compare("GetCurrentConnectionInfo", true) == 0) {
		return OnGetCurrentConnectionInfo(action);
	}  

	/* Is it a AVTransport Service Action ? */
    else if (name.Compare("Next", true) == 0) {
        return OnNext(action);
    }
    else if (name.Compare("Pause", true) == 0) {
        return OnPause(action);
    }
    else if (name.Compare("Play", true) == 0) {
        return OnPlay(action);
    }
    else if (name.Compare("Previous", true) == 0) {
        return OnPrevious(action);
    }
    else if (name.Compare("Seek", true) == 0) {
        return OnSeek(action);
    }
    else if (name.Compare("Stop", true) == 0) {
        return OnStop(action);
    }
    else if (name.Compare("SetAVTransportURI", true) == 0) {
        return OnSetAVTransportURI(action);
    }
    else if (name.Compare("SetPlayMode", true) == 0) {
        return OnSetPlayMode(action);
    }

    /* Is it a RendererControl Service Action ? */
    else if (name.Compare("SetVolume", true) == 0) {
        return OnSetVolume(action);
    }
	else if (name.Compare("SetVolumeDB", true) == 0) {
		return OnSetVolumeDB(action);
    }
	else if (name.Compare("GetVolumeDBRange", true) == 0) {
		return OnGetVolumeDBRange(action);
	}
    else if (name.Compare("SetMute", true) == 0) {
        return OnSetMute(action);
    }

	//cout << "plt name :" << name.GetChars() << endl;
    // other actions rely on state variables
    NPT_CHECK_LABEL_WARNING(action->SetArgumentsOutFromStateVariable(), failure);
    return NPT_SUCCESS;

failure:
	//cout << "No Such Action." << endl;
    action->SetError(401,"No Such Action.");
    return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnGetCurrentConnectionInfo
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnGetCurrentConnectionInfo(PLT_ActionReference& action)
{
	//cout << "plt OnGetCurrentConnectionInfo" << endl;
	
    if (m_Delegate) {
        return m_Delegate->OnGetCurrentConnectionInfo(action);
    }
    
    if (NPT_FAILED(action->VerifyArgumentValue("ConnectionID", "0"))) {
		//cout << "ConnectionID" << endl;
        action->SetError(706,"No Such Connection.");
        return NPT_FAILURE;
    }

    if (NPT_FAILED(action->SetArgumentValue("RcsID", "0"))){
		//cout << "RcsID" << endl;
        return NPT_FAILURE;
    }
    if (NPT_FAILED(action->SetArgumentValue("AVTransportID", "0"))) {
		//cout << "AVTransportID" << endl;
        return NPT_FAILURE;
    }
    if (NPT_FAILED(action->SetArgumentOutFromStateVariable("ProtocolInfo"))) {
		//cout << "ProtocolInfo" << endl;
        return NPT_FAILURE;
    }
    if (NPT_FAILED(action->SetArgumentValue("PeerConnectionManager", "/"))) {
		//cout << "PeerConnectionManager" << endl;
        return NPT_FAILURE;
    }
    if (NPT_FAILED(action->SetArgumentValue("PeerConnectionID", "-1"))) {
		//cout << "PeerConnectionID" << endl;
        return NPT_FAILURE;
    }
    if (NPT_FAILED(action->SetArgumentValue("Direction", "Input"))) {
		//cout << "Direction" << endl;
        return NPT_FAILURE;
    }
    if (NPT_FAILED(action->SetArgumentValue("Status", "Unknown"))) {
		//cout << "Status" << endl;
        return NPT_FAILURE;
    }

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnNext
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnNext(PLT_ActionReference& action)
{
	//cout << "plt OnNext" << endl;	//xx
	
    if (m_Delegate) {
        return m_Delegate->OnNext(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnPause
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnPause(PLT_ActionReference& action)
{
	//cout << "plt OnPause" << endl;
	
    if (m_Delegate) {
        return m_Delegate->OnPause(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnPlay
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnPlay(PLT_ActionReference& action)
{
	//cout << "plt OnPlay" << endl;
	
    if (m_Delegate) {
        return m_Delegate->OnPlay(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnPrevious
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnPrevious(PLT_ActionReference& action)
{
	//cout << "plt OnPrevious" << endl;	//xx
	
    if (m_Delegate) {
        return m_Delegate->OnPrevious(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnSeek
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnSeek(PLT_ActionReference& action)
{
	//cout << "plt OnSeek" << endl;
	
    if (m_Delegate) {
        return m_Delegate->OnSeek(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnStop
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnStop(PLT_ActionReference& action)
{
	//cout << "plt OnStop" << endl;
	
    if (m_Delegate) {
        return m_Delegate->OnStop(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnSetAVTransportURI
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnSetAVTransportURI(PLT_ActionReference& action)
{
	//cout << "plt OnSetAVTransportURI" << endl;
	
    if (m_Delegate) {
        return m_Delegate->OnSetAVTransportURI(action);
    }
    
    // default implementation is using state variable
    NPT_String uri;
    NPT_CHECK_WARNING(action->GetArgumentValue("CurrentURI", uri));

    NPT_String metadata;
    NPT_CHECK_WARNING(action->GetArgumentValue("CurrentURIMetaData", metadata));
    
    PLT_Service* serviceAVT;
    NPT_CHECK_WARNING(FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", serviceAVT));

    // update service state variables
    serviceAVT->SetStateVariable("AVTransportURI", uri);
    serviceAVT->SetStateVariable("AVTransportURIMetaData", metadata);

	//cout << "plt uri :" << uri.GetChars() << endl;
	//cout << "plt metadata :" << metadata.GetChars() << endl;

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnSetPlayMode
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnSetPlayMode(PLT_ActionReference& action)
{
	//cout << "plt OnSetPlayMode" << endl;	//xx
	
    if (m_Delegate) {
        return m_Delegate->OnSetPlayMode(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnSetVolume
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnSetVolume(PLT_ActionReference& action)
{
	//cout << "plt OnSetVolume" << endl;
	
    if (m_Delegate) {
        return m_Delegate->OnSetVolume(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnSetVolumeDB
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnSetVolumeDB(PLT_ActionReference& action)
{
	//cout << "plt OnSetVolumeDB" << endl;	//xx
	
    if (m_Delegate) {
        return m_Delegate->OnSetVolumeDB(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnGetVolumeDBRange
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnGetVolumeDBRange(PLT_ActionReference& action)
{
	//cout << "plt OnGetVolumeDBRange" << endl;	//xx
	
    if (m_Delegate) {
        return m_Delegate->OnGetVolumeDBRange(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_MediaRenderer::OnSetMute
+---------------------------------------------------------------------*/
NPT_Result
PLT_MediaRenderer::OnSetMute(PLT_ActionReference& action)
{
	//cout << "plt OnSetMute" << endl;
	
    if (m_Delegate) {
        return m_Delegate->OnSetMute(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}
