#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#define MAX_RESPONSE_LEN 65536
#define MAX_HTTP_LEN 8192
#define MAX_PATH_LEN 256

// Extracting file path requested by client
int parse_req(char *file_path, char *http_req) {
        char *req_method = strsep(&http_req, " ");
        char *req_file_path = strsep(&http_req, " ");
        if (req_method == NULL || req_file_path == NULL)
                return 1;

        memset(file_path, 0, MAX_PATH_LEN + 1);
        strncpy(file_path, req_file_path, MAX_PATH_LEN);

        return 0;
}

// Getting type of file from specified file path
int get_content_type(const char *file_path) {
    const char *extension = strrchr(file_path, '.');
    if (extension == NULL) {
        return -1;
    }
    extension++;  // Advance past the dot

    if (strcmp(extension, "html") == 0) {
        return 0;  // HTML
    } else if (strcmp(extension, "css") == 0 || strcmp(extension, "map") == 0) {
        return 1;  // CSS or Source Map
    } else if (strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg")) {
        return 2;  // JPG
    } else if (strcmp(extension, "png") == 0) {
        return 3;  // PNG
    } 
	/* Further file support */
      else if (strcmp(extension, "svg") == 0) {
        return 4;  // SVG
    } else if (strcmp(extension, "ico") == 0) {
        return 5;  // ICO
   	} else if (strcmp(extension, "js") == 0) {
        return 6;  // JavaScript
    } else {
        return -1;  // Unknown type
    }
}

// Getting file content from specified file path
char *get_content(FILE *fp, char *file_path) {
    char *content_buffer = NULL;
    long file_size;

    if (fp == NULL) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Allocating memory for the buffer to hold file contents
    content_buffer = malloc(file_size + 1);
    if (content_buffer == NULL) {
        fclose(fp);
        return NULL;
    }

    fread(content_buffer, 1, file_size, fp);

    // Null-terminating the buffer to ensure it can be used as a string
    content_buffer[file_size] = '\0';

    fclose(fp);

    return content_buffer;
}


//Sending HTTP response to client through socket
int send_response(int sockfd, const char *http_status, const char *content_type, const char *file_data, size_t data_len) {
    char response[MAX_RESPONSE_LEN];
    int len = snprintf(response, sizeof(response),
            "HTTP/1.1 %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "\r\n",
            http_status, content_type, data_len);

    /* Error Handling */
    if (len < 0 || len >= MAX_RESPONSE_LEN) {
        return -1; // Error: response too large
    }

    int ret = send(sockfd, response, len, 0);
    if (ret < 0) {
        return -1; // Error: failed to send response
    }

    ret = send(sockfd, file_data, data_len, 0);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

