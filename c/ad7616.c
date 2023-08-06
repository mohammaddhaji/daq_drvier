#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <gpiod.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "ad7616.h"



#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
//---------------------------------------------------------------------------------------------------
#define ADC7616_Mode_Burst				0x01
#define ADC7616_Mode_Normal				0x00
//---------------------------------------------------------------------------------------------------
#define AD7616_Add_Config__Register     0x02
#define AD7616_Add_Channel_Register     0x03
#define AD7616_Add_RangeA1_Register     0x04
#define AD7616_Add_RangeA2_Register     0x05
#define AD7616_Add_RangeB1_Register     0x06
#define AD7616_Add_RangeB2_Register     0x07
//---------------------------------------------------------------------------------------------------

typedef struct{
    uint8_t    CRCEN   :1;      //Enable CRC
    uint8_t    STATUSEN:1;      //Enable STATUS
    uint8_t    OverSamp:3;      //Over Sampling Ratio
    uint8_t    SEQEN   :1;      //Enable Sequencer
    uint8_t    BURSTEN :1;      //Enable Burst
    uint8_t    SDEF    :1;      //Self detector error flag
    uint8_t    Rsvrd   :1;      //Reserved = 0
    uint8_t    Address :6;      //Address of register => AD7616_Add_Config__Register
    uint8_t    W_nR    :1;      // 1 for write and 0 for read
}Configuration;

union Configuration_Reg{
    uint8_t         all[2];
    Configuration   bit;
};



//---------------------------------------------------------------------------------------------------

typedef struct{
    uint8_t    ADC_A_SEL   :4;       //Channel selection bits for ADC A Channels
    uint8_t    ADC_B_SEL   :4;       //Channel selection bits for ADC B Channels
    uint8_t    Rsvrd       :1;       //Reserved = 0
    uint8_t    Address     :6;       //Address of register => AD7616_Add_Channel_Register
    uint8_t    W_nR        :1;       // 1 for write and 0 for read
}Channels;

union Channels_Reg{
    uint8_t        all[2];
    Channels       bit;
};


//---------------------------------------------------------------------------------------------------

typedef struct{
    uint8_t    V0A_Range   :2;       //V0A voltage range selection
    uint8_t    V1A_Range   :2;       //V1A voltage range selection
    uint8_t    V2A_Range   :2;       //V2A voltage range selection
    uint8_t    V3A_Range   :2;       //V3A voltage range selection
    uint8_t    Rsvrd       :1;       //Reserved = 0
    uint8_t    Address     :6;       //Address of register => AD7616_Add_RangeA1_Register
    uint8_t    W_nR        :1;       // 1 for write and 0 for read
}InputRangeA1;

union InputRangeA1_Reg{
    uint8_t            all[2];
    InputRangeA1       bit;
};


typedef struct{
    uint8_t    V4A_Range   :2;       //V4A voltage range selection
    uint8_t    V5A_Range   :2;       //V5A voltage range selection
    uint8_t    V6A_Range   :2;       //V6A voltage range selection
    uint8_t    V7A_Range   :2;       //V7A voltage range selection
    uint8_t    Rsvrd       :1;       //Reserved = 0
    uint8_t    Address     :6;       //Address of register => AD7616_Add_RangeA2_Register
    uint8_t    W_nR        :1;       // 1 for write and 0 for read
}InputRangeA2;

union InputRangeA2_Reg{
    uint8_t            all[2];
    InputRangeA2       bit;
};


typedef struct{
    uint8_t    V0B_Range   :2;       //V0B voltage range selection
    uint8_t    V1B_Range   :2;       //V1B voltage range selection
    uint8_t    V2B_Range   :2;       //V2B voltage range selection
    uint8_t    V3B_Range   :2;       //V3B voltage range selection
    uint8_t    Rsvrd       :1;       //Reserved = 0
    uint8_t    Address     :6;       //Address of register => AD7616_Add_RangeA3_Register
    uint8_t    W_nR        :1;       // 1 for write and 0 for read
}InputRangeB1;

