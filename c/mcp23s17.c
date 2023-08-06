#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <gpiod.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "mcp23s17.h"


// Register addresses -----------------------------------------------------------------

#define MCP23S17_IODIRA      0x00
#define MCP23S17_IODIRB      0x01
#define MCP23S17_IPOLA       0x2
#define MCP23S17_IPOLB       0x3
#define MCP23S17_GPIOA       0x12
#define MCP23S17_GPIOB       0x13
#define MCP23S17_OLATA       0x14
#define MCP23S17_OLATB       0x15
#define MCP23S17_IOCON       0x0A
#define MCP23S17_GPPUA       0x0C
#define MCP23S17_GPPUB       0x0D

// Bit field flags ----------------------------------------------------------------------

#define IOCON_UNUSED         0x01
#define IOCON_INTPOL         0x02
#define IOCON_ODR            0x04
#define IOCON_HAEN           0x08
#define IOCON_DISSLW         0x10
#define IOCON_SEQOP          0x20
#define IOCON_MIRROR         0x40
#define IOCON_BANK_MODE      0x80

#define IOCON_INIT           0x28  // IOCON_SEQOP and IOCON_HAEN from above

#define MCP23S17_CMD_WRITE   0x40
#define MCP23S17_CMD_READ    0x41


// IOEs Private Data ---------------------------------------------------------------------

typedef struct{
    uint8_t    GPIOA;
    uint8_t    GPIOB;
    uint8_t    IODIRA;
    uint8_t    IODIRB;
    uint8_t    GPPUA;
    uint8_t    GPPUB;
    uint8_t    device_id;
    struct gpiod_line *cs_line;
    struct gpiod_line *reset_line;
}IOE;


static IOE MCP23S17[2];


// SPI Stuff -----------------------------------------------------------------------------

static const char *ioe_spi_dev = "/dev/spidev2.0";
static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static int ioe_spi_fd;

// GPIO Stuff -----------------------------------------------------------------------------

#ifndef	CONSUMER
#define	CONSUMER    "MCP23S17_DRIVER"
#endif


static const char *gpiochip7 = "gpiochip7";
static const char *gpiochip5 = "gpiochip5";
static const char *gpiochip4 = "gpiochip4";
static struct gpiod_chip *chip7;
static struct gpiod_chip *chip5;
static struct gpiod_chip *chip4;
static unsigned int ioe1_cs_line = 28;
static unsigned int ioe1_reset_line = 22;
static unsigned int ioe2_cs_line = 19;
static unsigned int ioe2_reset_line = 11;


//----------------------------------------------------------------------------------------

static uint8_t Read_Register(uint8_t ioe, uint8_t reg);
static uint16_t Read_Register_Word(uint8_t ioe, uint8_t reg);
static int Write_Register(uint8_t ioe, uint8_t reg, uint8_t value);
static int Write_Register_Word(uint8_t ioe, uint8_t reg, uint16_t data);
static int Transfer(uint8_t* tx, uint8_t* rx, uint8_t len);
static int _set_pullup_mode(uint8_t ioe, uint8_t pin, uint8_t mode);
static int _set_direction(uint8_t ioe, uint8_t pin, uint8_t direction);


static bool init_done = false;


