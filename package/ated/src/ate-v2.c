#include "precomp.h"
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#define VERSION_STR "1.2.2"
#define SIGNAL
/******************************************************************************************************
*	Constants
*******************************************************************************************************/
#define NUM_DRIVER_IF 2
/******************************************************************************************************
*	Prototype
*******************************************************************************************************/
static void RaCfg_Agent(void);	/*2013.12.04.Common Procedure*/
static void NetReceive(u8 *inpkt, int len);
static void sendATESTOP(int index);
static void sendATESTART(int index);
static int GetOpt(int argc, char *const argv[], const char *optstring);	/* GetOpt - only used in main.c */
static void Usage(void);
static int processPktOldMagic(u8 *inpkt, int len, int if_index);
static int processPktNewMagic(u8 *inpkt, int len);
static int if_match(const char *if_name);
static void *pkt_proc_logic(void *arg);
static void task_dispatcher(u8 *inpkt, int len, int if_index);
#ifdef DBG
static void ate_hexdump(int level, char *str, unsigned char *pSrcBufVA, unsigned long SrcBufLen)
{
	unsigned char *pt;
	int x;

	if (level < ate_debug_level)
		return;
	
	pt = pSrcBufVA;
	printf("%s: %p, len = %lu\n",str,  pSrcBufVA, SrcBufLen);
	for (x=0; x<SrcBufLen; x++)
	{
		if (x % 16 == 0) 
		{
			printf("0x%04x : ", x);
		}
		printf("%02x ", ((unsigned char)pt[x]));
		if (x%16 == 15) printf("\n");
	}
	printf("\n");
}

/**
 * ate_printf - conditional printf
 * @level: priority level (MSG_*) of the message
 * @fmt: printf format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages.
 *
 * Note: New line '\n' is added to the end of the text when printing to stdout.
 */
void ate_printf(int level, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (level >= ate_debug_level)
	{
		vprintf(fmt, ap);
		printf("\n");
	}
	va_end(ap);
}
#else /* DBG */
#define ate_printf(args...) do { } while (0)
#define ate_hexdump(l,t,b,le) do { } while (0)
#endif /* DBG */

#ifdef SIGNAL
#include <signal.h> 
void init_signals(void);
void signup(int);
#endif // SIGNAL //
int ate_debug_level = MSG_ERROR; /* default : ate_debug_level == 2 */
/******************************************************************************************************
*	Private Data
*******************************************************************************************************/
static const char *ate_daemon_version = "ate daemon v" VERSION_STR "\n";
static int sock = -1;
/* respond to QA by unicast frames if bUnicast == TRUE */
static boolean bUnicast = FALSE;
static unsigned char buffer[PKT_BUF_SIZE];
//static unsigned char *buffer;
static const char broadcast_addr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static int do_fork = 1;
static int signup_flag = 1;
static int optindex = 1;
static int optopts;
static char *optargs;
unsigned short ate_cmd_id_len_tbl[] =
{
	0, 0, 0, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 6, 6, 6, 2, 2, 2, 0,
};
unsigned short cmd_id_len_tbl[] =
{
	18, 2, 4, 0, 0xffff, 4, 8, 6, 2, 3, 0, 0, 0, 0xffff, 0xffff, 0xffff, 0xffff, 0, 0, 0, 0, 2,
};