union InputRangeB1_Reg{
    uint8_t            all[2];
    InputRangeB1       bit;
};


typedef struct{
    uint8_t    V4B_Range   :2;       //V4B voltage range selection
    uint8_t    V5B_Range   :2;       //V5B voltage range selection
    uint8_t    V6B_Range   :2;       //V6B voltage range selection
    uint8_t    V7B_Range   :2;       //V7B voltage range selection
    uint8_t    Rsvrd       :1;       //Reserved = 0
    uint8_t    Address     :6;       //Address of register => AD7616_Add_RangeA4_Register
    uint8_t    W_nR        :1;       // 1 for write and 0 for read
}InputRangeB2;

union InputRangeB2_Reg{
    uint8_t            all[2];
    InputRangeB2       bit;
};


// SPI Stuff -----------------------------------------------------------------------------

static uint8_t mode = 3;
static uint8_t bits = 8;
static uint32_t speed = 30000000;


// GPIO Stuff -----------------------------------------------------------------------------

#ifndef	CONSUMER
#define	CONSUMER    "AD7616_DRIVER"
#endif

static const char *gpiochip6 = "gpiochip6";
static const char *gpiochip4 = "gpiochip4";
static struct gpiod_chip *chip6;
static struct gpiod_chip *chip4;
static unsigned int adc1_convst_line = 7;
static unsigned int adc1_reset_line = 9;
static unsigned int adc1_busy_line = 8;
static unsigned int adc2_convst_line = 17;
static unsigned int adc2_reset_line = 21;
static unsigned int adc2_busy_line = 25;


//----------------------------------------------------------------------------
static int Transfer(uint8_t adc, uint8_t *tx, uint8_t *rx, uint8_t len);
static int Set_Channel(uint8_t adc, uint8_t A_channel,uint8_t B_channel);
static int Wait_For_Busy(uint8_t adc, uint8_t timeout_us);
static int Read_All_Channel(uint8_t adc, int8_t* Res);
static int Get_Input_Range(uint8_t adc, uint8_t channel);
static float Calc_Scale(uint8_t range);


typedef struct{
    union Configuration_Reg Config;   
    union InputRangeA1_Reg InRangeA1;
    union InputRangeA2_Reg InRangeA2;
    union InputRangeB1_Reg InRangeB1;
    union InputRangeB2_Reg InRangeB2;
    struct gpiod_line *convst_line;
    struct gpiod_line *reset_line;
    struct gpiod_line *busy_line;
    const char *adc_spi_dev;
    int adc_spi_fd;
}ADC;


static ADC AD7616[2];


static bool init_done = false;



