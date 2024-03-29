/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer
 *
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

#include "http.h"
#include "errors.h"
#include "set_error.h"

#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

struct HTTP_T {
    CURL *curl;
};

static size_t write_fn(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    HTTP_DATA *mem = (HTTP_DATA *) userp;

    void *allocated = realloc(mem->memory, mem->size + realsize + 1);
    assert(allocated != NULL);
    mem->memory = allocated;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

HTTP *http_create(const char *keydir) {
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        gs_set_error(GS_ERROR, "Failed to create cURL instance");
        return NULL;
    }

    char certificateFilePath[4096];
    sprintf(certificateFilePath, "%s%c%s", keydir, PATH_SEPARATOR, CERTIFICATE_FILE_NAME);

    char keyFilePath[4096];
    sprintf(&keyFilePath[0], "%s%c%s", keydir, PATH_SEPARATOR, KEY_FILE_NAME);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT, 1L);
    curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
    curl_easy_setopt(curl, CURLOPT_SSLCERT, certificateFilePath);
    curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
    curl_easy_setopt(curl, CURLOPT_SSLKEY, keyFilePath);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_fn);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_SESSIONID_CACHE, 0L);

    struct HTTP_T *http = malloc(sizeof(struct HTTP_T));
    assert(http != NULL);
    http->curl = curl;
    return http;
}

int http_request(HTTP *http, char *url, HTTP_DATA *data) {
    assert(http != NULL);
    assert(data != NULL);
    if (data->size > 0) {
        void *allocated = realloc(data->memory, 1);
        assert(allocated != NULL);
        data->memory = allocated;
        data->memory[0] = 0;
        data->size = 0;
    }
    CURL *curl = http->curl;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    int ret = GS_FAILED;
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        const char *errmsg = curl_easy_strerror(res);
        ret = gs_set_error(GS_IO_ERROR, "cURL error: %s", errmsg);
        goto finish;
    }
    assert (data->memory != NULL);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &http_code);

    ret = GS_OK;
    finish:
    return ret;
}

void http_destroy(HTTP *http) {
    assert(http != NULL);
    curl_easy_cleanup(http->curl);
    free((void *) http);
}

void http_set_timeout(HTTP *http, int timeout) {
    assert(http != NULL);
    curl_easy_setopt(http->curl, CURLOPT_TIMEOUT, timeout);
}

HTTP_DATA *http_data_alloc() {
    HTTP_DATA *data = malloc(sizeof(HTTP_DATA));
    assert(data != NULL);

    data->memory = malloc(1);
    assert(data->memory != NULL);
    data->size = 0;

    return data;
}

void http_data_free(HTTP_DATA *data) {
    if (data == NULL) {
        return;
    }
    if (data->memory != NULL) {
        free(data->memory);
    }

    free(data);
}