static char bridge_ifname[IFNAMSIZ];
static char driver_ifname[NUM_DRIVER_IF][IFNAMSIZ];	/* Backward competibilty */
static int dri_if_num = 0; /* Number of driver interface from user input*/
/* Interface Abstraction*/
struct HOST_IF *host_fd;
struct DRI_IF *dri_fd[NUM_DRIVER_IF];
static u16 version = 0;
static int is_default_if = 1;
/******************************************************************************************************
*	Functions
*******************************************************************************************************/
#ifdef SIGNAL
void signup(int signum)
{
	int count;
	ate_printf(MSG_INFO, "===>%s, interface num(%d), signum: %d\n", __FUNCTION__,dri_if_num, signum);
	// It's time to terminate myself.
	switch(signum){
	case SIGUSR1:
		ate_debug_level++;
		if(ate_debug_level > 4)
			ate_debug_level = 4;
		printf("Debug level++, %d\n",ate_debug_level);
		break;
	case SIGUSR2:
		ate_debug_level--;
		if(ate_debug_level < 0)
			ate_debug_level = 0;
		printf("Debug level--, %d\n",ate_debug_level);
		break;
	case SIGPIPE:
		printf("Broken PIPE\n");
	case SIGHUP:
	case SIGTERM:
	case SIGABRT:
		if(signup_flag == 1){
			/* Prepare Leave, free malloc*/
			signup_flag = 0;
			if(host_fd){
				host_fd->close();
				ate_printf(MSG_INFO,"%d, free host_fd-v1\n", getpid());
				free(host_fd);
				host_fd = NULL;
			}
			for(count=0;count<dri_if_num;count++){
				if(dri_fd[count]){
					ate_printf(MSG_INFO,"%d,free dri_fd[%d]-v1\n", getpid(),count);
					dri_fd[count]->ops->multi_proc_close(dri_fd[count]);
					free(dri_fd[count]);
					dri_fd[count] = NULL;
				}
			}
		} else {
			printf("Signup_flag is already 0\n");
		}
		break;
	default:
		printf("Do nothing, %d\n",signum);
		break;
	}
}

void init_signals(void)
{
	struct sigaction sa;

	sa.sa_flags = 0;

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGHUP);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaddset(&sa.sa_mask, SIGABRT);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaddset(&sa.sa_mask, SIGUSR2);
	sigaddset(&sa.sa_mask, SIGPIPE);
	sa.sa_handler = signup;
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
}
#endif /* SIGNAL */

static int if_match(const char *if_name)
{
	int if_index = 0;
	struct DRI_IF *if_to_send = NULL;
	for(if_index=0;if_index<dri_if_num;if_index++) {
		if_to_send = dri_fd[if_index];
		ate_printf(MSG_INFO, "if_to_send:%s, match name: %s\n",if_to_send->ifname, if_name);
		if(os_strcmp(if_to_send->ifname, if_name) == 0) {
			break;
		}
	}
	if(if_index == dri_if_num)
		return -1;
	return if_index;
}

static void clean_past_ated(){
	pid_t pid = getpid();
	pid_t fork_pid;
	int pipefd[2];//0:reading, 1:writing
	pipe(pipefd);
	ate_printf(MSG_INFO,"Pid of ated: %d\n",pid);
	fork_pid = fork();
	if(fork_pid==0){
		char *argv[] = {"ps",NULL/*"-AL"*/,NULL};
		close(pipefd[0]);	//children only DO write data
		dup2(pipefd[1],1);
		dup2(pipefd[1],2);
		execvp("ps", argv);
		exit(0);
	}else{
		/* Wait exec finish */
		char buffer[2048];
		char line[256];
		char ate_pid[16];
		unsigned char exist_ate = 0;
		close(pipefd[1]);
		while(read(pipefd[0],buffer, sizeof(buffer)) != 0){
			char *eol = os_strchr(buffer, '\n');
			char *tmp = buffer;
			while(eol){
				int dif = eol - tmp + 1;
				os_memset(line, '\0', 256);
				os_memcpy(line, tmp, dif);
				if(os_strstr(line, "ated")){
					int distance = 0;
					int dif2 = 0;
					char *l;
				ate_printf(MSG_DEBUG,"Parsing line: %s\n", line);
				repeat_parse:
					l = os_strchr(line+distance,' ');
					if(!l)
						break;
					ate_printf(MSG_DEBUG,"Line: 0x%x, l: 0x%x\n", line, l); 
					dif2 = l - line -distance;
					distance += dif2;
					/* The first char is space */
					if(dif2 == 0){
						distance += 1;
						goto repeat_parse;
					}
					if((dif2) > 16){
					ate_printf(MSG_DEBUG,"String too long for pid, continue to parse, [%s]\n",ate_pid);
						goto repeat_parse;
					}
					os_memset(ate_pid, 0, 16);
					os_memcpy(ate_pid, l - dif2, dif2); //For delete appending space
					ate_printf(MSG_DEBUG,"ate_pid: %s\n",ate_pid);
					exist_ate = 1;
					do{
						int pid_found = 0;
						int ret = -1;
						sscanf(ate_pid,"%d", &pid_found);
						if(pid_found != pid){
							ate_printf(MSG_DEBUG,"!pid_found: %d\n",pid_found);
							ret = kill((pid_t)pid_found, SIGHUP);
							if(ret)
								ate_printf(MSG_ERROR,"kill process %d fail\n",pid_found);
							else
								ate_printf(MSG_INFO, "kill process %d success\n",pid_found);
						}	
					}while(0);
				}
				tmp += dif;
				eol = os_strchr(tmp, '\n');
			}
		}
		close(pipefd[0]);
		waitpid(fork_pid, 0, 0);
		close(pipefd[1]);
	}
}