int AD7616_Open(void)
{
    if (init_done) return 0;

    int ret;
    AD7616[ADC_1].adc_spi_dev = "/dev/spidev4.0";
    AD7616[ADC_2].adc_spi_dev = "/dev/spidev3.0";

    // ADC 1 & 2 SPI Init---------------------------------------------------------
    for (int adc = 0; adc < 2; adc++)
    {
        AD7616[adc].adc_spi_fd = open(AD7616[adc].adc_spi_dev, O_RDWR);
        if (AD7616[adc].adc_spi_fd < 0)
        {
            printf("Can't open device adc %d\n", adc);
            return AD7616[adc].adc_spi_fd;
        }

        ret = ioctl(AD7616[adc].adc_spi_fd, SPI_IOC_WR_MODE, &mode);
        if (ret == -1)
        {
            printf("Can't set spi mode for adc %d\n", adc);
            return ret;
        }

        ret = ioctl(AD7616[adc].adc_spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        if (ret == -1)
        {
            printf("Can't set bits per word for adc %d\n", adc);
            return ret;
        }

        ret = ioctl(AD7616[adc].adc_spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        if (ret == -1)
        {
            printf("Can't set max speed hz for adc %d\n", adc);
            return ret;
        }
    }

    // ADC 1 GPIO Init----------------------------------------------------
    chip6 = gpiod_chip_open_by_name(gpiochip6);
    if (!chip6) 
    {
        printf("Open gpiochip6 failed\n");
        ret = -1;
        goto end;
    }

	AD7616[ADC_1].convst_line = gpiod_chip_get_line(chip6, adc1_convst_line);
	if (!AD7616[ADC_1].convst_line) 
    {
		printf("Get adc1_convst failed\n");
        ret = -1;
        goto close_adc1_chip;
	}

	ret = gpiod_line_request_output(AD7616[ADC_1].convst_line, CONSUMER, 0);
	if (ret < 0) 
    {
		printf("Request adc1_convst as output failed\n");
        goto release_adc1_convst;
	}

    AD7616[ADC_1].reset_line = gpiod_chip_get_line(chip6, adc1_reset_line);
	if (!AD7616[ADC_1].reset_line)
    {
		printf("Get adc1_reset failed\n");
        ret = -1;
        goto release_adc1_convst;
	}

	ret = gpiod_line_request_output(AD7616[ADC_1].reset_line, CONSUMER, 1);
	if (ret < 0) 
    {
		printf("Request adc1_reset as output failed\n");
        goto release_adc1_reset;
	}

    AD7616[ADC_1].busy_line = gpiod_chip_get_line(chip6, adc1_busy_line);
	if (!AD7616[ADC_1].busy_line) 
    {
		printf("Get adc1_busy failed\n");
        ret = -1;
        goto release_adc1_reset;
	}

	ret = gpiod_line_request_input(AD7616[ADC_1].busy_line, CONSUMER);
	if (ret < 0) 
    {
		printf("Request adc1_busy as input failed\n");
        goto release_adc1_busy;
	}

    // ADC 2 GPIO Init----------------------------------------------------        
    chip4 = gpiod_chip_open_by_name(gpiochip4);
    if (!chip4) 
    {
        printf("Open gpiochip4 failed\n");
        ret = -1;
        goto release_adc1_busy;
    }

    AD7616[ADC_2].convst_line = gpiod_chip_get_line(chip4, adc2_convst_line);
	if (!AD7616[ADC_2].convst_line) 
    {
		printf("Get adc2_convst failed\n");
        ret = -1;
        goto close_adc2_chip;
	}

	ret = gpiod_line_request_output(AD7616[ADC_2].convst_line, CONSUMER, 0);
	if (ret < 0)
    {
		printf("Request adc2_convst as output failed\n");
        goto release_adc2_convst;
	}

    AD7616[ADC_2].reset_line = gpiod_chip_get_line(chip4, adc2_reset_line);
	if (!AD7616[ADC_2].reset_line)
    {
		printf("Get adc2_reset failed\n");
        ret = -1;
        goto release_adc2_convst;
	}

	ret = gpiod_line_request_output(AD7616[ADC_2].reset_line, CONSUMER, 1);
	if (ret < 0)
    {
		printf("Request adc2_reset as output failed\n");
        goto release_adc2_reset;
	}

    AD7616[ADC_2].busy_line = gpiod_chip_get_line(chip4, adc2_busy_line);
	if (!AD7616[ADC_2].busy_line)
    {
		printf("Get adc2_busy failed\n");
        ret = -1;
        goto release_adc2_reset;
	}

	ret = gpiod_line_request_input(AD7616[ADC_2].busy_line, CONSUMER);
	if (ret < 0) 
    {
		printf("Request adc2_busy as input failed\n");
        goto release_adc2_busy;
	}

    // Registers Init------------------------------------------------------        

    AD7616[ADC_1].Config.bit.CRCEN       = 0;
    AD7616[ADC_1].Config.bit.STATUSEN    = 1;
    AD7616[ADC_1].Config.bit.OverSamp    = 0;
    AD7616[ADC_1].Config.bit.SEQEN       = 1;
    AD7616[ADC_1].Config.bit.BURSTEN     = ADC7616_Mode_Burst;
    AD7616[ADC_1].Config.bit.SDEF        = 0;
    AD7616[ADC_1].Config.bit.Rsvrd       = 0;
    AD7616[ADC_1].Config.bit.Address     = AD7616_Add_Config__Register;
    AD7616[ADC_1].Config.bit.W_nR        = 1;

    AD7616[ADC_1].InRangeA1.bit.Rsvrd    = 0;
    AD7616[ADC_1].InRangeA1.bit.Address  = AD7616_Add_RangeA1_Register;
    AD7616[ADC_1].InRangeA2.bit.Rsvrd    = 0;
    AD7616[ADC_1].InRangeA2.bit.Address  = AD7616_Add_RangeA2_Register;
    AD7616[ADC_1].InRangeB1.bit.Rsvrd    = 0;
    AD7616[ADC_1].InRangeB1.bit.Address  = AD7616_Add_RangeB1_Register;
    AD7616[ADC_1].InRangeB2.bit.Rsvrd    = 0;
    AD7616[ADC_1].InRangeB2.bit.Address  = AD7616_Add_RangeB2_Register;

    AD7616[ADC_2].Config.bit.CRCEN       = 0;
    AD7616[ADC_2].Config.bit.STATUSEN    = 1;
    AD7616[ADC_2].Config.bit.OverSamp    = 0;
    AD7616[ADC_2].Config.bit.SEQEN       = 1;
    AD7616[ADC_2].Config.bit.BURSTEN     = ADC7616_Mode_Burst;
    AD7616[ADC_2].Config.bit.SDEF        = 0;
    AD7616[ADC_2].Config.bit.Rsvrd       = 0;
    AD7616[ADC_2].Config.bit.Address     = AD7616_Add_Config__Register;
    AD7616[ADC_2].Config.bit.W_nR        = 1;

    AD7616[ADC_2].InRangeA1.bit.Rsvrd    = 0;
    AD7616[ADC_2].InRangeA1.bit.Address  = AD7616_Add_RangeA1_Register;
    AD7616[ADC_2].InRangeA2.bit.Rsvrd    = 0;
    AD7616[ADC_2].InRangeA2.bit.Address  = AD7616_Add_RangeA2_Register;
    AD7616[ADC_2].InRangeB1.bit.Rsvrd    = 0;
    AD7616[ADC_2].InRangeB1.bit.Address  = AD7616_Add_RangeB1_Register;
    AD7616[ADC_2].InRangeB2.bit.Rsvrd    = 0;
    AD7616[ADC_2].InRangeB2.bit.Address  = AD7616_Add_RangeB2_Register;

    uint8_t dummy_rx[2];

    ret = Transfer(ADC_1, AD7616[ADC_1].Config.all, dummy_rx, 2);
    if (ret < 0)
    {
        printf("Setting adc1 to burst mode failed\n");
        goto release_adc2_busy;
    }

    ret = Transfer(ADC_2, AD7616[ADC_2].Config.all, dummy_rx, 2);
    if (ret < 0)
    {
        printf("Setting adc2 to burst mode failed\n");
        goto release_adc2_busy;
    }

    init_done = true;

    ret = AD7616_Set_Input_Range_All(ADC_1, AD7616_InputRange_10V_PN);
    if (ret < 0)
    {
        printf("[INIT] Setting adc1 input range to +/- 10V failed\n");
        goto release_adc2_busy;
    }

    ret = AD7616_Set_Input_Range_All(ADC_2, AD7616_InputRange_10V_PN);
    if (ret < 0)
    {
        printf("[INIT] Setting adc2 input range to +/- 10V failed\n");
        goto release_adc2_busy;
    }

    ret = AD7616_Set_Oversampling(ADC_1, AD7616_OverSampling_Disabled);
    if (ret < 0)
    {
        printf("[INIT] Setting adc1 oversampling to disabled failed\n");
        goto release_adc2_busy;
    }

    ret = AD7616_Set_Oversampling(ADC_2, AD7616_OverSampling_Disabled);
    if (ret < 0)
    {
        printf("[INIT] Setting adc2 oversampling to disabled failed\n");
        goto release_adc2_busy;
    }

    return 0;

release_adc2_busy:
    gpiod_line_release(AD7616[ADC_2].busy_line);
release_adc2_reset:
    gpiod_line_release(AD7616[ADC_2].reset_line);
release_adc2_convst:
    gpiod_line_release(AD7616[ADC_2].convst_line);
close_adc2_chip:
    gpiod_chip_close(chip4);
release_adc1_busy:
    gpiod_line_release(AD7616[ADC_1].busy_line);
release_adc1_reset:
    gpiod_line_release(AD7616[ADC_1].reset_line);
release_adc1_convst:
    gpiod_line_release(AD7616[ADC_1].convst_line);
close_adc1_chip:
    gpiod_chip_close(chip6);
end:
    init_done = false;
    return ret;
}


void AD7616_Close(void)
{
    if (init_done)
    {
        close(AD7616[ADC_1].adc_spi_fd);
        close(AD7616[ADC_2].adc_spi_fd);
        gpiod_line_release(AD7616[ADC_1].busy_line);
        gpiod_line_release(AD7616[ADC_1].reset_line);
        gpiod_line_release(AD7616[ADC_1].convst_line);
        gpiod_line_release(AD7616[ADC_2].busy_line);
        gpiod_line_release(AD7616[ADC_2].reset_line);
        gpiod_line_release(AD7616[ADC_2].convst_line);
        gpiod_chip_close(chip4);
        gpiod_chip_close(chip6);

        init_done = false;
    }
}


int AD7616_Set_Input_Range_All(uint8_t adc, uint8_t range)
{
    if (!init_done)
    {
        printf("AD7616_Open() function must be called first.\n");
        return -1;
    }

    if (adc != ADC_1 && adc != ADC_2)
    {
        printf("Invalid adc selected: %d\n", adc);
        return -1;
    }

    if (range != AD7616_InputRange_2V5_PN && 
        range != AD7616_InputRange_5V0_PN &&
        range != AD7616_InputRange_10V_PN) {

        printf("Invalid range selected: %d\n", range);
        return -1;
    }

    int ret;

    AD7616[adc].InRangeA1.bit.V0A_Range = range;
    AD7616[adc].InRangeA1.bit.V1A_Range = range;
    AD7616[adc].InRangeA1.bit.V2A_Range = range;
    AD7616[adc].InRangeA1.bit.V3A_Range = range;
    AD7616[adc].InRangeA1.bit.W_nR      = 1;

    AD7616[adc].InRangeA2.bit.V4A_Range = range;
    AD7616[adc].InRangeA2.bit.V5A_Range = range;
    AD7616[adc].InRangeA2.bit.V6A_Range = range;
    AD7616[adc].InRangeA2.bit.V7A_Range = range;
    AD7616[adc].InRangeA2.bit.W_nR      = 1;

    AD7616[adc].InRangeB1.bit.V0B_Range = range;
    AD7616[adc].InRangeB1.bit.V1B_Range = range;
    AD7616[adc].InRangeB1.bit.V2B_Range = range;
    AD7616[adc].InRangeB1.bit.V3B_Range = range;
    AD7616[adc].InRangeB1.bit.W_nR      = 1;

    AD7616[adc].InRangeB2.bit.V4B_Range = range;
    AD7616[adc].InRangeB2.bit.V5B_Range = range;
    AD7616[adc].InRangeB2.bit.V6B_Range = range;
    AD7616[adc].InRangeB2.bit.V7B_Range = range;
    AD7616[adc].InRangeB2.bit.W_nR      = 1;

    uint8_t dummy_rx[2];

    ret = Transfer(adc, AD7616[adc].InRangeA1.all, dummy_rx, 2);
    if (ret < 0) return -1;
    ret = Transfer(adc, AD7616[adc].InRangeA2.all, dummy_rx, 2);
    if (ret < 0) return -1;
    ret = Transfer(adc, AD7616[adc].InRangeB1.all, dummy_rx, 2);
    if (ret < 0) return -1;
    ret = Transfer(adc, AD7616[adc].InRangeB2.all, dummy_rx, 2);
    if (ret < 0) return -1;

    return 0;
}


int AD7616_Set_Input_Range(uint8_t adc, uint8_t channel, uint8_t range)
{
    if (!init_done)
    {
        printf("AD7616_Open() function must be called first.\n");
        return -1;
    }

    if (adc != ADC_1 && adc != ADC_2)
    {
        printf("Invalid adc selected: %d\n", adc);
        return -1;
    }

    if (channel < 0 || channel > 15)
    {
        printf("Invalid channel selected: %d\n", channel);
        return -1;
    }

    if (range != AD7616_InputRange_2V5_PN && 
        range != AD7616_InputRange_5V0_PN &&
        range != AD7616_InputRange_10V_PN) {

        printf("Invalid range selected: %d\n", range);
        return -1;
    }

    uint8_t dummy_rx[2];
    int ret;

    if (channel == 0)
    {
        AD7616[adc].InRangeA1.bit.V0A_Range = range;
        AD7616[adc].InRangeA1.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeA1.all, dummy_rx, 2);
    }
    else if (channel == 1)
    {
        AD7616[adc].InRangeA1.bit.V1A_Range = range;
        AD7616[adc].InRangeA1.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeA1.all, dummy_rx, 2);
    }
    else if (channel == 2)
    {
        AD7616[adc].InRangeA1.bit.V2A_Range = range;
        AD7616[adc].InRangeA1.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeA1.all, dummy_rx, 2);
    }
    else if (channel == 3)
    {
        AD7616[adc].InRangeA1.bit.V3A_Range = range;
        AD7616[adc].InRangeA1.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeA1.all, dummy_rx, 2);
    }
    else if (channel == 4)
    {
        AD7616[adc].InRangeA2.bit.V4A_Range = range;
        AD7616[adc].InRangeA2.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeA2.all, dummy_rx, 2);
    }
    else if (channel == 5)
    {
        AD7616[adc].InRangeA2.bit.V5A_Range = range;
        AD7616[adc].InRangeA2.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeA2.all, dummy_rx, 2);
    }
    else if (channel == 6)
    {
        AD7616[adc].InRangeA2.bit.V6A_Range = range;
        AD7616[adc].InRangeA2.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeA2.all, dummy_rx, 2);
    }
    else if (channel == 7)
    {
        AD7616[adc].InRangeA2.bit.V7A_Range = range;
        AD7616[adc].InRangeA2.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeA2.all, dummy_rx, 2);
    }
    else if (channel == 8)
    {
        AD7616[adc].InRangeB1.bit.V0B_Range = range;
        AD7616[adc].InRangeB1.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeB1.all, dummy_rx, 2);
    }
    else if (channel == 9)
    {
        AD7616[adc].InRangeB1.bit.V1B_Range = range;
        AD7616[adc].InRangeB1.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeB1.all, dummy_rx, 2);
    }
    else if (channel == 10)
    {
        AD7616[adc].InRangeB1.bit.V2B_Range = range;
        AD7616[adc].InRangeB1.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeB1.all, dummy_rx, 2);
    }
    else if (channel == 11)
    {
        AD7616[adc].InRangeB1.bit.V3B_Range = range;
        AD7616[adc].InRangeB1.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeB1.all, dummy_rx, 2);
    }
    else if (channel == 12)
    {
        AD7616[adc].InRangeB2.bit.V4B_Range = range;
        AD7616[adc].InRangeB2.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeB2.all, dummy_rx, 2);
    }
    else if (channel == 13)
    {
        AD7616[adc].InRangeB2.bit.V5B_Range = range;
        AD7616[adc].InRangeB2.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeB2.all, dummy_rx, 2);
    }
    else if (channel == 14)
    {
        AD7616[adc].InRangeB2.bit.V6B_Range = range;
        AD7616[adc].InRangeB2.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeB2.all, dummy_rx, 2);
    }
    else if (channel == 15)
    {
        AD7616[adc].InRangeB2.bit.V7B_Range = range;
        AD7616[adc].InRangeB2.bit.W_nR      = 1;
        ret = Transfer(adc, AD7616[adc].InRangeB2.all, dummy_rx, 2);
    }
    
    return ret;
}


