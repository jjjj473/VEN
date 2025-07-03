#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;
    strncat((char*)userp, contents, total);
    return total;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s [get|set] [text]\n", argv[0]);
        return 1;
    }
    CURL *curl = curl_easy_init();
    if (!curl) return 1;

    if (strcmp(argv[1], "get") == 0) {
        char buffer[4096] = {0};
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/text");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
        curl_easy_perform(curl);
        printf("%s\n", buffer);
    } else if (strcmp(argv[1], "set") == 0 && argc >= 3) {
        char json[4096];
        snprintf(json, sizeof(json), "{\"content\":\"%s\"}", argv[2]);
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/text");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_perform(curl);
        curl_slist_free_all(headers);
    } else {
        printf("Invalid arguments\n");
    }

    curl_easy_cleanup(curl);
    return 0;
}