int main(int argc, char *argv[])
{
	pid_t pid;
	int c = 0;	/*For user input*/
	int tmp = 0;	/* For temporary usage*/
	/* To Do: Put it into DBG mode*/
	/* Clean ATED has been executed before */
	clean_past_ated();
	#ifdef SIGNAL
	init_signals();
	#endif
	/* initialize interface */
	os_memset(bridge_ifname, 0, IFNAMSIZ);
	for(tmp=0;tmp<NUM_DRIVER_IF;tmp++){
		os_memset(driver_ifname[tmp],'\0', IFNAMSIZ);
		dri_fd[tmp] = NULL;
	}
	/* set default interface name */
	os_memcpy(bridge_ifname, "br-lan", 7);
	os_memcpy(driver_ifname, "ra0", 4);	/*Act as old agent*/
	/* get interface name from arguments */
	for (;;)
	{	
	//	c = GetOpt(argc, argv, "b:hui:vdq");
		c = GetOpt(argc, argv, "b:hufi:vdq");
		if (c < 0)
			break;
		switch (c)
		{
			case 'b':
			{
				/*Default the host interface is set to Ethernet*/
				tmp = os_strlen(optargs) + 1;
				os_memset(bridge_ifname, 0, IFNAMSIZ);
				os_memcpy(bridge_ifname, optargs, tmp);
				if(tmp > IFNAMSIZ){
					return -1;
				}	
				break;
			}
			case 'h':
				Usage();
				return -1;
			case 'u':
				bUnicast = TRUE;
				break;
			case 'f':
				do_fork = 1;
				break;
			case 'i':
			{
				tmp = os_strlen(optargs) + 1;
				int index = dri_if_num;
				os_memcpy(&driver_ifname[index], optargs, tmp);
				if(tmp > IFNAMSIZ){
					return -1;
				}
				dri_if_num++;
				is_default_if = 0;
				break;
			}
			case 'v':
				printf("%s\n", ate_daemon_version);
				break;
			case 'd':
#ifndef DBG
				printf("Debugging disabled without "
				       "-DDBG compile time "
				       "option.\n");
				return -1;
#else // !DBG //
				ate_debug_level--;
				break;
#endif // !DBG //
			case 'q':
#ifndef DBG
				printf("Debugging disabled without "
				       "-DDBG compile time "
				       "option.\n");
				return -1;
#else // !DBG //
				ate_debug_level++;
				break;
#endif // !DBG //
			default:
				/* error */
				Usage();
				return -1;
		}
	}
	/* background ourself */
    if (do_fork){
        pid = fork();
    }else{
        pid = getpid();
    }

    switch (pid)
	{
	    case -1:
	        /* error */
	        perror("fork/getpid");
	        return -1;
	    case 0:
	        /* child, success */
	        break;
	    default:
	        /* parent, success */
	        if (do_fork)
	            return 0;
	        break;
    }

	/*Default the host interface is set to Ethernet*/
	if(is_default_if == 1)
		dri_if_num = 1;
	/* Initialize Host Interface */	
	host_fd = malloc(sizeof(*host_fd));
	ate_printf(MSG_INFO,"%d, malloc host_fd\n",getpid());
	host_fd->init_if = &init_eth;
	sock = host_fd->init_if(host_fd, bridge_ifname);
	/* Initialize Driver Interface */
	for(tmp=0;tmp<dri_if_num;tmp++){
		dri_fd[tmp] = (struct DRI_IF *)malloc(sizeof(*dri_fd[tmp]));
		ate_printf(MSG_INFO,"%d, malloc dri_fd[%d]\n",getpid(), tmp);
		memset(dri_fd[tmp], 0,sizeof(*dri_fd[tmp]));
		if(dri_fd[tmp] == NULL) {
			ate_printf(MSG_ERROR, "Socket error in IOCTL\n");
			return -1;
		}
		dri_fd[tmp]->init_if = &init_if_ioctl;
		dri_fd[tmp]->init_if(dri_fd[tmp], driver_ifname[tmp]);
		dri_fd[tmp]->status = 0;
		#ifdef MT_ATED_THREAD
		dri_fd[tmp]->ops = &thread_ops;
		#elif MT_ATED_SHM
		dri_fd[tmp]->ops = &fork_ops;
		#elif MT_ATED_IPC_SOCK
		dri_fd[tmp]->ops = &ipc_sock_ops;	
		#endif
		/* Init Multi-Processing */
		dri_fd[tmp]->ops->multi_proc_init((void **)dri_fd, tmp, dri_if_num, pkt_proc_logic);
	}
	/* While loop util program finish */
	RaCfg_Agent();
	
	ate_printf(MSG_INFO,"Program is finished, If num[%d]\n",dri_if_num);
	for(tmp=0;tmp<dri_if_num;tmp++){
 		if(dri_fd[tmp]){
			ate_printf(MSG_INFO,"%d, free dri_fd[%d]-v2\n", getpid(),tmp);
			dri_fd[tmp]->ops->multi_proc_close(dri_fd[tmp]);
			free(dri_fd[tmp]);
			dri_fd[tmp] = NULL;
		}
	}

	exit(0);
}