int MCP23S17_Open(void)
{
    if (init_done) return 0;

    int ret;

    // IOE SPI Init---------------------------------------------------------
	ioe_spi_fd = open(ioe_spi_dev, O_RDWR);
	if (ioe_spi_fd < 0)
    {
		printf("Can't open device ioe\n");
        return ioe_spi_fd;
    }

	ret = ioctl(ioe_spi_fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
    {
		printf("Can't set spi mode for ioe\n");
        return ret;
    }

    ret = ioctl(ioe_spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
    {
		printf("Can't set bits per word for ioe\n");
        return ret;
    }

    ret = ioctl(ioe_spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
    {
		printf("Can't set max speed hz for ioe\n");
        return ret;
    }

    // IOE 1 GPIO Init----------------------------------------------------
    chip7 = gpiod_chip_open_by_name(gpiochip7);
    if (!chip7) 
    {
        printf("Open gpiochip7 failed\n");
        ret = -1;
        goto end;
    }

    MCP23S17[IOE_1].cs_line = gpiod_chip_get_line(chip7, ioe1_cs_line);
	if (!MCP23S17[IOE_1].cs_line) 
    {
		printf("Get ioe1_cs failed\n");
        ret = -1;
        goto close_chip7;
	}

    ret = gpiod_line_request_output(MCP23S17[IOE_1].cs_line, CONSUMER, 1);
	if (ret < 0) 
    {
		printf("Request ioe1_cs as output failed\n");
        goto release_ioe1_cs;
	}

    chip4 = gpiod_chip_open_by_name(gpiochip4);
    if (!chip4) 
    {
        printf("Open gpiochip4 failed\n");
        ret = -1;
        goto release_ioe1_cs;
    }

    MCP23S17[IOE_1].reset_line = gpiod_chip_get_line(chip4, ioe1_reset_line);
	if (!MCP23S17[IOE_1].reset_line) 
    {
		printf("Get ioe1_reset failed\n");
        ret = -1;
        goto close_chip4;
	}

    ret = gpiod_line_request_output(MCP23S17[IOE_1].reset_line, CONSUMER, 1);
	if (ret < 0) 
    {
		printf("Request ioe1_reset as output failed\n");
        goto release_ioe1_reset;
	}
    
    // IOE 1 GPIO Init----------------------------------------------------
    MCP23S17[IOE_2].cs_line = gpiod_chip_get_line(chip4, ioe2_cs_line);
	if (!MCP23S17[IOE_2].cs_line) 
    {
		printf("Get ioe2_cs failed\n");
        ret = -1;
        goto release_ioe1_reset;
	}

    ret = gpiod_line_request_output(MCP23S17[IOE_2].cs_line, CONSUMER, 1);
	if (ret < 0) 
    {
		printf("Request ioe2_cs as output failed\n");
        goto release_ioe2_cs;
	}

    chip5 = gpiod_chip_open_by_name(gpiochip5);
    if (!chip5) 
    {
        printf("Open gpiochip5 failed\n");
        ret = -1;
        goto release_ioe2_cs;
    }

    MCP23S17[IOE_2].reset_line = gpiod_chip_get_line(chip5, ioe2_reset_line);
	if (!MCP23S17[IOE_2].reset_line) 
    {
		printf("Get ioe2_reset failed\n");
        ret = -1;
        goto close_chip5;
	}

    ret = gpiod_line_request_output(MCP23S17[IOE_2].reset_line, CONSUMER, 1);
	if (ret < 0) 
    {
		printf("Request ioe2_reset as output failed\n");
        goto release_ioe2_reset;
	}

    MCP23S17[IOE_1].GPIOA = 0;
    MCP23S17[IOE_1].GPIOB = 0;
    MCP23S17[IOE_1].IODIRA = 0;
    MCP23S17[IOE_1].IODIRB = 0;
    MCP23S17[IOE_1].GPPUA = 0;
    MCP23S17[IOE_1].GPPUB = 0;
    MCP23S17[IOE_1].device_id = 0;

    MCP23S17[IOE_2].GPIOA = 0;
    MCP23S17[IOE_2].GPIOB = 0;
    MCP23S17[IOE_2].IODIRA = 0;
    MCP23S17[IOE_2].IODIRB = 0;
    MCP23S17[IOE_2].GPPUA = 0;
    MCP23S17[IOE_2].GPPUB = 0;
    MCP23S17[IOE_2].device_id = 0;


    ret = Write_Register(IOE_1, MCP23S17_IOCON, IOCON_INIT);
    if (ret < 0)
    {
        printf("ioe1 register init failed\n");
        goto release_ioe2_reset;
    }

    ret = Write_Register(IOE_2, MCP23S17_IOCON, IOCON_INIT);
    if (ret < 0)
    {
        printf("ioe2 register init failed\n");
        goto release_ioe2_reset;
    }

    init_done = true;

    ret = MCP23S17_Set_Direction(IOE_1, ALL_PINS, DIR_INPUT);
    if (ret < 0)
    {
        printf("[INIT] ioe1 set direction input failed.\n");
        goto release_ioe2_reset;
    }

    ret = MCP23S17_Set_Direction(IOE_2, ALL_PINS, DIR_INPUT);
    if (ret < 0)
    {
        printf("[INIT] ioe2 set direction input failed.\n");
        goto release_ioe2_reset;
    }    

    return ret; 


release_ioe2_reset:
    gpiod_line_release(MCP23S17[IOE_2].reset_line);
close_chip5:
    gpiod_chip_close(chip5);
release_ioe2_cs:
    gpiod_line_release(MCP23S17[IOE_2].cs_line);
release_ioe1_reset:
    gpiod_line_release(MCP23S17[IOE_1].reset_line);
close_chip4:
    gpiod_chip_close(chip4);
release_ioe1_cs:
    gpiod_line_release(MCP23S17[IOE_1].cs_line);
close_chip7:
    gpiod_chip_close(chip7);
end:
    init_done = false;
    return ret;

}


void MCP23S17_Close(void)
{
    if (init_done)
    {
        close(ioe_spi_fd);
        gpiod_line_release(MCP23S17[IOE_1].reset_line);
        gpiod_line_release(MCP23S17[IOE_1].cs_line);
        gpiod_line_release(MCP23S17[IOE_2].reset_line);
        gpiod_line_release(MCP23S17[IOE_2].cs_line);
        gpiod_chip_close(chip5);
        gpiod_chip_close(chip4);
        gpiod_chip_close(chip7);

        init_done = false;
    }
}


int MCP23S17_Set_Pullup_Mode(uint8_t ioe, uint8_t pin, uint8_t mode)
{
    if (!init_done)
    {
        printf("MCP23S17_Open() function must be called first.\n");
        return -1;
    }

    if (ioe != IOE_1 && ioe != IOE_2)
    {
        printf("Invalid ioe selected: %d\n", ioe);
        return -1;
    }

    if (mode != PULLUP_ENABLED && mode != PULLUP_DISABLED)
    {
        printf("Invalid pullup mode selected: %d\n", mode);
        return -1;
    }

    int ret;
    if (pin > 15)
        for (int i = 0; i < 16; i++)
        {
            ret = _set_pullup_mode(ioe, i, mode);
            if (ret < 0) return ret;
        }
    else
        ret = _set_pullup_mode(ioe, pin, mode);

    return ret;

}


int MCP23S17_Set_Direction(uint8_t ioe, uint8_t pin, uint8_t direction)
{
    if (!init_done)
    {
        printf("MCP23S17_Open() function must be called first.\n");
        return -1;
    }

    if (ioe != IOE_1 && ioe != IOE_2)
    {
        printf("Invalid ioe selected: %d\n", ioe);
        return -1;
    }

    if (direction != DIR_INPUT && direction != DIR_OUTPUT)
    {
        printf("Invalid direction selected: %d\n", direction);
        return -1;
    }

    int ret;
    if (pin > 15)
        for (int i = 0; i < 16; i++)
        {
            ret = _set_direction(ioe, i, direction);
            if (ret < 0) return ret;
        }
    else
        ret = _set_direction(ioe, pin, direction);

    return ret;
}


int MCP23S17_Get_Level(uint8_t ioe, uint8_t pin)
{
    if (!init_done)
    {
        printf("MCP23S17_Open() function must be called first.\n");
        return -1;
    }

    if (ioe != IOE_1 && ioe != IOE_2)
    {
        printf("Invalid ioe selected: %d\n", ioe);
        return -1;
    }

    if (pin < 0 || pin > 15)
    {
        printf("Invalid pin selected: %d\n", pin);
        return -1;
    }

    int result;

    if (pin < 8)
    {
        MCP23S17[ioe].GPIOA = Read_Register(ioe, MCP23S17_GPIOA);

        if ((MCP23S17[ioe].GPIOA & (1 << pin)) != 0)
            result = LEVEL_HIGH;
        else
            result = LEVEL_LOW;
    }
    else
    {
        MCP23S17[ioe].GPIOB = Read_Register(ioe, MCP23S17_GPIOB);
        pin &= 0x07;

        if ((MCP23S17[ioe].GPIOB & (1 << pin)) != 0)
            result = LEVEL_HIGH;
        else
            result = LEVEL_LOW;
    }

    return result;
}


int MCP23S17_Set_Level(uint8_t ioe, uint8_t pin, uint8_t level)
{
    if (!init_done)
    {
        printf("MCP23S17_Open() function must be called first.\n");
        return -1;
    }

    if (ioe != IOE_1 && ioe != IOE_2)
    {
        printf("Invalid ioe selected: %d\n", ioe);
        return -1;
    }

    if (level != LEVEL_LOW && level != LEVEL_HIGH)
    {
        printf("Invalid level selected: %d\n", level);
        return -1;
    }

    if (pin < 0 || pin > 15)
    {
        printf("Invalid pin selected: %d\n", pin);
        return -1;
    }

    int ret;
    uint8_t reg, data, noshifts; 

    if (pin < 8)
    {
        reg = MCP23S17_GPIOA;
        data = MCP23S17[ioe].GPIOA;
        noshifts = pin;
    }
    else
    {
        reg = MCP23S17_GPIOB;
        noshifts = pin & 0x07;
        data = MCP23S17[ioe].GPIOB;
    }

    if (level == LEVEL_HIGH)
        data |= (1 << noshifts);
    else
        data &= (~(1 << noshifts));

    ret = Write_Register(ioe, reg, data);

    if (pin < 8)
        MCP23S17[ioe].GPIOA = data;
    else
        MCP23S17[ioe].GPIOB = data;

    return ret;
}


int MCP23S17_Set_Data(uint8_t ioe, uint16_t data)
{
    MCP23S17[ioe].GPIOA = (data & 0xFF);
    MCP23S17[ioe].GPIOB = (data >> 8);
    return Write_Register_Word(ioe, MCP23S17_GPIOA, data);
}


uint16_t MCP23S17_Get_Data(uint8_t ioe)
{
    uint16_t data = Read_Register_Word(ioe, MCP23S17_GPIOA);
    MCP23S17[ioe].GPIOA = (data & 0xFF);
    MCP23S17[ioe].GPIOB = (data >> 8);
    return data;
}


static int _set_direction(uint8_t ioe, uint8_t pin, uint8_t direction)
{
    int ret;
    uint8_t reg, data, noshifts;

    if (pin < 8)
    {
        reg = MCP23S17_IODIRA;
        data = MCP23S17[ioe].IODIRA;
        noshifts = pin;
    }
    else
    {
        reg = MCP23S17_IODIRB;
        data = MCP23S17[ioe].IODIRB;
        noshifts = pin & 0x07;
    }

    if (direction == DIR_INPUT)
        data |= (1 << noshifts);
    else
        data &= (~(1 << noshifts));

    ret = Write_Register(ioe, reg, data);

    if (pin < 8)
        MCP23S17[ioe].IODIRA = data;
    else
        MCP23S17[ioe].IODIRB = data;

    return ret;
}


static int _set_pullup_mode(uint8_t ioe, uint8_t pin, uint8_t mode)
{
    int ret;
    uint8_t reg, data, noshifts;

    if (pin < 8)
    {
        reg = MCP23S17_GPPUA;
        noshifts = pin;
        data = MCP23S17[ioe].GPPUA;
    }
    else
    {
        reg = MCP23S17_GPPUB;
        noshifts = pin & 0x07;
        data = MCP23S17[ioe].GPPUB;
    }

    if (mode == PULLUP_ENABLED)
        data |= (1 << noshifts);
    else
        data &= (~(1 << noshifts));

    ret = Write_Register(ioe, reg, data);

    if (pin < 8)
        MCP23S17[ioe].GPPUA = data;
    else
        MCP23S17[ioe].GPPUB = data;

    return ret;
}


static uint8_t Read_Register(uint8_t ioe, uint8_t reg)
{
    uint8_t command = MCP23S17_CMD_READ | (MCP23S17[ioe].device_id << 1);
    uint8_t tx[3] = {command, reg, 0};
    uint8_t rx[3] = {0};

    gpiod_line_set_value(MCP23S17[ioe].cs_line, 0);
    Transfer(tx, rx, 3);
    gpiod_line_set_value(MCP23S17[ioe].cs_line, 1);

    return rx[2];
}


static uint16_t Read_Register_Word(uint8_t ioe, uint8_t reg)
{
    uint8_t buffer[2] = {0};
    buffer[0] = Read_Register(ioe, reg);
    buffer[1] = Read_Register(ioe, reg + 1);

    return (buffer[1] << 8) | buffer[0];
}


static int Write_Register_Word(uint8_t ioe, uint8_t reg, uint16_t data)
{
    int ret;
    ret = Write_Register(ioe, reg, data & 0xFF);
    if (ret < 0) return ret;
    ret = Write_Register(ioe, reg + 1, data >> 8);

    return ret;
}


static int Write_Register(uint8_t ioe, uint8_t reg, uint8_t value)
{
    uint8_t command = MCP23S17_CMD_WRITE | (MCP23S17[ioe].device_id << 1);
    uint8_t tx[3] = {command, reg, value};
    uint8_t dummy_rx[3] = {0};
    int ret;
    gpiod_line_set_value(MCP23S17[ioe].cs_line, 0);
    ret = Transfer(tx, dummy_rx, 3);
    gpiod_line_set_value(MCP23S17[ioe].cs_line, 1);
    return ret;
}


static int Transfer(uint8_t* tx, uint8_t* rx, uint8_t len)
{
    int ret;

    struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

    
    ret = ioctl(ioe_spi_fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        printf("ioe: can't send spi message\n");
        return -1;
    }

    return 0;
}
