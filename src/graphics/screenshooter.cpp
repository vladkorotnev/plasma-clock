#include <graphics/screenshooter.h>
#include <esp32-hal-log.h>
#include <freertos/FreeRTOS.h>
#include <input/keys.h>
#include <graphics/framebuffer.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <os_config.h>

static char LOG_TAG[] = "SCAP";

#define SS_REQ_MAGIC 0x3939
#define SS_RES_MAGIC 0x8888

typedef struct ss_server_req_pkt {
    uint16_t magic;
    key_id_t pressed;
    key_id_t released;
} ss_server_req_pkt_t;

typedef struct ss_server_res_pkt {
    uint16_t magic;
    uint16_t width;
    uint16_t height;
    uint8_t data[DisplayFramebuffer::BUFFER_SIZE];
} ss_server_res_pkt_t;

static uint16_t server_port = 0;
static int server_sock = 0;

static void serverTaskFunc(void * pvFramebuffer) {
    FantaManipulator *fb = static_cast<FantaManipulator*> ( pvFramebuffer );
    static const TickType_t BAIL_DELAY = pdMS_TO_TICKS(5000);
    while(1) {
        server_sock = socket(AF_INET, SOCK_DGRAM, 0); // Create UDP socket
        if (server_sock < 0) {
            ESP_LOGE(LOG_TAG, "Failed to create socket");
            vTaskDelay(BAIL_DELAY);
            continue;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(server_port);
        if (bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            ESP_LOGE(LOG_TAG, "Failed to bind socket");
            close(server_sock);
            server_sock = 0;
            vTaskDelay(BAIL_DELAY);
            continue;
        }

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        ESP_LOGI(LOG_TAG, "Server up and running");
        while (1) {
            ss_server_req_pkt_t req;
            int bytes_read = recvfrom(server_sock, &req, sizeof(req), 0, (struct sockaddr *)&client_addr, &client_addr_len);
            if (bytes_read < 0) {
                ESP_LOGE(LOG_TAG, "Failed to receive request from client");
                continue;
            }

            if (bytes_read != sizeof(req)) {
                ESP_LOGE(LOG_TAG, "Incomplete request received, wanted %i bytes, got %i", sizeof(req), bytes_read);
                continue;
            }

            if (req.magic != SS_REQ_MAGIC) {
                ESP_LOGE(LOG_TAG, "Invalid request magic, wanted 0x%x, got 0x%x", SS_REQ_MAGIC, req.magic);
                continue;
            }

            ESP_LOGI(LOG_TAG, "Got packet: magic 0x%x, pressed 0x%x, released 0x%x", req.magic, req.pressed, req.released);

            // Virtual keyboard handling
            key_id_t mask = (key_id_t) 0;
            for(int i = 0; i < sizeof(key_id_t) * 8; i++) {
                mask = (key_id_t) (1 << i);
                if(mask >= KEY_MAX_INVALID) break;
                if((req.pressed & mask) != 0) {
                    ESP_LOGI(LOG_TAG, "PRESS 0x%x", mask);
                    hid_set_key_state((key_id_t) (req.pressed & mask), true);
                }
                if((req.released & mask) != 0) {
                    ESP_LOGI(LOG_TAG, "RELEASE 0x%x", mask);
                    hid_set_key_state((key_id_t) (req.released & mask), false);
                }
            }

            ss_server_res_pkt_t res;
            res.magic = SS_RES_MAGIC;
            res.width = fb->get_width();
            res.height = fb->get_height();
            // Capture and send the screen
            if(!fb->lock()) {
                ESP_LOGE(LOG_TAG, "Failed to lock framebuffer");
                continue;
            }
            memcpy(&res.data, fb->get_fanta(), DisplayFramebuffer::BUFFER_SIZE);
            fb->unlock();

            int bytes_sent = sendto(server_sock, &res, sizeof(res), 0, (struct sockaddr *)&client_addr, client_addr_len);
            if (bytes_sent < 0) {
                ESP_LOGE(LOG_TAG, "Failed to send response to client");
                continue;
            }
        }

        // Close the server socket
        close(server_sock);
        server_sock = 0;
    }
}

Screenshooter::Screenshooter(FantaManipulator * fb) {
    framebuffer = fb;
}

void Screenshooter::start_server(uint16_t port) {
    if(hServerTask != NULL) {
        ESP_LOGE(LOG_TAG, "Server already running!");
        return;
    }

    server_port = port;
    if(port == 0) return;

    xTaskCreate(
        serverTaskFunc,
        "VNC",
        2048,
        framebuffer,
        pisosTASK_PRIORITY_REMOTE_DESKTOP,
        &hServerTask
    );
}

void Screenshooter::stop_server() {
    if(hServerTask != NULL) vTaskDelete(hServerTask);
    if(server_sock != 0) {
        close(server_sock);
        server_sock = 0;
    }
}