static void RaCfg_Agent()
{
	int n, count;
	fd_set readfds;
	
	if (sock < 0)
	{
		ate_printf(MSG_ERROR, "No socket, so return.\n");
		return;
	}
	
	/* QA will send ATESTART to driver via me when it starts */
#if 0
	/* Stop AP first */
	/* pass command to driver */
	sendATESTART();
#endif
	ate_printf(MSG_INFO, "28xx ATE agent program start(%d)\n",getpid());

	while (signup_flag) {
		FD_ZERO(&readfds);
		FD_SET(sock,&readfds);
		count = select(sock+1,&readfds,NULL,NULL,NULL);

		if (count < 0) {
			ate_printf(MSG_WARNING, "select failed():");
			continue;
		} else if (count == 0) {
			continue;
			//usleep(1000);
		} else {
			/* Use Thread to Process IOCTL on Each Interface */
			unsigned char *cmd = malloc(sizeof(char)*PKT_BUF_SIZE);
			os_memset(cmd, 0, PKT_BUF_SIZE);
			if (FD_ISSET(sock, &readfds)) {
				n = host_fd->rev_cmd(cmd,PKT_BUF_SIZE);
				if(n>0) {
					if(cmd == NULL){
						ate_printf(MSG_ERROR, "No space for packet from host(malloc)\n");
						break;
					}
					NetReceive(cmd, n);
				}
			}
			free(cmd);
		}
	}

	/* QA will send ATESTOP to driver via me when it is closed. */
#if 0
	/* Start AP */
	/* pass command to driver */
	sendATESTOP();
#endif
	ate_printf(MSG_DEBUG, "28xx ATE agent is closed.\n");
#if 0
	host_fd->close();
	free(host_fd);
	host_fd = NULL;
#endif 
}

