#include "sys.h"
#include "usart.h"

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
//��׼����Ҫ��֧�ֺ���
struct __FILE
{
	int handle;
};

FILE __stdout;
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
void _sys_exit(int x)
{
	x = x;
}

#if EN_USART_RX //���ʹ���˽���
//�����жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
u8 USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA = 0; //����״̬���

/******************************************************
* ��λ		:���ڴ�ѧ��ⴴ��ʵ����
* ����      :������
* ��дʱ��  :2021��5��16��
* ������	:fputc
* ��������  :��׼��printf����
* ����		:��
* ����ֵ	:��
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
* ��λ	    :���ڴ�ѧ��ⴴ��ʵ����
* ����      :������
* ��дʱ��  :2021��5��16��
* ������	:uart_init
* ��������  :���ڳ�ʼ��
* ����		:bound ������
* ����ֵ	:��
*******************************************************/
void uart_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_ENABLE_GPIO(); //ʹ��GPIOʱ��
	RCC_ENABLE_AF();   //ʹ��USARTʱ��

	GPIO_PinAFConfig(USART_GPIO, USARt_PIN_SOURCE_TX, USART_AF); //GPIO���Ÿ���ΪUSART
	GPIO_PinAFConfig(USART_GPIO, USARt_PIN_SOURCE_RX, USART_AF); //GPIO���Ÿ���ΪUSART

	GPIO_InitStructure.GPIO_Pin = USART_PIN_TX | USART_PIN_RX; //�շ�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			   //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			   //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			   //����
	GPIO_Init(USART_GPIO, &GPIO_InitStructure);				   //��ʼ��GPIO����

	USART_InitStructure.USART_BaudRate = bound;										//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//�շ�ģʽ
	USART_Init(USART_PORT, &USART_InitStructure);									//��ʼ������

	USART_Cmd(USART_PORT, ENABLE); //ʹ�ܴ���

#if EN_USART_RX
	USART_ITConfig(USART_PORT, USART_IT_RXNE, ENABLE); //��������ж�

	NVIC_InitStructure.NVIC_IRQChannel = USART_IRQ;			  //�����ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  //����ָ���Ĳ�����ʼ��VIC�Ĵ�����

#endif
}

/******************************************************
* ��λ	    :���ڴ�ѧ��ⴴ��ʵ����
* ����      :������
* ��дʱ��  :2021��5��16��
* ������	:USART_IRQHandler
* ��������  :�����жϷ�����
* ����		:bound ������
* ����ֵ	:��
*******************************************************/
void USART_IRQHandler(void)
{
	u8 Res;
	if (USART_GetITStatus(USART_PORT, USART_IT_RXNE) != RESET) //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res = USART_ReceiveData(USART_PORT); //(USART1->DR);	//��ȡ���յ�������

		if ((USART_RX_STA & 0x8000) == 0) //����δ���
		{
			if (USART_RX_STA & 0x4000) //���յ���0x0d
			{
				if (Res != 0x0a)
					USART_RX_STA = 0; //���մ���,���¿�ʼ
				else
					USART_RX_STA |= 0x8000; //���������
			}
			else //��û�յ�0X0D
			{
				if (Res == 0x0d)
					USART_RX_STA |= 0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
					USART_RX_STA++;
					if (USART_RX_STA > (USART_REC_LEN - 1))
						USART_RX_STA = 0; //�������ݴ���,���¿�ʼ����
				}
			}
		}
	}
}
#endif