int AD7616_Set_Oversampling(uint8_t adc, uint8_t oversampling)
{   
    if (!init_done)
    {
        printf("AD7616_Open() function must be called first.\n");
        return -1;
    }

    if (adc != ADC_1 && adc != ADC_2) 
    {
        printf("Invalid adc selected: %d\n", adc);
        return -1;
    }

    if (oversampling < 0 || oversampling > 7)
    {
        printf("Invalid oversampling selected: %d\n", oversampling);
        return -1;
    }

    AD7616[adc].Config.bit.OverSamp = oversampling;
    AD7616[adc].Config.bit.W_nR     = 1;

    uint8_t dummy_rx[2];

    return Transfer(adc, AD7616[adc].Config.all, dummy_rx, 2);
}


uint8_t AD7616_Get_Oversampling(uint8_t adc)
{
    return AD7616[adc].Config.bit.OverSamp;
}


int AD7616_Read_All_Voltages(uint8_t adc, ADC_Channels* channels)
{
    if (!init_done)
    {
        printf("AD7616_Open() function must be called first.\n");
        return -1;
    }

    if (adc != ADC_1 && adc != ADC_2)
    {
        printf("Invalid adc selected: %d\n", adc);
        return -1;
    }
    
    float A_scale, B_scale;
    int ret;

    ret = Read_All_Channel(adc, channels->RawAB);
    if (ret < 0) return ret;

    int16_t RawAB[16];
    int i; 

    for (i = 0; i < 16; i++)
    {
        RawAB[i] = (int16_t)(channels->RawAB[2*i+0] << 8) | channels->RawAB[2*i+1]; 
    } 

	for(i = 0; i < 8; i++)
	{   
        A_scale = Calc_Scale(Get_Input_Range(adc, 2*i+0));
        B_scale = Calc_Scale(Get_Input_Range(adc, 2*i+1));
		channels->VA[i] = RawAB[2*i+0] * A_scale;
		channels->VB[i] = RawAB[2*i+1] * B_scale;
	}

    return 0;
}


