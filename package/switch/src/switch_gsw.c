#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <asm/types.h>
#include <linux/autoconf.h>

#include "ra_ioctl.h"

#ifdef CONFIG_CC_OPTIMIZE_FOR_SIZE
#define RT_SWITCH_HELP    1
#define RT_TABLE_MANIPULATE    1
#else
#define RT_SWITCH_HELP    1
#define RT_TABLE_MANIPULATE    1
#endif

#if defined (CONFIG_RALINK_MT7620)
#define MAX_PORT    7
#else
#define MAX_PORT    6
#endif

int esw_fd;

void switch_init(void)
{
    esw_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (esw_fd < 0) {
        perror("socket");
        exit(0);
    }
}

void switch_fini(void)
{
    close(esw_fd);
}

void usage(char *cmd)
{
#if RT_SWITCH_HELP
    printf("Usage:\n");
    printf(" %s acl etype add [ethtype] [portmap]          - drop etherytype packets\n", cmd);
    printf(" %s acl dip add [dip] [portmap]            - drop dip packets\n", cmd);
    printf(" %s acl dip meter [dip] [portmap][meter:kbps]      - rate limit dip packets\n", cmd);
    printf(" %s acl dip trtcm [dip] [portmap][CIR:kbps][CBS][PIR][PBS] - TrTCM dip packets\n", cmd);
    printf(" %s acl port add [sport] [portmap]       - drop src port packets\n", cmd);
    printf(" %s acl L4 add [2byes] [portmap]         - drop L4 packets with 2bytes payload\n", cmd);
    printf(" %s add [mac] [portmap]          - add an entry to switch table\n", cmd);
    printf(" %s add [mac] [portmap] [vlan id]    - add an entry to switch table\n", cmd);
    printf(" %s add [mac] [portmap] [vlan id] [age]  - add an entry to switch table\n", cmd);
    printf(" %s clear                - clear switch table\n", cmd);
    printf(" %s del [mac]                - delete an entry from switch table\n", cmd);
    printf(" %s del [mac] [fid]         - delete an entry from switch table\n", cmd);
    printf(" %s dip add [dip] [portmap]          - add a dip entry to switch table\n", cmd);
    printf(" %s dip del [dip]             - del a dip entry to switch table\n", cmd);
    printf(" %s dip dump                 - dump switch dip table\n", cmd);
    printf(" %s dip clear                - clear switch dip table\n", cmd);
    printf(" %s dump    - dump switch table\n", cmd);
    printf(" %s ingress-rate on [port] [Kbps]    - set ingress rate limit on port 0~4 \n", cmd);
    printf(" %s egress-rate on [port] [Kbps]     - set egress rate limit on port 0~4 \n", cmd);
    printf(" %s ingress-rate off [port]          - del ingress rate limit on port 0~4 \n", cmd);
    printf(" %s egress-rate off [port]           - del egress rate limit on port 0~4\n", cmd);
    printf(" %s filt [mac]               - add a SA filtering entry (with portmap 1111111) to switch table\n", cmd);
    printf(" %s filt [mac] [portmap]         - add a SA filtering entry to switch table\n", cmd);
    printf(" %s filt [mac] [portmap] [vlan id]       - add a SA filtering entry to switch table\n", cmd);
    printf(" %s filt [mac] [portmap] [vlan id] [age] - add a SA filtering entry to switch table\n", cmd);
    printf(" %s igmpsnoop on [Query Interval] [default router portmap] - turn on IGMP snoop and  router port learning (Query Interval 1~255)\n", cmd);
    printf(" %s igmpsnoop off                  - turn off IGMP snoop and router port learning\n", cmd);
    printf(" %s igmpsnoop enable [port#]               - enable IGMP HW leave/join/Squery/Gquery\n", cmd);
    printf(" %s igmpsnoop disable [port#]              - disable IGMP HW leave/join/Squery/Gquery\n", cmd);
    printf(" %s mymac [mac] [portmap]          - add a mymac entry to switch table\n", cmd);
    printf(" %s mirror monitor [portnumber]        - enable port mirror and indicate monitor port number\n", cmd);
    printf(" %s mirror target [portnumber] [0:off, 1:rx, 2:tx, 3:all]  - set port mirror target\n", cmd);
    printf(" %s phy [phy_addr]         - dump phy register of specific port\n", cmd);
    printf(" %s phy             - dump all phy registers\n", cmd);
    printf(" %s pvid [port] [pvid]        - set pvid on port 0~4 \n", cmd);
    printf(" %s reg r [offset]               - register read from offset\n", cmd);
    printf(" %s reg w [offset] [value]           - register write value to offset\n", cmd);
    printf(" %s sip add [sip] [dip] [portmap]        - add a sip entry to switch table\n", cmd);
    printf(" %s sip del [sip] [dip]             - del a sip entry to switch table\n", cmd);
    printf(" %s sip dump                 - dump switch sip table\n", cmd);
    printf(" %s sip clear                - clear switch sip table\n", cmd);
    printf(" %s tag on [port]            - keep vlan tag for egress packet on prot 0~4\n", cmd);
    printf(" %s tag off [port]               - remove vlan tag for egress packet on port 0~4\n", cmd);
    printf(" %s vlan dump                - dump switch table\n", cmd);
#if defined (CONFIG_RALINK_MT7621)
    printf(" %s vlan set [vlan idx (NULL)][vid] [portmap]  - set vlan id and associated member\n", cmd);
#else
    printf(" %s vlan set [vlan idx] [vid] [portmap]  - set vlan id and associated member\n", cmd);
#endif
#endif
    switch_fini();
    exit(0);
}

#if defined (CONFIG_RALINK_MT7621)
int reg_read(int offset, int *value)
{
    struct ifreq ifr;
    esw_reg reg;

    ra_mii_ioctl_data mii;
    strncpy(ifr.ifr_name, "eth0", 5);
    ifr.ifr_data = &mii;

    mii.phy_id = 0x1f;
    mii.reg_num = offset;

    if (-1 == ioctl(esw_fd, RAETH_MII_READ, &ifr)) {
        perror("ioctl");
        close(esw_fd);
        exit(0);
    }
    *value = mii.val_out;
    return 0;

}

int reg_write(int offset, int value)
{
    struct ifreq ifr;
    esw_reg reg;
    ra_mii_ioctl_data mii;

    strncpy(ifr.ifr_name, "eth0", 5);
    ifr.ifr_data = &mii;

    mii.phy_id = 0x1f;
    mii.reg_num = offset;
    mii.val_in = value;

    if (-1 == ioctl(esw_fd, RAETH_MII_WRITE, &ifr)) {
        perror("ioctl");
        close(esw_fd);
        exit(0);
    }
    return 0;
}


#else
int reg_read(int offset, int *value)
{
    struct ifreq ifr;
    esw_reg reg;

    if (value == NULL)
        return -1;
    reg.off = offset;
    strncpy(ifr.ifr_name, "eth0", 5);
    ifr.ifr_data = &reg;
    if (-1 == ioctl(esw_fd, RAETH_ESW_REG_READ, &ifr)) {
        perror("ioctl");
        close(esw_fd);
        exit(0);
    }
    *value = reg.val;
    return 0;
}

int reg_write(int offset, int value)
{
    struct ifreq ifr;
    esw_reg reg;

    reg.off = offset;
    reg.val = value;
    strncpy(ifr.ifr_name, "eth0", 5);
    ifr.ifr_data = &reg;
    if (-1 == ioctl(esw_fd, RAETH_ESW_REG_WRITE, &ifr)) {
        perror("ioctl");
        close(esw_fd);
        exit(0);
    }
    return 0;
}
#endif

int phy_dump(int phy_addr)
{
    struct ifreq ifr;
    esw_reg reg;

    reg.val = phy_addr;
    strncpy(ifr.ifr_name, "eth0", 5);
    ifr.ifr_data = &reg;
    if (-1 == ioctl(esw_fd, RAETH_ESW_PHY_DUMP, &ifr)) {
        perror("ioctl");
        close(esw_fd);
        exit(0);
    }
    return 0;

}