static void NetReceive(u8 *inpkt, int len)
{
	struct racfghdr *p_racfgh = NULL;
	unsigned char *p_tmp = inpkt;
	u16 Command_Type;

	p_racfgh = (struct racfghdr *) inpkt;
	ate_printf(MSG_INFO, "Receive Packet\n");

	if (be2cpu32/*ntohl*/(p_racfgh->magic_no) == RACFG_MAGIC_NO) {
		p_tmp = p_tmp + OFFSET_OF(struct racfghdr, comand_type);
		os_memcpy(&Command_Type, p_tmp, sizeof(Command_Type));
		Command_Type = be2cpu16(Command_Type);
		
		if ((Command_Type & RACFG_CMD_TYPE_PASSIVE_MASK) != RACFG_CMD_TYPE_ETHREQ) {
			ate_printf(MSG_ERROR, "Command_Type error = %x\n", Command_Type);
			return;
		}
		task_dispatcher(inpkt,len,0);
	} else if(be2cpu32/*ntohl*/(p_racfgh->magic_no) == NEW_MAGIC_NO){
		struct new_cfghdr *p_newhdr = (struct new_cfghdr *) inpkt;
		char *if_name = p_newhdr->if_name;
		int if_index;
		version = p_newhdr->ver_id;
		switch(version) {
			case 0:
			/* Old agent should not happen here*/
			break;
		case 1:
			ate_printf(MSG_INFO, "New Agent Version 1\n");
		default:
			/* Find Match MAC in interface */
			if_index = if_match(if_name);
			if(if_index!= -1){
				task_dispatcher(inpkt,len,if_index);
			} else {
			/* Fail to find the same MAC, PLZ Error Handle, OR its first time */
				ate_printf(MSG_ERROR, "Unknwon Cmd: %04x",p_newhdr->comand_id);
				if(p_newhdr->comand_id == 0xffff){
				}
			}
			break;
		}
	}else {
		ate_printf(MSG_ERROR, "Magic Error: %02x\n",be2cpu32/*ntohl*/(p_racfgh->magic_no));
		return;
	}
}

static void task_dispatcher(u8 *inpkt, int len, int if_index)
{
	struct DRI_IF *p_if = NULL;
	struct MULTI_PROC_OPS *q_ops;
	struct COMMON_PRIV *priv_data;
	struct cmd_queue *q;
	int ret = 0;

	p_if = dri_fd[if_index];	
	if(p_if == NULL){
		ate_printf(MSG_ERROR,"NULL fd\n");
	}
	priv_data = (struct COMMON_PRIV *)p_if->priv_data;
	q = &priv_data->q;
	q_ops = p_if->ops;
	/* Lock Queue */
	q_ops->multi_lock_q(p_if);
	if(((q->un_served + 1)%CMD_QUEUE_SIZE) == q->served){
		ate_printf(MSG_INFO,"DRI_IF queue FULL(%s)Served(%d):Unserved(%d)\n",p_if->ifname, q->served, q->un_served);
		/* Singal pkt_proc_logc */
		q_ops->multi_sig_data(p_if);
		ate_printf(MSG_DEBUG,"Un-lock in task_dispatcher\n");
		q_ops->multi_unlock_q(p_if);
		p_if->status = 2;
	} else{
		ret = q_ops->multi_insert_q(p_if, inpkt, len);
		if(ret != len){
			ate_printf(MSG_ERROR,"Broken PIPE, ret: %d\n",ret);
		}
		q->cmd_len[q->un_served] = len;
		q->un_served = (q->un_served + 1)%CMD_QUEUE_SIZE;
		ate_printf(MSG_DEBUG,"DRI_IF queue inserted(%s)Served(%d):Unserved(%d)\n",p_if->ifname, q->served, q->un_served);
		/* Singal pkt_proc_logc */
		q_ops->multi_sig_data(p_if);
		ate_printf(MSG_DEBUG,"Un-lock in task_dispatcher\n");
		q_ops->multi_unlock_q(p_if);
	}
	//free(inpkt); //KOKO
}

