#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <sys/utsname.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <linux/un.h>
#include <poll.h>
#include <assert.h>
#include <linux/if.h>
#include <linux/types.h>
#include <linux/wireless.h>
#include <syslog.h>

struct survey_table
{
	char channel[4];
	char ssid[33];
	char bssid[20];
	char security[23];
	char *crypto;
};

static struct survey_table st[64];
static int survey_count = 0;

#define DEBUG

#ifdef DEBUG
#include <stdarg.h>
#define LOG_FILE "/tmp/wifilog"
static int print_f(char *buf)
{
	FILE *fp;
    fp = fopen(LOG_FILE, "r");
    if (fp == NULL)
    {
        return -EIO;
    }
    fclose(fp);
	fp = fopen(LOG_FILE, "a+");
	if (fp == NULL)
	{
		return -EIO;
	}
	fwrite(buf, 1, strlen(buf), fp);        
	fclose(fp);
	return 0;
}
int print_log(const char *fmt, ...)
{
	va_list args;
	int i;
	char buf[512];
	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);
	print_f(buf);
	return i;
}
#else
int print_log(const char *fmt, ...)
{
	return 0;
}
#endif
#define SIOCIWFIRSTPRIV      0x8BE0
#define RTPRIV_IOCTL_SET (SIOCIWFIRSTPRIV + 0x02)
#define RTPRIV_IOCTL_GET_STATE_DATA (SIOCIWFIRSTPRIV + 0x1E)
static void iwpriv(const char *name, const char *key, const char *val)
{
	int socket_id;
	struct iwreq wrq;
	char data[64];

	snprintf(data, 64, "%s=%s", key, val);
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(wrq.ifr_ifrn.ifrn_name, name);
	wrq.u.data.length = strlen(data);
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ioctl(socket_id, RTPRIV_IOCTL_SET, &wrq);
	close(socket_id);
}
unsigned char get_failreason(void)
{
//	int ret;
	int socket_id;
	struct iwreq wrq;
	char data[10];
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	memset(data, 0x00, 10);
	strcpy(wrq.ifr_name, "apcli0");
	strcpy(data,"p");
	wrq.u.data.length = strlen(data);
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ioctl(socket_id, RTPRIV_IOCTL_GET_STATE_DATA, &wrq);
	close(socket_id);
	return (unsigned char)data[0];
}

static void next_field(char **line, char *output, int n) {
	char *l = *line;
	int i;

	memcpy(output, *line, n);
	*line = &l[n];

	for (i = n - 1; i > 0; i--) {
		if (output[i] != ' ')
			break;
		output[i] = '\0';
	}
}

#define RTPRIV_IOCTL_GSITESURVEY (SIOCIWFIRSTPRIV + 0x0D)
static void wifi_site_survey(const char *ifname, int print)
{
	char *s = malloc(IW_SCAN_MAX_DATA);
	int ret;
	int socket_id;
	struct iwreq wrq;
	char *line, *start;

	iwpriv(ifname, "SiteSurvey", "");
	sleep(5);
	memset(s, 0x00, IW_SCAN_MAX_DATA);
	strcpy(wrq.ifr_name, ifname);
	wrq.u.data.length = IW_SCAN_MAX_DATA;
	wrq.u.data.pointer = s;
	wrq.u.data.flags = 0;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	ret = ioctl(socket_id, RTPRIV_IOCTL_GSITESURVEY, &wrq);
	close(socket_id);
	if (ret != 0)
		goto out;

	if (wrq.u.data.length < 1)
		goto out;

	/* ioctl result starts with a newline, for some reason */
	start = s;
	while (*start == '\n')
		start++;

	line = strtok((char *)start, "\n");
	line = strtok(NULL, "\n");
	survey_count = 0;
	while(line && (survey_count < 64)) {
		next_field(&line, st[survey_count].channel, sizeof(st->channel));
		next_field(&line, st[survey_count].ssid, sizeof(st->ssid));
		next_field(&line, st[survey_count].bssid, sizeof(st->bssid));
		next_field(&line, st[survey_count].security, sizeof(st->security));
		line = strtok(NULL, "\n");
		st[survey_count].crypto = strstr(st[survey_count].security, "/");
		if (st[survey_count].crypto) {
			*st[survey_count].crypto = '\0';
			st[survey_count].crypto++;
			syslog(LOG_INFO, "Found network - %s %s %s %s\n",
				st[survey_count].channel, st[survey_count].ssid, st[survey_count].bssid, st[survey_count].security);
		} else {
			st[survey_count].crypto = "";
		}
		survey_count++;
	}

	if (survey_count == 0 && !print)
		syslog(LOG_INFO, "No results");
out:
	free(s);
}

