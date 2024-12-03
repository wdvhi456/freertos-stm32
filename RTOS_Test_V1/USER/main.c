#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "lcd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//宏定义						
#define MAX_TASKS 10	//最大任务数
#define TASK1_PRIORITY 1	//任务优先级
#define TASK2_PRIORITY 2			
#define TASK3_PRIORITY 3			
#define TIME_SLICE_INTERVAL 1000	//单个时间片长度，单位ms

//任务控制块		
typedef struct{
	uint32_t ID;	
	uint32_t arrival_time;	//到达时间
	uint32_t execution_time;	//执行时间
	uint32_t period;	//周期
	uint32_t deadline;	//截止时间
	uint32_t clock_weight;	//时间片权重
	TaskHandle_t task_handle;	//句柄
}Task_Control_Block;
Task_Control_Block tasks[MAX_TASKS];	//设置最大TCB数量，即最大可调度任务数

//定义全局变量
uint32_t current_time = 0;	//当前时间，单位ms
uint32_t current_task_count;	//当前任务总数
QueueHandle_t task_Queue;	//调度队列

//任务声明
void TASK_1(void *pvParameters);	
void TASK_2(void *pvParameters);
void TASK_3(void *pvParameters);

//定义调度器
void Preemptive_Priority_Scheduler(void *pvParameters);	//抢占式优先级调度器
void EDF_Scheduler(void *pvParameters);	//EDF调度器
void Round_Robin_Scheduler(void *pvParameters);	//Round-Robin调度器	
void Weight_Round_Robin_Scheduler(void *pvParameters);	//Weight-Round-Robin调度器	

//定义调度器句柄			
TaskHandle_t Preemptive_Priority_Scheduler_Handle;						
TaskHandle_t EDF_Scheduler_Handle;
TaskHandle_t Round_Robin_Scheduler_Handle;						
TaskHandle_t Weight_Round_Robin_Scheduler_Handle;

//任务初始化
void Initialize_Preemptive_Priority_Tasks(void);	
void Initialize_EDF_Tasks(void);	
void Initialize_Round_Robin_Tasks(void);	
void Initialize_Weight_Round_Robin_Tasks(void);	

//每个时间片按照特定调度策略选择任务
Task_Control_Block* Get_Earliest_Deadline_Task(void);	//获取EDF任务
Task_Control_Block* Get_Current_Weight_Round_Robin_Task(void);	//获取W-R-R任务

void TASK_1(void *pvParameters)
{
	while(1)
	{
		printf("task1\r\n");
		vTaskDelay(TIME_SLICE_INTERVAL);
	}
}

void TASK_2(void *pvParameters)
{
	while(1)
	{
		printf("task2\r\n");
		vTaskDelay(TIME_SLICE_INTERVAL);
	}
}

void TASK_3(void *pvParameters)
{
	while(1)
	{
		printf("task3\r\n");
		vTaskDelay(TIME_SLICE_INTERVAL);
	}
}

//抢占式优先级调度器
void Preemptive_Priority_Scheduler(void *pvParameters){

}

//Round-Robin调度器	
void Round_Robin_Scheduler(void *pvParameters){

}



//获取拥有最早deadline的task
Task_Control_Block* Get_Earliest_Deadline_Task(void){
	int i;
	Task_Control_Block *earliest_task = NULL;
	uint32_t min_deadline = UINT32_MAX;
	
	//搜索当前周期已经到达且为执行完毕的，拥有最近deadline的任务
	for(i=0;i<current_task_count;i++){
		if(tasks[i].arrival_time <= current_time && tasks[i].deadline < min_deadline && tasks[i].execution_time != 0){
			min_deadline = tasks[i].deadline;
			earliest_task = &tasks[i];
		}
	}
	return earliest_task;
}