static void *pkt_proc_logic(void *arg)
{
	struct DRI_IF *dri_if = arg;
	struct COMMON_PRIV *priv_data;
	struct cmd_queue *q;
	int serving = 0;
	int unserved = 0;
	int served = 0;
	static int num_pkt = 0;
	struct MULTI_PROC_OPS *q_ops;

	ate_printf(MSG_INFO,"Multi-processing for serving command packet has started for dri_if[%s],PId:%d\n", dri_if->ifname,getpid());
	q_ops = dri_if->ops;
	priv_data = (struct COMMON_PRIV *)dri_if->priv_data;
	q = &priv_data->q;
#if 1
	/* Get unserved number */
	while(signup_flag){
		unsigned char *pkt;
		struct racfghdr *p_tmp = NULL;
		int len = -1;
		int Command_Type;
		int i;
		int len_to_host = 0;//RA_CFG_HLEN + NEW_MAGIC_ADDITIONAL_LEN;
		/* Lock Queue */
		q_ops->multi_lock_q(dri_if);
		unserved = q->un_served;
		served = q->served;
		while(served == unserved && signup_flag){
			ate_printf(MSG_DEBUG, "[%d]Wait for cond SERVED/UN-SERVED(%d:%d)\n",getpid(),served,unserved);
			dri_if->status = 0;
			q_ops->multi_wait_data(dri_if);	
			if(!signup_flag)
				goto clean_multi_proc;
			unserved = q->un_served;
			served = q->served;
		}
		dri_if->status = 1;
		//ate_printf(MSG_DEBUG, "SERVED/UN-SERVED(%d:%d)\n",served,unserved);
		pkt = q->cmd_arr[served];
		p_tmp = (struct racfghdr *)pkt;
		len = q->cmd_len[served];
		//pthread_mutex_unlock(&dri_if->q.lock);
		q_ops->multi_unlock_q(dri_if);
		ate_printf(MSG_DEBUG,"Len (%d)\n",len);
		if(pkt == NULL){
			ate_printf(MSG_ERROR,"Null pkt pointer (%d)\n",serving);
			break;
		}
		ate_printf(MSG_DEBUG,"Len (0x%02x)\n",p_tmp->length);
		if (be2cpu32/*ntohl*/(p_tmp->magic_no) == RACFG_MAGIC_NO) {
			ate_printf(MSG_INFO, "Old Magic Packet\n");
			len_to_host = processPktOldMagic(pkt, len, 0);
		} else if(be2cpu32/*ntohl*/(p_tmp->magic_no) == NEW_MAGIC_NO){
			len_to_host = processPktNewMagic(pkt, len);
		}
		host_fd->rsp2host(pkt, len_to_host);
		num_pkt++;
		ate_printf(MSG_DEBUG,"Send reply to host SUCCESS[%s:%d]!\n",dri_if->ifname,num_pkt);
		/* Update Ring Buffer */
		//pthread_mutex_lock(&dri_if->q.lock);
		ate_printf(MSG_DEBUG,"Lock in pkt_process_logic\n");
		q_ops->multi_lock_q(dri_if);
		os_memset(q->cmd_arr[q->served],0,PKT_BUF_SIZE);
		q->cmd_len[q->served] = -1;
		q->served = (q->served + 1)%CMD_QUEUE_SIZE;
		//pthread_mutex_unlock(&dri_if->q.lock);
		ate_printf(MSG_DEBUG,"Un-lock in pkt_process_logic\n");
		q_ops->multi_unlock_q(dri_if);	
	}
clean_multi_proc:
#if 0
	for(serving=served;served<unserved;serving++){
		unsigned char *tmp = q->cmd_arr[serving];
		if(tmp!=NULL)
			free(tmp);
	}
#endif
	ate_printf(MSG_DEBUG, "Clean dri_cmd_q(%s)\n",dri_if->ifname);
	q_ops->multi_proc_close(dri_if);
	dri_if->close(dri_if);
	free(dri_if);
	dri_if_num--;
#endif
}