int ingress_rate_set(int on_off, int port, int bw)
{
    struct ifreq ifr;
    esw_rate reg;

    reg.on_off = on_off;
    reg.port = port;
    reg.bw = bw;
    strncpy(ifr.ifr_name, "eth0", 5);
    ifr.ifr_data = &reg;
    if (-1 == ioctl(esw_fd, RAETH_ESW_INGRESS_RATE, &ifr)) {
        perror("ioctl");
        close(esw_fd);
        exit(0);
    }
    return 0;
}

int egress_rate_set(int on_off, int port, int bw)
{
    struct ifreq ifr;
    esw_rate reg;

    reg.on_off = on_off;
    reg.port = port;
    reg.bw = bw;
    strncpy(ifr.ifr_name, "eth0", 5);
    ifr.ifr_data = &reg;
    if (-1 == ioctl(esw_fd, RAETH_ESW_EGRESS_RATE, &ifr)) {
        perror("ioctl");
        close(esw_fd);
        exit(0);
    }
    return 0;
}
#if RT_TABLE_MANIPULATE

int
getnext (
        char *    src,
        int    separator,
        char *    dest
    )
{
    char *    c;
    int    len;

    if ( (src == NULL) || (dest == NULL) ) {
        return -1;
    }

    c = strchr(src, separator);
    if (c == NULL) {
        strcpy(dest, src);
        return -1;
    }
    len = c - src;
    strncpy(dest, src, len);
    dest[len] = '\0';
    return len + 1;
}

int
str_to_ip (
        unsigned int *    ip,
        char *    str
      )
{
    int    len;
    char *    ptr = str;
    char    buf[128];
    unsigned char    c[4];
    int    i;

    for (i = 0; i < 3; ++i) {
        if ((len = getnext(ptr, '.', buf)) == -1) {
            return 1; /* parse error */
        }
        c[i] = atoi(buf);
        ptr += len;
    }
    c[3] = atoi(ptr);
    *ip = (c[0]<<24) + (c[1]<<16) + (c[2]<<8) + c[3];
    return 0;
}

/* convert IP address from number to string */
void
ip_to_str (
        char *    str,
        unsigned int    ip
      )
{
    unsigned char *    ptr = (char *)&ip;
    unsigned char    c[4];

    c[0] = *(ptr);
    c[1] = *(ptr+1);
    c[2] = *(ptr+2);
    c[3] = *(ptr+3);
    /* sprintf(str, "%d.%d.%d.%d", c[0], c[1], c[2], c[3]); */
    sprintf(str, "%d.%d.%d.%d", c[3], c[2], c[1], c[0]);
}



void acl_dip_meter(int argc, char *argv[])
{
    unsigned int i, j, value, ip_value, meter;
    unsigned int idx, vid;
    int stag = 0;
    char tmpstr[16];

    if (argc < 7) {
        printf("insufficient arguments!\n");
        return;
    }


    if (!argv[5] || strlen(argv[5]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[5][i] != '0' && argv[5][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[5][i] - '0') * (1 << i);
    }


    str_to_ip(&ip_value, argv[4]);
    //set pattern
    value = (ip_value >> 16);
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x2 << 16; //ip header
    value |= 0x8 << 1; //word offset
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 0);  //w_acl entry 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set pattern
    value = (ip_value &0xffff);
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x2 << 16; //ip header
    value |= 0x9 << 1; //word offset
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 1);  //w_acl entry 1
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set pattern
    value = 0x3; //bit0,1
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80009000 + 0);  //w_acl control mask  0
    //value = (0x80009000 + 1);  //w_acl control mask  1

    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");


    //set action
    meter = strtoul(argv[6], NULL, 0);
    value = meter >> 6;//divide 64, rate limit
    value |= 0x1 << 15; //enable rate control

    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //reserved
    reg_write(REG_ESW_VLAN_VAWD2, value);
    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);


    value = (0x8000d000 + 0);  //w_acl rate control 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");
}


void acl_dip_trtcm(int argc, char *argv[])
{
    unsigned int i, j, value, ip_value;
    unsigned int idx, vid;
    unsigned int CIR, CBS, PIR, PBS;
    int stag = 0;
    char tmpstr[16];

    if (argc < 6) {
        printf("insufficient arguments!\n");
        return;
    }

    if (!argv[5] || strlen(argv[5]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[5][i] != '0' && argv[5][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[5][i] - '0') * (1 << i);
    }


    str_to_ip(&ip_value, argv[4]);
    //set pattern
    value = (ip_value >> 16);
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x2 << 16; //ip header
    value |= 0x8 << 1; //word offset
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 0);  //w_acl entry 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set pattern
    value = (ip_value &0xffff);
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x2 << 16; //ip header
    value |= 0x9 << 1; //word offset
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 1);  //w_acl entry 1
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set CBS PBS
    CIR = strtoul(argv[6], NULL, 0);
    CBS = strtoul(argv[7], NULL, 0);
    PIR = strtoul(argv[8], NULL, 0);
    PBS = strtoul(argv[9], NULL, 0);

    value = CBS << 16; //bit16~31
    value |= PBS; //bit0~15
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    CIR = CIR >> 6;
    PIR = PIR >> 6;

    value = CIR << 16; //bit16~31
    value |= PIR; //bit0~15
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80007000 + 0);  //w_acl trtcm  #0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set pattern
    value = 0x3; //bit0,1
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80009000 + 0);  //w_acl control mask  0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");


    //set action
    value = 0x0; //No drop
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0x1 << (11+1); //TrTCM green  meter#0 Low drop
    value |= 0x2 << (8+1); //TrTCM yellow  meter#0 Med drop
    value |= 0x3 << (5+1); //TrTCM red  meter#0    Hig drop
    value |= 0x1 << 0; //TrTCM drop pcd select
    reg_write(REG_ESW_VLAN_VAWD2, value);
    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);


    value = (0x8000b000 + 0);  //w_acl rule control 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");
}




void acl_ethertype(int argc, char *argv[])
{
    unsigned int i, j, value, ethertype;
    unsigned int idx, vid;
    int stag = 0;
    char tmpstr[16];

    if (argc < 6) {
        printf("insufficient arguments!\n");
        return;
    }


    if (!argv[5] || strlen(argv[5]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[5][i] != '0' && argv[5][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[5][i] - '0') * (1 << i);
    }


    ethertype = strtoul(argv[4], NULL, 16);
    //set pattern
    value = ethertype;
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x0 << 16; //mac header
    value |= 0x6 << 1; //word offset
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 0);  //w_acl entry 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set pattern
    value = 0x1; //bit0
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80009000 + 0);  //w_acl control rule  0
    //value = (0x80009000 + 7);  //w_acl control rule 7
    //value = (0x80009000 + 15);  //w_acl control rule 15
    //value = (0x80009000 + 16);  //w_acl control rule 16
    //value = (0x80009000 + 31);  //w_acl control rule 31
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");


    //set action
    value = 0x0; //default. Nodrop
    value |= 1 << 28;//acl intterupt enable
    value |= 1 << 27;//acl hit count
    value |= 6 << 4;//acl UP
    value |= 6 << 16;//eg-tag tagged
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);
    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x8000b000 + 0);  //w_acl rule action control 0
    //value = (0x8000b000 + 7);  //w_acl rule action control 7
    //value = (0x8000b000 + 15);  //w_acl rule action control 15
    //value = (0x8000b000 + 16);  //w_acl rule action control 16
    //value = (0x8000b000 + 31);  //w_acl rule action control 31
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");
}