//EDF调度器
void EDF_Scheduler(void *pvParameters)
{
		int i;
		char str[10];
//		LED0=!LED0;
    while (1) {
			Task_Control_Block* task_to_execute = Get_Earliest_Deadline_Task();	//获取当前时间片拥有最近deadline的任务
			
			//测试用
			for(i=0;i<current_task_count;i++){	
				vTaskSuspend(tasks[i].task_handle);
			}
			printf("本轮调度任务：");
			if(task_to_execute == NULL){
				
			}

			LCD_ShowString(150,700,110,16,16,(u8*)"current time");
			LCD_ShowxNum(250,700,current_time/TIME_SLICE_INTERVAL,4,16,0x80);
			LCD_ShowString(300,700,110,16,16,(u8*)"current Task");
			LCD_ShowxNum(400,700,task_to_execute->ID,8,16,0x80);
			
			if (task_to_execute != NULL) {
				//抢占式EDF
				for(i=0;i<current_task_count;i++){	
					if(&tasks[i] == task_to_execute){
						vTaskResume(tasks[i].task_handle);
						break;
					}
				}
				//可视化调度
				POINT_COLOR = BLACK;
				for(i=0;i<current_task_count;i++){
					LCD_DrawRectangle(5+i*150,110,115+i*150,314); 	//画一个矩形	
					sprintf(str, "task%d", i+1);
					LCD_ShowString(40+i*150,350,60,16,16,(u8*)str);
					if(i+1 == task_to_execute->ID){
						LCD_Fill(6+i*150,111,114+i*150,313,RED); //填充区域
					}
					else{
						LCD_Fill(6+i*150,111,114+i*150,313,WHITE); //填充区域
					}
				}
				
				task_to_execute->execution_time -= TIME_SLICE_INTERVAL;	//当前任务执行时间减少一个时间片
			}
			else{	//如果当前周期内所有任务执行完毕，挂起所有任务
				for(i=0;i<current_task_count;i++){
					vTaskSuspend(tasks[i].task_handle);
				}
				vTaskSuspend(EDF_Scheduler_Handle);
			}
			
			// 当前时间片+1
			current_time+=TIME_SLICE_INTERVAL;
			vTaskDelay(TIME_SLICE_INTERVAL);//间隔TIME_SLICE_INTERVAL再执行
	}
}

////获取Weight-Round-Robin调度的当前任务：将先后到达的任务置入FIFO队列，按序运行
//Task_Control_Block* Get_Current_Weight_Round_Robin_Task(void){
//	int i;
//	Task_Control_Block *current_weight_round_robin_task = NULL;
//	uint32_t min_deadline = UINT32_MAX;
//	
//	//摘出FIFO队列的队首任务
//	for(i=0;i<current_task_count;i++){
//		if(tasks[i].arrival_time <= current_time && tasks[i].deadline < min_deadline && tasks[i].execution_time != 0){
//			min_deadline = tasks[i].deadline;
//			current_weight_round_robin_task = &tasks[i];
//		}
//	}

//	return current_weight_round_robin_task;
//}

//Weight-Round-Robin调度器
void Weight_Round_Robin_Scheduler(void *pvParameters){
	int i;
	while (1) {
		for(i=0;i<current_task_count;i++){	
			if(tasks[i].arrival_time >= current_time){
				xQueueSendToBack(task_Queue, &tasks[i].task_handle, portMAX_DELAY);
			}
		}
		
		// 当前时间片+1
		current_time+=TIME_SLICE_INTERVAL;
		vTaskDelay(TIME_SLICE_INTERVAL);//间隔TIME_SLICE_INTERVAL再执行
	}	
}


//定义任务控制块
void Initialize_Preemptive_Priority_Tasks(void){

}

