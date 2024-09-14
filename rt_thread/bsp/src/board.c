/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-24     Tanek        the first version
 * 2018-11-12     Ernest Chen  modify copyright
 */
 
#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>
#include "main.h"


#define _SCB_BASE       (0xE000E010UL)
#define _SYSTICK_CTRL   (*(rt_uint32_t *)(_SCB_BASE + 0x0))
#define _SYSTICK_LOAD   (*(rt_uint32_t *)(_SCB_BASE + 0x4))
#define _SYSTICK_VAL    (*(rt_uint32_t *)(_SCB_BASE + 0x8))
#define _SYSTICK_CALIB  (*(rt_uint32_t *)(_SCB_BASE + 0xC))
#define _SYSTICK_PRI    (*(rt_uint8_t  *)(0xE000ED23UL))

#ifdef RT_USING_CONSOLE

static UART_HandleTypeDef huart1;

static int uart_setup(void)
{

    huart1.Instance = USART1;
    huart1.Init.BaudRate   = 115200;
    huart1.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    huart1.Init.Mode       = UART_MODE_TX_RX;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits   = UART_STOPBITS_1;
    huart1.Init.Parity     = UART_PARITY_NONE;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        while(1);
    }

    return 0;
}

INIT_BOARD_EXPORT(uart_setup);

void rt_hw_console_output(const char *str)
{
    rt_size_t i = 0, size = 0;
    char a = '\r';

    __HAL_UNLOCK(&huart1);

    size = rt_strlen(str);

    for (i = 0; i < size; i++)
    {
        if (*(str + i) == '\n')
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)&a, 1, 1);
        }
        HAL_UART_Transmit(&huart1, (uint8_t *)(str + i), 1, 1);
    }
}
#endif

static void gpio_setup(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}

I2C_HandleTypeDef hi2c1;

static uint32_t i2c_setup(void) {

    __HAL_RCC_I2C1_CLK_ENABLE();    
    __HAL_RCC_GPIOB_CLK_ENABLE();  

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // SCL PB6  SDA PB7
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;    
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
   
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;            
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;    
    hi2c1.Init.OwnAddress1 = 0;                
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;  
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;     

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        while(1);
    }
    return 0;
}

void screen_setup(void)
{
    uint8_t screen_init_command[] = {
        0x00, // Command Stream
        0xa8, 0x3f, // Set MUX Ratio
        0xd3, 0x00, // Set Display Offset
        0x40, // Set Display Start Line
        0xa0, // Set Segment re-map
        0xc0, // Set COM Output Scan Direction
        0xda, 0x02, // Set COM Pins hardware configuration
        0x81, 0x7f, // Set Contrast Control
        0xa5, // Enable Entire Display On
        0xa6, // Set Normal Display
        0xd5, 0x80, // Set OSC Frequency
        0x8d, 0x14, // Enable charge pump regulator
        0xaf, // Display on
    };
    uint16_t DevAddress = 0x78;      

    if (HAL_I2C_Master_Transmit(&hi2c1, DevAddress << 1, screen_init_command, sizeof(screen_init_command) / sizeof(uint8_t), HAL_MAX_DELAY) == HAL_OK)
    {
      rt_kprintf("data transmit success!\n");
    }
    else
    {
      rt_kprintf("data transmit error!\n");
    }
}
// Updates the variable SystemCoreClock and must be called 
// whenever the core clock is changed during program execution.
extern void SystemCoreClockUpdate(void);
// Holds the system core clock, which is the system clock 
// frequency supplied to the SysTick timer and the processor 
// core clock.
extern uint32_t SystemCoreClock;

static uint32_t _SysTick_Config(rt_uint32_t ticks)
{
    if ((ticks - 1) > 0xFFFFFF)
    {
        return 1;
    }
    
    _SYSTICK_LOAD = ticks - 1; 
    _SYSTICK_PRI = 0xFF;
    _SYSTICK_VAL  = 0;
    _SYSTICK_CTRL = 0x07;  
    
    return 0;
}

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
#define RT_HEAP_SIZE 1024
static uint32_t rt_heap[RT_HEAP_SIZE];     // heap default size: 4K(1024 * 4)
RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

/**
 * This function will initial your board.
 */
void rt_hw_board_init()
{
    HAL_Init();
    /* System Clock Update */
    SystemCoreClockUpdate();
    /* System Tick Configuration */
    _SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

    uart_setup();
    
    gpio_setup();

    i2c_setup();

    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
}

void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}