void acl_dip_modify(int argc, char *argv[])
{
    unsigned int i, j, value, ip_value;
    unsigned int idx, vid;
    int stag = 0;
    char tmpstr[16];

    if (argc < 6) {
        printf("insufficient arguments!\n");
        return;
    }


    if (!argv[5] || strlen(argv[5]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[5][i] != '0' && argv[5][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[5][i] - '0') * (1 << i);
    }


    str_to_ip(&ip_value, argv[4]);
    //set pattern
    value = (ip_value >> 16);
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x2 << 16; //ip header
    value |= 0x8 << 1; //word offset
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 0);  //w_acl entry 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set pattern
    value = (ip_value &0xffff);
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x2 << 16; //ip header
    value |= 0x9 << 1; //word offset
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 1);  //w_acl entry 1
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set pattern
    value = 0x3; //bit0,1
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80009000 + 0);  //w_acl control rule  0
    //value = (0x80009000 + 7);  //w_acl control rule 7
    //value = (0x80009000 + 15);  //w_acl control rule 15
    //value = (0x80009000 + 16);  //w_acl control rule 16
    //value = (0x80009000 + 31);  //w_acl control rule 31
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");


    //set action
    value = 0x0; //default. Nodrop
    value |= 1 << 28;//acl intterupt enable
    value |= 1 << 27;//acl hit count
    value |= 6 << 4;//acl UP
    value |= 6 << 16;//eg-tag tagged
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);
    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);


    value = (0x8000b000 + 0);  //w_acl rule action control 0
    //value = (0x8000b000 + 7);  //w_acl rule action control 7
    //value = (0x8000b000 + 15);  //w_acl rule action control 15
    //value = (0x8000b000 + 16);  //w_acl rule action control 16
    //value = (0x8000b000 + 31);  //w_acl rule action control 31
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

}



void acl_dip_pppoe(int argc, char *argv[])
{
    unsigned int i, j, value, ip_value;
    unsigned int idx, vid;
    int stag = 0;
    char tmpstr[16];

    if (argc < 6) {
        printf("insufficient arguments!\n");
        return;
    }


    if (!argv[5] || strlen(argv[5]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[5][i] != '0' && argv[5][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[5][i] - '0') * (1 << i);
    }


    str_to_ip(&ip_value, argv[4]);
    //set pattern
    value = (ip_value >> 16);
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x2 << 16; //ip header
    value |= 0x8 << 1; //word offset
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 0);  //w_acl entry 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set pattern
    value = (ip_value &0xffff);
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x2 << 16; //ip header
    value |= 0x9 << 1; //word offset
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 1);  //w_acl entry 1
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set pattern
    value = 0x3; //bit0,1
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    //value = (0x80009000 + 0);  //w_acl control rule  0
    value = (0x80009000 + 7);  //w_acl control rule 7
    //value = (0x80009000 + 15);  //w_acl control rule 15
    //value = (0x80009000 + 16);  //w_acl control rule 16
    //value = (0x80009000 + 31);  //w_acl control rule 31
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set action
    value = 0x0; //default. Nodrop
    value |= 1 << 28;//acl intterupt enable
    value |= 1 << 27;//acl hit count
    value |= 1 << 20;//pppoe header remove
    value |= 1 << 21;//SA MAC SWAP
    value |= 1 << 22;//DA MAC SWAP

    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);
    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    //value = (0x8000b000 + 0);  //w_acl rule action control 0
    value = (0x8000b000 + 7);  //w_acl rule action control 7
    //value = (0x8000b000 + 15);  //w_acl rule action control 15
    //value = (0x8000b000 + 16);  //w_acl rule action control 16
    //value = (0x8000b000 + 31);  //w_acl rule action control 31
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

}




void acl_dip_add(int argc, char *argv[])
{
    unsigned int i, j, value, ip_value;
    unsigned int idx, vid;
    int stag = 0;
    char tmpstr[16];

    if (argc < 6) {
        printf("insufficient arguments!\n");
        return;
    }


    if (!argv[5] || strlen(argv[5]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[5][i] != '0' && argv[5][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[5][i] - '0') * (1 << i);
    }


    str_to_ip(&ip_value, argv[4]);
    //set pattern
    value = (ip_value >> 16);
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x2 << 16; //ip header
    value |= 0x8 << 1; //word offset
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 0);  //w_acl entry 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set pattern
    value = (ip_value &0xffff);
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x2 << 16; //ip header
    value |= 0x9 << 1; //word offset
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 1);  //w_acl entry 1
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set pattern
    value = 0x3; //bit0,1
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80009000 + 0);  //w_acl control rule  0
    //value = (0x80009000 + 2);  //w_acl control rule 2
    //value = (0x80009000 + 15);  //w_acl control rule 15
    //value = (0x80009000 + 16);  //w_acl control rule 16
    //value = (0x80009000 + 31);  //w_acl control rule 31
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");


    //set action
    //value = 0x0; //default
    value = 0x7; //drop
    value |= 1 << 28;//acl intterupt enable
    value |= 1 << 27;//acl hit count
    value |= 2 << 24;//acl hit count group index (0~3)
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);
    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);


    value = (0x8000b000 + 0);  //w_acl rule action control 0
    //value = (0x8000b000 + 2);  //w_acl rule action control 2
    //value = (0x8000b000 + 15);  //w_acl rule action control 15
    //value = (0x8000b000 + 16);  //w_acl rule action control 16
    //value = (0x8000b000 + 31);  //w_acl rule action control 31
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

}

void acl_l4_add(int argc, char *argv[])
{
    unsigned int i, j, value;
    unsigned int idx, vid;
    int stag = 0;
    char tmpstr[16];

    if (argc < 6) {
        printf("insufficient arguments!\n");
        return;
    }


    if (!argv[5] || strlen(argv[5]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[5][i] != '0' && argv[5][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[5][i] - '0') * (1 << i);
    }

    value = strtoul(argv[4], NULL, 16);
    //set pattern
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x5 << 16; //L4 payload
    value |= 0x0 << 1; //word offset 0 = tcp src port
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 0);  //w_acl entry 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");


    //set rue mask
    value = 0x1; //bit0
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80009000 + 0);  //w_acl control mask  0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set action
    value = 0x7; //drop
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);
    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);


    value = (0x8000b000 + 0);  //w_acl rule control 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");
}




void acl_sp_add(int argc, char *argv[])
{
    unsigned int i, j, value;
    unsigned int idx, vid;
    int stag = 0;
    char tmpstr[16];

    if (argc < 6) {
        printf("insufficient arguments!\n");
        return;
    }


    if (!argv[5] || strlen(argv[5]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[5][i] != '0' && argv[5][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[5][i] - '0') * (1 << i);
    }

    value = strtoul(argv[4], NULL, 0);
    //set pattern
    value |= 0xffff0000;//compare mask

    reg_write(REG_ESW_VLAN_VAWD1, value);

    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = j << 8; //w_port_map
    value |= 0x1 << 19; //enable
    value |= 0x4 << 16; //L4 header
    value |= 0x0 << 1; //word offset 0 = tcp src port
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80005000 + 0);  //w_acl entry 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

    //set rue mask
    value = 0x1; //bit0
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);

    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);

    value = (0x80009000 + 0);  //w_acl control mask  0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");


    //set action
    value = 0x7; //drop
    //value |= 1;//valid
    reg_write(REG_ESW_VLAN_VAWD1, value);
    printf("REG_ESW_VLAN_VAWD1 value is 0x%x\n\r", value);

    value = 0; //bit32~63
    reg_write(REG_ESW_VLAN_VAWD2, value);
    printf("REG_ESW_VLAN_VAWD2 value is 0x%x\n\r", value);


    value = (0x8000b000 + 0);  //w_acl rule control 0
    reg_write(REG_ESW_VLAN_VTCR, value);

    printf("REG_ESW_VLAN_VTCR value is 0x%x\n\r\n\r", value);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");

}


