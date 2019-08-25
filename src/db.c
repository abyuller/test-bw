#include "db.h"

memcached_st* my_memcached = NULL;

static void db_query_close(PELEMENT item) {
	if (item->db_query_result) {
		mysql_free_result(item->db_query_result);
		item->db_query_result = NULL;
		item->db_query_row = NULL;
	}
}

static int db_query_open(PELEMENT item, char* text) {
	int res = mysql_real_query(item->db_client, text, (int)strlen(text));
	if (!res) {
		item->db_query_result = mysql_use_result(item->db_client);
		if (item->db_query_result) {
			item->db_query_row = mysql_fetch_row(item->db_query_result);
		}
	}
	return res;
}

static char* db_query_get_value_by_index(PELEMENT item, int index) {
	return (item->db_query_row)? item->db_query_row[index]: NULL;
}

int db_init_and_open(PELEMENT item) {
	item->db_client = mysql_init(NULL);
	if (NULL == item->db_client) {
		fprintf(stderr, "Not enough memory\n");
		return ENOMEM;
	}
	if (NULL == mysql_real_connect(item->db_client, mydb_host, mydb_user, mydb_password, mydb_database, mydb_port, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(item->db_client));
		int err = mysql_errno(item->db_client);
		mysql_close(item->db_client);
		item->db_client = NULL;
		return err;
	}
	return 0;
}

void db_close_and_done(MYSQL *client) {
	mysql_close(client);
}

char* db_get_url(PELEMENT item, const char* name) {
	if (!my_memcached) {
		char str[271];
		sprintf(str, "--SERVER=%s:%u", memcache_host, memcache_port);
		my_memcached = memcached(str, strlen(str));
	}
	
	uint32_t flags;
	memcached_return_t error;
	size_t value_length, name_length = strlen(name);
	fprintf(stdout, "%s queried. length = %lu\n", name, name_length);
	char* mc = memcached_get(my_memcached, name, name_length, &value_length, &flags, &error);
	if (!mc) {
		if ((item->db_client) && (mysql_ping(item->db_client))) {
			//mysql connection lost
			db_close_and_done(item->db_client);
			item->db_client = NULL;
		}
		if (!item->db_client) {
			if (db_init_and_open(item))
				//mysql connection error
				return NULL;
		}
		char str_query[60];
		sprintf(str_query, "SELECT url FROM aliases WHERE name=\"%s\"", name);
		if (!db_query_open(item, str_query)) {
			if (item->db_query_row) {
				char* tmp = db_query_get_value_by_index(item, 0);
				value_length = strlen(tmp);
				//test on empty string
				if (tmp[0]) {
					memcached_add(my_memcached, name, name_length, tmp, value_length, memcache_timeout, 0);
					mc = (char*)malloc(value_length + 1);
					if (mc) strcpy(mc, tmp);
				}
			}
			db_query_close(item);
		}
	}
	return mc;
}


