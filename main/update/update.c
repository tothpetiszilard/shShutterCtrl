/*
 * update.c
 *
 *  Created on: Dec 10, 2020
 *      Author: tothpetiszilard
 */
#include "update.h"
#include "update_cfg.h"

#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_err.h"
#include "tcpip_adapter.h"
#include "esp_task_wdt.h"
#include <stdio.h>
#include <string.h>
#include <esp_http_server.h>
#include "esp_ota_ops.h"
#include "../shInfo.h"

#define RESPONSE_FOR_FS		"<META http-equiv=\"refresh\" content=\"1;URL=/update\">Upload success! File is stored in file system"

static httpd_handle_t server = NULL;

static httpd_handle_t start_webserver(void);
static void stop_webserver(httpd_handle_t server);
static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data);
static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data);
/* An HTTP POST handler */
static esp_err_t post_handler(httpd_req_t *req);
/* An HTTP GET handler */
static esp_err_t get_handler(httpd_req_t *req);

static esp_err_t getFilename(char * buf, char * out, uint8_t len);
static inline int getSizeOfData(char * str, int len);
static esp_err_t getHeaderSize(char * buf, int * out);

static httpd_uri_t post =
{
    .uri       = "/update",
    .method    = HTTP_POST,
    .handler   = post_handler,
    .user_ctx  = "<META http-equiv=\"refresh\" content=\"15;URL=/update\">Update Success! Rebooting to switch image"
};

static httpd_uri_t get =
{
    .uri       = "/update",
    .method    = HTTP_GET,
    .handler   = get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "<!DOCTYPE html>\
    	     <html lang='en'>\
    	     <head>\
    	         <meta charset='utf-8'>\
    	         <meta name='viewport' content='width=device-width,initial-scale=1'/>\
    	     </head>\
    	     <body>\
    	     " xstr(TYPE) xstr(SUBTYPE) "_VER:" xstr(VER_MAJOR) "." xstr(VER_MINOR) "." xstr(VER_PATCH) " \
    	     <br>\
    	     <form method='POST' action='' enctype='multipart/form-data'>\
    	         Firmware:<br>\
    	         <input type='file' accept='.bin,.bin.gz' name='firmware'>\
    	         <input type='submit' value='Update Firmware'>\
    	     </form>\
    		 <form method='POST' action='' enctype='multipart/form-data'>\
    	         File system:<br>\
    	         <input type='file' accept='.bmp,.bin,.cfg' name='fs_file'>\
    	         <input type='submit' value='Upload file'>\
    	     </form>\
    	     </body>\
    	     </html>"
};


static TaskHandle_t ResetTaskHdl = NULL;

static void Update_Restart(void *pvParameters);
#ifdef ESP_LOGI
#undef ESP_LOGI
#endif
#define ESP_LOGI(...)

//static const char *TAG="Updater";

#ifdef ESP_LOGE
#undef ESP_LOGE
#endif
#define ESP_LOGE(...)

#define OTA_BUF_SIZE    CONFIG_OTA_BUF_SIZE

void Update_Init(void)
{
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
	if (pdPASS != xTaskCreate(Update_Restart, "UpdateReset", 768, NULL, 2, &ResetTaskHdl))
	{
		ESP_LOGE(TAG, "Task failed!");
	}
	server = start_webserver();
}

static void Update_Restart(void *pvParameters)
{
	uint32_t event_u32 = 0ul;
	for(;;)
	{
		xTaskNotifyWait(0,0xFFFFFFFFul,&event_u32,portMAX_DELAY);
		vTaskDelay(1500 / portTICK_PERIOD_MS);
		if (0!= (event_u32 & 0x01ul))
		{

			esp_restart();
			event_u32 = 0ul;
		}
	}
}

