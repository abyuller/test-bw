#include "cfg.h"
#include "db.h"
#include <signal.h>
#include <sys/socket.h>

#define SOCKET_PATH "127.0.0.1:9000" 

PELEMENT elements;
static int socketId;

void stop_all() {
	for (int i = 0; i < thread_count; ++i) {
		//FCGX_Finish_r(&elements[i].cgi_request);
		db_close_and_done(elements[i].db_client);
		//close(elements[i].cgi_request.listen_sock);
	}
	
		//socketId = 0;
	fprintf(stdout, "Received STOP signal\n");
	exit(1);
}

void system_signals_init()
{
	signal(SIGSEGV, stop_all);
	signal(SIGINT, stop_all);
	signal(SIGTERM, stop_all);
	//signal(SIGUSR3, prn);
}


//open socket descriptor

static bool check_url(char* url) {
	//FIXME: apply PCRE to the this check
	size_t s = strlen(url);
	if (s > 2) {
		if (!strcmp(url, "localhost")) return true;
		//check dot in url
		char *p = strchr(url, '.');
		return ((p > url) && (p + 1 < url + s));
	}
	return false;
}

static void show_error(char* url, int error, char* txt, FCGX_Stream *out) {
	FCGX_FPrintF(out, "%d %s\r\n", error, txt);
	FCGX_PutS("Content-type: text/plain\r\n", out);
	FCGX_PutS("\r\n", out);
	FCGX_FPrintF(out, "Error %d\r\n", error);
	FCGX_FPrintF(out, "%s\r\n", txt);
	FCGX_FPrintF(out, "URL: %s\r\n", url);	
}

static void show_error_with_answer(char* url, int error, char* txt, char* valid_answer, FCGX_Stream *out) {
	show_error(url, error, txt, out);
	FCGX_FPrintF(out, "%s\r\n", valid_answer);	
}


static void redirect302(char* url, char* txt, FCGX_Stream *out) {
	FCGX_FPrintF(out, "302 %s\r\n", txt);
	FCGX_FPrintF(out, "Location: %s\r\n", url);
	FCGX_PutS("Content-type: text/html\r\n\r\n", out);
	FCGX_PutS("<html>\r\n", out);
	FCGX_PutS("<head>\r\n", out);
	FCGX_FPrintF(out, "<title>302 %s</title>\r\n", txt);
	FCGX_PutS("</head>\r\n", out);
	FCGX_FPrintF(out, "<body><p>URL: <b>%s</b><br>302 %s</p></body>\r\n", url, txt);
	FCGX_PutS("</html>\r\n", out);
}

static int redirect_fallback(char* url, FCGX_Stream *out) {
	if (check_url(url)) {
		redirect302(url, "Temporary moved",  out);
		return 0;
	}
	show_error(url, 502, "Incorrect Query", out);
	return 1;
}

static void *loop(void *par) { 
	PELEMENT item = (PELEMENT)par;
	
	if (0 != FCGX_InitRequest(&item->cgi_request, socketId, 0)) { 
		fprintf(stderr, "Can't init request\n"); 
		return NULL; 
	} 
	//we don’t care about the result of connection with database
	db_init_and_open(item);
	for (;;) { 
		static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER; 
		//try to get a new request
		pthread_mutex_lock(&accept_mutex); 
		int rc = FCGX_Accept_r(&item->cgi_request); 
		pthread_mutex_unlock(&accept_mutex); 
		if (rc < 0) { 
			fprintf(stderr, "Can not accept new request\n"); 
			break; 
		} 
		char* query_string = FCGX_GetParam("QUERY_STRING", item->cgi_request.envp);
		char* request_uri = FCGX_GetParam("REQUEST_URI", item->cgi_request.envp);
		bool success = false;
		char *username, *ran, *fallback_url;
		if (strstr(request_uri, "/app/testapp?") == request_uri) {
			//parsing of QUERY_STRING
			if (strstr(query_string, "username=") == query_string) {
				username = query_string + 9;
				char *tmp = strchr(username, '&');
				if (tmp) {
					ran = tmp + 1;
					//cutting username
					tmp[0] = 0;
					//FIXME: "ran=" search can be removed
					tmp = strstr(ran, "ran=");
					//checking first symbol of username
					if ((username[0] >= 'A') && (tmp == ran)) {
						ran = tmp + 4;
						tmp = strchr(ran, '&');
						if (tmp) {
							fallback_url = tmp + 1;
							//cutting ran
							tmp[0] = 0;
							tmp = strstr(fallback_url, "pageURL=");
							if (tmp == fallback_url) {
								fallback_url = tmp + 8;
								//will be check fallback_url later
								success = true;
							}
						}
					}
				}
			}
		}
		if (success) {
			char* url = db_get_url(item, username);
			if (url) {
				printf("url = %s\n", url);
				if (check_url(url)) {
					redirect302(url, "Temporary moved", item->cgi_request.out);
				} else redirect_fallback(fallback_url, item->cgi_request.out);
				free(url);
			} else {
				if (item->db_client) {
					//username is absent in database
					if (!my_memcached) success = false;
				} else success = false;
				if (success) {
					if (!redirect_fallback(fallback_url, item->cgi_request.out))
						//fallback url is correct
						memcached_add(my_memcached, username, strlen(username), fallback_url, strlen(fallback_url), memcache_timeout, 0);
				} else
					//connection with database (and memcached) lost
					show_error(request_uri, 502, "Database Connection Error", item->cgi_request.out);
				
			}
		} else {
			char answer[256];
			sprintf(answer, "Valid Query: http://localhost/app/testapp?username=Oliver&ran=0.97584378943&pageURL=http://localhost\n\r");
			show_error_with_answer(request_uri, 502, "Incorrect Query", answer, item->cgi_request.out);
		}
		//close current state
		FCGX_Finish_r(&item->cgi_request);
	}
	db_close_and_done(item->db_client);
	return NULL; 
}

int main(int argc, char *argv[]) {
	 int err = parse_args(argc, argv);
	 if (!err) {
		err = parse_cfg();
		if (!err) {
			//init fcgi library
			FCGX_Init();
			//open new socket
			//FIXME: Why 20? May be 200 or 2000?
			socketId = FCGX_OpenSocket(SOCKET_PATH, 20);
			if (socketId < 0) {
				//error creating socket
				err = ENOTSOCK;
			} else {
				system_signals_init();
				elements = (PELEMENT)malloc(thread_count * sizeof(ELEMENT));
				//creating working threads
				for (int i = 0; i < thread_count; i++)
					pthread_create(&elements[i].thread, NULL, loop, elements + i);
				//waiting for the completion of work threads
				for (int i = 0; i < thread_count; i++)
					pthread_join(elements[i].thread, NULL);
				free(elements);
			}
		}
	 }
	 if (err) fprintf(stdout, "%s\n", strerror(err));
	 return err;
 }
 