static float Calc_Scale(uint8_t range)
{
    if (range == AD7616_InputRange_2V5_PN) return 5/(float)(65536);
    else if (range == AD7616_InputRange_5V0_PN) return 10/(float)(65536);
    else if (range == AD7616_InputRange_10V_PN) return 20/(float)(65536);
}


static int Get_Input_Range(uint8_t adc, uint8_t channel)
{
    if (channel == 0) return AD7616[adc].InRangeA1.bit.V0A_Range;
    else if (channel == 1) return AD7616[adc].InRangeA1.bit.V1A_Range;
    else if (channel == 2) return AD7616[adc].InRangeA1.bit.V2A_Range;
    else if (channel == 3) return AD7616[adc].InRangeA1.bit.V3A_Range;
    else if (channel == 4) return AD7616[adc].InRangeA2.bit.V4A_Range;
    else if (channel == 5) return AD7616[adc].InRangeA2.bit.V5A_Range;
    else if (channel == 6) return AD7616[adc].InRangeA2.bit.V6A_Range;
    else if (channel == 7) return AD7616[adc].InRangeA2.bit.V7A_Range;
    else if (channel == 8) return AD7616[adc].InRangeB1.bit.V0B_Range;
    else if (channel == 9) return AD7616[adc].InRangeB1.bit.V1B_Range;
    else if (channel == 10) return AD7616[adc].InRangeB1.bit.V2B_Range;
    else if (channel == 11) return AD7616[adc].InRangeB1.bit.V3B_Range;
    else if (channel == 12) return AD7616[adc].InRangeB2.bit.V4B_Range;
    else if (channel == 13) return AD7616[adc].InRangeB2.bit.V5B_Range;
    else if (channel == 14) return AD7616[adc].InRangeB2.bit.V6B_Range;
    else if (channel == 15) return AD7616[adc].InRangeB2.bit.V7B_Range;
}


