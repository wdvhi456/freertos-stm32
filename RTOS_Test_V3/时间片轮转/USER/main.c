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

// ������
TaskHandle_t taskHandles[2] = {NULL, NULL};
 
// ������ԭ��
void vTask1(void *pvParameters);
void vTask2(void *pvParameters);

// ʱ��Ƭ��ʱ���ص�����
void vTimerCallback(TimerHandle_t xTimer) {
    // ����������������ȼ�����ʵ��ʱ��Ƭ����
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
 
// ����1����
void vTask1(void *pvParameters) {
    const char *pcTaskName = (const char *)pvParameters;
 
    for (;;) {
        // ��ӡ������
//        printf("%s is running\r\n", pcTaskName);
			   task_numb=1;
        
        // ��ʱһ��ʱ�䣬ģ������Ĺ�������
//        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
 
// ����2����
void vTask2(void *pvParameters) {
    const char *pcTaskName = (const char *)pvParameters;
 
    for (;;) {
        // ��ӡ������
//        printf("%s is running\r\n", pcTaskName);
			   task_numb=2;
 
        // ��ʱһ��ʱ�䣬ģ������Ĺ�������
//        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
 
int main(void) {

	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4	 
	delay_init();	    				//��ʱ������ʼ��	  
	uart_init(115200);					//��ʼ������
    // ��������
    xTaskCreate(vTask1, "Task 1", configMINIMAL_STACK_SIZE, "Task 1", TASK_PRIORITY, &taskHandles[0]);
    xTaskCreate(vTask2, "Task 2", configMINIMAL_STACK_SIZE, "Task 2", TASK_PRIORITY, &taskHandles[1]);
     // ����һ����ʱ�������������Ե��л��������ȼ�
    TimerHandle_t xTimer = xTimerCreate(
        "Timer", pdMS_TO_TICKS(TIME_SLICE_MS), pdTRUE, (void *)0, vTimerCallback);
	
	
	TimerHandle_t xTimer2 = xTimerCreate(
        "Timer2", pdMS_TO_TICKS(TIME_SLICE_MS2), pdTRUE, (void *)0, vTimerMonitor);
 
    // ������ʱ��
    xTimerStart(xTimer, 0);
	  xTimerStart(xTimer2, 0);


    // ����������
    vTaskStartScheduler();
 
    // �������������ʧ�ܣ������ᵽ������
    for (;;);
}


