#include <stdio.h>
#include <gpiod.h>
#include "uart.h"


// GPIO Stuff -----------------------------------------------------------------------------

#ifndef	CONSUMER
#define	CONSUMER    "UART_DRIVER"
#endif

static struct gpiod_chip *chip8;
static struct gpiod_chip *chip7;
static struct gpiod_chip *chip5;
static struct gpiod_chip *chip1;
static struct gpiod_line *UART_CTRL_1_485_232_1;
static struct gpiod_line *UART_CTRL_1_485_232_2;
static struct gpiod_line *UART_CTRL_1_LB;
static struct gpiod_line *UART_CTRL_1_RXEN1;
static struct gpiod_line *UART_CTRL_1_DXEN1;
static struct gpiod_line *UART_CTRL_1_DXEN2;
static struct gpiod_line *UART_CTRL_1_RXEN2;
static struct gpiod_line *UART_CTRL_2_485_232_1;
static struct gpiod_line *UART_CTRL_2_485_232_2;
static struct gpiod_line *UART_CTRL_2_LB;
static struct gpiod_line *UART_CTRL_2_RXEN1;
static struct gpiod_line *UART_CTRL_2_DXEN1;
static struct gpiod_line *UART_CTRL_2_DXEN2;
static struct gpiod_line *UART_CTRL_2_RXEN2;

// control pins digital level
const int uart_modes[][4] = {
    [UART_1X_4X_6X_8X]             = {1, 1, 1, 1},
    [UART_1X_4X_6X_8Y_9Y]          = {1, 1, 1, 0},
    [UART_1X_4X_6Y_7Y_8X]          = {1, 1, 0, 1},
    [UART_1X_4X_6Y_7Y_8Y_9Y]       = {1, 1, 0, 0},
    [UART_1X_4Y_5Y_6X_8X]          = {1, 0, 1, 1},
    [UART_1X_4Y_5Y_6X_8Y_9Y]       = {1, 0, 1, 0},
    [UART_1X_4Y_5Y_6Y_7Y_8X]       = {1, 0, 0, 1},
    [UART_1X_4Y_5Y_6Y_7Y_8Y_9Y]    = {1, 0, 0, 0},
    [UART_1Y_2Y_4X_6X_8X]          = {0, 1, 1, 1},
    [UART_1Y_2Y_4X_6X_8Y_9Y]       = {0, 1, 1, 0},
    [UART_1Y_2Y_4X_6Y_7Y_8X]       = {0, 1, 0, 1},
    [UART_1Y_2Y_4X_6Y_7Y_8Y_9Y]    = {0, 1, 0, 0},
    [UART_1Y_2Y_4Y_5Y_6X_8X]       = {0, 0, 1, 1},
    [UART_1Y_2Y_4Y_5Y_6X_8Y_9Y]    = {0, 0, 1, 0},
    [UART_1Y_2Y_4Y_5Y_6Y_7Y_8X]    = {0, 0, 0, 1},
    [UART_1Y_2Y_4Y_5Y_6Y_7Y_8Y_9Y] = {0, 0, 0, 0},
};

static bool init_done = false;

#define CHECK_AND_GOTO_ON_FAILURE(expr, label) \
    do { \
        if (!(expr)) { \
            goto label; \
        } \
    } while (0)


struct gpiod_line* REQ_GPIO(struct gpiod_chip* chip, int line, int value) {

    struct gpiod_line* gpio = gpiod_chip_get_line(chip, line);
    if (!gpio) {
        printf("Error getting GPIO: %s line: %d\n", gpiod_chip_name(chip), line);
        return NULL;
    }
    
    int request_result = gpiod_line_request_output(gpio, "UART_DRVIER", value);
    if (request_result < 0) {
        printf("Error requesting GPIO: %s line: %d as output\n", gpiod_chip_name(chip), line);
        gpiod_line_release(gpio);
        return NULL;
    }
    
    int set_value_result = gpiod_line_set_value(gpio, value);
    if (set_value_result < 0) {
        printf("Error setting GPIO: %s line: %d value: %d\n", gpiod_chip_name(chip), line, value);
        gpiod_line_release(gpio);
        return NULL;
    }
    
    return gpio;
}


