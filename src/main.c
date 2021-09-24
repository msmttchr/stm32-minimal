#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "stm32l1xx.h"
#include "stm32l1xx_conf.h"
#include "system_def.h"
#include "scpi/scpi.h"
#include "scpi-def.h"

void     SystemClock_Config(void);
void     Configure_USART(void);
void     LED_Init(void);
void     LED_On(void);
void     LED_Off(void);
void     LED_Blinking(uint32_t Period);
void     UserButton_Init(void);
size_t txBuffer(uint8_t*buffer, size_t size);

size_t SCPI_Write(scpi_t * context, const char * data, size_t len) {
  (void) context;
  return txBuffer((uint8_t *)data, len);
}

scpi_result_t SCPI_Flush(scpi_t * context) {
  (void) context;

  return SCPI_RES_OK;
}

int SCPI_Error(scpi_t * context, int_fast16_t err) {
  (void) context;

  my_fprintf(stderr, "**ERROR: %d, \"%s\"\r\n", (int16_t) err, SCPI_ErrorTranslate(err));
  return 0;
}

scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    (void) context;

    if (SCPI_CTRL_SRQ == ctrl) {
        my_fprintf(stderr, "**SRQ: 0x%X (%d)\r\n", val, val);
    } else {
        my_fprintf(stderr, "**CTRL %02x: 0x%X (%d)\r\n", ctrl, val, val);
    }
    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t * context) {
    (void) context;

    fprintf(stderr, "**Reset\r\n");
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t * context) {
  (void) context;

  return SCPI_RES_ERR;
}

void SystemClock_Config(void){

  /* Clock init stuff */ 
    
  LL_UTILS_PLLInitTypeDef sUTILS_PLLInitStruct = {LL_RCC_PLL_MUL_3, LL_RCC_PLL_DIV_3};
  LL_UTILS_ClkInitTypeDef sUTILS_ClkInitStruct = {LL_RCC_SYSCLK_DIV_1, LL_RCC_APB1_DIV_1, LL_RCC_APB2_DIV_1};
    
  LL_PLL_ConfigSystemClock_HSI(&sUTILS_PLLInitStruct, &sUTILS_ClkInitStruct);
    
  LL_Init1msTick(SystemCoreClock);
}

int my_fprintf(FILE *stream, const char *format, ...)
{
  char message[256];
  int res;
  va_list ap;
  va_start (ap, format);
  res = vsprintf(message, format, ap);
  va_end (ap);
  txBuffer((uint8_t *)message, res);
  return res;
}

int main(void){
  const char *welcome_message = "RF Switch 7 Channels (A-G)\r\n(c) 2021 STMictroelectronics RF Team\r\n";


  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize LED2 */
  LED_Init();
    
  /* Set LED2 Off */
  LED_Off();
    
  /* Initialize button in EXTI mode */
  UserButton_Init();
 
  /* Configure USARTx (USART IP configuration and related GPIO initialization) */
  Configure_USART();

  my_fprintf(stderr, "%s", welcome_message);
    
  SCPI_Init(&scpi_context,
            scpi_commands,
            &scpi_interface,
            scpi_units_def,
            SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
            scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
            scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);

  /* Toggle forever */
  LED_Blinking(LED_BLINK_SLOW);

  return 0;
}

/**
 * @brief  This function configures USARTx Instance.
 * @note   This function is used to :
 *         -1- Enable GPIO clock and configures the USART pins.
 *         -2- NVIC Configuration for USART interrupts.
 *         -3- Enable the USART peripheral clock and clock source.
 *         -4- Configure USART functional parameters.
 *         -5- Enable USART.
 * @note   Peripheral configuration is minimal configuration from reset values.
 *         Thus, some useless LL unitary functions calls below are provided as
 *         commented examples - setting is default configuration from reset.
 * @param  None
 * @retval None
 */
