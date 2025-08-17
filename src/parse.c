#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

void add_hours(struct dbheader_t *dbhdr, struct employee_t **employees, char *hoursstring) {
	char *name = strtok(hoursstring, ",");
	char *hours = strtok(NULL, ",");

	printf("Name: %s\nHours: %s\n", name, hours);

	int i = 0;
	for (; i < dbhdr->count; i++) {
		if (!strcmp(name, (*employees)[i].name)) {
			(*employees)[i].hours += atoi(hours);
		}
	}
}

int remove_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *remove){
	int read_idx = 0;
	int write_idx = 0;
	int count = dbhdr->count;

	for(; read_idx	< dbhdr->count; read_idx++) {
		if (!strcmp(remove, (*employees)[read_idx].name)) {
			count--;
		} else {
			if (write_idx != read_idx) {
				(*employees)[write_idx] = (*employees)[read_idx];
			}

			write_idx++;
		}
	}
	
	dbhdr->count = count;
	
	struct employee_t *temp = realloc(*employees, dbhdr->count*sizeof(struct employee_t));
	if (temp == NULL) {
		printf("Realloc failed\n");
		return STATUS_ERROR;
	}

	*employees = temp;

	return STATUS_SUCCESS;
}

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
	int i = 0;
	for(; i < dbhdr->count; i++) {
		printf("Employee %d\n", i+1);
		printf("\tName: %s\n", employees[i].name);
		printf("\tAddress: %s\n", employees[i].address);
		printf("\tHours: %d\n", employees[i].hours);
	}
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
  if (*dbhdr == NULL || *employees == NULL || *addstring == NULL) {
    printf("Argument is NULL\n");
    return STATUS_ERROR;
  }

	char *name = strtok(addstring, ",");
	char *addr = strtok(NULL, ",");
	char *hours = strtok(NULL, ",");
	
  if (name == NULL) {
    printf("name is null\n");
    return STATUS_ERROR;
  }
  if (addr == NULL) {
    printf("address is null\n");
    return STATUS_ERROR;
  }
  if (hours == NULL) {
    printf("hours is null\n");
    return STATUS_ERROR;
  }

  int i = dbhdr->count - 1;


  strncpy(employees[i].name, name, sizeof(employees[i].name));
  strncpy(employees[i].address, addr, sizeof(employees[i].address));
  employees[i].hours = atoi(hours);

  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  int count = dbhdr->count;

  struct employee_t *employees = calloc(count, sizeof(struct employee_t));
  if (employees == NULL) {
    printf("Malloc failed to create employees\n");
    return STATUS_ERROR;
  }

  read(fd, employees, count*sizeof(struct employee_t));

  int i = 0;
  for (; i < count; i++) {
    employees[i].hours = ntohl(employees[i].hours);
  }

  *employeesOut = employees;

  return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  int realcount = dbhdr->count;

  dbhdr->magic = htonl(dbhdr->magic);
  dbhdr->filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount));
  dbhdr->count = htons(dbhdr->count);
  dbhdr->version = htons(dbhdr->version);

  lseek(fd, 0, SEEK_SET);

  write(fd, dbhdr, sizeof(struct dbheader_t));

  int i = 0;
  for (; i < realcount; i++) {
    employees[i].hours = htonl(employees[i].hours);
    write(fd, &employees[i], sizeof(struct employee_t));
  }


  ftruncate(fd, sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount));

  return STATUS_SUCCESS;

}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == NULL) {
    printf("Malloc failed to create db header\n");
    return STATUS_ERROR;
  }

  if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
    perror("read");
    free(header);
    return STATUS_ERROR;
  }


  header->magic = ntohl(header->magic);
  header->version = ntohs(header->version);
  header->count = ntohs(header->count);
  header->filesize = ntohl(header->filesize);

  if (header->magic != HEADER_MAGIC) {
    printf("Improper header magic\n");
    return STATUS_ERROR;
  }

  if (header->version != 1) {
    printf("Improper header version\n");
    return STATUS_ERROR;
  }

  struct stat dbstat = {0};
  if (fstat(fd, &dbstat) == -1) {
    perror("fstat");
    return STATUS_ERROR;
  }

  if (header->filesize != dbstat.st_size) {
    printf("Corrupted database\n");
    free(header);
    return STATUS_ERROR;
  }

  *headerOut = header;

  return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut) {
  if (headerOut == NULL) {
    printf("Header not created in main\n");
    return STATUS_ERROR;
  }

  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == NULL) {
    printf("Malloc failed to create db header\n");
    return STATUS_ERROR;
  }

  header->version = 0x1;
  header->count = 0;
  header->magic = HEADER_MAGIC;
  header->filesize = sizeof(struct dbheader_t);

  *headerOut = header;

  return STATUS_SUCCESS;
}


