#include "HalSPI.h"
#include "HalGPIO.h"

#define W25Q64_PAGE_SIZE  4096
#define W25Q64_CHIP_ID 0xEF16
#define W25Q64_CS_PIN 0x1c
#define W25Q64_CS_ENABLE() HalGPIOSetLevel(W25Q64_CS_PIN, 0)
#define W25Q64_CS_DISABLE() HalGPIOSetLevel(W25Q64_CS_PIN, 1)

#define W25X_WriteEnable       0x06 
#define W25X_WriteDisable      0x04 
#define W25X_ReadStatusReg     0x05 
#define W25X_WriteStatusReg    0x01 
#define W25X_ReadData          0x03 
#define W25X_FastReadData      0x0B 
#define W25X_FastReadDual      0x3B 
#define W25X_PageProgram       0x02 
#define W25X_BlockErase        0xD8 
#define W25X_SectorErase       0x20 
#define W25X_ChipErase         0xC7 
#define W25X_PowerDown         0xB9 
#define W25X_ReleasePowerDown  0xAB 
#define W25X_DeviceID          0xAB 
#define W25X_ManufactDeviceID  0x90 
#define W25X_JedecDeviceID     0x9F 

extern uint8_t g_commonBuff[4096];

/************************************************/
//��ȡSPI_FLASH��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
/************************************************/
static uint8_t readSR(void)   
{  
    uint8_t byte=0;   
    W25Q64_CS_ENABLE();                            //ʹ������   
    HalSPIReadWriteByte(W25X_ReadStatusReg);    //���Ͷ�ȡ״̬�Ĵ�������    
    byte=HalSPIReadWriteByte(0Xff);             //��ȡһ���ֽ�  
    W25Q64_CS_DISABLE();                            //ȡ��Ƭѡ     
    return byte;   
} 


//�ȴ�����
static void waitBusy(void)   
{   
    while ((readSR()&0x01)==0x01);   // �ȴ�BUSYλ���
}  


/************************************************/
//дSPI_FLASH״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
/***********************************************
void SPI_FLASH_Write_SR(uint8_t sr)   
{   
    W25Q64_CS_ENABLE();                            //ʹ������   
    HalSPIReadWriteByte(W25X_WriteStatusReg);   //����дȡ״̬�Ĵ�������    
    HalSPIReadWriteByte(sr);               //д��һ���ֽ�  
    W25Q64_CS_DISABLE();                            //ȡ��Ƭѡ             
}   
*/

/************************************************/
//SPI_FLASHдʹ��  
//��WEL��λ   
/************************************************/
void W25Q64WriteEnable(void)   
{
    W25Q64_CS_ENABLE();                            //ʹ������   
    HalSPIReadWriteByte(W25X_WriteEnable);      //����дʹ��  
    W25Q64_CS_DISABLE();                            //ȡ��Ƭѡ             
} 

/************************************************/
//SPI_FLASHд��ֹ  
//��WEL����  
/************************************************/
void W25Q64WriteDisable(void)   
{  
    W25Q64_CS_ENABLE();                            //ʹ������   
    HalSPIReadWriteByte(W25X_WriteDisable);     //����д��ָֹ��    
    W25Q64_CS_DISABLE();                            //ȡ��Ƭѡ             
}

/************************************************/
//��ȡSPI FLASH  
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
/************************************************/
void W25Q64Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)   
{ 
    uint16_t i;
    
    W25Q64_CS_ENABLE(); //ʹ������   
    HalSPIReadWriteByte(W25X_ReadData); //���Ͷ�ȡ����   
    HalSPIReadWriteByte((uint8_t)((ReadAddr)>>16));  //����24bit��ַ    
    HalSPIReadWriteByte((uint8_t)((ReadAddr)>>8));
    HalSPIReadWriteByte((uint8_t)ReadAddr);
    for(i = 0; i < NumByteToRead; i++)
    { 
        pBuffer[i] = HalSPIReadWriteByte(0XFF); //ѭ������  
    }
    W25Q64_CS_DISABLE(); //ȡ��Ƭѡ             
}  

/************************************************/
//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!  
/************************************************/ 
static void writePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t i;  
    W25Q64WriteEnable(); //SET WEL 
    W25Q64_CS_ENABLE();  //ʹ������   
    HalSPIReadWriteByte(W25X_PageProgram); //����дҳ����   
    HalSPIReadWriteByte((uint8_t)((WriteAddr)>>16)); //����24bit��ַ    
    HalSPIReadWriteByte((uint8_t)((WriteAddr)>>8));   
    HalSPIReadWriteByte((uint8_t)WriteAddr);   
    for(i = 0; i < NumByteToWrite; i++)
    {
        HalSPIReadWriteByte(pBuffer[i]);//ѭ��д��  
    }
    W25Q64_CS_DISABLE();                            //ȡ��Ƭѡ 
    waitBusy();                     //�ȴ�д�����
} 

/************************************************/
//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
/************************************************/
static void writeNoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{                    
    uint16_t pageremain;    
    pageremain = 256 - WriteAddr % 256; //��ҳʣ����ֽ���                
    if(NumByteToWrite <= pageremain)
    {
        pageremain = NumByteToWrite;//������256���ֽ�
    }
    while(1)
    {      
        writePage(pBuffer, WriteAddr, pageremain);
        if(NumByteToWrite == pageremain)
        {
            break;//д�������
        }
        else //NumByteToWrite>pageremain
        {
            pBuffer+=pageremain;
            WriteAddr+=pageremain;  

            NumByteToWrite-=pageremain;           //��ȥ�Ѿ�д���˵��ֽ���
            if(NumByteToWrite>256)
                pageremain=256; //һ�ο���д��256���ֽ�
            else pageremain=NumByteToWrite;       //����256���ֽ���
        }
    };      
} 