void Configure_USART(void)
{

  /* (1) Enable GPIO clock and configures the USART pins *********************/

  /* Enable the peripheral clock of GPIO Port */
  USARTx_GPIO_CLK_ENABLE();

  /* Configure Tx Pin as : Alternate function, High Speed, Push pull, Pull up */
  LL_GPIO_SetPinMode(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_MODE_ALTERNATE);
  USARTx_SET_TX_GPIO_AF();
  LL_GPIO_SetPinSpeed(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_PULL_UP);

  /* Configure Rx Pin as : Alternate function, High Speed, Push pull, Pull up */
  LL_GPIO_SetPinMode(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_MODE_ALTERNATE);
  USARTx_SET_RX_GPIO_AF();
  LL_GPIO_SetPinSpeed(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_PULL_UP);

  /* (2) NVIC Configuration for USART interrupts */
  /*  - Set priority for USARTx_IRQn */
  /*  - Enable USARTx_IRQn */
  NVIC_SetPriority(USARTx_IRQn, 0);  
  NVIC_EnableIRQ(USARTx_IRQn);

  /* (3) Enable USART peripheral clock and clock source ***********************/
  USARTx_CLK_ENABLE();

  /* (4) Configure USART functional parameters ********************************/
  
  /* Disable USART prior modifying configuration registers */
  /* Note: Commented as corresponding to Reset value */
  // LL_USART_Disable(USARTx_INSTANCE);

  /* TX/RX direction */
  LL_USART_SetTransferDirection(USARTx_INSTANCE, LL_USART_DIRECTION_TX_RX);

  /* 8 data bit, 1 start bit, 1 stop bit, no parity */
  LL_USART_ConfigCharacter(USARTx_INSTANCE, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);

  /* No Hardware Flow control */
  /* Reset value is LL_USART_HWCONTROL_NONE */
  // LL_USART_SetHWFlowCtrl(USARTx_INSTANCE, LL_USART_HWCONTROL_NONE);

  /* Oversampling by 16 */
  /* Reset value is LL_USART_OVERSAMPLING_16 */
  //LL_USART_SetOverSampling(USARTx_INSTANCE, LL_USART_OVERSAMPLING_16);

  /* Set Baudrate to 115200 using APB frequency set to 32000000 Hz */
  /* Frequency available for USART peripheral can also be calculated through LL RCC macro */
  /* Ex :
     Periphclk = LL_RCC_GetUSARTClockFreq(Instance); or LL_RCC_GetUARTClockFreq(Instance); depending on USART/UART instance
  
     In this example, Peripheral Clock is expected to be equal to 32000000 Hz => equal to SystemCoreClock
  */
  LL_USART_SetBaudRate(USARTx_INSTANCE, SystemCoreClock, LL_USART_OVERSAMPLING_16, 115200); 

  /* (5) Enable USART *********************************************************/
  LL_USART_Enable(USARTx_INSTANCE);

  /* Enable RXNE and Error interrupts */
  LL_USART_EnableIT_RXNE(USARTx_INSTANCE);
  LL_USART_EnableIT_ERROR(USARTx_INSTANCE);
}

/******************************************************************************/
/*   IRQ HANDLER TREATMENT Functions                                          */
/******************************************************************************/
/**
 * @brief  Function to manage Button push
 * @param  None
 * @retval None
 */
void UserButton_Callback(void)
{
  /* Turn LED2 Off on User button press (allow to restart sequence) */
  LED_Off();
}

/**
 * @brief  Function called from USART IRQ Handler when RXNE flag is set
 *         Function is in charge of reading character received on USART RX line.
 * @param  None
 * @retval None
 */
void USART_CharReception_Callback(void)
{
  static char line[256];
  static int index=0;

  /* Read Received character. RXNE flag is cleared by reading of DR register */
  line[index++] = (char) LL_USART_ReceiveData8(USARTx_INSTANCE);
  /* Echo received character on TX */
  LL_USART_TransmitData8(USARTx_INSTANCE, line[index-1]);
  if ((index == sizeof(line)) || (line[index-1] == '\n')) {
    SCPI_Input(&scpi_context, line, index);
    index = 0;
  }
}

/**
 * @brief  Function called in case of error detected in USART IT Handler
 * @param  None
 * @retval None
 */
void Error_Callback(void)
{
  __IO uint32_t sr_reg;

  /* Disable USARTx_IRQn */
  NVIC_DisableIRQ(USARTx_IRQn);
  
  /* Error handling example :
     - Read USART SR register to identify flag that leads to IT raising
     - Perform corresponding error handling treatment according to flag
  */
  sr_reg = LL_USART_ReadReg(USARTx_INSTANCE, SR);
  if (sr_reg & LL_USART_SR_NE)
    {
      /* case Noise Error flag is raised : ... */
      LED_Blinking(LED_BLINK_FAST);
    }
  else
    {
      /* Unexpected IT source : Set LED to Blinking mode to indicate error occurs */
      LED_Blinking(LED_BLINK_ERROR);
    }
}

