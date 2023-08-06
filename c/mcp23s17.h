#include <stdint.h>


#ifndef MCP23S17DRIVER
#define MCP23S17DRIVER


#define PULLUP_ENABLED      0
#define PULLUP_DISABLED     1


#define DIR_INPUT   0
#define DIR_OUTPUT  1


#define LEVEL_LOW   0
#define LEVEL_HIGH  1


#define IOE_1   0
#define IOE_2   1

#define ALL_PINS    0xFF

#define PIN_0   0
#define PIN_1   1
#define PIN_2   2
#define PIN_3   3
#define PIN_4   4
#define PIN_5   5
#define PIN_6   6
#define PIN_7   7
#define PIN_8   8
#define PIN_9   9
#define PIN_10  10
#define PIN_11  11
#define PIN_12  12
#define PIN_13  13
#define PIN_14  14
#define PIN_15  15



/**
 * @brief Opens SPI device, request GPIO lines and init IOEs registers.
 * 
 * @return 0 on success, negative error code on failure.
 */
int MCP23S17_Open(void);


/**
 * @brief Close SPI device, release GPIO lines.
 */
void MCP23S17_Close(void);


/**
 * @brief Sets pullup mode of a pin or all pins.
 * 
 * @param ioe: IOE number:
 *          IOE_1
 *          IOE_2
 * 
 * @param pin: Selected pin. Must be a number between 0 to 15.
 *       Any number greater than 15 will set pullup mode for all pins.
 *       ALL_PINS flag can be used for this purpose.
 * 
 * @param mode: Must be one of the below flags:
 *          PULLUP_ENABLED
 *          PULLUP_DISABLED
 * 
 * @return 0 on success, -1 on failure.
 */
int MCP23S17_Set_Pullup_Mode(uint8_t ioe, uint8_t pin, uint8_t mode);


/**
 * @brief Sets direction of a pin or all pins.
 * 
 * @param ioe: IOE number:
 *          IOE_1
 *          IOE_2
 * 
 * @param pin: Selected pin. Must be a number between 0 to 15.
 *       Any number greater than 15 will set direction for all pins.
 *       ALL_PINS flag can be used for this purpose.
 * 
 * @param direction: Must be one of the below flags:
 *          DIR_INPUT
 *          DIR_OUTPUT
 * 
 * @return 0 on success, -1 on failure.
 */
int MCP23S17_Set_Direction(uint8_t ioe, uint8_t pin, uint8_t direction);


/**
 * @brief Reads digital level of a pin.
 * 
 * @param ioe: IOE number:
 *          IOE_1
 *          IOE_2
 * 
 * @param pin: Selected pin. Must be a number between 0 to 15.
 * 
 * @return: 0 for LEVEL_LOW, 1 for LEVEL_HIGH and -1 on failure.
 */
int MCP23S17_Get_Level(uint8_t ioe, uint8_t pin);


/**
 * @brief Sets digital level of a pin.
 * 
 * @param ioe: IOE number:
 *          IOE_1
 *          IOE_2
 * 
 * @param pin: Selected pin. Must be a number between 0 to 15.
 * 
 * @param level: Must be one of the below flags:
 *          LEVEL_LOW
 *          LEVEL_HIGH
 * 
 * @return 0 on success, -1 on failure.
 */
int MCP23S17_Set_Level(uint8_t ioe, uint8_t pin, uint8_t level);


/**
 * @brief Writes data on GPIOs.
 * 
 * @param ioe: IOE number:
 *          IOE_1
 *          IOE_2
 * 
 * @param data: Data to write on GPIOs. Must be a number
 *          between 0x0000 to 0xFFFF.
 * 
 * @return 0 on success, -1 on failure.
 */
int MCP23S17_Set_Data(uint8_t ioe, uint16_t data);


/**
 * @brief Reads data from GPIOs.
 * 
 * @param ioe: IOE number:
 *          IOE_1
 *          IOE_2
 * 
 * @return Data on GPIOs. 
 */
uint16_t MCP23S17_Get_Data(uint8_t ioe);


#endif // MCP23S17DRIVER