void dip_dump(void)
{
    int i, j, value, mac, mac2, value2;
    int vid[16];
    char tmpstr[16];
    for (i = 0; i < 8; i++) {
        reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &value);
        vid[2 * i] = value & 0xfff;
        vid[2 * i + 1] = (value & 0xfff000) >> 12;
    }

    reg_write(REG_ESW_WT_MAC_ATC, 0x8104);//dip search command
    printf("hash   port(0:6)   rsp_cnt  flag  timer    dip-address       ATRD\n");
    for (i = 0; i < 0x800; i++) {
        while(1) {
            reg_read(REG_ESW_WT_MAC_ATC, &value);

            if (value & (0x1 << 13)) { //search_rdy
                reg_read(REG_ESW_TABLE_ATRD, &value2);
                //printf("REG_ESW_TABLE_ATRD=0x%x\n\r",value2);

                printf("%03x:   ", (value >> 16) & 0xfff); //hash_addr_lu
                j = (value2 >> 4) & 0xff; //r_port_map
                printf("%c", (j & 0x01)? '1':'-');
                printf("%c", (j & 0x02)? '1':'-');
                printf("%c", (j & 0x04)? '1':'-');
                printf("%c ", (j & 0x08)? '1':'-');
                printf("%c", (j & 0x10)? '1':'-');
                printf("%c", (j & 0x20)? '1':'-');
                printf("%c", (j & 0x40)? '1':'-');

                reg_read(REG_ESW_TABLE_TSRA2, &mac2);

                printf("     0x%4x", (mac2 & 0xffff)); //RESP_CNT
                printf("  0x%2x", ((mac2 >> 16)& 0xff));//RESP_FLAG
                printf("  %3d", ((mac2 >> 24)& 0xff));//RESP_TIMER
                //printf(" %4d", (value2 >> 24) & 0xff); //r_age_field
                reg_read(REG_ESW_TABLE_TSRA1, &mac);
                ip_to_str(tmpstr, mac);
                printf("     %s", tmpstr);
                printf("  0x%8x\n", value2);//ATRD
                //printf("%04x", ((mac2 >> 16) & 0xffff));
                //printf("     %c\n", (((value2 >> 20) & 0x03)== 0x03)? 'y':'-');
                if (value & 0x4000) {
                    printf("end of table %d\n", i);
                    return;
                }
                break;
            }
            else if (value & 0x4000) { //at_table_end
                printf("found the last entry %d (not ready)\n", i);
                return;
            }
            usleep(5000);
        }
        reg_write(REG_ESW_WT_MAC_ATC, 0x8105); //search for next dip address
        usleep(5000);
    }
}




void dip_add(int argc, char *argv[])
{
    unsigned int i, j, value;
    char tmpstr[9];

    str_to_ip(&value, argv[3]);

    reg_write(REG_ESW_WT_MAC_ATA1, value);
    printf("REG_ESW_WT_MAC_ATA1 is 0x%x\n\r",value);

    value = 0;
#if 0
    reg_write(REG_ESW_WT_MAC_ATA2, value);
    printf("REG_ESW_WT_MAC_ATA2 is 0x%x\n\r",value);
#endif
    if (!argv[4] || strlen(argv[4]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[4][i] != '0' && argv[4][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[4][i] - '0') * (1 << i);
    }
    value = j << 4; //w_port_map
    value |= (0x3<< 2); //static


    reg_write(REG_ESW_WT_MAC_ATWD, value);

    usleep(5000);
    reg_read(REG_ESW_WT_MAC_ATWD, &value);
    printf("REG_ESW_WT_MAC_ATWD is 0x%x\n\r",value);


    value = 0x8011;  //single w_dip_cmd
    reg_write(REG_ESW_WT_MAC_ATC, value);

    usleep(1000);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_WT_MAC_ATC, &value);
        if ((value & 0x8000) == 0 ){ //mac address busy
            printf("done.\n");
            return;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");
}




void dip_del(int argc, char *argv[])
{
    unsigned int i, j, value;
    char tmpstr[9];

    str_to_ip(&value, argv[3]);

    reg_write(REG_ESW_WT_MAC_ATA1, value);


    value = 0;
    reg_write(REG_ESW_WT_MAC_ATA2, value);

    value = 0; //STATUS=0, delete dip
    reg_write(REG_ESW_WT_MAC_ATWD, value);

    value = 0x8011;  //w_dip_cmd
    reg_write(REG_ESW_WT_MAC_ATC, value);


    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_WT_MAC_ATC, &value);
        if ((value & 0x8000) == 0 ){ //mac address busy
            if (argv[1] != NULL)
                printf("done.\n");
            return;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");
}



void dip_clear(void)
{

    int value;
    reg_write(REG_ESW_WT_MAC_ATC, 0x8102);//clear all dip
    usleep(5000);
    reg_read(REG_ESW_WT_MAC_ATC, &value);
    printf("REG_ESW_WT_MAC_ATC is 0x%x\n\r",value);

}






void sip_dump(void)
{
    int i, j, value, mac, mac2, value2;
    int vid[16];
    char tmpstr[16];
    for (i = 0; i < 8; i++) {
        reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &value);
        vid[2 * i] = value & 0xfff;
        vid[2 * i + 1] = (value & 0xfff000) >> 12;
    }

    reg_write(REG_ESW_WT_MAC_ATC, 0x8204);//sip search command
    printf("hash  port(0:6)   dip-address    sip-address      ATRD\n");
    for (i = 0; i < 0x800; i++) {
        while(1) {
            reg_read(REG_ESW_WT_MAC_ATC, &value);

            if (value & (0x1 << 13)) { //search_rdy
                reg_read(REG_ESW_TABLE_ATRD, &value2);
                //printf("REG_ESW_TABLE_ATRD=0x%x\n\r",value2);

                printf("%03x:  ", (value >> 16) & 0xfff); //hash_addr_lu
                j = (value2 >> 4) & 0xff; //r_port_map
                printf("%c", (j & 0x01)? '1':'-');
                printf("%c", (j & 0x02)? '1':'-');
                printf("%c", (j & 0x04)? '1':'-');
                printf("%c", (j & 0x08)? '1':'-');
                printf(" %c", (j & 0x10)? '1':'-');
                printf("%c", (j & 0x20)? '1':'-');
                printf("%c", (j & 0x40)? '1':'-');

                reg_read(REG_ESW_TABLE_TSRA2, &mac2);

                ip_to_str(tmpstr, mac2);
                printf("   %s", tmpstr);


                //printf(" %4d", (value2 >> 24) & 0xff); //r_age_field
                reg_read(REG_ESW_TABLE_TSRA1, &mac);
                ip_to_str(tmpstr, mac);
                printf("    %s", tmpstr);
                printf("      0x%x\n", value2);
                //printf("%04x", ((mac2 >> 16) & 0xffff));
                //printf("     %c\n", (((value2 >> 20) & 0x03)== 0x03)? 'y':'-');
                if (value & 0x4000) {
                    printf("end of table %d\n", i);
                    return;
                }
                break;
            }
            else if (value & 0x4000) { //at_table_end
                printf("found the last entry %d (not ready)\n", i);
                return;
            }
            usleep(5000);
        }
        reg_write(REG_ESW_WT_MAC_ATC, 0x8205); //search for next sip address
        usleep(5000);
    }
}