static struct survey_table* wifi_find_ap(const char *name, const char *bssid, const char *hidden)
{
	int i,j=1;
//1: find mac addr
	if (bssid && strlen(bssid)) {
		for (i = 0; i < survey_count; i++)
			if (!strcmp(bssid, (char*)st[i].bssid))
				return &st[i];
		return 0;
	}
//2: find ssid
	for (i = 0; i < survey_count; i++)
		if (!strcmp(name, (char*)st[i].ssid))
			return &st[i];
//3: find hidden ssid
	if ((*hidden == '1') && strlen(hidden))
	{
	int total_hide_num=0; //update when scan complete.
	for (i = 0; i < survey_count; i++)
	{
		if (!strcmp(" ", (char*)st[i].ssid))
		total_hide_num++;
	}
	static int sq_hide_num=1;
	if(sq_hide_num > total_hide_num)
	{
		sq_hide_num=1;
	}	
	for (i = 0; i < survey_count; i++)
	{
			if (!strcmp(" ", (char*)st[i].ssid))
				{
					if(j++ == sq_hide_num)
					{
						print_log("find i=%d,total=%d,sq=%d",i,total_hide_num,sq_hide_num);
						sq_hide_num++;
						return &st[i];
					}
				}
	}
	}
	return 0;
}

#define lengthof(x) (sizeof(x) / sizeof(x[0]))

/* This function is heavily similar to the wifi_repeater_start in
 * net/wifi_core.c from microd (but changed to call ifdown/ifup instead
 * of fiddling with interface configuration manually. */
static void wifi_repeater_start(const char *ifname, const char *staname, const char *channel, const char *ssid,
				const char *bssid, const char *key, const char *enc, const char *crypto)
{
	char buf[100];
	int enctype = 0;

	iwpriv(ifname, "Channel", channel);
	iwpriv(staname, "ApCliEnable", "0");
	if ((strstr(enc, "WPA2PSK") || strstr(enc, "WPAPSKWPA2PSK")) && key) {
		enctype = 1;
		iwpriv(staname, "ApCliAuthMode", "WPA2PSK");
	} else if (strstr(enc, "WPAPSK") && key) {
		enctype = 1;
		iwpriv(staname, "ApCliAuthMode", "WPAPSK");
	} else if (strstr(enc, "WEP") && key) {
		iwpriv(staname, "ApCliAuthMode", "AUTOWEP");
		iwpriv(staname, "ApCliEncrypType", "WEP");
		iwpriv(staname, "ApCliDefaultKeyID", "1");
		iwpriv(staname, "ApCliKey1", key);
	} else if (!key || key[0] == '\0') {
		iwpriv(staname, "ApCliAuthMode", "NONE");
	} else {
		return;
	}

	if (enctype) {
		if (strstr(crypto, "AES") || strstr(crypto, "TKIPAES"))
			iwpriv(staname, "ApCliEncrypType", "AES");
		else
			iwpriv(staname, "ApCliEncrypType", "TKIP");
		iwpriv(staname, "ApCliWPAPSK", key);
	}
	if (bssid && strlen(bssid))
		iwpriv(staname, "ApCliBssid", bssid);
	else
		iwpriv(staname, "ApCliBssid", "");
	iwpriv(staname, "ApCliSsid", ssid);
	iwpriv(staname, "ApCliEnable", "1");
	snprintf(buf, lengthof(buf) - 1, "ifconfig '%s' up", staname);
	system(buf);
}

