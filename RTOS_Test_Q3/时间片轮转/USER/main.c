#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include<stdio.h>
#include "timers.h"





#define TASK_PRIORITY 1
#define TASK_STACK_SIZE 256
#define TIME_SLICE_MS 5000
#define TIME_SLICE_MS2 1000
int task_numb=0;
int  change_priority=0;

// 任务句柄
TaskHandle_t taskHandles[2] = {NULL, NULL};
 
// 任务函数原型
void vTask1(void *pvParameters);
void vTask2(void *pvParameters);

// 时间片定时器回调函数
void vTimerCallback(TimerHandle_t xTimer) {
    // 交换两个任务的优先级，以实现时间片调度
	if(change_priority==0){
		change_priority=1;
    vTaskPrioritySet(taskHandles[0], TASK_PRIORITY + 1);
    vTaskPrioritySet(taskHandles[1], TASK_PRIORITY);
	  printf("change");}
	    else{
		change_priority=0;
    vTaskPrioritySet(taskHandles[0], TASK_PRIORITY);
    vTaskPrioritySet(taskHandles[1], TASK_PRIORITY+1);
	  printf("change");}
	
}


void vTimerMonitor(TimerHandle_t xTimer2) {
	  printf("%d",task_numb);
}
 
// 任务1函数
void vTask1(void *pvParameters) {
    const char *pcTaskName = (const char *)pvParameters;
 
    for (;;) {
        // 打印任务名
//        printf("%s is running\r\n", pcTaskName);
			   task_numb=1;
        
        // 延时一段时间，模拟任务的工作负载
//        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
 
// 任务2函数
void vTask2(void *pvParameters) {
    const char *pcTaskName = (const char *)pvParameters;
 
    for (;;) {
        // 打印任务名
//        printf("%s is running\r\n", pcTaskName);
			   task_numb=2;
 
        // 延时一段时间，模拟任务的工作负载
//        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
 
int main(void) {

	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4	 
	delay_init();	    				//延时函数初始化	  
	uart_init(115200);					//初始化串口
    // 创建任务
    xTaskCreate(vTask1, "Task 1", configMINIMAL_STACK_SIZE, "Task 1", TASK_PRIORITY, &taskHandles[0]);
    xTaskCreate(vTask2, "Task 2", configMINIMAL_STACK_SIZE, "Task 2", TASK_PRIORITY, &taskHandles[1]);
     // 创建一个定时器，用于周期性地切换任务优先级
    TimerHandle_t xTimer = xTimerCreate(
        "Timer", pdMS_TO_TICKS(TIME_SLICE_MS), pdTRUE, (void *)0, vTimerCallback);
	
	
	TimerHandle_t xTimer2 = xTimerCreate(
        "Timer2", pdMS_TO_TICKS(TIME_SLICE_MS2), pdTRUE, (void *)0, vTimerMonitor);
 
    // 启动定时器
    xTimerStart(xTimer, 0);
	  xTimerStart(xTimer2, 0);


    // 启动调度器
    vTaskStartScheduler();
 
    // 如果调度器启动失败，将不会到达这里
    for (;;);
}


