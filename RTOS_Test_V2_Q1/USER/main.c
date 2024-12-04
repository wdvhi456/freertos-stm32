#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "lcd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//�궨��						
#define MAX_TASKS 10	//���������
#define TASK1_PRIORITY 1	//�������ȼ�
#define TASK2_PRIORITY 2			
#define TASK3_PRIORITY 3			
#define TIME_SLICE_INTERVAL 1000	//����ʱ��Ƭ���ȣ���λms
#define TASK_END -1	//����ִ����Ϻ��ͽ����źŸ�������

//������ƿ�		
typedef struct{
	uint32_t ID;	
	uint32_t arrival_time;	//����ʱ��
	uint32_t execution_time;	//ִ��ʱ��
	uint32_t period;	//����
	uint32_t deadline;	//��ֹʱ��
	uint32_t weight;	//ʱ��ƬȨ��
	TaskHandle_t task_handle;	//���
}Task_Control_Block;

Task_Control_Block tasks[MAX_TASKS];	//�������TCB�����������ɵ���������

//����ȫ�ֱ���
uint32_t current_time = 0;	//��ǰʱ�䣬��λms
uint32_t current_task_count;	//��ǰ��������
QueueHandle_t task_Queue;	//���ȶ���
int arrived_tasks[MAX_TASKS];	//�ѵ���������
int scheduling_id = 1;	//ѡ����Ȳ���

//��������
void TASK_1(void *pvParameters);	
void TASK_2(void *pvParameters);
void TASK_3(void *pvParameters);

//���������
void Preemptive_Priority_Scheduler(void *pvParameters);	//��ռʽ���ȼ�������
void EDF_Scheduler(void *pvParameters);	//EDF������
void Round_Robin_Scheduler(void *pvParameters);	//Round-Robin������	
void Weighted_Round_Robin_Scheduler(void *pvParameters);	//Weight-Round-Robin������	

//������������			
TaskHandle_t Preemptive_Priority_Scheduler_Handle;						
TaskHandle_t EDF_Scheduler_Handle;
TaskHandle_t Round_Robin_Scheduler_Handle;						
TaskHandle_t Weighted_Round_Robin_Scheduler_Handle;

//�����ʼ��
void Initialize_Preemptive_Priority_Tasks(void);	
void Initialize_EDF_Tasks(void);	
void Initialize_Round_Robin_Tasks(void);	
void Initialize_Weight_Round_Robin_Tasks(void);	

//ÿ��ʱ��Ƭ�����ض����Ȳ���ѡ������
Task_Control_Block* Get_Earliest_Deadline_Task(void);	//��ȡEDF����
Task_Control_Block* Get_Current_Weight_Round_Robin_Task(void);	//��ȡW-R-R����

void TASK_1(void *pvParameters)
{
	int task_index = 0;
	int weight = tasks[task_index].weight;
	int endFlag = TASK_END;
	while(1)
	{
		if(scheduling_id == 1){
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);
			current_time += TIME_SLICE_INTERVAL;
		}
		if(scheduling_id == 2){
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);
		}
		if(scheduling_id == 3 || scheduling_id == 4){
			//ִ����Ϻ�ʣ���ʱ��Ƭ��ת
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
					printf("T%d task%d is end. \r\n", current_time/TIME_SLICE_INTERVAL, 1);
					xQueueSendToBack(task_Queue, &endFlag, portMAX_DELAY);
				}
			}else{
				//vTaskDelete(tasks[0].task_handle);
				printf("T%d task%d is end. \r\n", current_time/TIME_SLICE_INTERVAL, 1);
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
	while(1)
	{
		if(scheduling_id == 1){
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);
			current_time += TIME_SLICE_INTERVAL;
		}
		if(scheduling_id == 2){
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);
		}
		if(scheduling_id == 3 || scheduling_id == 4){
			//ִ����Ϻ�ʣ���ʱ��Ƭ��ת
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
					printf("T%d task%d is end. \r\n", current_time/TIME_SLICE_INTERVAL, 1);
					xQueueSendToBack(task_Queue, &endFlag, portMAX_DELAY);
				}
			}else{
				//vTaskDelete(tasks[0].task_handle);
				printf("T%d task%d is end. \r\n", current_time/TIME_SLICE_INTERVAL, 2);
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
	while(1)
	{
		if(scheduling_id == 1){
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);
			current_time += TIME_SLICE_INTERVAL;
		}
		if(scheduling_id == 2){
			printf("T%d: task%d  \r\n", current_time/TIME_SLICE_INTERVAL, task_index+1);
		}
		if(scheduling_id == 3 || scheduling_id == 4){
			//ִ����Ϻ�ʣ���ʱ��Ƭ��ת
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
					printf("T%d task%d is end. \r\n", current_time/TIME_SLICE_INTERVAL, 1);
					xQueueSendToBack(task_Queue, &endFlag, portMAX_DELAY);
				}
			}else{
				//vTaskDelete(tasks[0].task_handle);
				printf("T%d task%d is end. \r\n", current_time/TIME_SLICE_INTERVAL, 3);
				xQueueSendToBack(task_Queue, &endFlag, portMAX_DELAY);
				//break;
			}
			current_time += TIME_SLICE_INTERVAL;
		}
		vTaskDelay(TIME_SLICE_INTERVAL);
	}
}

