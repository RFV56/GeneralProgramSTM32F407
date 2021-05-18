#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "stm32f4xx_conf.h"
#include "sys.h"

#define USART_PORT USART1
#define USART_GPIO GPIOA
#define USART_PIN_TX GPIO_Pin_9
#define USART_PIN_RX GPIO_Pin_10
#define USARt_PIN_SOURCE_TX GPIO_PinSource9
#define USARt_PIN_SOURCE_RX GPIO_PinSource10
#define USART_AF GPIO_AF_USART1
#define USART_IRQ USART1_IRQn
#define USART_IRQHandler USART1_IRQHandler
#define RCC_ENABLE_GPIO() RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); //使能GPIOA时钟
#define RCC_ENABLE_AF() RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  //使能USART1时钟

#define USART_REC_LEN 200 //定义最大接收字节数 200
#define EN_USART_RX 1     //使能（1）/禁止（0）串口1接收

extern u8 USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern u16 USART_RX_STA;               //接收状态标记

void uart_init(u32 bound);
#endif
