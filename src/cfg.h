#ifndef _CFG_H_
#define _CFG_H_

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

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