void Initialize_EDF_Tasks(void) {
	current_task_count = 3;	//总任务数

	tasks[0].arrival_time = 0*TIME_SLICE_INTERVAL;
	tasks[0].execution_time = 3*TIME_SLICE_INTERVAL;
	tasks[0].period = 10*TIME_SLICE_INTERVAL;
	tasks[0].deadline = 10*TIME_SLICE_INTERVAL;
	tasks[0].ID = 1;
//		LCD_ShowString(350,100,110,16,16,(u8*)"task1");
//		LCD_ShowxNum(400,100,tasks[0].execution_time,8,16,0x80);

	tasks[1].arrival_time = 2*TIME_SLICE_INTERVAL;
	tasks[1].execution_time = 3*TIME_SLICE_INTERVAL;
	tasks[1].period = 10*TIME_SLICE_INTERVAL;
	tasks[1].deadline = 14*TIME_SLICE_INTERVAL;
	tasks[1].ID = 2;
//		LCD_ShowString(350,200,110,16,16,(u8*)"task2");
//		LCD_ShowxNum(400,200,tasks[0].execution_time,8,16,0x80);

	tasks[2].arrival_time = 4*TIME_SLICE_INTERVAL;
	tasks[2].execution_time = 2*TIME_SLICE_INTERVAL;
	tasks[2].period = 10*TIME_SLICE_INTERVAL;
	tasks[2].deadline = 12*TIME_SLICE_INTERVAL;
	tasks[2].ID = 3;
//		LCD_ShowString(350,300,110,16,16,(u8*)"task3");
//		LCD_ShowxNum(400,300,tasks[0].execution_time,8,16,0x80);
}


void Initialize_Round_Robin_Tasks(void){

}

void Initialize_Weight_Round_Robin_Tasks(void){
	current_task_count = 3;	//总任务数
	task_Queue = xQueueCreate(10, sizeof(int));	//初始化任务队列
	if (task_Queue == NULL) {
    printf("Queue creation failed!\r\n");
	}else{
		printf("Queue creation successed!\r\n");
	}
	tasks[0].arrival_time = 0*TIME_SLICE_INTERVAL;
	tasks[0].execution_time = 4*TIME_SLICE_INTERVAL;
	tasks[0].clock_weight = 1;
	tasks[0].ID = 1;

	tasks[1].arrival_time = 2*TIME_SLICE_INTERVAL;
	tasks[1].execution_time = 2*TIME_SLICE_INTERVAL;
	tasks[1].clock_weight = 2;
	tasks[1].ID = 2;

	tasks[2].arrival_time = 4*TIME_SLICE_INTERVAL;
	tasks[2].execution_time = 8*TIME_SLICE_INTERVAL;
	tasks[2].clock_weight = 3;
	tasks[2].ID = 3;
}


int main(void)
{
	//选择调度策略: 1-4分别为抢占式优先级调度，抢占式EDF调度，Round-Robin调度，Weight-Round-Robin调度
	int scheduling_id = 2;
	//单片机初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4	 
	delay_init();	    				//延时函数初始化	 
	uart_init(115200);					//初始化串口
	LED_Init();		  					//初始化LED
	LCD_Init();							//初始化LCD
	POINT_COLOR = RED;
	//the size of the font must be 12/16/24
	LCD_ShowString(30,10,300,48,24,"RTOS TEST");
	
	//选择调度策略
	if(scheduling_id == 2){	//EDF调度
		Initialize_EDF_Tasks();
		xTaskCreate(TASK_1, "Task1", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, &tasks[0].task_handle);
		xTaskCreate(TASK_2, "Task2", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &tasks[1].task_handle);
		xTaskCreate(TASK_3, "Task3", configMINIMAL_STACK_SIZE, NULL, TASK3_PRIORITY, &tasks[2].task_handle);
		xTaskCreate(EDF_Scheduler, "EDF_Scheduler", configMINIMAL_STACK_SIZE, NULL, UINT32_MAX, EDF_Scheduler_Handle);
	}else if(scheduling_id == 4){
		Initialize_Weight_Round_Robin_Tasks();
		xTaskCreate(TASK_1, "Task1", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, &tasks[0].task_handle);
		xTaskCreate(TASK_2, "Task2", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &tasks[1].task_handle);
		xTaskCreate(TASK_3, "Task3", configMINIMAL_STACK_SIZE, NULL, TASK3_PRIORITY, &tasks[2].task_handle);
		xTaskCreate(Weight_Round_Robin_Scheduler, "Weight_Round_Robin_Scheduler", configMINIMAL_STACK_SIZE, NULL, UINT32_MAX, Weight_Round_Robin_Scheduler_Handle);
	}
	
	//开启调度
	vTaskStartScheduler();
}