void sip_add(int argc, char *argv[])
{
    unsigned int i, j, value;
    char tmpstr[9];

    str_to_ip(&value, argv[3]);//SIP

    reg_write(REG_ESW_WT_MAC_ATA2, value);
    printf("REG_ESW_WT_MAC_ATA2 is 0x%x\n\r",value);

    value = 0;

    str_to_ip(&value, argv[4]);//DIP
    reg_write(REG_ESW_WT_MAC_ATA1, value);
    printf("REG_ESW_WT_MAC_ATA1 is 0x%x\n\r",value);

    if (!argv[5] || strlen(argv[5]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[5][i] != '0' && argv[5][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[5][i] - '0') * (1 << i);
    }
    value = j << 4; //w_port_map
    value |= (0x3<< 2); //static


    reg_write(REG_ESW_WT_MAC_ATWD, value);

    usleep(5000);
    reg_read(REG_ESW_WT_MAC_ATWD, &value);
    printf("REG_ESW_WT_MAC_ATWD is 0x%x\n\r",value);


    value = 0x8021;  //single w_sip_cmd
    reg_write(REG_ESW_WT_MAC_ATC, value);

    usleep(1000);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_WT_MAC_ATC, &value);
        if ((value & 0x8000) == 0 ){ //mac address busy
            printf("done.\n");
            return;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");
}

void sip_del(int argc, char *argv[])
{
    unsigned int i, j, value;
    char tmpstr[9];

    str_to_ip(&value, argv[3]);

    reg_write(REG_ESW_WT_MAC_ATA2, value);//SIP


    str_to_ip(&value, argv[4]);
    reg_write(REG_ESW_WT_MAC_ATA1, value);//DIP

    value = 0; //STATUS=0, delete sip
    reg_write(REG_ESW_WT_MAC_ATWD, value);

    value = 0x8021;  //w_sip_cmd
    reg_write(REG_ESW_WT_MAC_ATC, value);


    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_WT_MAC_ATC, &value);
        if ((value & 0x8000) == 0 ){ //mac address busy
            if (argv[1] != NULL)
                printf("done.\n");
            return;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");
}

void sip_clear(void)
{

    int value;
    reg_write(REG_ESW_WT_MAC_ATC, 0x8202);//clear all sip
    usleep(5000);
    reg_read(REG_ESW_WT_MAC_ATC, &value);
    printf("REG_ESW_WT_MAC_ATC is 0x%x\n\r",value);

}






void table_dump(void)
{
    int i, j, value, mac, mac2, value2;
    int vid[16];

    for (i = 0; i < 8; i++) {
        reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &value);
        vid[2 * i] = value & 0xfff;
        vid[2 * i + 1] = (value & 0xfff000) >> 12;
    }

    reg_write(REG_ESW_WT_MAC_ATC, 0x8004);
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
    printf("hash  port(0:6)   fid   vid  age   mac-address     filter my_mac\n");
#else
    printf("hash  port(0:6)   fid   vid  age   mac-address     filter\n");
#endif
    for (i = 0; i < 0x800; i++) {
        while(1) {
            reg_read(REG_ESW_WT_MAC_ATC, &value);

            if (value & (0x1 << 13)) { //search_rdy
                printf("%03x:   ", (value >> 16) & 0xfff); //hash_addr_lu
                reg_read(REG_ESW_TABLE_ATRD, &value2);
                j = (value2 >> 4) & 0xff; //r_port_map
                printf("%c", (j & 0x01)? '1':'-');
                printf("%c", (j & 0x02)? '1':'-');
                printf("%c", (j & 0x04)? '1':'-');
                printf("%c ", (j & 0x08)? '1':'-');
                printf("%c", (j & 0x10)? '1':'-');
                printf("%c", (j & 0x20)? '1':'-');
                printf("%c", (j & 0x40)? '1':'-');
                printf("%c", (j & 0x80)? '1':'-');

                reg_read(REG_ESW_TABLE_TSRA2, &mac2);

                printf("   %2d", (mac2 >> 12) & 0x7); //FID
                printf("  %4d", (mac2 & 0xfff));
                printf(" %4d", (value2 >> 24) & 0xff); //r_age_field
                reg_read(REG_ESW_TABLE_TSRA1, &mac);
                printf("  %08x", mac);
                printf("%04x", ((mac2 >> 16) & 0xffff));
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7621)
                printf("     %c", (((value2 >> 20) & 0x03)== 0x03)? 'y':'-');
                printf("     %c\n", (((value2 >> 23) & 0x01)== 0x01)? 'y':'-');
#else
                printf("     %c\n", (((value2 >> 20) & 0x03)== 0x03)? 'y':'-');
#endif
                if (value & 0x4000) {
                    printf("end of table %d\n", i);
                    return;
                }
                break;
            }
            else if (value & 0x4000) { //at_table_end
                printf("found the last entry %d (not ready)\n", i);
                return;
            }
            usleep(5000);
        }
        reg_write(REG_ESW_WT_MAC_ATC, 0x8005); //search for next address
        usleep(5000);
    }
}

void table_add(int argc, char *argv[])
{
    unsigned int i, j, value, is_filter, is_mymac;
    char tmpstr[9];

    is_filter = (argv[1][0] == 'f')? 1 : 0;
    is_mymac = (argv[1][0] == 'm')? 1 : 0;
    if (!argv[2] || strlen(argv[2]) != 12) {
        printf("MAC address format error, should be of length 12\n");
        return;
    }
    strncpy(tmpstr, argv[2], 8);
    tmpstr[8] = '\0';
    value = strtoul(tmpstr, NULL, 16);
    reg_write(REG_ESW_WT_MAC_ATA1, value);
    printf("REG_ESW_WT_MAC_ATA1 is 0x%x\n\r",value);

    strncpy(tmpstr, argv[2]+8, 4);
    tmpstr[4] = '\0';

    value = strtoul(tmpstr, NULL, 16);
    value = (value << 16);
    value |= (1 << 15);//IVL=1

    if (argc > 4) {
        j = strtoul(argv[4], NULL, 0);
        if ( 4095 < j) {
            printf("wrong vid range, should be within 0~4095\n");
            return;
        }
        value |= j; //vid
    }

    reg_write(REG_ESW_WT_MAC_ATA2, value);
    printf("REG_ESW_WT_MAC_ATA2 is 0x%x\n\r",value);

    if (!argv[3] || strlen(argv[3]) != 8) {
        if (is_filter)
            argv[3] = "1111111";
        else {
            printf("portmap format error, should be of length 7\n");
            return;
        }
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[3][i] != '0' && argv[3][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[3][i] - '0') * (1 << i);
    }
    value = j << 4; //w_port_map



    if (argc > 5) {
        j = strtoul(argv[5], NULL, 0);
        if (j < 1 || 255 < j) {
            printf("wrong age range, should be within 1~255\n");
            return;
        }
        value |= (j << 24); //w_age_field
        value |= (0x1<< 2); //dynamic
    }
    else{
        value |= (0xff << 24); //w_age_field
        value |= (0x3<< 2); //static
    }


    if (argc > 6) {
        j = strtoul(argv[6], NULL, 0);
        if ( 7 < j) {
            printf("wrong eg-tag range, should be within 0~7\n");
            return;
        }
        value |= (j << 13); //EG_TAG
    }


    if (is_filter)
        value |= (7 << 20); //sa_filter

    if (is_mymac)
        value |= (1 << 23);


    reg_write(REG_ESW_WT_MAC_ATWD, value);

    usleep(5000);
    reg_read(REG_ESW_WT_MAC_ATWD, &value);
    printf("REG_ESW_WT_MAC_ATWD is 0x%x\n\r",value);


    value = 0x8001;  //w_mac_cmd
    reg_write(REG_ESW_WT_MAC_ATC, value);

    usleep(1000);

    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_WT_MAC_ATC, &value);
        if ((value & 0x8000) == 0 ){ //mac address busy
            printf("done.\n");
            return;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");
}

void table_del(int argc, char *argv[])
{
    int i, j, value;
    char tmpstr[9];

    if (!argv[2] || strlen(argv[2]) != 12) {
        printf("MAC address format error, should be of length 12\n");
        return;
    }
    strncpy(tmpstr, argv[2], 8);
    tmpstr[8] = '\0';
    value = strtoul(tmpstr, NULL, 16);
    reg_write(REG_ESW_WT_MAC_ATA1, value);

    strncpy(tmpstr, argv[2]+8, 4);
    tmpstr[4] = '\0';
    value = strtoul(tmpstr, NULL, 16);
    value = (value << 16);

    if (argc > 3) {
        j = strtoul(argv[3], NULL, 0);
        if (j < 0 || 7 < j) {
            printf("wrong fid range, should be within 0~7\n");
            return;
        }
        value |= (j << 12); //fid
    }

    //printf("write REG_ESW_WT_MAC_AT2  0x%x\n\r",value);

    reg_write(REG_ESW_WT_MAC_ATA2, value);


    value = 0; //STATUS=0, delete mac
    reg_write(REG_ESW_WT_MAC_ATWD, value);

    value = 0x8001;  //w_mac_cmd
    reg_write(REG_ESW_WT_MAC_ATC, value);



    for (i = 0; i < 20; i++) {
        reg_read(REG_ESW_WT_MAC_ATC, &value);
        if ((value & 0x8000) == 0 ){ //mac address busy
            if (argv[1] != NULL)
                printf("done.\n");
            return;
        }
        usleep(1000);
    }
    if (i == 20)
        printf("timeout.\n");
}



void table_clear(void)
{

    int i, value, mac;
    reg_write(REG_ESW_WT_MAC_ATC, 0x8002);
    usleep(5000);
    reg_read(REG_ESW_WT_MAC_ATC, &value);

    printf("REG_ESW_WT_MAC_ATC is 0x%x\n\r",value);

}


