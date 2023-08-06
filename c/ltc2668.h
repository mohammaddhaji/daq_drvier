#include <stdint.h>


#ifndef LTC2667DRIVER
#define LTC2667DRIVER


#define  LTC2668_SPAN_0_TO_5V             0x0000
#define  LTC2668_SPAN_0_TO_10V            0x0001
#define  LTC2668_SPAN_PLUS_MINUS_5V       0x0002
#define  LTC2668_SPAN_PLUS_MINUS_10V      0x0003
#define  LTC2668_SPAN_PLUS_MINUS_2V5      0x0004


#define REF_EXTERNAL    0x0001
#define REF_INTERNAL    0x0000


#define ALL_CHANNELS    0xFF


/**
 * @brief Opens SPI device, request GPIO lines and power up dac.
 * 
 * @return 0 on success, negative error code on failure.
 */
int LTC2668_Open(void);


/**
 * @brief Close SPI device, release GPIO lines and power down dac.
 */
void LTC2668_Close(void);


/**
 * @brief Sets reference mode for dac.
 * 
 * @param ref: Must be one of the below flags:
 *          REF_EXTERNAL
 *          REF_INTERNAL
 * 
 * @return 0 on success, -1 on failure.
 */
int LTC2668_Set_Reference_Mode(uint8_t ref);


/**
 * @brief Sets softspan range for a channel or all channels.
 * 
 * @param channel: Selected channel. Must be a number between 0 to 15.        
 *           Any number greater than 15 will set softspan range for all channels.
 *           ALL_CHANNELS flag can be used for this purpose.
 * 
 * @param softspan_range: Must be one of the below flags:
 *          LTC2668_SPAN_0_TO_5V
 *          LTC2668_SPAN_0_TO_10V
 *          LTC2668_SPAN_PLUS_MINUS_5V
 *          LTC2668_SPAN_PLUS_MINUS_10V
 *          LTC2668_SPAN_PLUS_MINUS_2V5
 * 
 * @return 0 on success, -1 on failure.
 */
int LTC2668_Set_Softspan_Range(uint8_t channel, uint8_t softspan_range);


/**
 * @brief Sets output voltage for a channel or all channels.
 * 
 * @param channel: Selected channel. Must be a number between 0 to 15.        
 *           Any number greater than 15 will set output voltage for all channels.
 *           ALL_CHANNELS flag can be used for this purpose.
 *
 * @param voltage: It must be chosen with respect to the minimum and maximum of
 *           the selected channel softspan.
 * 
 *           Note: If there are channles with different softspan range,
 *                 setting voltage for all channels will fail.
 * 
 * @return 0 on success, -1 on failure.
 */
int LTC2668_Set_Output_Voltage(uint8_t channel, float voltage);


#endif // LTC2667DRIVER