static int processPktOldMagic(u8 *inpkt, int len, int if_index)
{	
	struct racfghdr *p_racfgh = NULL;
	struct DRI_IF *p_if = NULL;
	u16 Command_Type;
	u16 Command_Id;
	u16 Sequence;
	u16 Len;
	pid_t pid;
	int ret = 0;
	unsigned char *p_tmp = inpkt;
	unsigned char to_host[PKT_BUF_SIZE];
#ifdef DBG /*DEBUG FER*/
	static int reset_cnt = 0;
	static int start_rx_cnt = 0;
#endif
	os_memset(to_host, 0, PKT_BUF_SIZE);
	p_racfgh = (struct racfghdr *) inpkt;
	p_tmp = p_tmp + OFFSET_OF(struct racfghdr, magic_no) + 4;
	
	/* Command Type */
	os_memcpy(&Command_Type, p_tmp, sizeof(Command_Type));
	Command_Type = be2cpu16(Command_Type);
	p_tmp += sizeof(Command_Type);

	/* Command Id */
	os_memcpy(&Command_Id, p_tmp, sizeof(Command_Id));
	Command_Id = be2cpu16(Command_Id);
	p_tmp += sizeof(Command_Id);

	/* Length */
	os_memcpy(&Len, p_tmp, sizeof(Len));
	Len	= be2cpu16(Len);
	p_tmp += sizeof(Len);

	/* Sequence */
	os_memcpy(&Sequence, p_tmp, sizeof(Sequence));
	Sequence = be2cpu16(Sequence);
	p_tmp += sizeof(Sequence);

	#if 0
	if (Command_Id == RACFG_CMD_ATE_STOP)
	{
		pid = getpid();
		// stuff my pid into the content
		os_memcpy(&p_racfgh->data[0], (u8 *)&pid, sizeof(pid));
		// We have content now.
		Len += sizeof(pid);
		p_racfgh->length = cpu2be16(Len);
	}
	#endif
	/* Default old agent use 1 interface*/	
	p_if = dri_fd[if_index];
	if(p_if == NULL){
		ate_printf(MSG_ERROR,"NULL fd\n");
		return -1;
	}
	#ifdef DBG
		ate_printf(MSG_INFO, "MAGIC_NO :%08x\n",be2cpu32/*ntohl*/(p_racfgh->magic_no));
		ate_printf(MSG_INFO, "Command_Type :%04x\n",Command_Type);
	 	ate_printf(MSG_INFO, "Command_Id :%04x\n",Command_Id);
		ate_printf(MSG_INFO, "Sequence     :%04x\n",Sequence);
		ate_printf(MSG_INFO, "Len          :%04x\n",Len);
	#endif

	os_memcpy(to_host, inpkt, Len);
	Command_Id = be2cpu16(p_racfgh->comand_id);
  	ret = p_if->send(p_if, inpkt, Len);
	if(ret == EAGAIN) {
		ate_printf(MSG_DEBUG, "Send IOCTL with non-block IO success!\n");
	} else{
		ate_printf(MSG_DEBUG, "Ret of IOCTL is(%s): %d\n",p_if->ifname,ret);
	}
	/* add ack bit to command type */
	p_tmp = inpkt + OFFSET_OF(struct racfghdr, magic_no) + 4;
	Command_Type = Command_Type | (~RACFG_CMD_TYPE_PASSIVE_MASK);
	Command_Type = cpu2be16/*htons*/(Command_Type);
	os_memcpy(p_tmp, &Command_Type, sizeof(Command_Type));
	p_tmp += (sizeof(Command_Id) + sizeof(Command_Type));

	/* Length */
	os_memcpy(&Len, p_tmp, sizeof(Len));
	Len	= be2cpu16(Len);
	return Len + RA_CFG_HLEN;
}
/*
* 	Support Unicast ONLY
*/
static int processPktNewMagic(u8 *inpkt, int len)
{
	struct new_cfghdr *p_newhdr = (struct new_cfghdr *) inpkt;
	struct DRI_IF *if_to_send = NULL;
	char *if_name = p_newhdr->if_name;
	int if_index;
	int ret_len = NEW_MAGIC_ADDITIONAL_LEN;
	version = p_newhdr->ver_id;
	unsigned char *old_format_head = inpkt + NEW_MAGIC_ADDITIONAL_LEN;
	switch(version) {
	case 0:
		/* Old agent should not happen here*/
		break;
	case 1:
		ate_printf(MSG_ERROR, "New Agent Version 1\n");
	default:
		/* Find Match MAC in interface */
		if_index = if_match(if_name);
				
		if(if_index!= -1){
			if_to_send = dri_fd[if_index];
			if(if_to_send != NULL){
				ate_printf(MSG_ERROR, "Find Corresponding MAC Address\n");
				ret_len += processPktOldMagic(old_format_head,len-NEW_MAGIC_ADDITIONAL_LEN, if_index);
			} else {
				ate_printf(MSG_ERROR, "if_to_send NULL Pointer\n");
			}
		} else {
			/* Fail to find the same MAC, PLZ Error Handle, OR its first time */
			ate_printf(MSG_ERROR, "Cmd: %04x",p_newhdr->comand_id);
			if(p_newhdr->comand_id == 0xffff){
				
			}
		}
		break;
	}
	
	return ret_len;
}

