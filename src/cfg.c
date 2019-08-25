#include "cfg.h" 
#include <linux/limits.h>

int thread_count = 0;

uint16_t mydb_port = 0;
char mydb_host[256] = {0};
char mydb_user[33] = {0};
char mydb_password[33] = {0};
char mydb_database[33] = {0};

uint16_t memcache_port = 0;
char memcache_host[256] = {0};
time_t memcache_timeout;

static char default_config_name[] = "test-bw.conf";
static char *config_name = default_config_name;

int parse_args(int argc, char *argv[]){
	if (argc == 1) return 0;
	if ((argc != 3) || (strcmp(argv[1], "-c"))) return EINVAL;
	config_name = argv[2];
	return ((strlen(config_name) > PATH_MAX)? EINVAL: 0);
}

static const char str_thread_count[] = "test-bw.thread.count";
static const char str_memcache_timeout[] = "test-bw.memcache.timeout";
static const char str_memcache_host[] = "test-bw.memcache.host";
static const char str_memcache_port[] = "test-bw.memcache.port";
static const char str_mydb_user[] = "test-bw.mysql.user";
static const char str_mydb_password[] = "test-bw.mysql.password";
static const char str_mydb_host[] = "test-bw.mysql.host";
static const char str_mydb_port[] = "test-bw.mysql.port";
static const char str_mydb_database[] = "test-bw.mysql.database";

static char* get_line_value(char* line) {
	while (strlen(line)) {
		const char c = line[0];
		if (c == ' ' || c == '=') line++;
		else return line;
	}
	return NULL;
}

static bool b_timeout_cfg = false;

static int parse_line(char* line) {
	char *tmp;
	//cleaning unvisible symbols
	int k = strlen(line);
	do {
		if (!k--) return EINVAL;
		const char c = line[k];
		if (c == ' ' || c == '\t' || c == '\n' || c == '\r') line[k] = 0;
		else break;
	} while (true);
	
	if (strstr(line, str_thread_count) == line) {
		//dual record in the config file
		if (thread_count > 0) return EINVAL;
		//reference to vaue string
		tmp = get_line_value(line + strlen(str_thread_count));
		if (!tmp) return EINVAL;
		int c = atoi(tmp);
		//limit of thread count or error 
		if (c <= 0 || c>= 65535) return EINVAL;
		thread_count = c;
	}

	if (strstr(line, str_memcache_timeout) == line) {
		if (b_timeout_cfg) return EINVAL;
		b_timeout_cfg = true;
		tmp = get_line_value(line + strlen(str_memcache_timeout));
		if (!tmp) return EINVAL;
		int c = atoi(tmp);
		if (c < 0 || c>= 65535) return EINVAL;
		memcache_timeout = (uint16_t)c;
	}

	if (strstr(line, str_memcache_port) == line) {
		if (memcache_port > 0) return EINVAL;
		tmp = get_line_value(line + strlen(str_memcache_port));
		if (!tmp) return EINVAL;
		int c = atoi(tmp);
		if (c <= 0 || c>= 65535) return EINVAL;
		memcache_port = (uint16_t)c;
	}

	if (strstr(line, str_mydb_port) == line) {
		if (mydb_port > 0) return EINVAL;
		tmp = get_line_value(line + strlen(str_mydb_port));
		if (!tmp) return EINVAL;
		int c = atoi(tmp);
		if (c <= 0 || c>= 65535) return EINVAL;
		mydb_port = (uint16_t)c;
	}

	if (strstr(line, str_memcache_host) == line) {
		if (memcache_host[0]) return EINVAL;
		tmp = get_line_value(line + strlen(str_memcache_host));
		if (!tmp) return EINVAL;
		strcpy(memcache_host, tmp);
	}

	if (strstr(line, str_mydb_host) == line) {
		if (mydb_host[0]) return EINVAL;
		tmp = get_line_value(line + strlen(str_mydb_host));
		if (!tmp) return EINVAL;
		strcpy(mydb_host, tmp);
	}

	if (strstr(line, str_mydb_user) == line) {
		if (mydb_user[0]) return EINVAL;
		tmp = get_line_value(line + strlen(str_mydb_user));
		if (!tmp) return EINVAL;
		strcpy(mydb_user, tmp);
	}

	if (strstr(line, str_mydb_password) == line) {
		if (mydb_password[0]) return EINVAL;
		tmp = get_line_value(line + strlen(str_mydb_password));
		if (!tmp) return EINVAL;
		strcpy(mydb_password, tmp);
	}

	if (strstr(line, str_mydb_database) == line) {
		if (mydb_database[0]) return EINVAL;
		tmp = get_line_value(line + strlen(str_mydb_database));
		if (!tmp) return EINVAL;
		strcpy(mydb_database, tmp);
	}
	return 0;
}

int parse_cfg() {

	char *line = NULL;
	size_t len;

	FILE *stream = fopen(config_name, "r");
	if (stream == NULL) 
		return errno;

	while (getline(&line, &len, stream) != -1) {
		if (len > 0) {
			const int e = parse_line(line);
			if (e) {
				free(line);
				fclose(stream);
				return e;
			}
		}
	}

	free(line);
	fclose(stream);
	
	return (((mydb_port == 0) || (memcache_port == 0) || (!b_timeout_cfg) ||
		(mydb_host[0] == 0) || (memcache_host[0] == 0) || (mydb_database[0] == 0) ||
		(mydb_password[0] == 0) || (mydb_user[0] == 0))? EINVAL: 0);
}
