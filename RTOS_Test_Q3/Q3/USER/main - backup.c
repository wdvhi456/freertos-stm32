#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "lcd.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
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

//LCD刷屏时使用的颜色
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
#define TIME_SLICE_INTERVAL 1000	//每个时间片长度，单位为ms
						
TaskHandle_t EDF_Scheduler_Handle;						
						
//定义任务的到达时间，执行时间，周期，deadline，句柄，动态优先级（用于实现抢占式EDF调度）				
typedef struct{
	uint32_t ID;
	uint32_t arrival_time;
	uint32_t execution_time;
	uint32_t period;
	uint32_t deadline;
	TaskHandle_t task_handle;
}EDF_Task;

EDF_Task tasks[MAX_TASKS];
uint32_t current_time = 0;	//当前时间片的序号
uint32_t current_task_count = 3;	//当前任务总数

//函数声明
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

//获取拥有最早deadline的task
EDF_Task* get_earliest_deadline_task(void){
	int i;
	EDF_Task *earliest_task = NULL;
	uint32_t min_deadline = UINT32_MAX;
	
	//搜索当前周期已经到达且为执行完毕的，拥有最近deadline的任务
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

//主调度器
void EDF_Scheduler(void *pvParameters)
{
		int i;
		int flag;
		char str[10];
		LED0=!LED0;
    while (1) {
			EDF_Task* task_to_execute = get_earliest_deadline_task();	//获取当前时间片拥有最近deadline的任务
			LED0=!LED0;
			LED1=!LED1;
			LCD_ShowString(150,700,110,16,16,(u8*)"current time");
			LCD_ShowxNum(250,700,current_time/TIME_SLICE_INTERVAL,4,16,0x80);
			LCD_ShowString(300,700,110,16,16,(u8*)"current Task");
			LCD_ShowxNum(400,700,task_to_execute->ID,8,16,0x80);
			if (task_to_execute != NULL) {
				//抢占式EDF
				for(i=0;i<current_task_count;i++){	
					if(&tasks[i] != task_to_execute){
						vTaskSuspend(tasks[i].task_handle);
					}else{
						vTaskResume(tasks[i].task_handle);
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
				flag = 0;
				//如果当前周期内所有任务执行完毕，恢复其初始值+周期
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

			// 当前时间片+1
			current_time+=TIME_SLICE_INTERVAL;
//			LCD_ShowString(300,600,110,16,16,(u8*)"current time");
//			LCD_ShowxNum(400,600,current_time,8,16,0x80);			
			vTaskDelay(TIME_SLICE_INTERVAL);//间隔TIME_SLICE_INTERVAL再执行
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
	//单片机初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4	 
	delay_init();	    				//延时函数初始化	 
	uart_init(115200);					//初始化串口
	LED_Init();		  					//初始化LED
	LCD_Init();							//初始化LCD
	POINT_COLOR = RED;
	//the size of the font must be 12/16/24
	LCD_ShowString(30,10,300,48,24,"RTOS TEST");
	
	
//EDF调度
	//define tasks
	initialize_tasks();
	
	//创建任务并使用EDF调度
	xTaskCreate(EDF_Task_1, "Task1", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, &tasks[0].task_handle);
	xTaskCreate(EDF_Task_2, "Task2", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &tasks[1].task_handle);
	xTaskCreate(EDF_Task_3, "Task3", configMINIMAL_STACK_SIZE, NULL, TASK3_PRIORITY, &tasks[2].task_handle);
	xTaskCreate(EDF_Scheduler, "Scheduler", configMINIMAL_STACK_SIZE, NULL, UINT32_MAX, EDF_Scheduler_Handle);
	//开启调度
	vTaskStartScheduler();
	
	
	
	//抢占式调度
//    xTaskCreate((TaskFunction_t )start_task,            //任务函数
//                (const char*    )"start_task",          //任务名称
//                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
//                (void*          )NULL,                  //传递给任务函数的参数
//                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
//                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
//    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
	
    //创建TASK1任务
    xTaskCreate((TaskFunction_t )task1_task,             
                (const char*    )"task1_task",           
                (uint16_t       )TASK1_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TASK1_TASK_PRIO,        
                (TaskHandle_t*  )&Task1Task_Handler);   
								
    //创建TASK2任务
    xTaskCreate((TaskFunction_t )task2_task,     
                (const char*    )"task2_task",   
                (uint16_t       )TASK2_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK2_TASK_PRIO,
                (TaskHandle_t*  )&Task2Task_Handler); 
								
//		//创建TASK3任务
//    xTaskCreate((TaskFunction_t )task3_task,     
//                (const char*    )"task3_task",   
//                (uint16_t       )TASK3_STK_SIZE,
//                (void*          )NULL,
//                (UBaseType_t    )TASK3_TASK_PRIO,
//                (TaskHandle_t*  )&Task3Task_Handler); 
								
    vTaskDelete(StartTask_Handler); //删除开始任务						
    taskEXIT_CRITICAL();            //退出临界区
}

//task1任务函数
void task1_task(void *pvParameters)
{
	TickType_t xStartTime;
	u8 task1_num=0;
	TickType_t xDuration = pdMS_TO_TICKS(5000);	//设置本次任务执行时间

	xStartTime = xTaskGetTickCount();	//获取本次任务启动时间
	
	POINT_COLOR = BLACK;
	LCD_DrawRectangle(5,110,115,314); 	//画一个矩形	
	LCD_DrawLine(5,130,115,130);		//画线
	POINT_COLOR = BLUE;
	LCD_ShowString(6,111,110,16,16,"Task1 Run:000");
	
	
	while(1)
	{
		task1_num++;	//任务执1行次数加1 注意task1_num1加到255的时候会清零！！
		LED0=!LED0;
		printf("任务1已经执行：%d次\r\n",task1_num);
		
		LCD_Fill(6,131,114,313,task1_num%2==0?RED:WHITE); //填充区域
		LCD_ShowxNum(86,111,task1_num,3,16,0x80);	//显示任务执行次数
		
		//运行5s并显示时间
		while(xTaskGetTickCount() - xStartTime < xDuration){
			LCD_ShowString(6,400,110,16,16,(u8*)pcTaskGetName(NULL));
			LCD_ShowxNum(120,400,xTaskGetTickCount()/1000,3,16,0x80);
		}
		//重复
		xDuration+=pdMS_TO_TICKS(5000);
		//挂起1s，让其他任务执行
    vTaskDelay(1000);                           
	}
}

//task2任务函数
void task2_task(void *pvParameters)
{
	u8 task2_num=0;
	
	POINT_COLOR = BLACK;

	LCD_DrawRectangle(125,110,234,314); //画一个矩形	
	LCD_DrawLine(125,130,234,130);		//画线
	POINT_COLOR = BLUE;
	LCD_ShowString(126,111,110,16,16,"Task2 Run:000");
	while(1)
	{
		task2_num++;	//任务2执行次数加1 注意task1_num2加到255的时候会清零！！
        LED1=!LED1;
		printf("任务2已经执行：%d次\r\n",task2_num);
		LCD_ShowString(6,400,110,16,16,(u8*)pcTaskGetName(NULL));
		LCD_ShowxNum(206,111,task2_num,3,16,0x80);  //显示任务执行次数
		LCD_Fill(126,131,233,313,task2_num%2==0?GREEN:WHITE); //填充区域
        vTaskDelay(200);                          
	}
}

//task3任务函数
void task3_task(void *pvParameters)
{
	u8 task3_num=0;
	
	POINT_COLOR = BLACK;

	LCD_DrawRectangle(245,110,355,314); //画一个矩形	
	LCD_DrawLine(245,130,355,130);		//画线
	POINT_COLOR = BLUE;
	LCD_ShowString(246,111,110,16,16,"Task3 Run:000");
	while(1)
	{
		task3_num++;	
		printf("任务3已经执行：%d次\r\n",task3_num);
		LCD_ShowxNum(325,111,task3_num,3,16,0x80);  //显示任务执行次数
		LCD_Fill(246,131,354,313,task3_num%2==0?BLUE:WHITE); //填充区域
    vTaskDelay(6000);                          
	}
}
