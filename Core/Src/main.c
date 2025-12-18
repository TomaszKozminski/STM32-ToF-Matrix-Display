/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// #include "TOFAdapter.h"
// #include "TOFUart.h"
// #include "calculate.h"
#include "RangeSensor.h"        // Nasz interfejs
#include "VL53L7CX_Adapter.h"   // Nasz adapter
#include "ObjectTracker.h"

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_uart.h"
#include <stdio.h>
#include <string.h>

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
I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
volatile uint8_t vl53l7cx_int = 0; // Interrupt flag for VL53L7CX
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C2_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void Debug_PrintState(const TrackedObject *obj); // Deklaracja funkcji debugującej
int _write(int file, char *ptr, int len);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//   if (GPIO_Pin == I2C_INT_Pin)
//   {
//     vl53l7cx_int = 1;
//   }
// }

// Przekierowanie printf na UART2
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, 100);
    return len;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == I2C_INT_Pin)
  {
    vl53l7cx_int = 1;
  }
}

// Funkcja rysująca "Radar" w terminalu ASCII
void Debug_PrintMatrix(const TrackedObject *obj, const RangeSensorFrame *frame) {
    // 1. Wyczyść ekran
    printf("\033[2J\033[H"); 

    printf("=== MAPA ODLEGLOSCI (8x8) ===\r\n");
    printf("     0    1    2    3    4    5    6    7\r\n");
    printf("   ----------------------------------------\r\n");

    // 2. Pętla rysująca siatkę 8x8
    for (int y = 0; y < 8; y++) {
        printf("%d | ", y); // Numer wiersza
        for (int x = 0; x < 8; x++) {
            // Obliczamy indeks w tablicy liniowej (zakładamy układ wierszami)
            // Czasami czujniki mają układ kolumnami - zobaczysz to machając ręką
            int idx = y * 8 + x; 
            
            uint16_t dist = frame->distances_mm[idx];
            uint8_t stat = frame->statuses[idx];

            // Status 5 i 9 to poprawne pomiary. 
            // Jeśli inny -> wyświetl kreski, żeby nie zaciemniać obrazu zerami.
            if (stat == 5 || stat == 9) {
                printf("%4d ", dist);
            } else {
                printf("---- "); 
            }
        }
        printf("|\r\n"); // Koniec wiersza
    }
    printf("   ----------------------------------------\r\n");

    // 3. Wyniki Trackera (pod spodem)
    printf("\r\n=== WYNIKI ALGORYTMU ===\r\n");
    printf("Status:      %s\r\n", obj->is_detected ? "[ OBIEKT WYKRYTY ]" : "[ Szukam... ]");
    printf("Pozycja X:   %5.2f  (Srodek to 3.5)\r\n", obj->position_x);
    printf("Pozycja Y:   %5.2f  (Srodek to 3.5)\r\n", obj->position_y);
    printf("Odleglosc:   %5.0f mm\r\n", obj->distance_mm);
    printf("Pewnosc:     %5.0f %%\r\n", obj->probability * 100.0f);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  // TOFAdapter tofAdapter;
  // TOFConfiguration tofConfig = {
  //     .resolution = VL53L7CX_RESOLUTION_8X8,
  //     .ranging_mode = VL53L7CX_RANGING_MODE_CONTINUOUS,
  //     .ranging_frequency = 15, // Hz
  //     .sharpener_percent = 40, // percent
  //     .target_order = VL53L7CX_TARGET_ORDER_CLOSEST,
  //     .power_mode = VL53L7CX_POWER_MODE_WAKEUP
  // };

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
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  // initTOF(&tofAdapter);
  // setTOF(&tofAdapter, &tofConfig);
  // startRangingTOF(&tofAdapter);
  // zone_tracker_init();
 
  // vl53l7cx_int = 0; // Initialize the interrupt flag

  // const char bootmsg[] = "BOOT\r\n";
  // HAL_UART_Transmit(&huart2, (uint8_t*)bootmsg, sizeof(bootmsg)-1, 100);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // if (vl53l7cx_int) {
    //   vl53l7cx_int = 0; // Reset the interrupt flag
    //   getRangingDataTOF(&tofAdapter);
    //   sendTOFData(&tofAdapter, TOF8X8);
    // }
    // __WFI();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00B10E24;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }

  /** I2C Fast mode Plus enable
  */
  HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C2);
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|I2C_PWREN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(I2C_LPN_GPIO_Port, I2C_LPN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(I2C_RST_GPIO_Port, I2C_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin I2C_PWREN_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|I2C_PWREN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : I2C_LPN_Pin I2C_RST_Pin */
  GPIO_InitStruct.Pin = I2C_LPN_Pin|I2C_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : I2C_INT_Pin */
  GPIO_InitStruct.Pin = I2C_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(I2C_INT_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  static const ObjectTrackerConfig tracker_cfg = {
      .min_dist_mm     = 25.0f,
      .max_dist_mm     = 2000.0f,
      .fade_dist_mm    = 300.0f,
      .min_mass_detect = 3.5f,
      .smooth_factor   = 0.15f,
      .sensor_fov_deg  = 45.0f  // Standardowe FOV dla VL53L7CX (45x45)
  };
  
  // Zmienne statyczne (static żeby działało?)
  static RangeSensor mySensor; 
  static ObjectTracker myTracker;
  static RangeSensorFrame currentFrame;

  // Wyłącz buforowanie, żeby printf wychodził od razu
  setvbuf(stdout, NULL, _IONBF, 0);

  printf("\r\n--- START SYSTEMU ---\r\n");

  // A. Inicjalizacja
  VL53L7CX_Adapter_Construct(&mySensor, &hi2c2, 0x52);

  printf("Inicjalizacja czujnika... \r\n");
  RangeSensor_Status init_status = RangeSensor_Init(&mySensor);
  
  if (init_status != RS_OK) {
      printf("BLAD KRYTYCZNY: Init kod: %d\r\n", init_status);
      for(;;) osDelay(1000); 
  } else {
      printf("Czujnik OK.\r\n");
  }

  ObjectTracker_Init(&myTracker, &tracker_cfg);

  // B. Start
  printf("Startuje pomiary...\r\n");
  if (RangeSensor_Start(&mySensor) != RS_OK) {
      printf("BLAD: Start nieudany.\r\n");
  }

  // Timer do spowalniania wyświetlania
  uint32_t last_print_tick = 0;

  for(;;)
  {
      // 1. Pobierz dane (SZYBKO - jak najczęściej)
      RangeSensor_Status status = RangeSensor_GetFrame(&mySensor, &currentFrame);

      if (status == RS_OK) {
          // 2. Przelicz matematykę (SZYBKO - dla płynności filtrów)
          ObjectTracker_Process(&myTracker, &currentFrame);

          // 3. Wyświetl wyniki (WOLNO - np. co 300ms, żeby dało się czytać)
          if (HAL_GetTick() - last_print_tick > 400) { // Zwiększyłem do 400ms dla czytelności
              
              // TU WYWOŁUJEMY NOWĄ FUNKCJĘ:
              Debug_PrintMatrix(&myTracker.target, &currentFrame);
              
              last_print_tick = HAL_GetTick();
          }
      }
      else if (status == RS_ERROR_HW_FAILURE) {
          printf("Critical HW Error!\r\n");
          osDelay(1000); // Zwolnij przy błędzie
      }

      // Ważne: Opóźnienie pętli musi być krótkie dla I2C, ale print rzadki
      osDelay(5); 
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