/* An HTTP GET handler */
static esp_err_t get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            }
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0)
    {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t post_handler(httpd_req_t *req)
{
    char buf[OTA_BUF_SIZE+1u];
    bool first_b = true;
    FILE * file = NULL;
	esp_err_t err = ESP_OK;
	const esp_partition_t *update_partition = NULL;
    int headerLen = 0;
    esp_ota_handle_t update_handle = 0;
    int ret, remaining = req->content_len;
	bool isFw = false, isFs = false;
    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, OTA_BUF_SIZE))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry receiving if timeout occurred */
                continue;
            }
            ESP_LOGE(TAG, "HTTP Rx failed, err: %d", ret);
            return ESP_FAIL;
        }
        buf[ret] = 0;
        /* Checking things in the header (in first chunk of data) */
        if (true == first_b)
        {
        	isFw = (NULL != strstr(buf,"name=\"firmware\""));
        	isFs = (NULL != strstr(buf,"name=\"fs_file\";"));
			if (isFw)
			{
				// FW detected
				if (ESP_OK == getHeaderSize(buf, &headerLen))
				{
#ifdef UPDATE_START_CB
					// Let app know, we are updating the fw
					UPDATE_START_CB();
#endif
					ESP_LOGI(TAG, "Starting OTA...");
					update_partition = esp_ota_get_next_update_partition(NULL);
					if (update_partition == NULL)
					{
						ESP_LOGE(TAG, "Passive OTA partition not found");
						return ESP_FAIL;
					}
					ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x", update_partition->subtype, update_partition->address);

					err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
					if (err != ESP_OK)
					{
						ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);

						return err;
					}
					ESP_LOGI(TAG, "esp_ota_begin succeeded");
				}
				else
				{
					ESP_LOGE(TAG, "Not supported content received");
					return ESP_FAIL;
				}
			}
			else if (isFs)
			{
				char filename[30] = "/littlefs/";
				getFilename(buf, &filename[strlen("/littlefs/")], sizeof(filename) - strlen("/littlefs/"));
				ESP_LOGI(TAG, "Filename : %s", filename);
				getHeaderSize(buf, &headerLen);
				ESP_LOGI(TAG, "Header size: %d", headerLen);
				file = fopen(filename,"w");
				if (NULL == file)
				{
					ESP_LOGE(TAG, "Can not create file.");
					return ESP_FAIL;
				}
			}
			else
			{
				ESP_LOGE(TAG, "Not a firmware received");
				return ESP_FAIL;
			}
        }

        esp_task_wdt_reset(); // Feed watchdog
        if (isFw)
        {
			if (first_b)
			{
				// First data chunk
				if ( ESP_OK != esp_ota_write( update_handle, (const void *)&buf[headerLen], ret - headerLen))
				{
					ESP_LOGE(TAG, "Writing first chunk FAILED, size: %d ", ( ret - headerLen));
					return ESP_FAIL;
				}
				//binary_file_len += (ret - headerLen);
				first_b = false;
			}
			else if (remaining == ret)
			{
				// Last data chunk
				int lastlen = getSizeOfData(buf,ret);
				//ESP_LOGI(TAG, "Writing last chunk");
				if (ESP_OK != esp_ota_write( update_handle, (const void *)buf, lastlen))
				{
					ESP_LOGE(TAG, "Writing last chunk FAILED");
					return ESP_FAIL;
				}
				//binary_file_len += lastlen;

			}
			else
			{
				// Consecutive data chunk
				if (ESP_OK != esp_ota_write( update_handle, (const void *)buf, ret))
				{

					ESP_LOGE(TAG, "Writing chunk FAILED");
					return ESP_FAIL;
				}
				//binary_file_len += ret;
			}
        }
        else if (isFs)
        {
        	if (first_b)
        	{
        		// First data chunk
        		int result;
        		result = fwrite(&buf[headerLen], 1, ret - headerLen,file);

        		if (0 >= result)
        		{
        			ESP_LOGE(TAG, "Writing first chunk FAILED, size: %d ", ( ret - headerLen));
					return ESP_FAIL;
        		}
        		first_b = false;
        	}
        	else if (remaining == ret)
        	{
        		// Last data chunk
				int lastlen = getSizeOfData(buf,ret);
				int result;
				//ESP_LOGI(TAG, "Writing last chunk");
				result = fwrite(buf, 1, lastlen,file);
				if (0 >= result)
				{
					ESP_LOGE(TAG, "Writing last chunk FAILED, size: %d ", lastlen);
					return ESP_FAIL;
				}
        	}
        	else
        	{
        		// Consecutive data chunk
        		int result;
        		result = fwrite(buf, 1, ret,file);
        		if (0 >= result)
				{
					ESP_LOGE(TAG, "Writing chunk FAILED, result: %d ", result);
					return ESP_FAIL;
				}
        	}
        }
        else
		{
			// Not firmware, nor file, do nothing
		}
        remaining -= ret;
        ESP_LOGI(TAG, "Remaining %d", remaining);
    }
    if (isFw)
    {
		esp_err_t ota_end_err = esp_ota_end(update_handle);
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%d", err);
			return err;
		} else if (ota_end_err != ESP_OK) {
			ESP_LOGE(TAG, "Error: esp_ota_end failed! err=0x%d. Image is invalid", ota_end_err);
			return ota_end_err;
		}

		err = esp_ota_set_boot_partition(update_partition);
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%d", err);
			return err;
		}
		ESP_LOGI(TAG, "esp_ota_set_boot_partition succeeded");
		// End response
		httpd_resp_send(req, (const char *)post.user_ctx, strlen(post.user_ctx));
		xTaskNotify(ResetTaskHdl,0x01ul,eIncrement);
    }
    else if (isFs)
    {
    	fclose(file);
    	ESP_LOGI(TAG, "File closed");
    	// End response
    	httpd_resp_send(req, RESPONSE_FOR_FS, strlen(RESPONSE_FOR_FS));
    }
    else
    {
    	// Not firmware, nor file, do nothing
    }

    return ESP_OK;
}


