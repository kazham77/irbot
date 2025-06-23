/*
 * led_task.h
 *
 *  Created on: 27 сент. 2024 г.
 *      Author: kazham
 */

#ifndef MAIN_LED_TASK_H_
#define MAIN_LED_TASK_H_

#define LED_TASK_STACK_SIZE_WORDS		1500

#define LED_PIN							GPIO_NUM_2
#define LED_OFF							1U
#define LED_ON							0U

#define LED_ON_TIME_ms					50U
#define LED_OFF_TIME_ms					(60U * 1000U)	// 60 sec

void LEDInit(void);
void LEDTask(void* pvParameters);

#endif /* MAIN_LED_TASK_H_ */
