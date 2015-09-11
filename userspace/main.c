#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


int parser (int argc, char *argv[], char **message, ssize_t *size_of_line)
{
	char temp[32];
	if (argc == 1) {
		printf("Write message which you would like to present.\n");
		fscanf(stdin, "%s", temp);
		*message = malloc(strlen(temp) + 1);
		strcpy(*message, temp);
	} else if (argc == 2)
	{
		*message = malloc(strlen(argv[1]) + 1);
		strcpy(*message, argv[1]);
	} else {
		printf("Wrong input argument. Put message into double quotes.\n");
	       	return 1;
	}
	printf("String to show: %s", *message);
	*size_of_line = strlen(*message);
	return 0;
}

int main (int argc, char *argv[])
{
	char *line_to_write = NULL;
	ssize_t size_of_line;
	ssize_t ret = 0;
	int fd;
	if (parser(argc, argv, &line_to_write, &size_of_line))
		return 1;
	printf("\nOpening device\n");
	fd = open("/dev/reminder", O_WRONLY);
	if (fd < 0) {
		printf("fd equal to %d, should be > 0\n", fd);
		perror("open");
		return 1;
	} else {
		printf("Device has been opened properly\n");
	}
	ret = write(fd, line_to_write, (size_t)size_of_line);
	if(ret >= 0) {
		printf("Number of bytes in entered string: %d, ", (int)size_of_line); 
		if (ret == size_of_line)
			printf("is equal ");
		else 
			printf("is not equal ");
		printf("to the number of written bytes: %d.\n",(int)ret);
		printf("Driver is ready to present string.\n");
	} else {
		perror("write");
		printf("Error occured, ret value equal to %d\n", (int)ret);
	}
	return 0;
}
