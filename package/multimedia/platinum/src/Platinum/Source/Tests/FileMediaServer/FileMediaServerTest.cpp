/*****************************************************************
|
|   Platinum - Test UPnP A/V MediaServer
|
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
#include "PltFileMediaServer.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

NPT_SET_LOCAL_LOGGER("platinum.media.server.file.test")

bool run_flag = true;

/*----------------------------------------------------------------------
|   globals
+---------------------------------------------------------------------*/
struct Options {
    const char* path;
    const char* friendly_name;
    const char* guid;
    NPT_UInt32  port;
} Options;

/*----------------------------------------------------------------------
|   PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, "usage: FileMediaServerTest [-f <friendly_name>] [-p <port>] [-g <guid>] <path>\n");
    fprintf(stderr, "-f : optional upnp device friendly name\n");
    fprintf(stderr, "-p : optional http port\n");
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

    /* default values */
    Options.path     = NULL;
    Options.friendly_name = NULL;
    Options.guid = NULL;
    Options.port = 0;

    while ((arg = *args++)) {
        if (!strcmp(arg, "-f")) {
            Options.friendly_name = *args++;
        } else if (!strcmp(arg, "-g")) {
            Options.guid = *args++;
        } else if (!strcmp(arg, "-p")) {
            if (NPT_FAILED(NPT_ParseInteger32(*args++, Options.port))) {
                fprintf(stderr, "ERROR: invalid argument\n");
                PrintUsageAndExit();
            }
        } else if (Options.path == NULL) {
            Options.path = arg;
        } else {
            fprintf(stderr, "ERROR: too many arguments\n");
            PrintUsageAndExit();
        }
    }

    /* check args */
    if (Options.path == NULL) {
        fprintf(stderr, "ERROR: path missing\n");
        PrintUsageAndExit();
    }
}

/*----------------------------------------------------------------------
|   get_uuid
+---------------------------------------------------------------------*/
static void
get_uuid(char *uuid)
{
	printf("%s.\n", __FUNCTION__);
	
	// read
	FILE *fp = NULL;
	fp = fopen("/tmp/DmsUuid", "r");
    if(fp)
    {
		fread(uuid, strlen(uuid), 1, fp);
		fclose(fp);
		return;
	}
	printf("%s: not found file.\n", __FUNCTION__);
	
	// generate
	int fd = open("/proc/sys/kernel/random/uuid", O_RDONLY);
    if(fd<0)
	{
		printf("%s: open failed.\n", __FUNCTION__);
		return;
	}
	else
    {  
        read(fd, uuid, strlen(uuid));
		close(fd);
    }

	// save
	fp = fopen("/tmp/DmsUuid", "w");
    if(fp)
    {
		fwrite(uuid, strlen(uuid), 1, fp);
		fclose(fp);
		return;
	}
	printf("%s: save error.\n", __FUNCTION__);
}

/*----------------------------------------------------------------------
|   exit_loop_sighandler
+---------------------------------------------------------------------*/
static void
exit_loop_sighandler(int sig)
{
	(void)sig;

	printf("%s: exit loop.\n", __FUNCTION__);
	run_flag = false;
}

/*----------------------------------------------------------------------
|   main
+---------------------------------------------------------------------*/
int
main(int /* argc */, char** argv)
{
    // setup Neptune logging
    //NPT_LogManager::GetDefault().Configure("plist:.level=FINE;.handlers=ConsoleHandler;.ConsoleHandler.colors=off;.ConsoleHandler.filter=42");

    /* parse command line */
    ParseCommandLine(argv+1);

	/* for faster DLNA faster testing */
    PLT_Constants::GetInstance().SetDefaultDeviceLease(NPT_TimeInterval(60.));

	char uuid[] = "079a57e3-f208-4239-aeb0-fc44b2bf93cd";	//37
	get_uuid(uuid);
	printf("%s: uuid:%s.\n", __FUNCTION__, uuid);
    
    PLT_UPnP upnp;
    PLT_DeviceHostReference device(
        new PLT_FileMediaServer(
            Options.path, 
            Options.friendly_name?Options.friendly_name:"Platinum UPnP Media Server",
            false,
            uuid, // NULL for random ID
            (NPT_UInt16)Options.port)
            );

    NPT_List<NPT_IpAddress> list;
    NPT_CHECK_SEVERE(PLT_UPnPMessageHelper::GetIPAddresses(list));
    NPT_String ip = list.GetFirstItem()->ToString();

    device->m_ModelDescription = "Platinum File Media Server";
    device->m_ModelURL = "http://www.plutinosoft.com/";
    device->m_ModelNumber = "1.0";
    device->m_ModelName = "Platinum File Media Server";
    device->m_Manufacturer = "Plutinosoft";
    device->m_ManufacturerURL = "http://www.plutinosoft.com/";

	printf("%s: upnp.AddDevice.\n", __FUNCTION__);
    upnp.AddDevice(device);
    NPT_String uuid_r = device->GetUUID();
	printf("%s: GetUUID:%s.\n", __FUNCTION__, (const char*)uuid_r);

	printf("%s: upnp.Start.\n", __FUNCTION__);
    NPT_CHECK_SEVERE(upnp.Start());
    //NPT_LOG_INFO("Press 'q' to quit.");

    /*char buf[256];
    while (gets(buf)) {
        if (*buf == 'q')
            break;
    }*/
	signal(SIGINT, &exit_loop_sighandler);
	signal(SIGTERM, &exit_loop_sighandler);

	run_flag = true;
	while(run_flag)
	{
		sleep(1);
	}

	printf("%s: upnp.Stop.\n", __FUNCTION__);
    upnp.Stop();

    return 0;
}
