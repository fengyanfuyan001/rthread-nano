/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "main.h"
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

extern void screen_setup(void);

static rt_thread_t tid1 = RT_NULL;
static void thread1_entry(void *parameter) {
  while (1){
    screen_setup();
  }
}


ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;

static void thread2_entry(void *param) {
  while(1) {
    HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
    rt_thread_mdelay(500);
  }
} 

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{


  tid1 = rt_thread_create("thread1",
                          thread1_entry, RT_NULL,
                          THREAD_STACK_SIZE,
                          THREAD_PRIORITY, THREAD_TIMESLICE);
  if (tid1 != RT_NULL)
      rt_thread_startup(tid1);

  rt_thread_init(&thread2,
                  "thread2",
                  thread2_entry,
                  RT_NULL,
                  &thread2_stack[0],
                  sizeof(thread2_stack),
                  THREAD_PRIORITY - 1, THREAD_TIMESLICE);
  rt_thread_startup(&thread2);
  
  while (1)
  {
    rt_kprintf("This is the main thread!\n");
    rt_thread_mdelay(1000);
  }
  return 0;
}