//��ռʽ���ȼ�������
void Preemptive_Priority_Scheduler(void *pvParameters){
	while(1){
		
	}
}

//Round-Robin������	
void Round_Robin_Scheduler(void *pvParameters){
	while(1){
			
	}
}


//��ȡӵ������deadline��task
Task_Control_Block* Get_Earliest_Deadline_Task(void){
	int i;
	Task_Control_Block *earliest_task = NULL;
	uint32_t min_deadline = UINT32_MAX;
	
	//������ǰ�����Ѿ�������Ϊִ����ϵģ�ӵ�����deadline������
	for(i=0;i<current_task_count;i++){
		if(tasks[i].arrival_time <= current_time && tasks[i].deadline < min_deadline && tasks[i].execution_time != 0){
			min_deadline = tasks[i].deadline;
			earliest_task = &tasks[i];
		}
	}
	return earliest_task;
}

//EDF������
void EDF_Scheduler(void *pvParameters)
{
		int i;
		char str[10];
//		LED0=!LED0;
    while (1) {
			Task_Control_Block* task_to_execute = Get_Earliest_Deadline_Task();	//��ȡ��ǰʱ��Ƭӵ�����deadline������
			
			//ÿ��ʱ��Ƭ��ʼǰ������������
			for(i=0;i<current_task_count;i++){	
				vTaskSuspend(tasks[i].task_handle);
			}
			
			printf("���ֵ�������");
			if(task_to_execute == NULL){
				
			}

			LCD_ShowString(150,700,110,16,16,(u8*)"current time");
			LCD_ShowxNum(250,700,current_time/TIME_SLICE_INTERVAL,4,16,0x80);
			LCD_ShowString(300,700,110,16,16,(u8*)"current Task");
			LCD_ShowxNum(400,700,task_to_execute->ID,8,16,0x80);
			
			if (task_to_execute != NULL) {
				//��ռʽEDF
				for(i=0;i<current_task_count;i++){	
					if(&tasks[i] == task_to_execute){
						vTaskResume(tasks[i].task_handle);
						break;
					}
				}
				//���ӻ�����
				POINT_COLOR = BLACK;
				for(i=0;i<current_task_count;i++){
					LCD_DrawRectangle(5+i*150,110,115+i*150,314); 	//��һ������	
					sprintf(str, "task%d", i+1);
					LCD_ShowString(40+i*150,350,60,16,16,(u8*)str);
					if(i+1 == task_to_execute->ID){
						LCD_Fill(6+i*150,111,114+i*150,313,RED); //�������
					}
					else{
						LCD_Fill(6+i*150,111,114+i*150,313,WHITE); //�������
					}
				}
				
				task_to_execute->execution_time -= TIME_SLICE_INTERVAL;	//��ǰ����ִ��ʱ�����һ��ʱ��Ƭ
			}
			else{	//�����ǰ��������������ִ����ϣ�������������
				for(i=0;i<current_task_count;i++){
					vTaskSuspend(tasks[i].task_handle);
				}
				vTaskSuspend(EDF_Scheduler_Handle);
			}
			
			// ��ǰʱ��Ƭ+1
			current_time+=TIME_SLICE_INTERVAL;
			vTaskDelay(TIME_SLICE_INTERVAL);//���TIME_SLICE_INTERVAL��ִ��
	}
}

//Weight-Round-Robin������
void Weighted_Round_Robin_Scheduler(void *pvParameters){
	int i;
	int task_index;	//�����������е����
	int front_task_index;
	int task_weight;
	while (1) {
		LCD_ShowString(150,700,110,16,16,(u8*)"current time");
		LCD_ShowxNum(250,700,current_time/TIME_SLICE_INTERVAL,4,16,0x80);
		//ÿ��ʱ��Ƭ��ʼǰ������������
		for(i=0;i<current_task_count;i++){	
			vTaskSuspend(tasks[i].task_handle);
		}
		//���ѵ�����������,ÿ�����������һ�β���
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
			if(xQueueReceive(task_Queue, &front_task_index, portMAX_DELAY) == pdPASS){	//��ȡ���׾��
				if(front_task_index != -1){
					printf("task%u gets %u time slices\r\n", tasks[front_task_index].ID, tasks[front_task_index].weight);
					vTaskResume(tasks[front_task_index].task_handle);
					//�ж��Ƿ��Ȩ
					task_weight = tasks[front_task_index].weight;
					//�ó�ʱ��Ƭ
					vTaskDelay(task_weight * TIME_SLICE_INTERVAL);
				}else{
					current_time += TIME_SLICE_INTERVAL;
					vTaskDelay(TIME_SLICE_INTERVAL);	
				}
			}else{
				printf("Failed to dequeue task. \r\n");
			}
		}else{
			//������û������ȴ�һ��ʱ��Ƭ
			current_time += TIME_SLICE_INTERVAL;
			printf("��ǰʱ��Ƭ%d \r\n", current_time/TIME_SLICE_INTERVAL);
			vTaskDelay(TIME_SLICE_INTERVAL);	
		}
	}	
}


