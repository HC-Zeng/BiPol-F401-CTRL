//
// Created by HC on 2025/9/21.
//

#ifndef BIPOL_F401_CTRL_DEBUG_PRINTF_H
#define BIPOL_F401_CTRL_DEBUG_PRINTF_H

#include "stdio.h"
#include "usart.h"
#include "stm32f4xx_hal.h"  // 包含HAL库

#ifdef __GNUC__

#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

//PUTCHAR_PROTOTYPE
//{
//    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
//    return ch;
//}
#endif

#endif //BIPOL_F401_CTRL_DEBUG_PRINTF_H
