#ifndef __IRRX_TASK_H_
#define __IRRX_TASK_H_

#define IR_TASK_STACK_SIZE_WORDS		(2048U)

#define IR_RX_IO_NUM					GPIO_NUM_13	// On the original board was GPIO_NUM_14
#define IR_TX_IO_NUM					GPIO_NUM_14	// Or GPIO_NUM_2. On the original board was GPIO_NUM_4

#define IR_RX_BUF_LEN 					(128U)

void IRTask(void* pvParameters);

#endif // __IRRX_TASK_H_