int UART_Open(uint8_t mode)
{
    if (init_done) return 0;

    chip8 = gpiod_chip_open_by_name("gpiochip8");
    CHECK_AND_GOTO_ON_FAILURE(chip8, cleanup);

    chip7 = gpiod_chip_open_by_name("gpiochip7");
    CHECK_AND_GOTO_ON_FAILURE(chip7, cleanup_chip8);

    chip5 = gpiod_chip_open_by_name("gpiochip5");
    CHECK_AND_GOTO_ON_FAILURE(chip5, cleanup_chip7);

    chip1 = gpiod_chip_open_by_name("gpiochip1");
    CHECK_AND_GOTO_ON_FAILURE(chip1, cleanup_chip5);

    // CTRL 1 (LTC2872)------------------------------------------------------------

    UART_CTRL_1_485_232_1 = REQ_GPIO(chip5, 13, 1);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_1_485_232_1, cleanup_chip1);

    UART_CTRL_1_485_232_2 = REQ_GPIO(chip5, 14, 1);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_1_485_232_2, cleanup_UART_CTRL_1_485_232_1);

    UART_CTRL_1_LB = REQ_GPIO(chip5, 25, 0);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_1_LB, cleanup_UART_CTRL_1_485_232_2);

    UART_CTRL_1_RXEN1 = REQ_GPIO(chip5, 27, 0);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_1_RXEN1, cleanup_UART_CTRL_1_LB);

    UART_CTRL_1_DXEN1 = REQ_GPIO(chip5, 28, 1);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_1_DXEN1, cleanup_UART_CTRL_1_RXEN1);

    UART_CTRL_1_DXEN2 = REQ_GPIO(chip5, 29, 1);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_1_DXEN2, cleanup_UART_CTRL_1_DXEN1);

    UART_CTRL_1_RXEN2 = REQ_GPIO(chip5, 30, 0);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_1_RXEN2, cleanup_UART_CTRL_1_DXEN2);

    // CTRL 2 (LTC2872)------------------------------------------------------------

    UART_CTRL_2_485_232_1 = REQ_GPIO(chip5, 31, 1);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_2_485_232_1, cleanup_UART_CTRL_1_RXEN2);

    UART_CTRL_2_485_232_2 = REQ_GPIO(chip1, 22, 1);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_2_485_232_2, cleanup_UART_CTRL_2_485_232_1);

    UART_CTRL_2_LB = REQ_GPIO(chip7, 29, 0);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_2_LB, cleanup_UART_CTRL_2_485_232_2);

    UART_CTRL_2_RXEN1 = REQ_GPIO(chip7, 31, 0);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_2_RXEN1, cleanup_UART_CTRL_2_LB);

    UART_CTRL_2_DXEN1 = REQ_GPIO(chip1, 0, 1);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_2_DXEN1, cleanup_UART_CTRL_2_RXEN1);

    UART_CTRL_2_DXEN2 = REQ_GPIO(chip1, 1, 1);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_2_DXEN2, cleanup_UART_CTRL_2_DXEN1);

    UART_CTRL_2_RXEN2 = REQ_GPIO(chip1, 2, 0);
    CHECK_AND_GOTO_ON_FAILURE(UART_CTRL_2_RXEN2, cleanup_UART_CTRL_2_DXEN2);

    init_done = true;

    UART_Change_Mode(mode);

    return 0;


cleanup_UART_CTRL_2_DXEN2:
    gpiod_line_release(UART_CTRL_2_DXEN2);
cleanup_UART_CTRL_2_DXEN1:
    gpiod_line_release(UART_CTRL_2_DXEN1);
cleanup_UART_CTRL_2_RXEN1:
    gpiod_line_release(UART_CTRL_2_RXEN1);
cleanup_UART_CTRL_2_LB:
    gpiod_line_release(UART_CTRL_2_LB);
cleanup_UART_CTRL_2_485_232_2:
    gpiod_line_release(UART_CTRL_2_485_232_2);
cleanup_UART_CTRL_2_485_232_1:
    gpiod_line_release(UART_CTRL_2_485_232_1);
cleanup_UART_CTRL_1_RXEN2:
    gpiod_line_release(UART_CTRL_1_RXEN2);
cleanup_UART_CTRL_1_DXEN2:
    gpiod_line_release(UART_CTRL_1_DXEN2);
cleanup_UART_CTRL_1_DXEN1:
    gpiod_line_release(UART_CTRL_1_DXEN1);
cleanup_UART_CTRL_1_RXEN1:
    gpiod_line_release(UART_CTRL_1_RXEN1);
cleanup_UART_CTRL_1_LB:
    gpiod_line_release(UART_CTRL_1_LB);
cleanup_UART_CTRL_1_485_232_2:
    gpiod_line_release(UART_CTRL_1_485_232_2);
cleanup_UART_CTRL_1_485_232_1:
    gpiod_line_release(UART_CTRL_1_485_232_1);
cleanup_chip1:
    gpiod_chip_close(chip1);
cleanup_chip5:
    gpiod_chip_close(chip5);
cleanup_chip7:
    gpiod_chip_close(chip7);
cleanup_chip8:
    gpiod_chip_close(chip8);
cleanup:
    init_done = false;
    return -1;
}


void UART_Close(void)
{
    if (init_done)
    {
        gpiod_line_release(UART_CTRL_2_RXEN2);
        gpiod_line_release(UART_CTRL_2_DXEN2);
        gpiod_line_release(UART_CTRL_2_DXEN1);
        gpiod_line_release(UART_CTRL_2_RXEN1);
        gpiod_line_release(UART_CTRL_2_LB);
        gpiod_line_release(UART_CTRL_2_485_232_2);
        gpiod_line_release(UART_CTRL_2_485_232_1);
        gpiod_line_release(UART_CTRL_1_RXEN2);  
        gpiod_line_release(UART_CTRL_1_DXEN2);
        gpiod_line_release(UART_CTRL_1_DXEN1);
        gpiod_line_release(UART_CTRL_1_RXEN1);
        gpiod_line_release(UART_CTRL_1_LB);
        gpiod_line_release(UART_CTRL_1_485_232_2);
        gpiod_line_release(UART_CTRL_1_485_232_1);
        gpiod_chip_close(chip1);
        gpiod_chip_close(chip5);
        gpiod_chip_close(chip7);
        gpiod_chip_close(chip8);

        init_done = false;
    }
}


void UART_Change_Mode(uint8_t mode)
{
    if (!init_done)
    {
        printf("UART_Open() function must be called first.\n");
        return;
    }

    if (mode < 1 || mode > 16)
    {
        printf("Invalid mode selected: %d\n", mode);
        return;
    }

    const int* config = uart_modes[mode];

    gpiod_line_set_value(UART_CTRL_1_485_232_1, config[0])
    gpiod_line_set_value(UART_CTRL_1_485_232_2, config[1])
    gpiod_line_set_value(UART_CTRL_2_485_232_1, config[2])
    gpiod_line_set_value(UART_CTRL_2_485_232_2, config[3])
    
}