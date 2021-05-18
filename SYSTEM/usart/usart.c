#include "sys.h"
#include "usart.h"

//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}

#if EN_USART_RX //如果使能了接收
//串口中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误
u8 USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA = 0; //接收状态标记

/******************************************************
* 单位		:深圳大学物光创新实验室
* 作者      :赖怀磊
* 编写时间  :2021年5月16日
* 函数名	:fputc
* 函数功能  :标准库printf调用
* 输入		:无
* 返回值	:无
*******************************************************/
int fputc(int ch, FILE *f)
{
	USART_SendData(USART_PORT, (u8)ch);
	while (USART_GetFlagStatus(USART_PORT, USART_FLAG_TXE) == RESET)
		;
	return ch;
}
#endif

/******************************************************
* 单位	    :深圳大学物光创新实验室
* 作者      :赖怀磊
* 编写时间  :2021年5月16日
* 函数名	:uart_init
* 函数功能  :串口初始化
* 输入		:bound 波特率
* 返回值	:无
*******************************************************/
void uart_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_ENABLE_GPIO(); //使能GPIO时钟
	RCC_ENABLE_AF();   //使能USART时钟

	GPIO_PinAFConfig(USART_GPIO, USARt_PIN_SOURCE_TX, USART_AF); //GPIO引脚复用为USART
	GPIO_PinAFConfig(USART_GPIO, USARt_PIN_SOURCE_RX, USART_AF); //GPIO引脚复用为USART

	GPIO_InitStructure.GPIO_Pin = USART_PIN_TX | USART_PIN_RX; //收发引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			   //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			   //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			   //上拉
	GPIO_Init(USART_GPIO, &GPIO_InitStructure);				   //初始化GPIO引脚

	USART_InitStructure.USART_BaudRate = bound;										//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式
	USART_Init(USART_PORT, &USART_InitStructure);									//初始化串口

	USART_Cmd(USART_PORT, ENABLE); //使能串口

#if EN_USART_RX
	USART_ITConfig(USART_PORT, USART_IT_RXNE, ENABLE); //开启相关中断

	NVIC_InitStructure.NVIC_IRQChannel = USART_IRQ;			  //串口中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  //根据指定的参数初始化VIC寄存器、

#endif
}

/******************************************************
* 单位	    :深圳大学物光创新实验室
* 作者      :赖怀磊
* 编写时间  :2021年5月16日
* 函数名	:USART_IRQHandler
* 函数功能  :串口中断服务函数
* 输入		:bound 波特率
* 返回值	:无
*******************************************************/
void USART_IRQHandler(void)
{
	u8 Res;
	if (USART_GetITStatus(USART_PORT, USART_IT_RXNE) != RESET) //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res = USART_ReceiveData(USART_PORT); //(USART1->DR);	//读取接收到的数据

		if ((USART_RX_STA & 0x8000) == 0) //接收未完成
		{
			if (USART_RX_STA & 0x4000) //接收到了0x0d
			{
				if (Res != 0x0a)
					USART_RX_STA = 0; //接收错误,重新开始
				else
					USART_RX_STA |= 0x8000; //接收完成了
			}
			else //还没收到0X0D
			{
				if (Res == 0x0d)
					USART_RX_STA |= 0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
					USART_RX_STA++;
					if (USART_RX_STA > (USART_REC_LEN - 1))
						USART_RX_STA = 0; //接收数据错误,重新开始接收
				}
			}
		}
	}
}
#endif
