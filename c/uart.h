#include <stdint.h>

#ifndef UART_DRIVER
#define UART_DRIVER



#define UART1_485_422   "/dev/ttyS0"
#define UART2_232       "/dev/ttyS1"
#define UART4_485_422   "/dev/ttyS3"
#define UART5_232       "/dev/ttyS4"
#define UART6_485_422   "/dev/ttyS5"
#define UART7_232       "/dev/ttyS6"
#define UART8_485_422   "/dev/ttyS7"
#define UART9_232       "/dev/ttyS8"


#define UART_1_4_6_8    1
#define UART_1_4_6_9    2
#define UART_1_4_7_8    3
#define UART_1_4_7_9    4
#define UART_1_5_6_8    5
#define UART_1_5_6_9    6
#define UART_1_5_7_8    7
#define UART_1_5_7_9    8
#define UART_2_4_6_8    9
#define UART_2_4_6_9    10
#define UART_2_4_7_8    11
#define UART_2_4_7_9    12
#define UART_2_5_6_8    13
#define UART_2_5_6_9    14
#define UART_2_5_7_8    15
#define UART_2_5_7_9    16



/**
 * @brief This function requests GPIO control lines.
 *        
 * @param mode: Initial mode for UART ports.
 *        Must be one of above flags.
 * 
 * @return 0 on success, negative error code on failure.
 */
int UART_Open(uint8_t mode);


/**
 * @brief This function changes mode for UART ports.
 *        
 * @param mode: mode for UART ports.
 *        Must be one of above flags.
 */
void UART_Change_Mode(uint8_t mode);


/**
 * @brief This function releases GPIO control lines.
 */
void UART_Close();


#endif // UART_DRIVER