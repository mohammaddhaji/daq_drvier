#include <stdint.h>

#ifndef UART_DRIVER
#define UART_DRIVER



#define UART1_485_422   "/dev/ttyS0"
#define UART1_232       "/dev/ttyS0"
#define UART2_232       "/dev/ttyS1"
#define UART4_485_422   "/dev/ttyS3"
#define UART4_232       "/dev/ttyS3"
#define UART5_232       "/dev/ttyS4"
#define UART6_485_422   "/dev/ttyS5"
#define UART6_232       "/dev/ttyS5"
#define UART7_232       "/dev/ttyS6"
#define UART8_485_422   "/dev/ttyS7"
#define UART8_232       "/dev/ttyS7"
#define UART9_232       "/dev/ttyS8"


// X: (485/422)
// Y: (232)
#define UART_1X_4X_6X_8X              0                 
#define UART_1X_4X_6X_8Y_9Y           1                 
#define UART_1X_4X_6Y_7Y_8X           2                 
#define UART_1X_4X_6Y_7Y_8Y_9Y        3                 
#define UART_1X_4Y_5Y_6X_8X           4                 
#define UART_1X_4Y_5Y_6X_8Y_9Y        5                 
#define UART_1X_4Y_5Y_6Y_7Y_8X        6                 
#define UART_1X_4Y_5Y_6Y_7Y_8Y_9Y     7                 
#define UART_1Y_2Y_4X_6X_8X           8                 
#define UART_1Y_2Y_4X_6X_8Y_9Y        9                 
#define UART_1Y_2Y_4X_6Y_7Y_8X        10                
#define UART_1Y_2Y_4X_6Y_7Y_8Y_9Y     11                
#define UART_1Y_2Y_4Y_5Y_6X_8X        12                
#define UART_1Y_2Y_4Y_5Y_6X_8Y_9Y     13                
#define UART_1Y_2Y_4Y_5Y_6Y_7Y_8X     14                
#define UART_1Y_2Y_4Y_5Y_6Y_7Y_8Y_9Y  15                


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