void set_mirror_to(int argc, char *argv[])
{
    unsigned int value;
    int idx;

    idx = strtoul(argv[3], NULL, 0);
    if (idx < 0 || MAX_PORT < idx) {
        printf("wrong port member, should be within 0~%d\n", MAX_PORT);
        return;
    }

    reg_read(REG_ESW_WT_MAC_MFC, &value);
    value |= 0x1 << 3;
    value &= 0xfffffff8;
    value |= idx << 0;

    reg_write(REG_ESW_WT_MAC_MFC, value);

}


void set_mirror_from(int argc, char *argv[])
{
    unsigned int offset, value;
    int idx, mirror;

    idx = strtoul(argv[3], NULL, 0);
    mirror = strtoul(argv[4], NULL, 0);

    if (idx < 0 || MAX_PORT < idx) {
        printf("wrong port member, should be within 0~%d\n", MAX_PORT);
        return;
    }

    if (mirror < 0 || 3 < mirror) {
        printf("wrong mirror setting, should be within 0~3\n");
        return;
    }


    offset = (0x2004 | (idx << 8));
    reg_read(offset, &value);

    value &= 0xfffffcff;
    value |= mirror << 8;

    reg_write(offset, value);




}

#if defined (CONFIG_RALINK_MT7621)
void vlan_dump(void)
{
    int i, j, vid, value, value2;
    printf("  vid  fid  portmap    s-tag\n");
    for (i = 1; i < 4095; i++) {
        //reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &vid);

        //value = (0x80000000 + 2*i);  //r_vid_cmd
        value = (0x80000000 + i);  //r_vid_cmd
        reg_write(REG_ESW_VLAN_VTCR, value);

        for (j = 0; j < 20; j++) {
            reg_read(REG_ESW_VLAN_VTCR, &value);
            if ((value & 0x80000000) == 0 ){ //mac address busy
                break;
            }
            usleep(1000);
        }
        if (j == 20)
            printf("timeout.\n");


        reg_read(REG_ESW_VLAN_VAWD1, &value);
        reg_read(REG_ESW_VLAN_VAWD2, &value2);
        //printf("REG_ESW_VLAN_VAWD1 value%d is 0x%x\n\r", i, value);
        //printf("REG_ESW_VLAN_VAWD2 value%d is 0x%x\n\r", i, value2);

        if((value & 0x01) != 0){
            printf(" %4d  ", i);
            printf(" %2d ",((value & 0xe)>>1));
            printf(" %c", (value & 0x00010000)? '1':'-');
            printf("%c", (value & 0x00020000)? '1':'-');
            printf("%c", (value & 0x00040000)? '1':'-');
            printf("%c", (value & 0x00080000)? '1':'-');
            printf("%c", (value & 0x00100000)? '1':'-');
            printf("%c", (value & 0x00200000)? '1':'-');
            printf("%c", (value & 0x00400000)? '1':'-');
            printf("%c", (value & 0x00800000)? '1':'-');
            printf("    %4d\n", ((value & 0xfff0)>>4)) ;
        }
        else{
            /*print 16 vid for reference information*/
            if(i<=16){
                printf(" %4d  ", i);
                printf(" %2d ",((value & 0xe)>>1));
                printf(" invalid\n");
            }
        }
#if 0
        value = (0x80000000 + 2*i +1);  //r_vid_cmd
        reg_write(REG_ESW_VLAN_VTCR, value);
        for (j = 0; j < 20; j++) {
            reg_read(REG_ESW_VLAN_VTCR, &value);
            if ((value & 0x80000000) == 0 ){ //mac address busy
                break;
            }
            usleep(1000);
        }
        if (j == 20)
            printf("timeout.\n");


        reg_read(REG_ESW_VLAN_VAWD1, &value);
        reg_read(REG_ESW_VLAN_VAWD2, &value2);

        //printf("\n\rREG_ESW_VLAN_VAWD1 value%d is 0x%x\n\r", i, value);
        //printf("REG_ESW_VLAN_VAWD2 value%d is 0x%x\n\r", i, value2);

        printf(" %2d  %4d  ", 2*i+1, ((vid & 0xfff000) >> 12));

        printf(" %2d ",((value & 0xe)>>1));

        if((value & 0x01) != 0){
            printf(" %c", (value & 0x00010000)? '1':'-');
            printf("%c", (value & 0x00020000)? '1':'-');
            printf("%c", (value & 0x00040000)? '1':'-');
            printf("%c", (value & 0x00080000)? '1':'-');
            printf("%c", (value & 0x00100000)? '1':'-');
            printf("%c", (value & 0x00200000)? '1':'-');
            printf("%c", (value & 0x00400000)? '1':'-');
            printf("%c", (value & 0x00800000)? '1':'-');
            printf("    %4d\n", ((value & 0xfff0)>>4)) ;
        }
        else{
            printf(" invalid\n");
        }
#endif
    }
}

#else
void vlan_dump(void)
{
    int i, j, vid, value, value2;
    printf("idx   vid  fid  portmap    s-tag\n");
    for (i = 0; i < 8; i++) {
        reg_read(REG_ESW_VLAN_ID_BASE + 4*i, &vid);


        value = (0x80000000 + 2*i);  //r_vid_cmd
        reg_write(REG_ESW_VLAN_VTCR, value);

        for (j = 0; j < 20; j++) {
            reg_read(REG_ESW_VLAN_VTCR, &value);
            if ((value & 0x80000000) == 0 ){ //mac address busy
                break;
            }
            usleep(1000);
        }
        if (j == 20)
            printf("timeout.\n");


        reg_read(REG_ESW_VLAN_VAWD1, &value);
        reg_read(REG_ESW_VLAN_VAWD2, &value2);
        //printf("REG_ESW_VLAN_VAWD1 value%d is 0x%x\n\r", i, value);
        //printf("REG_ESW_VLAN_VAWD2 value%d is 0x%x\n\r", i, value2);
        printf(" %2d  %4d  ", 2*i, vid & 0xfff);
        printf(" %2d ",((value & 0xe)>>1));

        if((value & 0x01) != 0){
            printf(" %c", (value & 0x00010000)? '1':'-');
            printf("%c", (value & 0x00020000)? '1':'-');
            printf("%c", (value & 0x00040000)? '1':'-');
            printf("%c", (value & 0x00080000)? '1':'-');
            printf("%c", (value & 0x00100000)? '1':'-');
            printf("%c", (value & 0x00200000)? '1':'-');
            printf("%c", (value & 0x00400000)? '1':'-');
            printf("%c", (value & 0x00800000)? '1':'-');
            printf("    %4d\n", ((value & 0xfff0)>>4)) ;
        }
        else{
            printf(" invalid\n");
        }

        value = (0x80000000 + 2*i +1);  //r_vid_cmd
        reg_write(REG_ESW_VLAN_VTCR, value);
        for (j = 0; j < 20; j++) {
            reg_read(REG_ESW_VLAN_VTCR, &value);
            if ((value & 0x80000000) == 0 ){ //mac address busy
                break;
            }
            usleep(1000);
        }
        if (j == 20)
            printf("timeout.\n");


        reg_read(REG_ESW_VLAN_VAWD1, &value);
        reg_read(REG_ESW_VLAN_VAWD2, &value2);

        //printf("\n\rREG_ESW_VLAN_VAWD1 value%d is 0x%x\n\r", i, value);
        //printf("REG_ESW_VLAN_VAWD2 value%d is 0x%x\n\r", i, value2);

        printf(" %2d  %4d  ", 2*i+1, ((vid & 0xfff000) >> 12));

        printf(" %2d ",((value & 0xe)>>1));

        if((value & 0x01) != 0){
            printf(" %c", (value & 0x00010000)? '1':'-');
            printf("%c", (value & 0x00020000)? '1':'-');
            printf("%c", (value & 0x00040000)? '1':'-');
            printf("%c", (value & 0x00080000)? '1':'-');
            printf("%c", (value & 0x00100000)? '1':'-');
            printf("%c", (value & 0x00200000)? '1':'-');
            printf("%c", (value & 0x00400000)? '1':'-');
            printf("%c", (value & 0x00800000)? '1':'-');
            printf("    %4d\n", ((value & 0xfff0)>>4)) ;
        }
        else{
            printf(" invalid\n");
        }
    }
}
#endif


