#include <stdio.h>  

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define FALSE  -1
#define TRUE    0

#define DEV_NAME "/dev/regopt"

int main(int argc, char **argv)
{
	int fd;
	unsigned long val[2];
	
	if (argc < 3 || argc > 4)
	{
		printf("Usage : %s r addr \n", argv[0]);
		printf("Usage : %s w addr value\n", argv[0]);
		return FALSE;
	}

	fd = open(DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		printf("can't open!\n");
		return FALSE;
	}

	val[0]=strtol(argv[2],NULL,16);
	if(*(argv[1]) == 'w')
		{
		val[1]=strtol(argv[3],NULL,16);
		write(fd, val, 8);
		}
	else if(*(argv[1]) == 'r')
		{
		read(fd, val, 8);
		printf("add:0x%x = 0x%x\n", val[0], val[1]);
		}
	close(fd);
	return 0;
}
