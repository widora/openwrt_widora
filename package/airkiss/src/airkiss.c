#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include "pthread.h"
#include <sys/socket.h>
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include "airkiss.h"

#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE      0x8BE0
#endif
#define SIOCIWFIRSTPRIV      SIOCDEVPRIVATE
#endif
#define RTPRIV_IOCTL_GET_80211_DATA	(SIOCIWFIRSTPRIV + 0x1D)
#define RTPRIV_IOCTL_SHOW_CONNSTATUS	(SIOCIWFIRSTPRIV + 0x1B)

#define RX_80211_PKT_SIZE   (1500)
#define RX_BUF_SIZE         (3000)


typedef struct _RX_PKT
{
	unsigned int  length;
	unsigned char  data[RX_80211_PKT_SIZE];
}RX_PKT;


airkiss_context_t akcontext;
uint8_t cur_channel = 1;
airkiss_config_t config;
airkiss_result_t ak_result;

int set_channel(int channel)
{
        char cmd[128] = "\0";
	snprintf(cmd,128,"iwpriv ra0 elian set_ch=%d",channel);
	system(cmd);
	return 0;
}

void time_callback(int signo)
{
        if(cur_channel >= 13)
	    cur_channel = 1;
	else
	    cur_channel ++;
	set_channel(cur_channel);
	airkiss_change_channel(&akcontext);
}

static void exit_airkiss(int sig)
{
	int ret;
	int    socket_id;
	struct   iwreq wrq;
	char data[64];
	system("iwpriv ra0 elian monitor_off");
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_id < 0)
	{
		printf("error::Open socket error!\n\n");
		exit(1);
	}
	memset(data, 0x00, 64);
	strcpy(data,"mangop");
	strcpy(wrq.ifr_name, "ra0");
	wrq.u.data.length = 64;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ret = ioctl(socket_id, RTPRIV_IOCTL_GET_80211_DATA, &wrq);
	if(ret != 0)
	{
		printf("error::stop error\n\n");
		exit(1);
	}
	close(socket_id);
	exit(1);
}

static void signal_handle(void)
{
    signal(SIGPIPE, &exit_airkiss);//pipe broken
    signal(SIGINT,  &exit_airkiss);//ctrl+c
    signal(SIGTERM, &exit_airkiss);//kill
    signal(SIGSEGV, &exit_airkiss);//segmentfault
    signal(SIGBUS,  &exit_airkiss);//bus error/**/
}

int main(int argc, char* argv[])
{
	int length;
	RX_PKT * mp;
	int ret= -1;
	int i;
        int socket_id;
        struct iwreq wrq;
	char data[RX_BUF_SIZE];
	struct itimerval tick;

	signal_handle();

	config.memcpy = memcpy;
	config.memset = memset;
	config.memcmp = memcmp;
	config.printf = printf;
	printf("version=%s\n",airkiss_version());
	usleep(100000);
	if(airkiss_init(&akcontext, &config))
	{
		printf("airkiss init fail\n");
		return -1;
	}

	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_id < 0)
	{
	        printf("error::Open socket error!\n");
		return -1;
	}
	printf("start monitor mode\n");
	system("iwpriv ra0 elian monitor_on");


	memset(data, 0x00, RX_BUF_SIZE);
	strcpy(data,"mangos");
	strcpy(wrq.ifr_name, "ra0");
	wrq.u.data.length = RX_BUF_SIZE;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ret = ioctl(socket_id, RTPRIV_IOCTL_GET_80211_DATA, &wrq);
	if(ret != 0)
	{
		printf("error::start monitor mode\n\n");
		system("iwpriv ra0 elian monitor_off");
		return -1;
	}

	signal(SIGALRM, time_callback);

	memset(&tick, 0, sizeof(tick));

	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_usec = 100000;//100000;
	tick.it_value.tv_sec = 0;
	tick.it_value.tv_usec = 100000;//100000;
	if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
	    printf("Set timer failed!\n");
    while(1)
    {
	memset(data, 0x00, RX_BUF_SIZE);
	strcpy(data,"mangor");
	strcpy(wrq.ifr_name, "ra0");
	wrq.u.data.length = RX_BUF_SIZE;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ret = ioctl(socket_id, RTPRIV_IOCTL_GET_80211_DATA, &wrq);
	if(ret != 0)
	{
	                usleep(1000);
	                continue;
	}
	else
	{
	    mp = (RX_PKT *)wrq.u.data.pointer;
#if 0
	    for(i=0;i<((mp->length > 24)? 24:mp->length);i++)
	    {
		printf("%02X ",mp->data[i]);
	    }
	    printf("\n");
#endif
	                ret = airkiss_recv(&akcontext,mp->data,mp->length);
			if(ret == AIRKISS_STATUS_CHANNEL_LOCKED)
			{
			tick.it_value.tv_usec = 0;
			tick.it_interval.tv_usec = 0;
			if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
			    printf("Set timer failed!\n");
			printf("channel locked\n");
			}
			else if(ret == AIRKISS_STATUS_COMPLETE)
			{
			if(airkiss_get_result(&akcontext , &ak_result) < 0)
			    {
			        printf("airkiss get result fail\n");
				break;
			    }
			    else
			    {
			        printf("result ok!ssid is %s , key is %s\n" , ak_result.ssid , ak_result.pwd);
				break;
			    }
			}

	}
    }
	system("iwpriv ra0 elian monitor_off");
	memset(data, 0x00, RX_BUF_SIZE);
	strcpy(data,"mangop");
	strcpy(wrq.ifr_name, "ra0");
	wrq.u.data.length = RX_BUF_SIZE;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ret = ioctl(socket_id, RTPRIV_IOCTL_GET_80211_DATA, &wrq);
	if(ret != 0)
	{
		printf("error::stop error\n\n");
		return -1;
	}
	close(socket_id);
	return 0;
}
