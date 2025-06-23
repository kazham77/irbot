/*
 * led.c
 *
 *  Created on: 27 сент. 2024 г.
 *      Author: kazham
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "led_task.h"

static const char *TAG = "LEDTask";

void LEDTask(void* pvParameters)
{
	LEDInit();

	while(1)
	{
		gpio_set_level(LED_PIN, LED_ON);
		vTaskDelay(LED_ON_TIME_ms / portTICK_RATE_MS);
		gpio_set_level(LED_PIN, LED_OFF);
		vTaskDelay(LED_OFF_TIME_ms / portTICK_RATE_MS);
	}
}

//-----------------------------------------------------------------------------

void LEDInit(void)
{
	gpio_config_t io_conf;
	//disable interrupt
	io_conf.intr_type = GPIO_INTR_DISABLE;
	//set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	//bit mask of the pins that you want to set,e.g.GPIO15/16
	io_conf.pin_bit_mask = (1 << LED_PIN);
	//disable pull-down mode
	io_conf.pull_down_en = 0;
	//disable pull-up mode
	io_conf.pull_up_en = 0;
	//configure GPIO with the given settings
	gpio_config(&io_conf);

	ESP_LOGI(TAG, "LED GPIO is initialized");
}
