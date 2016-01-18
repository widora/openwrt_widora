#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
  
  
#define PWM_DEV "/dev/widora_pwm"  
#define PWM_0 0
#define PWM_1 1
#define PWM_2 2
#define PWM_3 3

struct pwm_cfg{
        unsigned int enable;
        unsigned int duty_ns;
        unsigned int period_ns;
};

 
int main(int argc, char **argv)  
{  
    int ret = -1;  
    int pwm_fd;  
    int pwmno;  
    struct pwm_cfg  cfg[2];  
  
    pwm_fd = open(PWM_DEV, O_RDWR);  
    if (pwm_fd < 0) {  
        printf("open pwm fd failed\n");  
        return -1;  
    } 
 	cfg[0].enable = atoi(argv[1]);
	cfg[0].duty_ns =atoi(argv[2]);
	cfg[0].period_ns = atoi(argv[3]);
    ioctl(pwm_fd, PWM_0, &cfg[0]);   
    close(pwm_fd);          
    return 0;  
}  
