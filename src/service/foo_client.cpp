#include <service/foo_client.h>
#include <service/prefs.h>
#include <freertos/FreeRTOS.h>
#include <esp32-hal-log.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

static char LOG_TAG[] = "FOOCLI";

#define RECV_BUF_SIZE 1024
#define TEXT_BUF_SIZE 256

static char server_address[64] = { 0 };
static int server_port = 3333;

static int sockfd;
static bool running = true;
static bool connected = false;
static struct sockaddr_in serv_addr;
static struct hostent * server;
static TaskHandle_t hTask;
static char recv_buf[RECV_BUF_SIZE];

static char title_buf[TEXT_BUF_SIZE];
static char artist_buf[TEXT_BUF_SIZE];
static bool play_sts = false;
static SemaphoreHandle_t sts_semaphore;
static TickType_t last_recv = 0;

bool foo_is_playing() {
    if(!xSemaphoreTake(sts_semaphore, pdMS_TO_TICKS(10))) {
        ESP_LOGE(LOG_TAG, "Semaphore timed out");
        return false;
    }

    bool sts = play_sts;

    xSemaphoreGive(sts_semaphore);

    return sts;
}

void foo_get_title(char * buf, size_t buf_size) {
    if(!xSemaphoreTake(sts_semaphore, pdMS_TO_TICKS(10))) {
        ESP_LOGE(LOG_TAG, "Semaphore timed out");
        return;
    }

    strncpy(buf, title_buf, buf_size-1);

    xSemaphoreGive(sts_semaphore);
}

void foo_get_artist(char * buf, size_t buf_size) {
    if(!xSemaphoreTake(sts_semaphore, pdMS_TO_TICKS(10))) {
        ESP_LOGE(LOG_TAG, "Semaphore timed out");
        return;
    }

    strncpy(buf, artist_buf, buf_size-1);

    xSemaphoreGive(sts_semaphore);
}

TickType_t foo_last_recv() {
    return last_recv;
}

#define check(expr) if (!(expr)) { ESP_LOGE(LOG_TAG, #expr); return; }

void enable_keepalive(int sock) {
    int yes = 1;
    check(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) != -1);

    int idle = 1;
    check(setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int)) != -1);

    int interval = 5;
    check(setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int)) != -1);

    int maxpkt = 10;
    check(setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int)) != -1);
}

char * parse_field(char ** current) {
    char * old_current = *current;
    char * next = strchr(old_current, FOO_SEPARATOR);
    if(next == nullptr) {
        ESP_LOGE(LOG_TAG, "Parse error: no next field");
        return nullptr;
    }
    next[0] = 0;

    *current = next + 1;
    ESP_LOGV(LOG_TAG, "Field: %s", old_current);
    return old_current;
}

void parse_line(char * line) {
    ESP_LOGV(LOG_TAG, "Line: %s", line);
    
    char * current = line;

    char * type_str = parse_field(&current);
    int type = atoi(type_str);
    ESP_LOGI(LOG_TAG, "Type = %i", type);
    if(type < 111 || type > 113) {
        return;
    }

    char * dummy = parse_field(&current);
    dummy = parse_field(&current);
    dummy = parse_field(&current);

    char * artist = parse_field(&current);
    strncpy(artist_buf, artist, TEXT_BUF_SIZE);

    // Cannot use parse_field here in case the track name might contain the separator character
    char * txt = current;
    size_t txt_len = strlen(txt);
    txt[txt_len - 1] = 0;

    if(!xSemaphoreTake(sts_semaphore, pdMS_TO_TICKS(1000))) {
        ESP_LOGE(LOG_TAG, "Semaphore timed out");
        return;
    }

    play_sts = (type == 111);
    strncpy(title_buf, txt, TEXT_BUF_SIZE);
    ESP_LOGI(LOG_TAG, "Track: %s, Artist: %s, Play_sts: %i", title_buf, artist_buf, play_sts);
    last_recv = xTaskGetTickCount();

    xSemaphoreGive(sts_semaphore);
}

void parse_block(char * block) {
    char * current = block;
    char * next = strstr(current, "\r\n");
    if(next == nullptr) {
        ESP_LOGE(LOG_TAG, "Malformed string (missing \\r\\n)");
    }
    while(next != nullptr) {
        next[0] = 0x0;

        parse_line(current);

        current = &next[2];
        if(*current == 0x0) return;
        next = strstr(current, "\r\n");
    }
}

void foo_task(void * pvParameters) {
    int r = 0;

    memset(recv_buf, 0, RECV_BUF_SIZE);
    memset(title_buf, 0, TEXT_BUF_SIZE);

    server = gethostbyname(server_address);
    if (server == NULL) {
        ESP_LOGE(LOG_TAG, "ERROR, no such host");
    }

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr,
           server->h_length);
    serv_addr.sin_port = htons(server_port);

    while(running) {
        if(sockfd > 0) {
            close(sockfd);
            sockfd = -1;
        }
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd <= 0) {
            ESP_LOGE(LOG_TAG, "ERROR opening socket");
            return;
        }
        ESP_LOGI(LOG_TAG, "Connect to %s:%i, sockfd = %i", server_address, server_port, sockfd);

        if(r = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
            if(r != 1 && (errno != ECONNABORTED && errno != EHOSTUNREACH && errno != EHOSTDOWN)) {
                ESP_LOGI(LOG_TAG, "Error (%i, errno = %i) connecting, retry in 5s...", r, errno);
            }
            vTaskDelay(pdMS_TO_TICKS(5000));
        } else {
            ESP_LOGI(LOG_TAG, "Connected");
            enable_keepalive(sockfd);
            connected = true;
            while(connected) {
                r = recv(sockfd, recv_buf, RECV_BUF_SIZE, 0);
                if(r < 0) {
                    connected = false;
                    last_recv = xTaskGetTickCount();
                    play_sts = false;
                    vTaskDelay(pdMS_TO_TICKS(5000));
                } else if (r > 0) {
                    recv_buf[r] = 0x0;
                    ESP_LOGV(LOG_TAG, "Receive: %s", recv_buf);
                    parse_block(recv_buf);
                }
            }
        }
    }
}

void foo_client_begin() {
    sts_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(sts_semaphore);

    String srvaddr = prefs_get_string(PREFS_KEY_FOOBAR_SERVER, "0.0.0.0");
    strncpy(server_address, srvaddr.c_str(), 63);

    int port = prefs_get_int(PREFS_KEY_FOOBAR_PORT);
    if(port != 0) {
        server_port = port;
    }

    if(xTaskCreate(
        foo_task,
        "FOOCLI",
        4096,
        nullptr,
        10,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}