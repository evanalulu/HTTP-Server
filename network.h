#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

int parse_req(char *file_path, char *http_req);

int get_content_type(char *file_path);

char *get_content(FILE *fp, char *file_path);

int send_response(int sockfd, const char *http_status, const char *content_type, const char *file_data, size_t data_len);
