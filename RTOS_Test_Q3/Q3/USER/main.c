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
#define TASK_END -1	//任务执行完毕后发送结束信号给调度器

//任务控制块		
typedef struct{
	uint32_t ID;	
	uint32_t arrival_time;	//到达时间
	uint32_t execution_time;	//执行时间
	uint32_t period;	//周期
	uint32_t deadline;	//截止时间
	uint32_t softdeadline;	//软截止时间
	uint32_t weight;	//时间片权重
	TaskHandle_t task_handle;	//句柄
}Task_Control_Block;

Task_Control_Block tasks[MAX_TASKS];	//设置最大TCB数量，即最大可调度任务数

//定义全局变量
uint32_t current_time = 0;	//当前时间，单位ms
uint32_t current_task_count;	//当前任务总数
QueueHandle_t task_Queue;	//调度队列
int arrived_tasks[MAX_TASKS];	//已到达任务标记
int scheduling_id = 1;	//选择调度策略

//任务声明
void TASK_1(void *pvParameters);	
void TASK_2(void *pvParameters);
void TASK_3(void *pvParameters);

//定义调度器
void Preemptive_Priority_Scheduler(void *pvParameters);	//抢占式优先级调度器
void EDF_Scheduler(void *pvParameters);	//EDF调度器
void Round_Robin_Scheduler(void *pvParameters);	//Round-Robin调度器	
void Weighted_Round_Robin_Scheduler(void *pvParameters);	//Weight-Round-Robin调度器	

//定义调度器句柄			
TaskHandle_t Preemptive_Priority_Scheduler_Handle;						
TaskHandle_t EDF_Scheduler_Handle;
TaskHandle_t Round_Robin_Scheduler_Handle;						
TaskHandle_t Weighted_Round_Robin_Scheduler_Handle;

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
	int task_index = 0;
	int weight = tasks[task_index].weight;
	int endFlag = TASK_END;
	int softdeadline_timeout=0;
	while(1)
	{
		if(scheduling_id == 1){
			tasks[task_index].execution_time -= TIME_SLICE_INTERVAL;
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);
			current_time += TIME_SLICE_INTERVAL;
			if((current_time/TIME_SLICE_INTERVAL)>tasks[0].softdeadline&&tasks[task_index].execution_time == 0){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[0].softdeadline;
				printf("T%d task%d is end. softdeadline_timeout=%d\r\n", current_time/TIME_SLICE_INTERVAL, 1,softdeadline_timeout);
					}
					
		}
		if(scheduling_id == 2){
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);
			if((current_time/TIME_SLICE_INTERVAL)>tasks[0].softdeadline&&tasks[task_index].execution_time == 0){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[0].softdeadline;
				printf("T%d task%d is end. softdeadline_timeout=%d\r\n", current_time/TIME_SLICE_INTERVAL, 1,softdeadline_timeout);
					}
					
		}
		if(scheduling_id == 3 || scheduling_id == 4){
			//执行完毕后剩余的时间片空转
			if(tasks[task_index].execution_time != 0){
				tasks[task_index].execution_time -= TIME_SLICE_INTERVAL;
				printf("T%d: task%d, remains %d \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1, tasks[task_index].execution_time);
				weight--;
				if(weight == 0 && tasks[task_index].execution_time != 0){
					if(xQueueSendToBack(task_Queue, &task_index, portMAX_DELAY) == pdPASS){
						printf("task%d in again. \r\n", task_index + 1);
					}else{
						printf("Failed to enqueue task. \r\n");
					}
					weight = tasks[task_index].weight;
				}else if(weight == 0 && tasks[task_index].execution_time == 0){
					if((current_time/TIME_SLICE_INTERVAL)>tasks[0].softdeadline){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[0].softdeadline;
					}
					printf("T%d task%d is end. softdeadline_timeout=%d\r\n", current_time/TIME_SLICE_INTERVAL, 1,softdeadline_timeout);
					xQueueSendToBack(task_Queue, &endFlag, portMAX_DELAY);
				}
			}else{
				if((current_time/TIME_SLICE_INTERVAL)>tasks[0].softdeadline){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[0].softdeadline;
					}
				//vTaskDelete(tasks[0].task_handle);
				printf("T%d task%d is end. softdeadline_timeout=%d \r\n", current_time/TIME_SLICE_INTERVAL, 1,softdeadline_timeout);
				xQueueSendToBack(task_Queue, &endFlag, portMAX_DELAY);
				//break;
			}
			current_time += TIME_SLICE_INTERVAL;
		}
		vTaskDelay(TIME_SLICE_INTERVAL);
	}
}

