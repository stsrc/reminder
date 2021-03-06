#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
	char *line_to_write = "One, two\0";
	ssize_t size_of_line = strlen(line_to_write);
	ssize_t ret = 0;
	int fd;
	/* opening with bad oflag */
	printf("\nOpening char dev with bad operation flag - O_RDONLY\n");
	fd = open("/dev/reminder", O_RDONLY);
	perror("open");
	printf("fd equal to %d, should be < 0\n", fd);
	printf("\nOpening char dev with bad operation flag - O_RDWR\n");
	fd = open("/dev/reminder", O_RDWR);
	perror("open");
	printf("fd equal to %d, should be < 0\n", fd);
	printf("\nOpening char dev with good operation flag - O_WRONLY\n");
	fd = open("/dev/reminder", O_WRONLY);
	printf("fd equal to %d, should be > 0\n", fd);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	printf("Starting writing test\n");
	if(argc == 2) {
		line_to_write = "THIS STRING CONTAINS FAR MORE THAN 32 CHARS" 
			"THIS STRING CONTAINS FAR MORE THAN 32 CHARS THIS ST"
			"RING CONTAINS FAR MORE THAN 32 CHARS THIS STRING CO"
			"NTAINS FAR MORE THAN 32 CHARS THIS STRING CONTAINS "
			"FAR MORE THAN 32 CHARS THIS STRING CONTAINS FAR MOR"
			"E THAN 32 CHARS \0";
		size_of_line = strlen(line_to_write);
	}
	printf("Line to write: %s\n", line_to_write);
	ret = write(fd, line_to_write, (size_t)size_of_line);
	if(ret >= 0) {
		printf("Number of bytes in written string: %d, ", (int)size_of_line); 
		if (ret == size_of_line)
			printf("is equal ");
		else 
			printf("is not equal ");
		printf("to the number of written bytes: %d!\n",(int)ret);
	} else {
		perror("write");
		printf("Error occured, ret value equal to %d\n", (int)ret);
	}
	return 0;
}
