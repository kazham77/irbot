/*
 * irrx_task.c
 *
 *  Created on: 30 мая 2025 г.
 *      Author: kazham
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ir_rx.h"
#include "driver/ir_tx.h"
#include "esp_log.h"
#include "esp_system.h"

#include "ir_task.h"
#include "led_task.h"

#define AUTO_MODE_KEYS_NUM			(7U)
#define CUSTOM_MODE_KEYS_NUM		(7U)

static const char* const TAG = "IRTask";

static const char* const home  = "HOME";
static const char* const right = "RIGHT";
static const char* const left  = "LEFT";
static const char* const up    = "UP";
static const char* const ok    = "OK";

typedef struct button_seq_t
{
	ir_tx_nec_data_t* codes;
	const unsigned delay_s;
	const char* const name;
} button_seq_t;


// TOSHIBA TV
static ir_tx_nec_data_t button_B		= { .addr1 = 0x40, .addr2 = 0xBF, .cmd1 = 0x4B, .cmd2 = 0xB4 };
static ir_tx_nec_data_t button_Y		= { .addr1 = 0x40, .addr2 = 0xBF, .cmd1 = 0x4A, .cmd2 = 0xB5 };
static ir_tx_nec_data_t button_HOME		= { .addr1 = 0x40, .addr2 = 0xBE, .cmd1 = 0x34, .cmd2 = 0xCB };
static ir_tx_nec_data_t button_RIGHT	= { .addr1 = 0x40, .addr2 = 0xBF, .cmd1 = 0x40, .cmd2 = 0xBF };
static ir_tx_nec_data_t button_LEFT		= { .addr1 = 0x40, .addr2 = 0xBF, .cmd1 = 0x42, .cmd2 = 0xBD };
static ir_tx_nec_data_t button_UP		= { .addr1 = 0x40, .addr2 = 0xBF, .cmd1 = 0x19, .cmd2 = 0xE6 };
static ir_tx_nec_data_t button_OK		= { .addr1 = 0x40, .addr2 = 0xBF, .cmd1 = 0x21, .cmd2 = 0xDE };

static button_seq_t autoMode[AUTO_MODE_KEYS_NUM] = {	{ &button_HOME,		6000U,	home	},		// Must not be shorter than 6 sec for my Toshiba TV
														{ &button_RIGHT,	3000U, 	right	},		// Must not be shorter than 3 sec
														{ &button_UP, 		300U, 	up		},		// Must not be shorter than 1 sec
														{ &button_RIGHT,	300U, 	right	},		// Must not be shorter than 1 sec
														{ &button_OK,		300U, 	ok		},		// Must not be shorter than 1 sec
														{ &button_RIGHT,	300U, 	right	},		// Must not be shorter than 1 sec
														{ &button_OK, 		300U, 	ok		}	};	// Must not be shorter than 1 sec

static button_seq_t customMode[CUSTOM_MODE_KEYS_NUM] = {	{ &button_HOME,		6000U,	home	},
															{ &button_RIGHT,	3000U, 	right	},
															{ &button_UP, 		300U, 	up		},
															{ &button_RIGHT,	300U, 	right	},
															{ &button_OK, 		300U, 	ok		},
															{ &button_LEFT,		300U, 	left	},
															{ &button_OK, 		300U, 	ok		}	};

static void IRRxInit(void);
static void IRTxInit(void);

//-----------------------------------------------------------------------------

void IRTask(void* pvParameters)
{
	IRTxInit();
	vTaskDelay(1000U / portTICK_RATE_MS);
	IRRxInit();

	while(1)
	{
		ir_rx_nec_data_t ir_data = { 0 };

		ESP_LOGI(TAG, "Waiting for a signal...");
		ir_rx_enable();
		int len = ir_rx_recv_data(&ir_data, 1, portMAX_DELAY);
		ir_rx_disable();
		ESP_LOGI(TAG, "Got %u commands: [addr1: 0x%x, addr2: 0x%x, cmd1: 0x%x, cmd2: 0x%x]", len, ir_data.addr1, ir_data.addr2, ir_data.cmd1, ir_data.cmd2);

		if (ir_data.addr1 == button_B.addr1 && ir_data.addr2 == button_B.addr2 &&
			ir_data.cmd1  == button_B.cmd1 && ir_data.cmd2  == button_B.cmd2)
		{
			ESP_LOGI(TAG, "Button <B> is pressed");

			for (unsigned i = 0U; i < AUTO_MODE_KEYS_NUM; ++i)
			{
				ESP_LOGI(TAG, "\tSending <%s> button", autoMode[i].name);
				ir_tx_send_data(autoMode[i].codes, 1, portMAX_DELAY);
				ESP_LOGI(TAG, "\tPause %ums", autoMode[i].delay_s);

				gpio_set_level(LED_PIN, LED_ON);
				vTaskDelay(100U / portTICK_RATE_MS);
				gpio_set_level(LED_PIN, LED_OFF);
				vTaskDelay(autoMode[i].delay_s / portTICK_RATE_MS);
			}
		}
		else if (ir_data.addr1 == button_Y.addr1 && ir_data.addr2 == button_Y.addr2 &&
				 ir_data.cmd1 == button_Y.cmd1 && ir_data.cmd2 == button_Y.cmd2)
		{
			ESP_LOGI(TAG, "Button <Y> is pressed");

			for (unsigned i = 0U; i < CUSTOM_MODE_KEYS_NUM; ++i)
			{
				ESP_LOGI(TAG, "\tSending <%s> button", customMode[i].name);
				ir_tx_send_data(customMode[i].codes, 1, portMAX_DELAY);
				ESP_LOGI(TAG, "\tPause %ums", customMode[i].delay_s);

				gpio_set_level(LED_PIN, LED_ON);
				vTaskDelay(100U / portTICK_RATE_MS);
				gpio_set_level(LED_PIN, LED_OFF);
				vTaskDelay(customMode[i].delay_s / portTICK_RATE_MS);
			}
		}
		else // Neither <B> nor <Y> was pressed
		{
			gpio_set_level(LED_PIN, LED_ON);
			vTaskDelay(100U / portTICK_RATE_MS);
			gpio_set_level(LED_PIN, LED_OFF);
			vTaskDelay(100U / portTICK_RATE_MS);
		}
	}
}

//-----------------------------------------------------------------------------

static void IRTxInit(void)
{
	ir_tx_config_t ir_tx_config =	{
	        							.io_num = IR_TX_IO_NUM,
										.freq = 38000,
										.timer = IR_TX_WDEV_TIMER // WDEV timer will be more accurate, but PWM will not work
	    							};

	esp_err_t res = ir_tx_init(&ir_tx_config);
	if (res == ESP_OK) ESP_LOGI(TAG, "IRTx port is initialized");
	else
	{
		ESP_LOGE(TAG, "Cannot initialize IRTx port!");
		while(1) vTaskDelay(1000U / portTICK_RATE_MS);
	}
}

//-----------------------------------------------------------------------------

static void IRRxInit(void)
{
	ir_rx_config_t ir_rx_config =	{
	        							.io_num = IR_RX_IO_NUM,
										.buf_len = IR_RX_BUF_LEN
	    							};
	esp_err_t res = ir_rx_init(&ir_rx_config);
	if (res == ESP_OK) ESP_LOGI(TAG, "IRRx port is initialized");
	else
	{
		ESP_LOGE(TAG, "Cannot initialize IRRx port!");
		while(1) vTaskDelay(1000U / portTICK_RATE_MS);
	}
}
