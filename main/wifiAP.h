#ifndef WIFIAP_H
#define WIFIAP_H

#include <stdio.h>												// Acesso as operacoes de entradas e/ou saidas.
#include <string.h>                                             // Acesso a manipulacao de strings.
#include <sys/param.h>          								// Acesso aos parametros do sistema. (=> MIN())
// #include <driver/adc.h>										// Modulo conversor ADC.
#include "freertos/FreeRTOS.h"									// Acesso aos termos.
#include "freertos/task.h"										// Acesso as prioridades da TASK.
#include "driver/gpio.h"										// Acesso ao uso das GPIOs.
#include "esp_err.h"                                            // Acesso aos tipos de erros.
#include "esp_mac.h"                                            // Acesso ao s recursos MAC.
#include "esp_wifi.h"                                           // Acesso ao modulo WiFi do ESP.
#include "esp_event.h"          								// Acesso ao eventos do ESP. (=> esp_event_base_t)
#include "esp_log.h"											// Acesso ao uso dos LOGs.
#include "nvs_flash.h"          								// Acesso as funcoes da memoria flash interna. (=> nvs_flash_init)
#include "esp_http_server.h"    								// Acesso aos recursos de Servidor Web. (=> httpd_uri_t)
#include "lwip/err.h"                                           // Acesso as trativas de erros do modulo WiFi (IP).
#include "lwip/sys.h"                                           // Acesso aos recursos do WiFi.
#include "mpasso.h"                                             // Carrega a biblioteca do motor de passo.

/* RTOS */
#define CONFIG_FREERTOS_HZ 100									// Definicao da Espressif. Escala de tempo base (vTaskDelay).

/* WiFi */
#define CONFIG_ESP_WIFI_SSID            "RoboV1R1"              // SSID do AP.
#define CONFIG_ESP_WIFI_PASSWORD        "abcdefgh"              // Senha do AP.
#define CONFIG_ESP_WIFI_CHANNEL         1                       // Canal a ser utilizado (BR).
#define CONFIG_ESP_MAX_STA_CONN         4                       // Numero maximo de conexoes.

// Pelo SDKCONFIG
#define CONFIG_ESP32_WIFI_STATIC_RX_BUFFER_NUM       10         // Define quantas pilhas estaticas (min).
#define CONFIG_ESP32_WIFI_DYNAMIC_RX_BUFFER_NUM      32         // Define quantas pilhas dinamicas (max).
#define CONFIG_ESP32_WIFI_TX_BUFFER_TYPE             1          // Define o tipo de Buffer da Tx do WiFi.

static const char *TAG = "RoboV1R1";                            // Nome da Task.

/* Bloco Inicio: WiFi AP Mode */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "Estacao "MACSTR" entrou, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "Estacao "MACSTR" saiu, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .ssid_len = strlen(CONFIG_ESP_WIFI_SSID),
            .channel = CONFIG_ESP_WIFI_CHANNEL,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .max_connection = CONFIG_ESP_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };

    if (strlen(CONFIG_ESP_WIFI_PASSWORD) == 0) 
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

	esp_wifi_connect();
    ESP_LOGI(TAG, "Soft AP Ok: SSID:%s Pass:%s Canal:%d", CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD, CONFIG_ESP_WIFI_CHANNEL);
}

void nvs_init(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Modo: AP");
}

esp_err_t get_handler(httpd_req_t *req)
{
    const char resp[] = "<!DOCTYPE html><html><head><title>ESP-IDF Robo V1 R1</title>\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>\
<body><style>*{font-family: Verdana;text-align: center;}</style>\
<h2>Ajuda</h2><h3>Utilize: http://192.168.4.1/robo?mover=____</h3><h5> (um numero de -1000 ate 1000 [mm]) ou</h5> <br>\
<h3>http://192.168.4.1/robo?girar=____</h3><h5> (um numero de -360 ate 360 [graus])</h5>\
</body></html>";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_handler_robo(httpd_req_t *req)
{
    int paramTest=0;
    // Le a linha da URI e pega o host.
    char *buf;
    size_t buf_len;
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Host: %s", buf);
        }
        free(buf);
    }

    // Le a linha da URI e pega o(s) parametro(s).
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) 
    {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) 
        {
            ESP_LOGI(TAG, "Dado na URL: %s", buf);
            char param[32];
            if (httpd_query_key_value(buf, "mover", param, sizeof(param)) == ESP_OK) 
            {
                // ESP_LOGI(TAG, "Mover = %s", param);
                paramTest=atol(param);
                if(paramTest<-1000) paramTest=-1000;
                if(paramTest> 1000) paramTest= 1000;
                if(paramTest<0) roboTraz(abs(paramTest));	// Valor negativo: Mover para traz.
                if(paramTest>0) roboFrente(paramTest);		// Valor positivo: Mover para frente.
                // ESP_LOGI(TAG, "Mover1 = %d", paramTest);
            }
            if(httpd_query_key_value(buf, "girar", param, sizeof(param)) == ESP_OK)
            {
                // ESP_LOGI(TAG, "Girar0 = %s", param);
                paramTest=atol(param);
                if(paramTest<-360) paramTest=-360;
                if(paramTest> 360) paramTest= 360;
                if(paramTest<0) roboEsq(abs(paramTest));	// Valor negativo: Girar esquerda (antihorario).
                if(paramTest>0) roboEsq(paramTest);		    // Valor positivo: Girar direita (horario).
                // ESP_LOGI(TAG, "Girar1 = %d", paramTest);
            }
            if(httpd_query_key_value(buf, "id", param, sizeof(param)) == ESP_OK)
            {
                ESP_LOGI(TAG, "Id = %s", param);
                // Mostrar no LCD a Identificacao do Cliente
            }
        }
        free(buf);
    }
    // get_handler(req);
    const char resp[] = "<!DOCTYPE html><html><head><title>ESP-IDF Robo V1 R1</title>\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>\
<body><style>*{font-family: Verdana;text-align: center;}</style>\
<h2>Ok!</h2><h3>Comando recebido!</h3></body></html>";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* Manipulador da estrutura da URI para metodo GET /uri */
httpd_uri_t uri_get = 
    {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL
    };

httpd_uri_t uri_get_robo = 
    {
    .uri = "/robo",
    .method = HTTP_GET,
    .handler = get_handler_robo,
    .user_ctx = NULL
    };

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_get_robo);
    }
    return server;
}

void stop_webserver(httpd_handle_t server)
{
    if (server)
    {
        httpd_stop(server);
    }
}

void wifi_init( void )
{
    nvs_init();
    wifi_init_softap();
    start_webserver();
}
/* Bloco Fim: WiFi AP Mode */


#endif