void TASK_2(void *pvParameters)
{
	int task_index = 1;
	int weight = tasks[task_index].weight;
	int endFlag = TASK_END;
	int softdeadline_timeout=0;
	while(1)
	{
		if(scheduling_id == 1){
			tasks[task_index].execution_time -= TIME_SLICE_INTERVAL;
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);

			current_time += TIME_SLICE_INTERVAL;
			if(tasks[task_index].execution_time <= 0){
			   if((current_time/TIME_SLICE_INTERVAL)>tasks[1].softdeadline&&tasks[task_index].execution_time == 0){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[1].softdeadline;
					 printf("T%d task%d is end. softdeadline_timeout=%d\r\n", current_time/TIME_SLICE_INTERVAL, 2,softdeadline_timeout);
					}
				
			}
		}
		if(scheduling_id == 2){
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);
			if(tasks[task_index].execution_time <= 0){
			   if((current_time/TIME_SLICE_INTERVAL)>tasks[1].softdeadline&&tasks[task_index].execution_time == 0){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[1].softdeadline;
					 printf("T%d task%d is end. softdeadline_timeout=%d\r\n", current_time/TIME_SLICE_INTERVAL, 2,softdeadline_timeout);
					}
					
			}
		}
		if(scheduling_id == 3 || scheduling_id == 4){
			//执行完毕后剩余的时间片空转
			if(tasks[task_index].execution_time != 0){
				tasks[task_index].execution_time -= TIME_SLICE_INTERVAL;
				printf("T%d: task%d, remains %d \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1, tasks[task_index].execution_time);
				weight--;
				if(weight == 0 && tasks[task_index].execution_time != 0){
					if(xQueueSendToBack(task_Queue, &task_index, portMAX_DELAY) == pdPASS){
						printf("task%d in again. \r\n", task_index + 1);
					}else{
						printf("Failed to enqueue task. \r\n");
					}
					weight = tasks[task_index].weight;
				}else if(weight == 0 && tasks[task_index].execution_time == 0){
					if((current_time/TIME_SLICE_INTERVAL)>tasks[1].softdeadline){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[1].softdeadline;
					}
					printf("T%d task%d is end. softdeadline_timeout=%d\r\n", current_time/TIME_SLICE_INTERVAL, 2,softdeadline_timeout);
					xQueueSendToBack(task_Queue, &endFlag, portMAX_DELAY);
				}
			}else{
				//vTaskDelete(tasks[0].task_handle);
				if((current_time/TIME_SLICE_INTERVAL)>tasks[1].softdeadline){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[1].softdeadline;
					}
				printf("T%d task%d is end. softdeadline_timeout=%d\r\n", current_time/TIME_SLICE_INTERVAL, 2,softdeadline_timeout);
				xQueueSendToBack(task_Queue, &endFlag, portMAX_DELAY);
				//break;
			}
			current_time += TIME_SLICE_INTERVAL;
		}
		vTaskDelay(TIME_SLICE_INTERVAL);
	}
}

void TASK_3(void *pvParameters)
{
	int task_index = 2;
	int weight = tasks[task_index].weight;
	int endFlag = TASK_END;
	int softdeadline_timeout=0;
	while(1)
	{
		if(scheduling_id == 1){
			tasks[task_index].execution_time -= TIME_SLICE_INTERVAL;
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);
			current_time += TIME_SLICE_INTERVAL;
			if((current_time/TIME_SLICE_INTERVAL)>tasks[2].softdeadline&&tasks[task_index].execution_time == 0){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[2].softdeadline;
				printf("T%d task%d is end. softdeadline_timeout=%d\r\n", current_time/TIME_SLICE_INTERVAL, 3,softdeadline_timeout);
					}
					
				
		}
		if(scheduling_id == 2){
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);
			if((current_time/TIME_SLICE_INTERVAL)>tasks[2].softdeadline&&tasks[task_index].execution_time == 0){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[2].softdeadline;
				printf("T%d task%d is end. softdeadline_timeout=%d\r\n", current_time/TIME_SLICE_INTERVAL, 3,softdeadline_timeout);
					}
					
				
		}
		if(scheduling_id == 3 || scheduling_id == 4){
			//执行完毕后剩余的时间片空转
			if(tasks[task_index].execution_time != 0){
				tasks[task_index].execution_time -= TIME_SLICE_INTERVAL;
				printf("T%d: task%d, remains %d \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1, tasks[task_index].execution_time);
				weight--;
				if(weight == 0 && tasks[task_index].execution_time != 0){
					if(xQueueSendToBack(task_Queue, &task_index, portMAX_DELAY) == pdPASS){
						printf("task%d in again. \r\n", task_index + 1);
					}else{
						printf("Failed to enqueue task. \r\n");
					}
					weight = tasks[task_index].weight;
				}else if(weight == 0 && tasks[task_index].execution_time == 0){
					if((current_time/TIME_SLICE_INTERVAL)>tasks[2].softdeadline){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[2].softdeadline;
					}
					printf("T%d task%d is end. softdeadline_timeout=%d\r\n", current_time/TIME_SLICE_INTERVAL, 3,softdeadline_timeout);
					xQueueSendToBack(task_Queue, &endFlag, portMAX_DELAY);
				}
			}else{
				//vTaskDelete(tasks[0].task_handle);
				if((current_time/TIME_SLICE_INTERVAL)>tasks[2].softdeadline){
						softdeadline_timeout=current_time/TIME_SLICE_INTERVAL-tasks[2].softdeadline;
					}
				printf("T%d task%d is end. softdeadline_timeout=%d\r\n", current_time/TIME_SLICE_INTERVAL, 3,softdeadline_timeout);
				xQueueSendToBack(task_Queue, &endFlag, portMAX_DELAY);
				//break;
			}
			current_time += TIME_SLICE_INTERVAL;
		}
		vTaskDelay(TIME_SLICE_INTERVAL);
	}
}

