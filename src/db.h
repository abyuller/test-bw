#ifndef _DB_H_
#define _DB_H_

#include "cfg.h"
#include <mysql/mysql.h>
#include <pthread.h> 
#include <fcgi_config.h>
#include <fcgiapp.h>
#include <libmemcached/memcached.h>

typedef struct _ELEMENT {
	pthread_t thread;
	MYSQL *db_client;
	MYSQL_RES *db_query_result;
	MYSQL_ROW db_query_row;
	FCGX_Request cgi_request;
} ELEMENT, *PELEMENT;

int db_init_and_open(PELEMENT item);
void db_close_and_done(MYSQL *client);
char* db_get_url(PELEMENT item, const char* name);

extern memcached_st* my_memcached;

#endif
