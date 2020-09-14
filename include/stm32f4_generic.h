#ifndef __STM32F4_DISCOVERY_H
#define __STM32F4_DISCOVERY_H

#ifdef __cplusplus
 extern "C" {
#endif
                                              
#include "stm32f4xx_hal.h"
   
typedef enum 
{
  LED1 = 0,
  LED2 = 1
} Led_TypeDef;

#define LEDn                             2

#define LED1_PIN                         GPIO_PIN_13
#define LED1_GPIO_PORT                   GPIOC
#define LED1_GPIO_CLK_ENABLE()           __GPIOC_CLK_ENABLE()  
#define LED1_GPIO_CLK_DISABLE()          __GPIOC_CLK_DISABLE()  

#define LED2_PIN                         GPIO_PIN_2
#define LED2_GPIO_PORT                   GPIOB
#define LED2_GPIO_CLK_ENABLE()           __GPIOB_CLK_ENABLE()  
#define LED2_GPIO_CLK_DISABLE()          __GPIOB_CLK_DISABLE()   

#define LEDx_GPIO_CLK_ENABLE(__INDEX__) do{if((__INDEX__) == 0) LED1_GPIO_CLK_ENABLE(); else \
                                           if((__INDEX__) == 1) LED2_GPIO_CLK_ENABLE(); \
                                           }while(0)

#define LEDx_GPIO_CLK_DISABLE(__INDEX__) do{if((__INDEX__) == 0) LED1_GPIO_CLK_DISABLE(); else \
                                            if((__INDEX__) == 1) LED2_GPIO_CLK_DISABLE(); \
                                           }while(0)

#define USARTx                       	 USART2

#define USARTx_CLK_ENABLE()              __USART2_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()

#define USARTx_TX_PIN                    GPIO_PIN_2
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART2
#define USARTx_RX_PIN                    GPIO_PIN_3
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF7_USART2

void BSP_LED_Init(Led_TypeDef Led);
void BSP_LED_On(Led_TypeDef Led);
void BSP_LED_Off(Led_TypeDef Led);
void BSP_LED_Toggle(Led_TypeDef Led);


#ifdef __cplusplus
}
#endif

#endif /* __STM32F4_DISCOVERY_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
