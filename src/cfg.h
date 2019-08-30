#ifndef _CFG_H_
#define _CFG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

int parse_args(int argc, char *argv[]);
int parse_cfg();

extern int thread_count;

extern uint16_t mydb_port;
extern char mydb_host[256];
extern char mydb_user[33];
extern char mydb_password[33];
extern char mydb_database[33];

extern uint16_t memcache_port;
extern char memcache_host[256];
extern time_t memcache_timeout;

#endif