static int Read_All_Channel(uint8_t adc, int8_t* Res)
{
    static int first_time = 1;
    int ret;

	if(first_time)
	{
		first_time = 0;

        gpiod_line_set_value(AD7616[adc].convst_line, 1);
		gpiod_line_set_value(AD7616[adc].convst_line, 0);
        
        ret = Wait_For_Busy(adc, 100);
        if (ret < 0)
        {
            printf("adc%d: Waiting for busy line failed.\n", adc);
            return ret;
        }
        
        uint8_t dummy1[6] = {0,};
        uint8_t res1[6];

        Transfer(adc, dummy1, res1, 6);

        gpiod_line_set_value(AD7616[adc].convst_line, 1);
        gpiod_line_set_value(AD7616[adc].convst_line, 0);
		
        ret = Wait_For_Busy(adc, 100);
        if (ret < 0)
        {
            printf("adc%d: Waiting for busy line failed.\n", adc);
            return ret;
        }
        
        uint8_t dummy2[32] = {0, };
        
	    return Transfer(adc, dummy2, Res, 32);
	}
	else
	{
        gpiod_line_set_value(AD7616[adc].convst_line, 1);
		gpiod_line_set_value(AD7616[adc].convst_line, 0);
		
        ret = Wait_For_Busy(adc, 100);
        if (ret < 0)
        {
            printf("adc%d: Waiting for busy line failed.\n", adc);
            return ret;
        }

        uint8_t dummy2[32] = {0, };
        
	    return Transfer(adc, dummy2, Res, 32);
	}
}


