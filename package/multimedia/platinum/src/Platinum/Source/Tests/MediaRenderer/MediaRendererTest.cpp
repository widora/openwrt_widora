/*****************************************************************
|
|   Platinum - Test UPnP A/V MediaRenderer
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
#include "PltUPnP.h"
#include "PltMediaRenderer.h"
#include "UPnPRenderer.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>


static GstElement *player_ = NULL;
static GMainLoop *main_loop_ = NULL;
static guint bus_watch_id;
static CUPnPRenderer* pRender = NULL;

/*----------------------------------------------------------------------
|   globals
+---------------------------------------------------------------------*/
struct Options {
    const char* friendly_name;
} Options;

/*----------------------------------------------------------------------
|   PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit(char** args)
{
    fprintf(stderr, "usage: %s [-f <friendly_name>]\n", args[0]);
    fprintf(stderr, "-f : optional upnp server friendly name\n");
    fprintf(stderr, "<path> : local path to serve\n");
    exit(1);
}

/*----------------------------------------------------------------------
|   ParseCommandLine
+---------------------------------------------------------------------*/
static void
ParseCommandLine(char** args)
{
    const char* arg;
    char**      tmp = args+1;

    /* default values */
    Options.friendly_name = NULL;

    while ((arg = *tmp++)) {
        if (!strcmp(arg, "-f")) {
            Options.friendly_name = *tmp++;
        } else {
            fprintf(stderr, "ERROR: too many arguments\n");
            PrintUsageAndExit(args);
        }
    }
}

/*----------------------------------------------------------------------
|   get_uuid
+---------------------------------------------------------------------*/
static void
get_uuid(char *uuid)
{
	g_print("%s.\n", __FUNCTION__);
	
	// read
	FILE *fp = NULL;
	fp = fopen("/tmp/DmrUuid", "r");
    if(fp)
    {
		fread(uuid, strlen(uuid), 1, fp);
		fclose(fp);
		return;
	}
	g_print("%s: not found file.\n", __FUNCTION__);
	
	// generate
	int fd = open("/proc/sys/kernel/random/uuid", O_RDONLY);
    if(fd<0)
	{
		g_print("%s: open failed.\n", __FUNCTION__);
		return;
	}
	else
    {  
        read(fd, uuid, strlen(uuid));
		close(fd);
    }

	// save
	fp = fopen("/tmp/DmrUuid", "w");
    if(fp)
    {
		fwrite(uuid, strlen(uuid), 1, fp);
		fclose(fp);
		return;
	}
	g_print("%s: save error.\n", __FUNCTION__);
}

#define ENABLE_STATE_CHG 0
#if ENABLE_STATE_CHG
/*----------------------------------------------------------------------
|   gststate_get_name
+---------------------------------------------------------------------*/
static const char *
gststate_get_name(GstState state)
{
	switch(state) {
	case GST_STATE_VOID_PENDING:
		return "VOID_PENDING";
	case GST_STATE_NULL:
		return "NULL";
	case GST_STATE_READY:
		return "READY";
	case GST_STATE_PAUSED:
		return "PAUSED";
	case GST_STATE_PLAYING:
		return "PLAYING";
	default:
		return "Unknown";
	}
}
#endif
/*----------------------------------------------------------------------
|   my_bus_callback
+---------------------------------------------------------------------*/
static gboolean
my_bus_callback (GstBus     *bus,
		 GstMessage *message,
		 gpointer    data)
{
	(void)bus;
	(void)data;

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR: {
      GError *err;
      gchar *debug;

      gst_message_parse_error (message, &err, &debug);
      g_print("%s: Error: %s.\n", __FUNCTION__, err->message);
      g_error_free (err);
      g_free (debug);
      break;
    }
	
    case GST_MESSAGE_EOS:
      /* end-of-stream */
      //g_print("%s: End of stream.\n", __FUNCTION__);
	  pRender->end_of_stream();
      break;
	  
	case GST_MESSAGE_BUFFERING:
	  /*gint percent;
	  gst_message_parse_buffering (message, &percent);
	  g_print("%s: Buffering (%3d%%).\n", __FUNCTION__, percent);*/
	  break;
#if ENABLE_STATE_CHG
	case GST_MESSAGE_STATE_CHANGED:
	  GstState oldstate, newstate, pending;
	  gst_message_parse_state_changed(message, &oldstate, &newstate, &pending);
	  g_print("%s: State change: '%s' -> '%s', PENDING: '%s'.\n", __FUNCTION__,
			gststate_get_name(oldstate),
			gststate_get_name(newstate),
			gststate_get_name(pending));
	  break;
#endif	  
    default:
      /* unhandled message */
	  //g_print("%s: Got %s message.\n", __FUNCTION__, GST_MESSAGE_TYPE_NAME (message));
      break;
  }

  /* we want to be notified again the next time there is a message
   * on the bus, so returning TRUE (FALSE means we want to stop watching
   * for messages on the bus and our callback should not be called again)
   */
  return TRUE;
}

