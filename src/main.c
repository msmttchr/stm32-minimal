#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32l1xx.h"
#include "stm32l1xx_conf.h"
#include "system_def.h"
#include "scpi/scpi.h"
#include "scpi-def.h"
#include "periph.h"

void     SystemClock_Config(void);
size_t SCPI_Write(scpi_t * context, const char * data, size_t len) {
  (void) context;
  return USART_TxBuffer((uint8_t *)data, len);
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

void SystemClock_Config(void){

  /* Clock init stuff */ 
    
  LL_UTILS_PLLInitTypeDef sUTILS_PLLInitStruct = {LL_RCC_PLL_MUL_3, LL_RCC_PLL_DIV_3};
  LL_UTILS_ClkInitTypeDef sUTILS_ClkInitStruct = {LL_RCC_SYSCLK_DIV_1, LL_RCC_APB1_DIV_1, LL_RCC_APB2_DIV_1};
    
  LL_PLL_ConfigSystemClock_HSI(&sUTILS_PLLInitStruct, &sUTILS_ClkInitStruct);
    
  LL_Init1msTick(SystemCoreClock);
}

int main(void){
  const char *welcome_message = "RF Switch 7 Channels (A-G) " SCPI_IDN2 " version " SCPI_IDN4 "\r\n(c) 2021 STMictroelectronics RF Team\r\n";


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
#ifdef UART_ECHO
  /* Echo received character on TX */
  LL_USART_TransmitData8(USARTx_INSTANCE, line[index-1]);
#endif
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