/************************************************/
//����һ������
//Dst_Addr:������ַ 0~511 for w25x16
//����һ��ɽ��������ʱ��:150ms
static void eraseSector(uint32_t Dst_Addr)   
{   
    Dst_Addr *= W25Q64_PAGE_SIZE;
    W25Q64WriteEnable();                  //SET WEL     
    waitBusy();   
    W25Q64_CS_ENABLE();                            //ʹ������   
    HalSPIReadWriteByte(W25X_SectorErase);      //������������ָ�� 
    HalSPIReadWriteByte((uint8_t)((Dst_Addr)>>16));  //����24bit��ַ    
    HalSPIReadWriteByte((uint8_t)((Dst_Addr)>>8));   
    HalSPIReadWriteByte((uint8_t)Dst_Addr);  
    W25Q64_CS_DISABLE();                            //ȡ��Ƭѡ             
    waitBusy();                     //�ȴ��������
}  

void W25Q64PageWrite(uint8_t *pBuffer, uint32_t page)
{
    eraseSector(page); //�����������
    writeNoCheck(pBuffer, page * W25Q64_PAGE_SIZE, W25Q64_PAGE_SIZE);//д���������� 
}
/************************************************/
//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)      
/************************************************/
void W25Q64Write(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{ 
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;     
    uint16_t i;    
    uint8_t *writeBuff = g_commonBuff;//һ��������С

    secpos = WriteAddr / W25Q64_PAGE_SIZE; //������ַ 0~511 for w25x16
    secoff = WriteAddr % W25Q64_PAGE_SIZE; //�������ڵ�ƫ��
    secremain = W25Q64_PAGE_SIZE - secoff; //����ʣ��ռ��С   

    if(NumByteToWrite <= secremain)
        secremain = NumByteToWrite; //������4096���ֽ�
    while(1) 
    {   
        W25Q64Read(writeBuff, secpos * W25Q64_PAGE_SIZE, W25Q64_PAGE_SIZE); //������������������
        for(i = 0; i < secremain; i++) //У������
        {
            if(writeBuff[secoff + i] != 0XFF)
            {
                break; //��Ҫ����     
            }
        }
        
        if(i < secremain) //��Ҫ����
        {
            eraseSector(secpos); //�����������
            for(i = 0; i < secremain; i++)       //����
            {
                writeBuff[i + secoff] = pBuffer[i];   
            }
            writeNoCheck(writeBuff, secpos * W25Q64_PAGE_SIZE, W25Q64_PAGE_SIZE);//д����������  
        }
        else 
        {
            writeNoCheck(pBuffer, WriteAddr, secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 
        }
        
        if(NumByteToWrite == secremain)
        {
            break;//д�������
        }
        else//д��δ����
        {
            secpos++;//������ַ��1
            secoff = 0;//ƫ��λ��Ϊ0    

            pBuffer += secremain;  //ָ��ƫ��
            WriteAddr += secremain;//д��ַƫ��       
            NumByteToWrite -= secremain;              //�ֽ����ݼ�
            if(NumByteToWrite > W25Q64_PAGE_SIZE)
            {
                secremain = W25Q64_PAGE_SIZE;  //��һ����������д����
            }
            else 
            {
                secremain = NumByteToWrite;          //��һ����������д����
            }
        }    
    };       
}

/************************************************/
//��������оƬ
//��Ƭ����ʱ��:
//W25X16:25s 
//W25X32:40s 
//W25X64:40s 
//�ȴ�ʱ�䳬��...
/************************************************/
void W25Q64EraseChip(void)   
{                                             
    W25Q64WriteEnable();                  //SET WEL 
    waitBusy();   
    W25Q64_CS_ENABLE();                            //ʹ������   
    HalSPIReadWriteByte(W25X_ChipErase);        //����Ƭ��������  
    W25Q64_CS_DISABLE();                            //ȡ��Ƭѡ             
    waitBusy();                     //�ȴ�оƬ��������
}   

//�������ģʽ
void W25Q64PowerDown(void)   
{ 
    W25Q64_CS_ENABLE();                            //ʹ������   
    HalSPIReadWriteByte(W25X_PowerDown);        //���͵�������  
    W25Q64_CS_DISABLE();                            //ȡ��Ƭѡ             
    //delayUs(3);                               //�ȴ�TPD  
}   

//����
void W25Q64Wakeup(void)   
{  
    W25Q64_CS_ENABLE();                            //ʹ������   
    HalSPIReadWriteByte(W25X_ReleasePowerDown);   //  send W25X_PowerDown command 0xAB    
    W25Q64_CS_DISABLE();                            //ȡ��Ƭѡ               
    //delayUs(3);                               //�ȴ�TRES1
}   

static uint16_t readID(void)
{
    uint16_t id = 0;     
    W25Q64_CS_ENABLE();                 
    HalSPIReadWriteByte(0x90);//���Ͷ�ȡID����     
    HalSPIReadWriteByte(0x00);       
    HalSPIReadWriteByte(0x00);       
    HalSPIReadWriteByte(0x00);                  
    id = HalSPIReadWriteByte(0xFF)<<8;  
    id += HalSPIReadWriteByte(0xFF);  
    W25Q64_CS_DISABLE();                 
    return id;
}

uint16_t W25Q64Initialize(void)
{
    HalGPIOConfig(W25Q64_CS_PIN, HAL_IO_OUTPUT);

    W25Q64Wakeup();
    if(W25Q64_CHIP_ID == readID())
    {
        return 0;
    }
    return HAL_EXCEPTION_ID_EXFLASH;
}

void W25Q64Poll(void)
{
}