//����������ƿ�
void Initialize_Preemptive_Priority_Tasks(void){

}

void Initialize_EDF_Tasks(void) {
	current_task_count = 3;	//��������

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
	int i;
	current_task_count = 3;	//��������
	for(i=0;i<current_task_count;i++){	//��ʼ��������
		arrived_tasks[i] = 0;
	}
	
	tasks[0].arrival_time = 0*TIME_SLICE_INTERVAL;
	tasks[0].execution_time = 4*TIME_SLICE_INTERVAL;
	tasks[0].weight = 1;
	tasks[0].ID = 1;

	tasks[1].arrival_time = 0*TIME_SLICE_INTERVAL;
	tasks[1].execution_time = 5*TIME_SLICE_INTERVAL;
	tasks[1].weight = 1;
	tasks[1].ID = 2;

	tasks[2].arrival_time = 0*TIME_SLICE_INTERVAL;
	tasks[2].execution_time = 8*TIME_SLICE_INTERVAL;
	tasks[2].weight = 1;
	tasks[2].ID = 3;
}

void Initialize_Weight_Round_Robin_Tasks(void){
	int i;
	current_task_count = 3;	//��������
	for(i=0;i<current_task_count;i++){	//��ʼ��������
		arrived_tasks[i] = 0;
	}
	
	tasks[0].arrival_time = 0*TIME_SLICE_INTERVAL;
	tasks[0].execution_time = 4*TIME_SLICE_INTERVAL;
	tasks[0].weight = 1;
	tasks[0].ID = 1;

	tasks[1].arrival_time = 0*TIME_SLICE_INTERVAL;
	tasks[1].execution_time = 5*TIME_SLICE_INTERVAL;
	tasks[1].weight = 2;
	tasks[1].ID = 2;

	tasks[2].arrival_time = 0*TIME_SLICE_INTERVAL;
	tasks[2].execution_time = 8*TIME_SLICE_INTERVAL;
	tasks[2].weight = 3;
	tasks[2].ID = 3;
}


int main(void)
{
	
	//��Ƭ����ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4	 
	delay_init();	    				//��ʱ������ʼ��	 
	uart_init(115200);					//��ʼ������
	LED_Init();		  					//��ʼ��LED
	LCD_Init();							//��ʼ��LCD
	POINT_COLOR = RED;
	//the size of the font must be 12/16/24
	LCD_ShowString(30,10,300,48,24,"RTOS TEST");
	
	//ѡ����Ȳ���: 1-5�ֱ�Ϊ��ռʽ���ȼ����ȣ���ռʽEDF���ȣ�Round-Robin���ȣ�Weight-Round-Robin���ȣ�MLFQ����
	scheduling_id = 3;
	
	xTaskCreate(TASK_1, "Task1", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, &tasks[0].task_handle);
	xTaskCreate(TASK_2, "Task2", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &tasks[1].task_handle);
	xTaskCreate(TASK_3, "Task3", configMINIMAL_STACK_SIZE, NULL, TASK3_PRIORITY, &tasks[2].task_handle);
	//ѡ����Ȳ���
	if(scheduling_id == 1){	//��ռʽ���ȣ�Ĭ�ϣ�
		Initialize_EDF_Tasks();
		xTaskCreate(Preemptive_Priority_Scheduler, "Preemptive_Priority_Scheduler", configMINIMAL_STACK_SIZE, NULL, 0, Preemptive_Priority_Scheduler_Handle);
	}else if(scheduling_id == 2){	//EDF����
		Initialize_EDF_Tasks();
		xTaskCreate(EDF_Scheduler, "EDF_Scheduler", configMINIMAL_STACK_SIZE, NULL, UINT32_MAX, EDF_Scheduler_Handle);
	}else if(scheduling_id == 3){	//ʱ��Ƭ��ת
		Initialize_Round_Robin_Tasks();
		task_Queue = xQueueCreate(MAX_TASKS, sizeof(int));	
		if (task_Queue == NULL) {
			printf("Queue creation failed!\r\n");
		}else{
			printf("Queue creation successed!\r\n");
		}
		xTaskCreate(Weighted_Round_Robin_Scheduler, "Weighted_Round_Robin_Scheduler", configMINIMAL_STACK_SIZE, NULL, UINT32_MAX, Weighted_Round_Robin_Scheduler_Handle);
	}else if(scheduling_id == 4){	//��Ȩʱ��Ƭ��ת
		Initialize_Weight_Round_Robin_Tasks();
		task_Queue = xQueueCreate(MAX_TASKS, sizeof(int));	//��ʼ�����ȶ���
		if (task_Queue == NULL) {
			printf("Queue creation failed!\r\n");
		}else{
			printf("Queue creation successed!\r\n");
		}
		xTaskCreate(Weighted_Round_Robin_Scheduler, "Weighted_Round_Robin_Scheduler", configMINIMAL_STACK_SIZE, NULL, UINT32_MAX, Weighted_Round_Robin_Scheduler_Handle);
	}
	
	//��������
	vTaskStartScheduler();
}
