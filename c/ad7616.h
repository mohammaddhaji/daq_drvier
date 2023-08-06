#include <stdint.h>


#ifndef AD7616DRIVER
#define AD7616DRIVER


typedef struct{
 	uint8_t RawAB[32];
	float   VA[8];
	float   VB[8];
}ADC_Channels;


#define AD7616_OverSampling_Disabled    0x0
#define AD7616_OverSampling_2           0x1
#define AD7616_OverSampling_4           0x2
#define AD7616_OverSampling_8           0x3
#define AD7616_OverSampling_16          0x4
#define AD7616_OverSampling_32          0x5
#define AD7616_OverSampling_64          0x6
#define AD7616_OverSampling_128         0x7


#define AD7616_Channel_V0       0x0
#define AD7616_Channel_V1       0x1
#define AD7616_Channel_V2       0x2
#define AD7616_Channel_V3       0x3
#define AD7616_Channel_V4       0x4
#define AD7616_Channel_V5       0x5
#define AD7616_Channel_V6       0x6
#define AD7616_Channel_V7       0x7
#define AD7616_Channel_AVCC     0x8
#define AD7616_Channel_ALDO     0x9


#define AD7616_InputRange_2V5_PN     0x1
#define AD7616_InputRange_5V0_PN     0x2
#define AD7616_InputRange_10V_PN     0x3

#define ADC_1   0
#define ADC_2   1



/**
 * @brief Opens SPI devices, request GPIO lines and init adc registers.
 * 
 * @return 0 on success, negative error code on failure.
 */
int AD7616_Open(void);


/**
 * @brief Close SPI devices and release GPIO lines.
 */
void AD7616_Close(void);


/**
 * @brief Sets input range for all channels.
 * 
 * @param adc: ADC number:
 *           ADC_1
 *           ADC_2
 * 
 * @param range: Must be one of the below flags:
 *           AD7616_InputRange_2V5_PN
 *           AD7616_InputRange_5V0_PN
 *           AD7616_InputRange_10V_PN
 * 
 * @return 0 on success, -1 on failure.
 */
int AD7616_Set_Input_Range_All(uint8_t adc, uint8_t range);


/**
 * @brief Sets input range for a single channel.
 * 
 * @param adc: ADC number:
 *           ADC_1
 *           ADC_2
 * 
 * @param channel: Channel number.
 * 
 * @param range: Must be one of the below flags:
 *           AD7616_InputRange_2V5_PN
 *           AD7616_InputRange_5V0_PN
 *           AD7616_InputRange_10V_PN
 * 
 * @return 0 on success, -1 on failure.
 */
int AD7616_Set_Input_Range(uint8_t adc, uint8_t channel, uint8_t range);


/**
 * @brief Sets oversampling ratio for adc.
 * 
 * @param adc: ADC number:
 *           ADC_1
 *           ADC_2
 * 
 * @param oversampling: Must be one of the below flags:
 *           AD7616_OverSampling_Disabled
 *           AD7616_OverSampling_2
 *           AD7616_OverSampling_4
 *           AD7616_OverSampling_8
 *           AD7616_OverSampling_16
 *           AD7616_OverSampling_32
 *           AD7616_OverSampling_64
 *           AD7616_OverSampling_128
 * 
 * @return 0 on success, -1 on failure.
 */
int AD7616_Set_Oversampling(uint8_t adc, uint8_t oversampling);


/**
 * @brief Gets oversampling ratio for adc.
 * 
 * @param adc: ADC number:
 *           ADC_1
 *           ADC_2
 * 
 * @return Returns one of the below flags:
 *           AD7616_OverSampling_Disabled
 *           AD7616_OverSampling_2
 *           AD7616_OverSampling_4
 *           AD7616_OverSampling_8
 *           AD7616_OverSampling_16
 *           AD7616_OverSampling_32
 *           AD7616_OverSampling_64
 *           AD7616_OverSampling_128
 */
uint8_t AD7616_Get_Oversampling(uint8_t adc);


/**
 * @brief Read all voltages.
 * 
 * @param adc: ADC number:
 *           ADC_1
 *           ADC_2
 * 
 * @param channels: A pointer to ADC_Channels struct for storing voltages values.
 * 
 * @return 0 on success, -1 on failure.
 */
int AD7616_Read_All_Voltages(uint8_t adc, ADC_Channels* channels);


#endif // AD7616_DRIVER