static int Set_Channel(uint8_t adc, uint8_t A_channel, uint8_t B_channel)
{
    if (!init_done)
    {
        printf("AD7616_Open() function must be called first.\n");
        return -1;
    }

    if (adc != ADC_1 && adc != ADC_2)
    {
        printf("Invalid adc selected: %d\n", adc);
        return -1;
    }

    if (A_channel < 0 || A_channel > 9)
    {
        printf("Invalid A_channel selected: %d\n", A_channel);
        return -1;
    }


    if (B_channel < 0 || B_channel > 9)
    {
        printf("Invalid B_channel selected: %d\n", B_channel);
        return -1;
    }

    union Channels_Reg  Ch_Reg;
    uint8_t dummy_rx[2] = {0};

    Ch_Reg.bit.ADC_A_SEL   = A_channel & 0xF;
    Ch_Reg.bit.ADC_B_SEL   = B_channel & 0xF;
    Ch_Reg.bit.Rsvrd       = 0;
    Ch_Reg.bit.Address     = AD7616_Add_Channel_Register;
    Ch_Reg.bit.W_nR        = 1;

    return Transfer(adc, Ch_Reg.all, dummy_rx, 2);
}


static int Wait_For_Busy(uint8_t adc, uint8_t timeout_us)
{   
    struct timespec start_time, current_time;
    long elapsed_time = 0;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while (gpiod_line_get_value(AD7616[adc].busy_line)) 
    {
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        elapsed_time = (current_time.tv_sec - start_time.tv_sec) * 1000000 +
                       (current_time.tv_nsec - start_time.tv_nsec) / 1000;

        if (elapsed_time > timeout_us)
            return -1;
        
    }

    return 0;
}


static int Transfer(uint8_t adc, uint8_t* tx, uint8_t* rx, uint8_t len)
{
    int ret;

    uint8_t tx_swapped[len];

    for(int i = 0; i < len; i+=2) 
    {
        tx_swapped[i] = tx[i+1];
        tx_swapped[i+1] = tx[i];
    }

    struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx_swapped,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

    ret = ioctl(AD7616[adc].adc_spi_fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        printf("adc %d: can't send spi message\n", adc);
        return -1;
    }  

    return 0;
}