/**
 * @brief  Initialize LED2.
 * @param  None
 * @retval None
 */
void LED_Init(void)
{
  /* Enable the LED2 Clock */
  LED2_GPIO_CLK_ENABLE();

  /* Configure IO in output push-pull mode to drive external LED2 */
  LL_GPIO_SetPinMode(LED2_GPIO_PORT, LED2_PIN, LL_GPIO_MODE_OUTPUT);
  /* Reset value is LL_GPIO_OUTPUT_PUSHPULL */
  //LL_GPIO_SetPinOutputType(LED2_GPIO_PORT, LED2_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  /* Reset value is LL_GPIO_SPEED_FREQ_LOW */
  //LL_GPIO_SetPinSpeed(LED2_GPIO_PORT, LED2_PIN, LL_GPIO_SPEED_FREQ_LOW);
  /* Reset value is LL_GPIO_PULL_NO */
  //LL_GPIO_SetPinPull(LED2_GPIO_PORT, LED2_PIN, LL_GPIO_PULL_NO);
}

/**
 * @brief  Turn-on LED2.
 * @param  None
 * @retval None
 */
void LED_On(void)
{
  /* Turn LED2 on */
  LL_GPIO_SetOutputPin(LED2_GPIO_PORT, LED2_PIN);
}

/**
 * @brief  Turn-off LED2.
 * @param  None
 * @retval None
 */
void LED_Off(void)
{
  /* Turn LED2 off */
  LL_GPIO_ResetOutputPin(LED2_GPIO_PORT, LED2_PIN);
}

/**
 * @brief  Set LED2 to Blinking mode for an infinite loop (toggle period based on value provided as input parameter).
 * @param  Period : Period of time (in ms) between each toggling of LED
 *   This parameter can be user defined values. Pre-defined values used in that example are :
 *     @arg LED_BLINK_FAST : Fast Blinking
 *     @arg LED_BLINK_SLOW : Slow Blinking
 *     @arg LED_BLINK_ERROR : Error specific Blinking
 * @retval None
 */
void LED_Blinking(uint32_t Period)
{
  /* Toggle LED2 in an infinite loop */
  while (1)
    {
      LL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);  
      LL_mDelay(Period);
    }
}

/**
 * @brief  Configures User push-button in GPIO or EXTI Line Mode.
 * @param  None 
 * @retval None
 */
void UserButton_Init(void)
{
  /* Enable the BUTTON Clock */
  USER_BUTTON_GPIO_CLK_ENABLE();
  
  /* Configure GPIO for BUTTON */
  LL_GPIO_SetPinMode(USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN, LL_GPIO_PULL_NO);

  /* Connect External Line to the GPIO*/
  USER_BUTTON_SYSCFG_SET_EXTI();

  /* Enable a rising trigger External line 15 to 10 Interrupt */
  USER_BUTTON_EXTI_LINE_ENABLE();
  USER_BUTTON_EXTI_FALLING_TRIG_ENABLE();

  /* Configure NVIC for USER_BUTTON_EXTI_IRQn */
  NVIC_SetPriority(USER_BUTTON_EXTI_IRQn, 3);  
  NVIC_EnableIRQ(USER_BUTTON_EXTI_IRQn); 
}

/**
 * @brief  Function called for achieving TX buffer sending
 * @param  None
 * @retval None
 */
size_t txBuffer(uint8_t*buffer, size_t size)
{
  uint32_t index = 0;

  /* Send characters one per one, until last char to be sent */
  while (index < size)
    {
      /* Wait for TXE flag to be raised */
      while (!LL_USART_IsActiveFlag_TXE(USARTx_INSTANCE))
	{
	}

      /* If last char to be sent, clear TC flag */
      if (index == (size - 1))
	{
	  LL_USART_ClearFlag_TC(USARTx_INSTANCE); 
	}

      /* Write character in Transmit Data register.
	 TXE flag is cleared by writing data in DR register */
      LL_USART_TransmitData8(USARTx_INSTANCE, buffer[index++]);
    }

  /* Wait for TC flag to be raised for last char */
  while (!LL_USART_IsActiveFlag_TC(USARTx_INSTANCE))
    {
    }

  /* Turn LED2 On at end of transfer : Tx sequence completed successfully */
  //LED_On();
  return size;
}
