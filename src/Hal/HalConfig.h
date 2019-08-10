#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H


#define HAL_EXCEPTION_ID_RTC     0x0001
#define HAL_EXCEPTION_ID_EXFLASH 0x0002

#define HAL_FLASH_INVALID_ADDR 0xffffffff

#define HAL_DEVICE_TYPE 1

typedef enum
{
    HAL_SENSOR_ID_GEOMAGNETISM = 0, //�ش�
    HAL_SENSOR_ID_NOISE,            //����
    HAL_SENSOR_ID_PRESS1,           //Ӧ��1
    HAL_SENSOR_ID_PRESS2,           //Ӧ��2
    HAL_SENSOR_ID_COUNT,
}HalSensorID_t;


#define HAL_IO_UART_PIN  0x3c  //ģ�⴮��IO PD12
#define HAL_LED1_PIN 0x41 //PE1
#define HAL_SENSORS_POWER_PIN 0x2d //PC13
#define HAL_485_POWER_PIN 0x0c //PA12
#define HAL_IR_POWER_PIN 0x42


//#define HAL_ADC_CH_NUM 4
#define HAL_DAC_BASE_VALUE 1830 //1.51, 1.515v = 1892
#define HAL_DAC_STEP_VALUE 62   //0.05v

#define KB(x) ((x)*1024)
/*flash����
* 0 ~ 5k     :boot
* 5k ~ 125k  :app
* 125k ~ 245k:ota
* 245k ~ 256k:args
*/
#define HAL_FLASH_SIZE (KB(256))  //256KB 120k(ota), 4k(boot) 4+240+12
#define HAL_FLASH_PAGE_SIZE (KB(2))
#define HAL_FLASH_OTA_SIZE (KB(120))

#define HAL_FLASH_BASE_ADDR  0x8000000
#define HAL_BOOT_FLASH_ADDR  (HAL_FLASH_BASE_ADDR + 0)
#define HAL_APP_FLASH_ADDR   (HAL_FLASH_BASE_ADDR + KB(4))
#define HAL_OTA_FLASH_ADDR   (HAL_FLASH_BASE_ADDR + KB(124))
#define HAL_ARGS_FLASH_ADDR  (HAL_FLASH_BASE_ADDR + KB(230))

#define HAL_DEVICE_ARGS_ADDR (HAL_ARGS_FLASH_ADDR + 0) //�豸����
#define HAL_DATA_RECORD_ADDR (HAL_ARGS_FLASH_ADDR + KB(2)) //������¼
#define HAL_DATA_POINT_INFO_ADDR (HAL_ARGS_FLASH_ADDR + KB(4)) //��λ��Ϣ

#define HAL_DATA_POINT_INFO_PAGE 11 //11 page
#endif

