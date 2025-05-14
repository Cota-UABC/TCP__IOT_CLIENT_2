#include "real_time.h"

static const char *TAG_RT = "TIME";

const char *REQUEST = "GET /api/timezone/America/Tijuana.txt HTTP/1.1\r\n"
                            "Host: worldtimeapi.org\r\n"
                            "Connection: close\r\n"
                            "\r\n";

uint32_t get_real_time() 
{
    struct sockaddr_in dest_addr = {
        .sin_addr.s_addr = inet_addr(REAL_TIME_IP), 
        .sin_family = AF_INET,
        .sin_port = htons(atoi(REAL_TIME_PORT))
    };

    char rx_buffer[1024], *date;
    int len, sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if(sock < 0) 
    {
        ESP_LOGE(TAG_RT, "Unable to create socket: errno %d", errno);
        return 0;
    }
    ESP_LOGI(TAG_RT, "Socket created, connecting to %s:%s", REAL_TIME_IP, REAL_TIME_PORT);


    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    if(connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) 
    {
        ESP_LOGE(TAG_RT, "error connecting to server");
        shutdown(sock, 0);
        close(sock);
        return 0;
    }

    send(sock, REQUEST, strlen(REQUEST), 0);
    printf("Request send\n");
    
    len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    printf("Recieved len: %d", len);
    if(len > 0) 
    {
        rx_buffer[len] = '\0';
        ESP_LOGI(TAG_RT, "Rx len: %d", len);
        
        printf("Received: %s\n", rx_buffer);
    }
    else
    {
        ESP_LOGE(TAG_RT, "No response");
    }

    //date = strstr(rx_buffer, "Date: ");
    /*if(date) 
    {
        date += 6; 
        char *hora = strchr(date, ' ');
        if(hora) 
        {
            hora = strchr(hora + 1, ' ');
            if (hora) 
            {
                hora = strchr(hora + 1, ' ');
                if (hora) 
                {
                    hora += 1;
                    char hour[128];
                    strncpy(hour, hora, 8);
                    hour[8] = '\0';
                    ESP_LOGI(TAG_RT, "Hora: %s", hour);
                }
            }
        }
    } 
    else
        ESP_LOGE(TAG_RT, "Could not get time data received...");
    */

    shutdown(sock, 0);
    close(sock);
    
    return 0;
}
