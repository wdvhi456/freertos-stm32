#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "lcd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//task1 defination
#define TASK1_TASK_PRIO		10
#define TASK1_STK_SIZE 		128  
TaskHandle_t Task1Task_Handler;
void task1_task(void *pvParameters);

//task2 defination
#define TASK2_TASK_PRIO		3
#define TASK2_STK_SIZE 		128  
TaskHandle_t Task2Task_Handler;
void task2_task(void *pvParameters);

//task3 defination
#define TASK3_TASK_PRIO		4
#define TASK3_STK_SIZE 		128  
TaskHandle_t Task3Task_Handler;
void task3_task(void *pvParameters);

//LCDˢ��ʱʹ�õ���ɫ
int lcd_discolor[14]={	WHITE, BLACK, BLUE,  BRED,      
						GRED,  GBLUE, RED,   MAGENTA,       	 
						GREEN, CYAN,  YELLOW,BROWN, 			
						BRRED, GRAY };
int RGB_Color[3]={RED, BLUE, YELLOW};
//EDF Test						
#define MAX_TASKS 10
						
#define TASK1_PRIORITY 3
#define TASK2_PRIORITY 2			
#define TASK3_PRIORITY 1					
#define TIME_SLICE_INTERVAL 1000	//ÿ��ʱ��Ƭ���ȣ���λΪms
						
TaskHandle_t EDF_Scheduler_Handle;						
						
//��������ĵ���ʱ�䣬ִ��ʱ�䣬���ڣ�deadline���������̬���ȼ�������ʵ����ռʽEDF���ȣ�				
typedef struct{
	uint32_t ID;
	uint32_t arrival_time;
	uint32_t execution_time;
	uint32_t period;
	uint32_t deadline;
	TaskHandle_t task_handle;
}EDF_Task;

EDF_Task tasks[MAX_TASKS];
uint32_t current_time = 0;	//��ǰʱ��Ƭ�����
uint32_t current_task_count = 3;	//��ǰ��������

//��������
void EDF_TASK_1(void *pvParameters);
void EDF_TASK_2(void *pvParameters);
void EDF_TASK_3(void *pvParameters);
void EDF_Scheduler(void *pvParameters);
void initialize_tasks(void);
EDF_Task* get_earliest_deadline_task(void);


void EDF_Task_1(void *pvParameters)
{
	while(1)
	{
		
//		LCD_ShowString(6,300,50,16,16,(u8*)pcTaskGetName(NULL));
//		LCD_ShowxNum(70,300,xTaskGetTickCount()/TIME_SLICE_INTERVAL,3,16,0x80);
//		LCD_ShowString(100,300,80,16,16,(u8*)"exeTime");
//		LCD_ShowxNum(200,300,tasks[0].execution_time,8,16,0x80);
	}
}

void EDF_Task_2(void *pvParameters)
{
	while(1)
	{
//		LED1=!LED1;
//		LCD_ShowString(6,400,50,16,16,(u8*)pcTaskGetName(NULL));
//		LCD_ShowxNum(70,400,xTaskGetTickCount()/TIME_SLICE_INTERVAL,3,16,0x80);
//		LCD_ShowString(100,400,80,16,16,(u8*)"exeTime");
//		LCD_ShowxNum(200,400,tasks[1].execution_time,8,16,0x80);
	}
}

void EDF_Task_3(void *pvParameters)
{
	while(1)
	{
//		LCD_ShowString(6,500,50,16,16,(u8*)pcTaskGetName(NULL));
//		LCD_ShowxNum(70,500,xTaskGetTickCount()/TIME_SLICE_INTERVAL,3,16,0x80);
//		LCD_ShowString(100,500,80,16,16,(u8*)"exeTime");
//		LCD_ShowxNum(200,500,tasks[2].execution_time,8,16,0x80);
	}
}

