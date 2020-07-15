#ifndef __STM32F4xx_LL_USB_H
#define __STM32F4xx_LL_USB_H

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef USB_StopHostChannel(USB_OTG_GlobalTypeDef *USBx, uint8_t chnum);
HAL_StatusTypeDef HAL_HCD_StopHC(HCD_HandleTypeDef *hhcd, uint8_t chnum);