#endif



#if defined (CONFIG_RALINK_MT7621)
void vlan_set(int argc, char *argv[])
{
    unsigned int i, j, value, value2;
    int idx, vid;
    int stag = 0;
    unsigned char eg_con = 0;
    unsigned char eg_tag = 0;

    if (argc < 5) {
        printf("insufficient arguments!\n");
        return;
    }
    vid = strtoul(argv[4], NULL, 0);
    if (vid < 0 || 0xfff < vid) {
        printf("wrong vlan id range, should be within 0~4095\n");
        return;
    }
    if (strlen(argv[5]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 8; i++) {
        if (argv[5][i] != '0' && argv[5][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[5][i] - '0') * (1 << i);
    }
    //set vlan identifier
    /*
       reg_read(REG_ESW_VLAN_ID_BASE + 4*(idx/2), &value);
       if (idx % 2 == 0) {
       value &= 0xfff000;
       value |= vid;
       }
       else {
       value &= 0xfff;
       value |= (vid << 12);
       }
       reg_write(REG_ESW_VLAN_ID_BASE + 4*(idx/2), value);
       */

    /*port stag*/
    if (argc > 6) {
        stag = strtoul(argv[6], NULL, 16);
        printf("STAG index is 0x%x\n", stag);
    }

    //set vlan member
    value = (j << 16);
    //value |= (idx << 1);//fid
    value |= (1 << 30);//IVL=1
    value |= ((stag & 0xfff) << 4);//stag
    value |= 1;//valid

    if(argc > 7) {
        value |= (eg_con << 29);//eg_con
        value |= (1 << 28);//eg tag control enable
    }

    if (argc > 8) {
        value |= (1 << 28);//eg tag control enable
        value2 = eg_tag; //port 0
        value2 |= eg_tag << 2; //port  1
        value2 |= eg_tag << 4; //port 2
        reg_write(REG_ESW_VLAN_VAWD2, value2);
    }
    reg_write(REG_ESW_VLAN_VAWD1, value);

    //value = (0x80001000 + idx);  //w_vid_cmd
    value = (0x80001000 + vid);  //w_vid_cmd
    reg_write(REG_ESW_VLAN_VTCR, value);


    for (j = 0; j < 300; j++) {
        usleep(1000);
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
    }

    if (j == 300)
        printf("config vlan timeout.\n");
}
#else
void vlan_set(int argc, char *argv[])
{
    unsigned int i, j, value, value2;
    int idx, vid;
    int stag = 0;
    unsigned char eg_con = 0;
    unsigned char eg_tag = 0;

    if (argc < 6) {
        printf("insufficient arguments!\n");
        return;
    }
    idx = strtoul(argv[3], NULL, 0);
    if (idx < 0 || 15 < idx) {
        printf("wrong member index range, should be within 0~15\n");
        return;
    }
    vid = strtoul(argv[4], NULL, 0);
    if (vid < 0 || 0xfff < vid) {
        printf("wrong vlan id range, should be within 0~4095\n");
        return;
    }
    if (strlen(argv[5]) != 8) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 8; i++) {
        if (argv[5][i] != '0' && argv[5][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[5][i] - '0') * (1 << i);
    }
    //set vlan identifier
    reg_read(REG_ESW_VLAN_ID_BASE + 4*(idx/2), &value);
    if (idx % 2 == 0) {
        value &= 0xfff000;
        value |= vid;
    }
    else {
        value &= 0xfff;
        value |= (vid << 12);
    }
    reg_write(REG_ESW_VLAN_ID_BASE + 4*(idx/2), value);


#if 0 // for testing
    if (argc > 6) {
        stag = strtoul(argv[6], NULL, 0);
        if (stag < 0 || 4095 < stag) {
            printf("wrong STAG range, should be within 0~4095\n");
            return;
        }
    }
    if (argc > 7) {
        eg_con = strtoul(argv[7], NULL, 0);
        if (1 < eg_con) {
            printf("wrong eg_con range, should be within 0~1\n");
            return;
        }
    }
    if (argc > 8) {
        eg_tag = strtoul(argv[8], NULL, 0);
        if (3 < eg_tag) {
            printf("wrong eg_tag range, should be within 0~3\n");
            return;
        }
    }
#else
    /*test port stag*/
    if (argc > 6) {
        stag = strtoul(argv[6], NULL, 16);
        printf("STAG index is 0x%x\n", stag);
    }



#endif


    //set vlan member
    value = (j << 16);
#if 0
    /*port based stag*/
    value |= ((stag & 0xfff) << 4);//stag
    value |= (1 << 31);//port based stag=1
#else
    //value |= (idx << 1);//fid

    value |= (1 << 30);//IVL=1
    value |= ((stag & 0xfff) << 4);//stag

#endif
    value |= 1;//valid

    if(argc > 7) {
        value |= (eg_con << 29);//eg_con
        value |= (1 << 28);//eg tag control enable
    }

    if (argc > 8) {
        value |= (1 << 28);//eg tag control enable
        value2 = eg_tag; //port 0
        value2 |= eg_tag << 2; //port  1
        value2 |= eg_tag << 4; //port 2
        reg_write(REG_ESW_VLAN_VAWD2, value2);
    }
#if 0
    /*port based stag*/
    value2 = 0;
    value2 |= ((stag & 0x3f) << 2);//stag
    reg_write(REG_ESW_VLAN_VAWD2, value2);

    printf("REG_ESW_VLAN_VAWD1 is 0x%x\n\r", value);
    printf("REG_ESW_VLAN_VAWD2 is 0x%x\n\r", value2);
#endif
    reg_write(REG_ESW_VLAN_VAWD1, value);

    value = (0x80001000 + idx);  //w_vid_cmd
    reg_write(REG_ESW_VLAN_VTCR, value);


    for (j = 0; j < 300; j++) {
        usleep(1000);
        reg_read(REG_ESW_VLAN_VTCR, &value);
        if ((value & 0x80000000) == 0 ){ //table busy
            break;
        }
    }

    if (j == 300)
        printf("config vlan timeout.\n");
}



#endif

void igmp_on(int argc, char *argv[])
{
    unsigned int i, j, value;
    int interval;

    if (argc < 5) {
        printf("insufficient arguments!\n");
        return;
    }
    interval = strtoul(argv[3], NULL, 0);

    if (strlen(argv[4]) != 7) {
        printf("portmap format error, should be of length 7\n");
        return;
    }
    j = 0;
    for (i = 0; i < 7; i++) {
        if (argv[4][i] != '0' && argv[4][i] != '1') {
            printf("portmap format error, should be of combination of 0 or 1\n");
            return;
        }
        j += (argv[4][i] - '0') * (1 << i);
    }

    //set ISC: IGMP Snooping Control Register (offset: 0x0018)
    value = j ;
    value |= (interval << 8);//QRY_INTL
    value |= (2 << 16);//robust value
    value |= (1 << 18);//enable

    reg_write(REG_ESW_ISC, value);

    printf("config igmpsnoop.\n");
}


void igmp_off()
{
    unsigned int value;

    //set ISC: IGMP Snooping Control Register (offset: 0x0018)
    reg_read(REG_ESW_ISC, &value);
    value &= ~(1 << 18);//disable
    reg_write(REG_ESW_ISC, value);
    printf("config igmpsnoop off.\n");
}



void igmp_disable(int argc, char *argv[])
{
    unsigned int reg_offset, value;
    int port_num;

    if (argc < 4) {
        printf("insufficient arguments!\n");
        return;
    }
    port_num = strtoul(argv[3], NULL, 0);
    if (port_num < 0 || 6 < port_num) {
        printf("wrong port range, should be within 0~6\n");
        return;
    }

    //set ISC: IGMP Snooping Control Register (offset: 0x0018)
    reg_offset = 0x2008 ;
    reg_offset |= (port_num << 8);
    value = 0x8000;

    reg_write(reg_offset, value);

}


void igmp_enable(int argc, char *argv[])
{
    unsigned int reg_offset, value;
    int port_num;

    if (argc < 4) {
        printf("insufficient arguments!\n");
        return;
    }
    port_num = strtoul(argv[3], NULL, 0);
    if (port_num < 0 || 6 < port_num) {
        printf("wrong port range, should be within 0~6\n");
        return;
    }

    //set ISC: IGMP Snooping Control Register (offset: 0x0018)
    reg_offset = 0x2008 ;
    reg_offset |= (port_num << 8);
    value = 0x9755;
    reg_write(reg_offset, value);

}






int main(int argc, char *argv[])
{
    switch_init();

    if (argc < 2)
        usage(argv[0]);
#if RT_TABLE_MANIPULATE
    if (argc == 2) {
        if (!strncmp(argv[1], "dump", 5))
            table_dump();
        else if (!strncmp(argv[1], "clear", 6)) {
            table_clear();
            printf("done.\n");
        }
        else if (!strncmp(argv[1], "phy", 4)) {
            phy_dump(32); //dump all phy register
        }
        else
            usage(argv[0]);
    }
    else if (!strncmp(argv[1], "add", 4))
        table_add(argc, argv);
    else if (!strncmp(argv[1], "mymac", 4))
        table_add(argc, argv);
    else if (!strncmp(argv[1], "filt", 5))
        table_add(argc, argv);
    else if (!strncmp(argv[1], "del", 4))
        table_del(argc, argv);
    else if (!strncmp(argv[1], "phy", 4)) {
        if (argc == 3) {
            int phy_addr = strtoul(argv[2], NULL, 10);
            phy_dump(phy_addr);
        }else {
            phy_dump(32); //dump all phy register
        }
    }
    else if (!strncmp(argv[1], "sip", 5)) {
        if (argc < 3)
            usage(argv[0]);
        if (!strncmp(argv[2], "dump", 5))
            sip_dump();
        else if (!strncmp(argv[2], "add", 4))
            sip_add(argc, argv);
        else if (!strncmp(argv[2], "del", 4))
            sip_del(argc, argv);
        else if (!strncmp(argv[2], "clear", 6))
            sip_clear();
        else
            usage(argv[0]);
    }
    else if (!strncmp(argv[1], "dip", 4)) {
        if (argc < 3)
            usage(argv[0]);
        if (!strncmp(argv[2], "dump", 5))
            dip_dump();
        else if (!strncmp(argv[2], "add", 4))
            dip_add(argc, argv);
        else if (!strncmp(argv[2], "del", 4))
            dip_del(argc, argv);
        else if (!strncmp(argv[2], "clear", 6))
            dip_clear();
        else
            usage(argv[0]);
    }
    else if (!strncmp(argv[1], "mirror", 7)) {
        if (argc < 3)
            usage(argv[0]);
        if (!strncmp(argv[2], "monitor", 8))
            set_mirror_to(argc, argv);
        else if (!strncmp(argv[2], "target", 7))
            set_mirror_from(argc, argv);
        else
            usage(argv[0]);
    }

    else if (!strncmp(argv[1], "acl", 4)) {
        if (argc < 3)
            usage(argv[0]);
        if (!strncmp(argv[2], "dip", 4)){
            if (!strncmp(argv[3], "add", 4))
                acl_dip_add(argc, argv);
            else if (!strncmp(argv[3], "modup", 6))
                acl_dip_modify(argc, argv);
            else if (!strncmp(argv[3], "pppoe", 6))
                acl_dip_pppoe(argc, argv);
            else if (!strncmp(argv[3], "trtcm", 4))
                acl_dip_trtcm(argc, argv);
            else if (!strncmp(argv[3], "meter", 6))
                acl_dip_meter(argc, argv);
            else
                usage(argv[0]);
        }
        else if (!strncmp(argv[2], "etype", 6)){
            if (!strncmp(argv[3], "add", 4))
                acl_ethertype(argc, argv);
            else
                usage(argv[0]);
        }

        else if (!strncmp(argv[2], "port", 5)){
            if (!strncmp(argv[3], "add", 4))
                acl_sp_add(argc, argv);
            else
                usage(argv[0]);
        }
        else if (!strncmp(argv[2], "L4", 5)){
            if (!strncmp(argv[3], "add", 4))
                acl_l4_add(argc, argv);
            else
                usage(argv[0]);
        }
    }
#endif
    else if (!strncmp(argv[1], "vlan", 5)) {
        if (argc < 3)
            usage(argv[0]);
#if RT_TABLE_MANIPULATE
        if (!strncmp(argv[2], "dump", 5))
            vlan_dump();
        else
#endif
            if (!strncmp(argv[2], "set", 4))
                vlan_set(argc, argv);
            else
                usage(argv[0]);
    }
    else if (!strncmp(argv[1], "reg", 4)) {
        int off, val=0;
        if (argc < 4)
            usage(argv[0]);
        if (argv[2][0] == 'r') {
            off = strtoul(argv[3], NULL, 16);
            reg_read(off, &val);
            printf("switch reg read offset=%x, value=%x\n", off, val);
        }
        else if (argv[2][0] == 'w') {
            if (argc != 5)
                usage(argv[0]);
            off = strtoul(argv[3], NULL, 16);
            val = strtoul(argv[4], NULL, 16);
            printf("switch reg write offset=%x, value=%x\n", off, val);
            reg_write(off, val);
        }
        else
            usage(argv[0]);
    }
    else if (!strncmp(argv[1], "ingress-rate", 6)) {
        int port=0, bw=0;

        if (argv[2][1] == 'n') {
            port = strtoul(argv[3], NULL, 0);
            bw = strtoul(argv[4], NULL, 0);
            ingress_rate_set(1, port, bw);
            printf("switch port=%d, bw=%d\n", port, bw);
        }
        else if (argv[2][1] == 'f') {
            if (argc != 4)
                usage(argv[0]);
            port = strtoul(argv[3], NULL, 0);
            ingress_rate_set(0, port, bw);
            printf("switch port=%d ingress rate limit off\n", port);
        }
        else
            usage(argv[0]);
    }
    else if (!strncmp(argv[1], "egress-rate", 6)) {
        int port=0, bw=0;

        if (argv[2][1] == 'n') {
            port = strtoul(argv[3], NULL, 0);
            bw = strtoul(argv[4], NULL, 0);
            egress_rate_set(1, port, bw);
            printf("switch port=%d, bw=%d\n", port, bw);
        }
        else if (argv[2][1] == 'f') {
            if (argc != 4)
                usage(argv[0]);
            port = strtoul(argv[3], NULL, 0);
            egress_rate_set(0, port, bw);
            printf("switch port=%d egress rate limit off\n", port);
        }
        else
            usage(argv[0]);
    }
    else if (!strncmp(argv[1], "tag", 4)) {
        int offset=0, value=0, port=0;

        port = strtoul(argv[3], NULL, 0);
        offset = 0x2004 + port * 0x100;
        reg_read(offset, &value);
        if (argv[2][1] == 'n') {
            value |= 0x20000000;
            reg_write(offset, value);
            printf("tag vid at port %d\n", port);
        }
        else if (argv[2][1] == 'f') {
            value &= 0xc0ffffff;
            reg_write(offset, value);
            printf("untag vid at port %d\n", port);
        }
        else
            usage(argv[0]);
    }
    else if (!strncmp(argv[1], "pvid", 5)) {
        int offset=0, value=0, port=0, pvid=0;

        port = strtoul(argv[2], NULL, 0);
        pvid = strtoul(argv[3], NULL, 0);
        offset = 0x2014 + port * 0x100;
        reg_read(offset, &value);
        value &= 0xfffff000;
        value |= pvid;
        reg_write(offset, value);
        printf("Set port %d pvid %d.\n", port, pvid);
    }
    else if (!strncmp(argv[1], "igmpsnoop", 10)) {
        if (argc < 3)
            usage(argv[0]);
        if (!strncmp(argv[2], "on", 3))
            igmp_on(argc, argv);
        else if (!strncmp(argv[2], "off", 4))
            igmp_off();
        else if (!strncmp(argv[2], "enable", 7))
            igmp_enable(argc, argv);
        else if (!strncmp(argv[2], "disable", 8))
            igmp_disable(argc, argv);
        else
            usage(argv[0]);
    }


    else
        usage(argv[0]);

    switch_fini();
    return 0;
}