static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &get);
        httpd_register_uri_handler(server, &post);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

static inline int getSizeOfData(char * str, int len)
{
	int size = 0;
	for (int var = 0; var < len-7u; ++var)
	{
		if((str[var] == 0x0d) && (str[var+1u] == 0x0a) && (str[var+2u] == 0x2d) && (str[var+3u] == 0x2d) && (str[var+4u] == 0x2d) && (str[var+5u] == 0x2d) && (str[var+6u] == 0x2d))
		{
			size = var;
		}
	}
	return size;
}

static esp_err_t getHeaderSize(char * buf, int * out)
{
	esp_err_t ret = ESP_FAIL;
	char * firstByte = strstr(buf,"Content-Type: application/octet-stream");
	if ((NULL != firstByte)&& ((uint32_t)firstByte > (uint32_t)buf))
	{
		*out = (int)((uint32_t)firstByte - (uint32_t)buf) + strlen("Content-Type: application/octet-stream\r\n\r\n");
		ret = ESP_OK;
	}
	else
	{
		firstByte = strstr(buf,"Content-Type: image/bmp");
		if ((NULL != firstByte)&& ((uint32_t)firstByte > (uint32_t)buf))
		{
			*out = (int)((uint32_t)firstByte - (uint32_t)buf) + strlen("Content-Type: image/bmp\r\n\r\n");
			ret = ESP_OK;
		}
	}
	return ret;
}

static esp_err_t getFilename(char * buf, char * out, uint8_t len)
{
	esp_err_t ret = ESP_FAIL;
    char * filename_p = strstr(buf,"filename=\"");
	if (NULL != filename_p)
	{
		filename_p += strlen("filename=\"");
		for (uint8_t i = 0u; i < len; ++i)
		{
			if (*(filename_p + i) == '\"')
			{
				out[i] = 0;
				ret = ESP_OK;
				break;
			}
			else
			{
				out[i] = *(filename_p + i);
			}
		}
	}
	return ret;
}