int check_assoc(char *ifname)
{
	int socket_id, i;
	struct iwreq wrq;

	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(wrq.ifr_ifrn.ifrn_name, ifname);
	ioctl(socket_id, SIOCGIWAP, &wrq);
	close(socket_id);

	for (i = 0; i < 6; i++)
		if(wrq.u.ap_addr.sa_data[i])
			return 1;
	return 0;
}

static void assoc_loop(char *ifname, char *staname, char *essid, char *pass, char *bssid, char *hidden)
{
	unsigned char cur_state,next_state=0;
	unsigned char fail=0;
	unsigned char i=1;
	unsigned char fail5=0;
	unsigned char fail6=0;
	unsigned char timeout=0;
	struct survey_table *c=0;
	while (1) {
		sleep(1);
		switch (cur_state = next_state)
		{
			case 0:			//check
			if(check_assoc(staname)){	//connected
			print_log("connected\n");
			sleep(8);
			}else{
			print_log("disconnect\n");
			next_state = 1;
			}
			i=1;
			timeout=0;
			break;

			case 1:			//scan
			print_log("scanning:");
			wifi_site_survey(ifname, 0); //run 5S
			c = wifi_find_ap(essid, bssid,hidden);
			if(c){	
				print_log("find ap...\n");
				next_state=2;
			}else{
				print_log("no ap...\n"); //no ap,wait for a long time
				sleep(10*i++);
				if(i>12)
					i=12;
				next_state = 1;
			}
			break;

			case 2:			//try
			wifi_repeater_start(ifname, staname, c->channel, essid, bssid, pass, c->security, c->crypto);
			sleep(6);
			next_state = 3;
			i=1;
			fail5=0;
			fail6=0;

			break;

			case 3:			//get reason
			fail = get_failreason()&0x0f;
			print_log("%x",fail);
			if(fail == 0){	//0000000
				fail5=0;
				fail6=0;
				if(i++ > 8)
				next_state = 0;	//connected
			}else{
		/*		if(	fail == 5){
					i=1;
					fail6=0;
					fail5++;
					if(fail5 > 20){
						fail5 = 0;
						print_log("connect timeout!!!!!\n");
						next_state = 2;	//try again
						if(timeout++>4)
							next_state = 4; //fail
						}
						
				}  */
				if(	fail == 6){
					i=1;
					fail5 =0;
					fail6++;
					if(fail6 > 10){			//connect error
						fail6 = 0;
						print_log("connect error!!!!!\n");
						next_state = 4; //fail
						}
				}
			}
			break;			

			case 4:			//error!
			timeout=0;		
			print_log("failure!!!!!\n");
			iwpriv(staname, "ApCliEnable", "0");	//停掉sta	
			sleep(600);
			next_state = 0;
			break;
			default:
			break;
		}
	}


}


int main(int argc, char **argv)
{
	FILE *fp;
	char path[256];

//	if (argc == 3)
//		return main_led(argc, argv);
	if (argc == 1)
	{
		if(check_assoc("apcli0")){
			printf("ok\n");
		}else{
			printf("no\n");
		}
		return 0;
	}
	if (argc ==2)
	{
		printf("%x",get_failreason());
		return 0;
	}
	if (argc < 5)
		return -1;

	daemon(0, 0);
	snprintf(path, sizeof(path), "/tmp/apcli-%s.pid", argv[2]);
	fp = fopen(path, "w+");
	if (fp) {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
	print_log("main\n");
	setbuf(stdout, NULL);
	openlog("ap_client", 0, 0);

	assoc_loop(argv[1], argv[2], argv[3], argv[4], argv[5] ,argv[6]);

	return 0;
}
