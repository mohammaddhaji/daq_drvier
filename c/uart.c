#include <stdio.h>
#include <gpiod.h>
#include "uart.h"


// GPIO Stuff -----------------------------------------------------------------------------

#ifndef	CONSUMER
#define	CONSUMER    "UART_DRIVER"
#endif

static const char *gpiochip8 = "gpiochip8";
static const char *gpiochip7 = "gpiochip7";
static const char *gpiochip5 = "gpiochip5";
static const char *gpiochip1 = "gpiochip1";
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


static bool init_done = false;


int UART_Open(uint8_t mode)
{
    if (init_done) return 0;

    int ret;

    chip8 = gpiod_chip_open_by_name(gpiochip8);
    if (!chip8) 
    {
        printf("UART: Open gpiochip8 failed\n");
        ret = -1;
        goto end;
    }
    chip7 = gpiod_chip_open_by_name(gpiochip7);
    if (!chip7) 
    {
        printf("UART: Open gpiochip7 failed\n");
        ret = -1;
        goto close_chip8;
    }
    chip5 = gpiod_chip_open_by_name(gpiochip5);
    if (!chip5) 
    {
        printf("UART: Open gpiochip5 failed\n");
        ret = -1;
        goto close_chip7;
    }
    chip1 = gpiod_chip_open_by_name(gpiochip1);
    if (!chip1) 
    {
        printf("UART: Open gpiochip1 failed\n");
        ret = -1;
        goto close_chip5;
    }

    // CTRL 1 (LTC2872)-------------------------------------------------------------------

    UART_CTRL_1_485_232_1 = gpiod_chip_get_line(chip5, 13);
	if (!UART_CTRL_1_485_232_1)
    {
		printf("Get UART_CTRL_1_485_232_1 failed\n");
        ret = -1;
        goto close_chip1;
	}
    // 1 ---> uart1 (485/422), 0 ---> uart2 (232)
    ret = gpiod_line_request_output(UART_CTRL_1_485_232_1, CONSUMER, 1);
	if (ret < 0)
    {
		printf("Request UART_CTRL_1_485_232_1 as output failed\n");
        goto release_UART_CTRL_1_485_232_1;
	}

    UART_CTRL_1_485_232_2 = gpiod_chip_get_line(chip5, 14);
	if (!UART_CTRL_1_485_232_2)
    {
		printf("Get UART_CTRL_1_485_232_2 failed\n");
        ret = -1;
        goto release_UART_CTRL_1_485_232_1;
	}
    // 1 ---> uart4 (485/422), 0 ---> uart5 (232)
    ret = gpiod_line_request_output(UART_CTRL_1_485_232_2, CONSUMER, 1);
	if (ret < 0)
    {
		printf("Request UART_CTRL_1_485_232_2 as output failed\n");
        goto release_UART_CTRL_1_485_232_2;
	}

    UART_CTRL_1_LB = gpiod_chip_get_line(chip5, 25);
	if (!UART_CTRL_1_LB)
    {
		printf("Get UART_CTRL_1_LB failed\n");
        ret = -1;
        goto release_UART_CTRL_1_485_232_2;
	}
    // Loop Back
    ret = gpiod_line_request_output(UART_CTRL_1_LB, CONSUMER, 0);
	if (ret < 0)
    {
		printf("Request UART_CTRL_1_LB as output failed\n");
        goto release_UART_CTRL_1_LB;
	}

    UART_CTRL_1_RXEN1 = gpiod_chip_get_line(chip5, 27);
	if (!UART_CTRL_1_RXEN1)
    {
		printf("Get UART_CTRL_1_RXEN1 failed\n");
        ret = -1;
        goto release_UART_CTRL_1_LB;
	}
    // 0 ---> Enable RX uart1,2 
    ret = gpiod_line_request_output(UART_CTRL_1_RXEN1, CONSUMER, 0);
	if (ret < 0)
    {
		printf("Request UART_CTRL_1_RXEN1 as output failed\n");
        goto release_UART_CTRL_1_RXEN1;
	}

    UART_CTRL_1_DXEN1 = gpiod_chip_get_line(chip5, 28);
	if (!UART_CTRL_1_DXEN1)
    {
		printf("Get UART_CTRL_1_DXEN1 failed\n");
        ret = -1;
        goto release_UART_CTRL_1_RXEN1;
	}
    // 1 ---> Enable Tx uart1,2 
    ret = gpiod_line_request_output(UART_CTRL_1_DXEN1, CONSUMER, 1);
	if (ret < 0)
    {
		printf("Request UART_CTRL_1_DXEN1 as output failed\n");
        goto release_UART_CTRL_1_DXEN1;
	}

    UART_CTRL_1_DXEN2 = gpiod_chip_get_line(chip5, 29);
	if (!UART_CTRL_1_DXEN2)
    {
		printf("Get UART_CTRL_1_DXEN2 failed\n");
        ret = -1;
        goto release_UART_CTRL_1_DXEN1;
	}
    // 1 ---> Enable Tx uart4,5
    ret = gpiod_line_request_output(UART_CTRL_1_DXEN2, CONSUMER, 1);
	if (ret < 0)
    {
		printf("Request UART_CTRL_1_DXEN2 as output failed\n");
        goto release_UART_CTRL_1_DXEN2;
	}

    UART_CTRL_1_RXEN2 = gpiod_chip_get_line(chip5, 30);
	if (!UART_CTRL_1_RXEN2)
    {
		printf("Get UART_CTRL_1_RXEN2 failed\n");
        ret = -1;
        goto release_UART_CTRL_1_DXEN2;
	}
    // 0 ---> Enable Rx uart4,5
    ret = gpiod_line_request_output(UART_CTRL_1_RXEN2, CONSUMER, 0);
	if (ret < 0)
    {
		printf("Request UART_CTRL_1_RXEN2 as output failed\n");
        goto release_UART_CTRL_1_RXEN2;
	}

    // CTRL 2 (LTC2872)-------------------------------------------------------------------

    UART_CTRL_2_485_232_1 = gpiod_chip_get_line(chip5, 31);
	if (!UART_CTRL_2_485_232_1)
    {
		printf("Get UART_CTRL_2_485_232_1 failed\n");
        ret = -1;
        goto release_UART_CTRL_1_RXEN2;
	}
    // 1 ---> uart6 (485/422), 0 ---> uart7 (232)
    ret = gpiod_line_request_output(UART_CTRL_2_485_232_1, CONSUMER, 1);
	if (ret < 0)
    {
		printf("Request UART_CTRL_2_485_232_1 as output failed\n");
        goto release_UART_CTRL_2_485_232_1;
	}

    UART_CTRL_2_485_232_2 = gpiod_chip_get_line(chip1, 22);
	if (!UART_CTRL_2_485_232_2)
    {
		printf("Get UART_CTRL_2_485_232_2 failed\n");
        ret = -1;
        goto release_UART_CTRL_2_485_232_1;
	}
    // 1 ---> uart8 (485/422), 0 ---> uart9 (232)
    ret = gpiod_line_request_output(UART_CTRL_2_485_232_2, CONSUMER, 1);
	if (ret < 0)
    {
		printf("Request UART_CTRL_2_485_232_2 as output failed\n");
        goto release_UART_CTRL_2_485_232_2;
	}

    UART_CTRL_2_LB = gpiod_chip_get_line(chip7, 29);
	if (!UART_CTRL_2_LB)
    {
		printf("Get UART_CTRL_2_LB failed\n");
        ret = -1;
        goto release_UART_CTRL_2_485_232_2;
	}
    // Loop Back
    ret = gpiod_line_request_output(UART_CTRL_2_LB, CONSUMER, 0);
	if (ret < 0)
    {
		printf("Request UART_CTRL_2_LB as output failed\n");
        goto release_UART_CTRL_2_LB;
	}

    UART_CTRL_2_RXEN1 = gpiod_chip_get_line(chip7, 31);
	if (!UART_CTRL_2_RXEN1)
    {
		printf("Get UART_CTRL_2_RXEN1 failed\n");
        ret = -1;
        goto release_UART_CTRL_2_LB;
	}
    // 0 ---> Enable RX uart6,7
    ret = gpiod_line_request_output(UART_CTRL_2_RXEN1, CONSUMER, 0);
	if (ret < 0)
    {
		printf("Request UART_CTRL_2_RXEN1 as output failed\n");
        goto release_UART_CTRL_2_RXEN1;
	}

    UART_CTRL_2_DXEN1 = gpiod_chip_get_line(chip1, 0);
	if (!UART_CTRL_2_DXEN1)
    {
		printf("Get UART_CTRL_2_DXEN1 failed\n");
        ret = -1;
        goto release_UART_CTRL_2_RXEN1;
	}
    // 1 ---> Enable TX uart6,7
    ret = gpiod_line_request_output(UART_CTRL_2_DXEN1, CONSUMER, 1);
	if (ret < 0)
    {
		printf("Request UART_CTRL_2_DXEN1 as output failed\n");
        goto release_UART_CTRL_2_DXEN1;
	}

    UART_CTRL_2_DXEN2 = gpiod_chip_get_line(chip1, 1);
	if (!UART_CTRL_2_DXEN2)
    {
		printf("Get UART_CTRL_2_DXEN2 failed\n");
        ret = -1;
        goto release_UART_CTRL_2_DXEN1;
	}
    // 1 ---> Enable TX uart8,9
    ret = gpiod_line_request_output(UART_CTRL_2_DXEN2, CONSUMER, 1);
	if (ret < 0)
    {
		printf("Request UART_CTRL_2_DXEN2 as output failed\n");
        goto release_UART_CTRL_2_DXEN2;
	}

    UART_CTRL_2_RXEN2 = gpiod_chip_get_line(chip1, 2);
	if (!UART_CTRL_2_RXEN2)
    {
		printf("Get UART_CTRL_2_RXEN2 failed\n");
        ret = -1;
        goto release_UART_CTRL_2_DXEN2;
	}
    // 1 ---> Enable TX uart8,9
    ret = gpiod_line_request_output(UART_CTRL_2_RXEN2, CONSUMER, 0);
	if (ret < 0)
    {
		printf("Request UART_CTRL_2_RXEN2 as output failed\n");
        goto release_UART_CTRL_2_RXEN2;
	}

    init_done = true;

    UART_Change_Mode(mode);

    return 0;


release_UART_CTRL_2_RXEN2:
    gpiod_line_release(UART_CTRL_2_RXEN2);
release_UART_CTRL_2_DXEN2:
    gpiod_line_release(UART_CTRL_2_DXEN2);
release_UART_CTRL_2_DXEN1:
    gpiod_line_release(UART_CTRL_2_DXEN1);
release_UART_CTRL_2_RXEN1:
    gpiod_line_release(UART_CTRL_2_RXEN1);
release_UART_CTRL_2_LB:
    gpiod_line_release(UART_CTRL_2_LB);
release_UART_CTRL_2_485_232_2:
    gpiod_line_release(UART_CTRL_2_485_232_2);
release_UART_CTRL_2_485_232_1:
    gpiod_line_release(UART_CTRL_2_485_232_1);
release_UART_CTRL_1_RXEN2:
    gpiod_line_release(UART_CTRL_1_RXEN2);
release_UART_CTRL_1_DXEN2:
    gpiod_line_release(UART_CTRL_1_DXEN2);
release_UART_CTRL_1_DXEN1:
    gpiod_line_release(UART_CTRL_1_DXEN1);
release_UART_CTRL_1_RXEN1:
    gpiod_line_release(UART_CTRL_1_RXEN1);
release_UART_CTRL_1_LB:
    gpiod_line_release(UART_CTRL_1_LB);
release_UART_CTRL_1_485_232_2:
    gpiod_line_release(UART_CTRL_1_485_232_2);
release_UART_CTRL_1_485_232_1:
    gpiod_line_release(UART_CTRL_1_485_232_1);
close_chip1:
    gpiod_chip_close(chip1);
close_chip5:
    gpiod_chip_close(chip5);
close_chip7:
    gpiod_chip_close(chip7);
close_chip8:
    gpiod_chip_close(chip8);
end:
    init_done = false;
    return ret;
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

    if (mode == UART_1_4_6_8)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 1);
    }
    else if (mode == UART_1_4_6_9)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 0);
    }
    else if (mode == UART_1_4_7_8)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 1);
    }
    else if (mode == UART_1_4_7_9)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 0);
    }
    else if (mode == UART_1_5_6_8)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 1);
    }
    else if (mode == UART_1_5_6_9)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 0);
    }
    else if (mode == UART_1_5_7_8)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 1);
    }
    else if (mode == UART_1_5_7_9)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 0);
    }
    else if (mode == UART_2_4_6_8)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 1);
    }
    else if (mode == UART_2_4_6_9)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 0);
    }
    else if (mode == UART_2_4_7_8)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 1);
    }
    else if (mode == UART_2_4_7_9)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 0);
    }
    else if (mode == UART_2_5_6_8)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 1);
    }
    else if (mode == UART_2_5_6_9)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 1);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 0);
    }
    else if (mode == UART_2_5_7_8)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 1);
    }
    else if (mode == UART_2_5_7_9)
    {
        gpiod_line_set_value(UART_CTRL_1_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_1_485_232_2, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_1, 0);
        gpiod_line_set_value(UART_CTRL_2_485_232_2, 0);
    }
}