/*
* 	Use optstring as a pattern, and comparing argv[x] with optstring to parse variables
*/
static int GetOpt(int argc, char *const argv[], const char *optstring)
{
	static int optchr = 1;
	char *cp;

	if (optchr == 1) {
		if (optindex >= argc) {
			/* all arguments processed */
			return EOF;
		}

		if (argv[optindex][0] != '-' || argv[optindex][1] == '\0') {
			/* no option characters */
			return EOF;
		}
	}

	if (os_strcmp(argv[optindex], "--") == 0) {
		/* no more options */
		optindex++;
		return EOF;
	}

	optopts = argv[optindex][optchr];
	cp = os_strchr(optstring, optopts);
	if (cp == NULL || optopts == ':') {
		if (argv[optindex][++optchr] == '\0') {
			optchr = 1;
			optindex++;
		}
		return '?';
	}

	if (cp[1] == ':') {
		/* Argument required */
		optchr = 1;
		if (argv[optindex][optchr + 1]) {
			/* No space between option and argument */
			optargs = &argv[optindex++][optchr + 1];
		} else if (++optindex >= argc) {
			/* option requires an argument */
			return '?';
		} else {
			/* Argument in the next argv */
			optargs = argv[optindex++];
		}
	} else {
		/* No argument */
		if (argv[optindex][++optchr] == '\0') {
			optchr = 1;
			optindex++;
		}
		optargs = NULL;
	}
	return *cp;
}

static void Usage(void)
{
	printf("%s\n\n\n"
	       "usage:\n"
	       "  ated [-huvd]"
	       "[-b<br_ifname>] \\\n"
	       "[-i<driver_ifname>] \\\n"
	       "\n",
	       ate_daemon_version);

	printf("options:\n"
	       "  -b = bridge interface name\n"
		   "  -h = show this help text\n"
	       "  -u = respond to QA by unicast frame\n"
		   "  -f = daemonize ATED\n"
	       "  -i = driver interface name\n"
	       "  -v = show version\n"
	       "  -d = increase debugging verbosity (-dd even more)\n"
           "  -q = decrease debugging verbosity (-qq even less)\n");

	printf("example 1:\n"
	       "  ated -h\n");
	
	printf("example 2:\n"
	       "  ated -bbr1 -ira1 -v\n");

	printf("example 3:\n"
	       "  ated -u\n");

	printf("example 4:\n"
	       "  ated -d\n");
	printf("example 5 for Dual Adapter and QA support Dual Adapter:\n"
		   "  ated -ira0 -ira1\n");
}

static void sendATESTOP(int index)
{
	unsigned char *pkt = malloc(sizeof(struct racfghdr));
	struct racfghdr *hdr = (struct racfghdr *)pkt;
	/* Send ATESTOP command to driver before I am killed by command line(not by GUI). */
	bzero(hdr, sizeof(hdr));
	hdr->magic_no =  cpu2be32(RACFG_MAGIC_NO);
	hdr->comand_type = cpu2be16(RACFG_CMD_TYPE_ETHREQ);
	hdr->comand_id = cpu2be16(RACFG_CMD_ATE_STOP);
	hdr->length = 0;
	hdr->sequence = 0;
	/* Conditional Branch to Check if its new DLL or old DLL*/
	if(0){
		/* Parse */
	} else {
		/* Default old agent use 1 interface*/
		dri_fd[index]->send(dri_fd[index],pkt, 0);
	}
}

static void sendATESTART(int index)
{
	unsigned char *pkt = malloc(sizeof(struct racfghdr));
	struct racfghdr *hdr = (struct racfghdr *)pkt;
	/* Send ATESTOP command to driver before I am killed by command line(not by GUI). */
	bzero(hdr, sizeof(hdr));
	hdr->magic_no =  cpu2be32(RACFG_MAGIC_NO);
	hdr->comand_type = cpu2be16(RACFG_CMD_TYPE_ETHREQ);
	hdr->comand_id = cpu2be16(RACFG_CMD_ATE_START);
	hdr->length = 0;
	hdr->sequence = 0;
	/* Conditional Branch to Check if its new DLL or old DLL*/
	if(0){
		/* Parse */
	} else {
		/* Default old agent use 1 interface*/
		dri_fd[index]->send(dri_fd[index],pkt, 0);
	}
}