//��ȡӵ������deadline��task
EDF_Task* get_earliest_deadline_task(void){
	int i;
	EDF_Task *earliest_task = NULL;
	uint32_t min_deadline = UINT32_MAX;
	
	//������ǰ�����Ѿ�������Ϊִ����ϵģ�ӵ�����deadline������
	for(i=0;i<current_task_count;i++){
		if(tasks[i].arrival_time <= current_time && tasks[i].deadline < min_deadline && tasks[i].execution_time != 0){
			min_deadline = tasks[i].deadline;
			earliest_task = &tasks[i];
		}
	}
//	LCD_ShowString(150,600,110,16,16,(u8*)"current time");
//	LCD_ShowxNum(250,600,current_time/TIME_SLICE_INTERVAL,4,16,0x80);
//	LCD_ShowString(300,600,110,16,16,(u8*)"current min_d");
//	LCD_ShowxNum(400,600,min_deadline,8,16,0x80);
	return earliest_task;
}

//��������
void EDF_Scheduler(void *pvParameters)
{
		int i;
		int flag;
		char str[10];
		LED0=!LED0;
    while (1) {
			EDF_Task* task_to_execute = get_earliest_deadline_task();	//��ȡ��ǰʱ��Ƭӵ�����deadline������
			LED0=!LED0;
			LED1=!LED1;
			LCD_ShowString(150,700,110,16,16,(u8*)"current time");
			LCD_ShowxNum(250,700,current_time/TIME_SLICE_INTERVAL,4,16,0x80);
			LCD_ShowString(300,700,110,16,16,(u8*)"current Task");
			LCD_ShowxNum(400,700,task_to_execute->ID,8,16,0x80);
			if (task_to_execute != NULL) {
				//��ռʽEDF
				for(i=0;i<current_task_count;i++){	
					if(&tasks[i] != task_to_execute){
						vTaskSuspend(tasks[i].task_handle);
					}else{
						vTaskResume(tasks[i].task_handle);
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
				flag = 0;
				//�����ǰ��������������ִ����ϣ��ָ����ʼֵ+����
				for(i=0;i<current_task_count;i++){	
//					LCD_ShowString(5+i*100,600,100,16,16,(u8*)"exeT");
//					LCD_ShowxNum(5+i*100,650,tasks[i].execution_time,8,16,0x80);
					if(tasks[i].execution_time == 0){
						flag++;
					}
				}
//				LCD_ShowString(350,650,110,16,16,(u8*)"flag");
//				LCD_ShowxNum(450,650,flag,4,16,0x80);
//				LCD_ShowString(300,650,110,16,16,(u8*)"current Task");
//				LCD_ShowxNum(400,650,task_to_execute->ID,8,16,0x80);				
				
				if(flag == 3){
					for(i=0;i<current_task_count;i++){
						vTaskSuspend(tasks[i].task_handle);
					}
					vTaskSuspend(EDF_Scheduler_Handle);
				}
			}

			// ��ǰʱ��Ƭ+1
			current_time+=TIME_SLICE_INTERVAL;
//			LCD_ShowString(300,600,110,16,16,(u8*)"current time");
//			LCD_ShowxNum(400,600,current_time,8,16,0x80);			
			vTaskDelay(TIME_SLICE_INTERVAL);//���TIME_SLICE_INTERVAL��ִ��
	}
}

void initialize_tasks(void) {
	current_task_count = 3;

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
	
	
//EDF����
	//define tasks
	initialize_tasks();
	
	//��������ʹ��EDF����
	xTaskCreate(EDF_Task_1, "Task1", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, &tasks[0].task_handle);
	xTaskCreate(EDF_Task_2, "Task2", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &tasks[1].task_handle);
	xTaskCreate(EDF_Task_3, "Task3", configMINIMAL_STACK_SIZE, NULL, TASK3_PRIORITY, &tasks[2].task_handle);
	xTaskCreate(EDF_Scheduler, "Scheduler", configMINIMAL_STACK_SIZE, NULL, UINT32_MAX, EDF_Scheduler_Handle);
	//��������
	vTaskStartScheduler();
	
	
	
	//��ռʽ����
//    xTaskCreate((TaskFunction_t )start_task,            //������
//                (const char*    )"start_task",          //��������
//                (uint16_t       )START_STK_SIZE,        //�����ջ��С
//                (void*          )NULL,                  //���ݸ��������Ĳ���
//                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
//                (TaskHandle_t*  )&StartTask_Handler);   //������              
//    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
	
    //����TASK1����
    xTaskCreate((TaskFunction_t )task1_task,             
                (const char*    )"task1_task",           
                (uint16_t       )TASK1_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TASK1_TASK_PRIO,        
                (TaskHandle_t*  )&Task1Task_Handler);   
								
    //����TASK2����
    xTaskCreate((TaskFunction_t )task2_task,     
                (const char*    )"task2_task",   
                (uint16_t       )TASK2_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK2_TASK_PRIO,
                (TaskHandle_t*  )&Task2Task_Handler); 
								
//		//����TASK3����
//    xTaskCreate((TaskFunction_t )task3_task,     
//                (const char*    )"task3_task",   
//                (uint16_t       )TASK3_STK_SIZE,
//                (void*          )NULL,
//                (UBaseType_t    )TASK3_TASK_PRIO,
//                (TaskHandle_t*  )&Task3Task_Handler); 
								
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����						
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//task1������
void task1_task(void *pvParameters)
{
	TickType_t xStartTime;
	u8 task1_num=0;
	TickType_t xDuration = pdMS_TO_TICKS(5000);	//���ñ�������ִ��ʱ��

	xStartTime = xTaskGetTickCount();	//��ȡ������������ʱ��
	
	POINT_COLOR = BLACK;
	LCD_DrawRectangle(5,110,115,314); 	//��һ������	
	LCD_DrawLine(5,130,115,130);		//����
	POINT_COLOR = BLUE;
	LCD_ShowString(6,111,110,16,16,"Task1 Run:000");
	
	
	while(1)
	{
		task1_num++;	//����ִ1�д�����1 ע��task1_num1�ӵ�255��ʱ������㣡��
		LED0=!LED0;
		printf("����1�Ѿ�ִ�У�%d��\r\n",task1_num);
		
		LCD_Fill(6,131,114,313,task1_num%2==0?RED:WHITE); //�������
		LCD_ShowxNum(86,111,task1_num,3,16,0x80);	//��ʾ����ִ�д���
		
		//����5s����ʾʱ��
		while(xTaskGetTickCount() - xStartTime < xDuration){
			LCD_ShowString(6,400,110,16,16,(u8*)pcTaskGetName(NULL));
			LCD_ShowxNum(120,400,xTaskGetTickCount()/1000,3,16,0x80);
		}
		//�ظ�
		xDuration+=pdMS_TO_TICKS(5000);
		//����1s������������ִ��
    vTaskDelay(1000);                           
	}
}

//task2������
void task2_task(void *pvParameters)
{
	u8 task2_num=0;
	
	POINT_COLOR = BLACK;

	LCD_DrawRectangle(125,110,234,314); //��һ������	
	LCD_DrawLine(125,130,234,130);		//����
	POINT_COLOR = BLUE;
	LCD_ShowString(126,111,110,16,16,"Task2 Run:000");
	while(1)
	{
		task2_num++;	//����2ִ�д�����1 ע��task1_num2�ӵ�255��ʱ������㣡��
        LED1=!LED1;
		printf("����2�Ѿ�ִ�У�%d��\r\n",task2_num);
		LCD_ShowString(6,400,110,16,16,(u8*)pcTaskGetName(NULL));
		LCD_ShowxNum(206,111,task2_num,3,16,0x80);  //��ʾ����ִ�д���
		LCD_Fill(126,131,233,313,task2_num%2==0?GREEN:WHITE); //�������
        vTaskDelay(200);                          
	}
}

//task3������
void task3_task(void *pvParameters)
{
	u8 task3_num=0;
	
	POINT_COLOR = BLACK;

	LCD_DrawRectangle(245,110,355,314); //��һ������	
	LCD_DrawLine(245,130,355,130);		//����
	POINT_COLOR = BLUE;
	LCD_ShowString(246,111,110,16,16,"Task3 Run:000");
	while(1)
	{
		task3_num++;	
		printf("����3�Ѿ�ִ�У�%d��\r\n",task3_num);
		LCD_ShowxNum(325,111,task3_num,3,16,0x80);  //��ʾ����ִ�д���
		LCD_Fill(246,131,354,313,task3_num%2==0?BLUE:WHITE); //�������
    vTaskDelay(6000);                          
	}
}
