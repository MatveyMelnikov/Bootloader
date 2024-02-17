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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "bootloader_cmd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_START_ADDRESS 0x08000000 + (0x400 * 10) /* page 10 */
#define SRAM_SIZE 20 * 1024
#define SRAM_END (SRAM_BASE + SRAM_SIZE)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define GET_VALUE_FROM_ADDR(ADDR) \
  *((volatile uint32_t*)ADDR)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef* bootloader_uart = &huart1;

/* USER CODE BEGIN PV */
extern uint32_t _sidata; // .data in flash (VMA)
extern uint32_t _sdata; // start of .data (initialize vars) in flash
extern uint32_t _edata;
extern uint32_t _sbss; // start of .bss (not initialize vars) in SRAM
extern uint32_t _ebss;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void _bootloader_start(void);
int main(void);
void SysTick_Handler(void);
void HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin);
static void led_blink(void);
static void gpio_deinit(void);
static void start_application_code(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Minimal vector table
uint32_t *vector_table[] __attribute__((section(".isr_vector"))) = {
  (uint32_t *) SRAM_END, // initial stack pointer
  (uint32_t *) _bootloader_start,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  (uint32_t *)SysTick_Handler 
};

__attribute__((always_inline))
inline static void __initialize_data(
    uint32_t* flash_begin,
    uint32_t* data_begin,
    uint32_t* data_end
) {
    uint32_t *p = data_begin;
    while (p < data_end)
        *p++ = *flash_begin++;
}

__attribute__((always_inline))
inline static void __initialize_bss(
    uint32_t* bss_begin,
    uint32_t* bss_end
) {
    uint32_t *p = bss_begin;
    while (p < bss_end)
        *p++ = 0;
}

__attribute__((noreturn,weak))
void _bootloader_start(void)
{
    __initialize_data(&_sidata, &_sdata, &_edata);
    __initialize_bss(&_sbss, &_ebss);
    main();

    for(;;);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  uint32_t current_ticks = HAL_GetTick();

  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET)
  {
    MX_USART1_UART_Init();
    bootloader_start_output();

    while (true)
    {
      led_blink();

      if ((HAL_GetTick() - current_ticks) < UART_DELAY)
        continue;
      if (bootloader_proccess_input() == BOOTLOADER_TIMEOUT)
        continue;

      current_ticks = HAL_GetTick();
    }
  }
  else
  {
    start_application_code();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // while (1)
  // {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  // }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

static void led_blink(void)
{
  static uint32_t current_ticks = LED_DELAY;

  if ((HAL_GetTick() - current_ticks) < LED_DELAY)
    return;
  
  current_ticks = HAL_GetTick();
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

static void start_application_code()
{
  // volatile?
  //volatile uint32_t *application_start = (uint32_t *)APP_START_ADDRESS;
  uint32_t app_msp = *((volatile uint32_t*)APP_START_ADDRESS);
  if (app_msp != SRAM_END)
    Error_Handler();

  HAL_UART_DeInit(&huart1);
  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_13);
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
  HAL_DeInit();

  //RCC->CIR = 0x00000000; //Disable all interrupts related to clock
  __disable_irq();
  __set_MSP(app_msp);
  // programming manual pg. 99
  __DMB();
  SCB->VTOR = APP_START_ADDRESS; // not address! offset is value
  /// programming manual pg. 100
  __DSB();

  // ???
  // uint32_t JumpAddress = *((volatile uint32_t*) (APP_START_ADDRESS + 4));
  // void (*reset_handler)(void) = (void*)JumpAddress;

  // uint32_t jump_address = (volatile uint32_t)(0x1FFF 0000 + 4);
  // void (*boot_loader)(void) =  jump_address;

  volatile uint32_t *jump_address = (volatile uint32_t)(
    APP_START_ADDRESS + sizeof(uint32_t)
  );
  void (*reset_handler)(void) = (void*)jump_address;
  reset_handler();

  // Never coming here
  Error_Handler();
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  //__disable_irq();
  while (1)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    HAL_Delay(LED_ERROR_DELAY);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
