/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "dma.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "key.h"
#include "sht30.h"
#include "stdio.h"
#include "stdarg.h"
#include "lvgl.h"
#include "lv_port_indev.h"
#include "lv_port_lcd_stm32.h"
// #include "lv_demos.h"
#include "../lvgl/demos/lv_demos.h"
#include "xpt2046.h" //���Դ���������
#define RX_BUFFER_SIZE 100
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t rx_buffer[100];
uint16_t size = 0;
uint8_t Rxflag = 0;

Sht30Data sht30_data;
uint32_t system_time = 0; // ms
uint32_t last_time = 0;   // ms
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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_SPI3_Init();
  MX_RTC_Init();
  MX_USB_DEVICE_Init();
  MX_TIM6_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer, RX_BUFFER_SIZE);
  int printf_dma(const char *fmt, ...);
  HAL_TIM_Base_Start(&htim6);

  lv_init();
  lv_tick_set_cb(HAL_GetTick);
  lv_port_disp_init();
  lv_port_indev_init();
  lv_demo_widgets();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  while (1)
  {
    uint32_t tick = lv_timer_handler();
    HAL_Delay(tick > 0 ? tick : 1);
    // HAL_Delay(5);
    // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    // HAL_Delay(500);
    // if (Touchpad_IsPressed())
    // {
    //   HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    //   printf_dma("Touchpad Pressed: X=%d, Y=%d\r\n", (Touchpad_ReadRawX() * 240 / (RAW_X_MAX - RAW_X_MIN)),
    //              (Touchpad_ReadRawY() * 320 / (RAW_Y_MAX - RAW_Y_MIN)));
    //   HAL_Delay(100);
    // }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // KeyEvent_t event = Key_GetEvent();
    // if (event == KEY_1)
    // {
    //   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    //   HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"KEY_1 Pressed\r\n", 15);
    //   // HAL_UART_Transmit(&huart1, (uint8_t *)"KEY_1 Pressed\r\n", 15, 1000);
    // }
    // else if (event == KEY_2)
    // {
    //   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    //   HAL_UART_Transmit_DMA(&huart1, (uint8_t *)"KEY_2 Pressed\r\n", 15);
    //   // HAL_UART_Transmit(&huart1, (uint8_t *)"KEY_2 Pressed\r\n", 15, 1000);
    // }
    // if (Rxflag)
    // {
    //   Rxflag = 0;
    //   HAL_UART_Transmit_DMA(&huart1, rx_buffer, size);
    //   // HAL_UART_Transmit(&huart1, rx_buffer, strlen((char *)rx_buffer), 1000);
    // }

    // system_time = HAL_GetTick();
    // if (system_time - last_time >= 1500)
    // {
    //   last_time = system_time;
    //   sht30_data = Sht30_Getdata();
    //   printf_dma("Temperature: %.1f C, Humidity: %.1f %%\r\n", sht30_data.temp, sht30_data.humi);

    // }
    // system_time = HAL_GetTick();
    // if (system_time - last_time >= 1500)
    // {
    //   HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    // }
  }
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

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart->Instance == USART1)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // �л�LED״̬
    if (Rxflag == 0)
    {
      Rxflag = 1; //
      if (Size < RX_BUFFER_SIZE)
      {
        rx_buffer[Size] = '\0'; //
        size = Size;            //
      }
    }
    HAL_UARTEx_ReceiveToIdle_DMA(huart, rx_buffer, RX_BUFFER_SIZE); //
  }
}

int printf_dma(const char *fmt, ...)
{
  static char buffer[128]; //
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  while (HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX)
    ;
  if (len > 0)
  {
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)buffer, len);
  }

  return len;
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