/*----------------------------------------------------------------------
|   player_init
+---------------------------------------------------------------------*/
static void
player_init()
{
	g_print("%s: glib-%d.%d.%d; gstreamer-%d.%d.%d.\n", __FUNCTION__, 
		GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION,
		GST_VERSION_MAJOR, GST_VERSION_MINOR, GST_VERSION_MICRO);

	gst_init (NULL, NULL);
	
	player_ = gst_element_factory_make("playbin", "play");
	assert(player_ != NULL);

	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(player_));
	bus_watch_id = gst_bus_add_watch(bus, (GstBusFunc)my_bus_callback, NULL);
	gst_object_unref(bus);

	if (gst_element_set_state(player_, GST_STATE_READY) ==
	    GST_STATE_CHANGE_FAILURE) {
		g_print("%s: Error: pipeline doesn't become ready.\n", __FUNCTION__);
	}
	
	g_print("%s: gst ok.\n", __FUNCTION__);
}

/*----------------------------------------------------------------------
|   exit_loop_sighandler
+---------------------------------------------------------------------*/
static void
exit_loop_sighandler(int sig)
{
	(void)sig;

	g_print("%s: exit loop.\n", __FUNCTION__);
	g_main_loop_quit (main_loop_);
}

/*----------------------------------------------------------------------
|   cb_print_position
+---------------------------------------------------------------------*/
static gboolean
cb_print_position (GstElement *pipeline)
{
	(void)pipeline;

	//g_print("%s.\n", __FUNCTION__);

	// is exit
	if( (access("/tmp/DmrStop", F_OK)) == 0 )	// 0-ok
	{
		g_print("%s: exit loop.\n", __FUNCTION__);

		//remove("/tmp/DmrStop");
		rename("/tmp/DmrStop", "/tmp/DmrStart");
		g_main_loop_quit (main_loop_);
	}
	else
	{
		pRender->get_position();
	}
	
	/* call me again */
	return TRUE;
}

/*----------------------------------------------------------------------
|   main_loop
+---------------------------------------------------------------------*/
static void
main_loop()
{
	g_print("%s.\n", __FUNCTION__);
	
	main_loop_ = g_main_loop_new (NULL, FALSE);

	signal(SIGINT, &exit_loop_sighandler);
	signal(SIGTERM, &exit_loop_sighandler);

	guint timeout_source = g_timeout_add (500, (GSourceFunc)cb_print_position, player_);
	g_main_loop_run (main_loop_);

	/* clean up */
	gst_element_set_state (player_, GST_STATE_NULL);
	gst_object_unref (player_);
	g_source_remove (bus_watch_id);
	g_main_loop_unref (main_loop_);
	g_source_remove(timeout_source);
	
	g_print("%s: clean up.\n", __FUNCTION__);
}

/*----------------------------------------------------------------------
|   main
+---------------------------------------------------------------------*/
int
main(int /* argc */, char** argv)
{   
    PLT_UPnP upnp;

	// setup Neptune logging
    //NPT_LogManager::GetDefault().Configure("plist:.level=FINE;.handlers=ConsoleHandler;.ConsoleHandler.colors=off;.ConsoleHandler.filter=42");

    /* parse command line */
    ParseCommandLine(argv);

	player_init();

	char uuid[] = "e6572b54-f3c7-2d91-2fb5-b757f2537e21";	//37
	get_uuid(uuid);
	g_print("%s: uuid:%s.\n", __FUNCTION__, uuid);
	
    PLT_DeviceHostReference device(
        new CUPnPRenderer(Options.friendly_name?Options.friendly_name:"Platinum Media Renderer",
                          false,
                          uuid));
	g_print("%s: upnp.AddDevice.\n", __FUNCTION__);
    upnp.AddDevice(device);

	g_print("%s: upnp.Start.\n", __FUNCTION__);
    upnp.Start();

	//((CUPnPRenderer*)device.AsPointer())->out_loop();
	pRender = static_cast<CUPnPRenderer*>(device.AsPointer());
	pRender->SetPlayer(player_);
	main_loop();

    g_print("%s: upnp.RemoveDevice.\n", __FUNCTION__);
    upnp.RemoveDevice(device);

	g_print("%s: upnp.Stop.\n", __FUNCTION__);
    upnp.Stop();
    return 0;
}
