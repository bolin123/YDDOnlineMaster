#include "HalSPI.h"

uint8_t HalSPIReadWriteByte(uint8_t txData)
{       
    uint8_t retry = 0;                 
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
    {
        retry++;
        if(retry>200)
            return 0;
    }             
    SPI_I2S_SendData(SPI2, txData); //ͨ������SPIx����һ������

    retry = 0;
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
    {
        retry++;
        if(retry>200)
            return 0;
    }                               
    return SPI_I2S_ReceiveData(SPI2); //����ͨ��SPIx������յ�����                     
}

void HalSPIInitialize(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);

    //SPI �ٶ�����
    //SPI_BaudRatePrescaler_2   2��Ƶ   (SPI 36M@sys 72M)
    //SPI_BaudRatePrescaler_8   8��Ƶ   (SPI 9M@sys 72M)
    //SPI_BaudRatePrescaler_16  16��Ƶ  (SPI 4.5M@sys 72M)
    //SPI_BaudRatePrescaler_256 256��Ƶ (SPI 281.25K@sys 72M)

    SPI_InitTypeDef  SPI_InitStructure;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4 ; //18M
    SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;       //����SPI����ģʽ:����Ϊ��SPI
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;       //����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;     //ѡ���˴���ʱ�ӵ���̬:ʱ�����ո�
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;    //���ݲ���(����)�ڵڶ���ʱ����
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;       //NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;        //���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;  //ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial     = 7;    //CRCֵ����Ķ���ʽ
    SPI_Init(SPI2, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

    SPI_Cmd(SPI2, ENABLE); //ʹ��SPI����

    HalSPIReadWriteByte(0xff);//��������      

}

void HalSPIPoll(void)
{
}