//抢占式优先级调度器
void Preemptive_Priority_Scheduler(void *pvParameters){
	while(1){
		
	}
}

//Round-Robin调度器	
void Round_Robin_Scheduler(void *pvParameters){
	while(1){
			
	}
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
//		LED0=!LED0;
    while (1) {
			Task_Control_Block* task_to_execute = Get_Earliest_Deadline_Task();	//获取当前时间片拥有最近deadline的任务
			
			//每个时间片开始前挂起所有任务
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

//Weight-Round-Robin调度器
void Weighted_Round_Robin_Scheduler(void *pvParameters){
	int i;
	int task_index;	//任务在数组中的序号
	int front_task_index;
	int task_weight;
	while (1) {
		LCD_ShowString(150,700,110,16,16,(u8*)"current time");
		LCD_ShowxNum(250,700,current_time/TIME_SLICE_INTERVAL,4,16,0x80);
		//每个时间片开始前挂起所有任务
		for(i=0;i<current_task_count;i++){	
			vTaskSuspend(tasks[i].task_handle);
		}
		//将已到达的任务入队,每个任务仅进行一次操作
		for(i=0;i<current_task_count;i++){	
//			printf("a:%d at:%d\r\n",arrived_tasks[i],tasks[i].arrival_time);
			if(arrived_tasks[i] == 0 && tasks[i].arrival_time >= current_time){
				task_index = i;
				if(xQueueSendToBack(task_Queue, &task_index, portMAX_DELAY) == pdPASS){
					arrived_tasks[i] = 1;
					printf("task%d in. \r\n", tasks[i].ID);
//					printf("task%u's weight: %u\r\n", tasks[i].ID, tasks[i].weight);
//					if(i == 1){
//						vTaskResume(tasks[i].task_handle);
//					}
				}else{
					printf("Failed to enqueue task. \r\n");
				}
			}
		}
		
		if(uxQueueMessagesWaiting(task_Queue) != 0){
			if(xQueueReceive(task_Queue, &front_task_index, portMAX_DELAY) == pdPASS){	//读取队首句柄
				if(front_task_index != -1){
					printf("task%u gets %u time slices\r\n", tasks[front_task_index].ID, tasks[front_task_index].weight);
					vTaskResume(tasks[front_task_index].task_handle);
					//判断是否带权
					task_weight = tasks[front_task_index].weight;
					//让出时间片
					vTaskDelay(task_weight * TIME_SLICE_INTERVAL);
				}else{
					current_time += TIME_SLICE_INTERVAL;
					vTaskDelay(TIME_SLICE_INTERVAL);	
				}
			}else{
				printf("Failed to dequeue task. \r\n");
			}
		}else{
			//队列中没任务则等待一个时间片
			current_time += TIME_SLICE_INTERVAL;
			printf("当前时间片%d \r\n", current_time/TIME_SLICE_INTERVAL);
			vTaskDelay(TIME_SLICE_INTERVAL);	
		}
	}	
}


//定义任务控制块

void Initialize_softdeadline_Tasks(void) {
		int i;
	current_task_count = 3;	//总任务数
	for(i=0;i<current_task_count;i++){	//初始化任务标记
		arrived_tasks[i] = 0;
	}

	tasks[0].arrival_time = 0*TIME_SLICE_INTERVAL;
	tasks[0].execution_time = 10*TIME_SLICE_INTERVAL;
	tasks[0].period = 10*TIME_SLICE_INTERVAL;
	tasks[0].deadline = 37*TIME_SLICE_INTERVAL;
	tasks[0].ID = 1;
	tasks[0].weight=1;
	tasks[0].softdeadline=15;
	
//		LCD_ShowString(350,100,110,16,16,(u8*)"task1");
//		LCD_ShowxNum(400,100,tasks[0].execution_time,8,16,0x80);

	tasks[1].arrival_time = 0*TIME_SLICE_INTERVAL;
	tasks[1].execution_time = 10*TIME_SLICE_INTERVAL;
	tasks[1].period = 10*TIME_SLICE_INTERVAL;
	tasks[1].deadline = 35*TIME_SLICE_INTERVAL;
	tasks[1].ID = 2;
	tasks[1].weight=1;
	tasks[1].softdeadline=28;
//		LCD_ShowString(350,200,110,16,16,(u8*)"task2");
//		LCD_ShowxNum(400,200,tasks[0].execution_time,8,16,0x80);

	tasks[2].arrival_time = 10*TIME_SLICE_INTERVAL;
	tasks[2].execution_time = 10*TIME_SLICE_INTERVAL;
	tasks[2].period = 10*TIME_SLICE_INTERVAL;
	tasks[2].deadline = 40*TIME_SLICE_INTERVAL;
	tasks[2].ID = 3;
	tasks[2].weight=1;
	tasks[2].softdeadline=25;
//		LCD_ShowString(350,300,110,16,16,(u8*)"task3");
//		LCD_ShowxNum(400,300,tasks[0].execution_time,8,16,0x80);
}




int main(void)
{
	
	//单片机初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4	 
	delay_init();	    				//延时函数初始化	 
	uart_init(115200);					//初始化串口
	LED_Init();		  					//初始化LED
	LCD_Init();							//初始化LCD
	POINT_COLOR = RED;
	//the size of the font must be 12/16/24
	LCD_ShowString(30,10,300,48,24,"RTOS TEST");
	
	//选择调度策略: 1-5分别为抢占式优先级调度，抢占式EDF调度，Round-Robin调度，Weight-Round-Robin调度，MLFQ调度
	scheduling_id = 3;
	
	xTaskCreate(TASK_1, "Task1", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, &tasks[0].task_handle);
	xTaskCreate(TASK_2, "Task2", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &tasks[1].task_handle);
	xTaskCreate(TASK_3, "Task3", configMINIMAL_STACK_SIZE, NULL, TASK3_PRIORITY, &tasks[2].task_handle);
	//选择调度策略
	if(scheduling_id == 1){	//抢占式调度（默认）
		Initialize_softdeadline_Tasks();
		xTaskCreate(Preemptive_Priority_Scheduler, "Preemptive_Priority_Scheduler", configMINIMAL_STACK_SIZE, NULL, 0, Preemptive_Priority_Scheduler_Handle);
	}else if(scheduling_id == 2){	//EDF调度
		Initialize_softdeadline_Tasks();
		xTaskCreate(EDF_Scheduler, "EDF_Scheduler", configMINIMAL_STACK_SIZE, NULL, UINT32_MAX, EDF_Scheduler_Handle);
	}else if(scheduling_id == 3){	//时间片轮转
		Initialize_softdeadline_Tasks();
		task_Queue = xQueueCreate(MAX_TASKS, sizeof(int));	
		if (task_Queue == NULL) {
			printf("Queue creation failed!\r\n");
		}else{
			printf("Queue creation successed!\r\n");
		}
		xTaskCreate(Weighted_Round_Robin_Scheduler, "Weighted_Round_Robin_Scheduler", configMINIMAL_STACK_SIZE, NULL, UINT32_MAX, Weighted_Round_Robin_Scheduler_Handle);
	}else if(scheduling_id == 4){	//带权时间片轮转
		Initialize_softdeadline_Tasks();
		task_Queue = xQueueCreate(MAX_TASKS, sizeof(int));	//初始化调度队列
		if (task_Queue == NULL) {
			printf("Queue creation failed!\r\n");
		}else{
			printf("Queue creation successed!\r\n");
		}
		xTaskCreate(Weighted_Round_Robin_Scheduler, "Weighted_Round_Robin_Scheduler", configMINIMAL_STACK_SIZE, NULL, UINT32_MAX, Weighted_Round_Robin_Scheduler_Handle);
	}
	
	//开启调度
	vTaskStartScheduler();
}

