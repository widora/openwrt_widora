#include <stdio.h>  

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define FALSE  -1
#define TRUE    0

#define DEV_NAME "/dev/regopt"

unsigned char char2num(unsigned char c)
{
	if((c >= '0')&&(c <= '9')){
		return (c-'0');
	}else if((c >= 'a')&&(c <= 'f')){
		return (c-'a'+10);
	}else if((c >= 'A')&&(c <= 'F')){
		return (c-'A'+10);
	}
	return FALSE;
}

int main(int argc, char **argv)
{
	int fd;
	unsigned long val[2];
	unsigned int i;
	unsigned char *p_char = NULL;
	
	if (argc != 4)
	{
		printf("Usage : %s w/r addr value\n", argv[0]);
		return FALSE;
	}

	p_char = argv[2];
	val[0] = 0;
	for(i = 0; *p_char != '\0'; i++,p_char++)
	{
		if((i > 7)||(char2num(*p_char) == FALSE)){
			printf("addr is wrong");
			return FALSE;
		}
		val[0] <<= 4;
		val[0] |= char2num(*p_char);
	}

	p_char = argv[3];
	val[1] = 0;
	for(i = 0; *p_char != '\0'; i++,p_char++)
	{
		if((i > 7)||(char2num(*p_char) == FALSE)){
			printf("value is wrong");
			return FALSE;
		}
		val[1] <<= 4;
		val[1] |= char2num(*p_char);
	}


	fd = open(DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
		return FALSE;
	}

	if(strcmp(argv[1], "w") == 0){
		write(fd, val, 8);
	}else if(strcmp(argv[1], "r") == 0){
		read(fd, val, 8);
		printf("add:0x%x = %x \n", val[0], val[1]);
	}
	close(fd);
	return 0;
}
