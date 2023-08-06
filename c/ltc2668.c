#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <gpiod.h>
#include <math.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "ltc2668.h"





//! @name LTC2668 Command Codes
//! OR'd together with the DAC address to form the command byte
#define  LTC2668_CMD_WRITE_N              0x00  //!< Write to input register n
#define  LTC2668_CMD_UPDATE_N             0x10  //!< Update (power up) DAC register n
#define  LTC2668_CMD_WRITE_N_UPDATE_ALL   0x20  //!< Write to input register n, update (power-up) all
#define  LTC2668_CMD_WRITE_N_UPDATE_N     0x30  //!< Write to input register n, update (power-up)
#define  LTC2668_CMD_POWER_DOWN_N         0x40  //!< Power down n
#define  LTC2668_CMD_POWER_DOWN_ALL       0x50  //!< Power down chip (all DAC's, MUX and reference)

#define  LTC2668_CMD_SPAN                 0x60  //!< Write span to dac n
#define  LTC2668_CMD_CONFIG               0x70  //!< Configure reference / toggle
#define  LTC2668_CMD_WRITE_ALL            0x80  //!< Write to all input registers
#define  LTC2668_CMD_UPDATE_ALL           0x90  //!< Update all DACs
#define  LTC2668_CMD_WRITE_ALL_UPDATE_ALL 0xA0  //!< Write to all input reg, update all DACs
#define  LTC2668_CMD_MUX                  0xB0  //!< Select MUX channel (controlled by 5 LSbs in data word)
#define  LTC2668_CMD_TOGGLE_SEL           0xC0  //!< Select which DACs can be toggled (via toggle pin or global toggle bit)
#define  LTC2668_CMD_GLOBAL_TOGGLE        0xD0  //!< Software toggle control via global toggle bit
#define  LTC2668_CMD_SPAN_ALL             0xE0  //!< Set span for all DACs
#define  LTC2668_CMD_NO_OPERATION         0xF0  //!< No operation
//! @}


//! @name LTC2668 Minimums and Maximums for each Span
//! @{
//! Lookup tables for minimum and maximum outputs for a given span
const float LTC2668_MIN_OUTPUT[5] = {0.0, 0.0, -5.0, -10.0, -2.5};
const float LTC2668_MAX_OUTPUT[5] = {5.0, 10.0, 5.0, 10.0, 2.5};
//! @}

//! @name LTC2668 Configuration options
//! @{
//! Used in conjunction with LTC2668_CMD_CONFIG command
#define  LTC2668_REF_DISABLE              0x0001  //! Disable internal reference to save power when using an ext. ref.
#define  LTC2668_THERMAL_SHUTDOWN         0x0002  //! Disable thermal shutdown (NOT recommended)
//! @}

//! @name LTC2668 MUX enable
//! @{
//! Used in conjunction with LTC2668_CMD_MUX command
#define  LTC2668_MUX_DISABLE              0x0000  //! Disable MUX
#define  LTC2668_MUX_ENABLE               0x0010  //! Enable MUX, OR with MUX channel to be monitored
//! @}

//! @name LTC2668 Global Toggle
//! @{
//! Used in conjunction with LTC2668_CMD_GLOBAL_TOGGLE command, affects DACs whose
//! Toggle Select bits have been set to 1
#define  LTC2668_TOGGLE_REG_A              0x0000  //! Update DAC with register A
#define  LTC2668_TOGGLE_REG_B              0x0010  //! Update DAC with register B
//! @}


typedef union
{
	int16_t  LT_int16;    //!< 16-bit signed integer to be converted to two bytes
	uint16_t LT_uint16;   //!< 16-bit unsigned integer to be converted to two bytes
	uint8_t  LT_byte[2];  //!< 2 bytes (unsigned 8-bit integers) to be converted to a 16-bit signed or unsigned integer
}LT_union_int16_2bytes;


// SPI Stuff -----------------------------------------------------------------------------

static const char *dac_spi_dev = "/dev/spidev1.0";
static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 20000000;
static int dac_spi_fd;

// GPIO Stuff -----------------------------------------------------------------------------

#ifndef	CONSUMER
#define	CONSUMER    "LTC2668_DRIVER"
#endif

static const char *gpiochip5 = "gpiochip5";
static struct gpiod_chip *chip5;
static struct gpiod_line *dac_cs;
static struct gpiod_line *adc_cs;
static unsigned int dac_cs_line = 12;
static unsigned int adc_cs_line = 7;

//-----------------------------------------------------------------------------------------

static uint16_t Voltage_To_Code(float dac_voltage, float min_output, float max_output);
static bool All_Softspan_Same(void);
static int Update_Power_Up_Channel(uint8_t channel);
static void Power_Down_Channel(uint8_t channel);
static int LTC2668_write(uint8_t dac_command, uint8_t dac_address, uint16_t dac_code);
static int Transfer(uint8_t* tx, uint8_t* rx, uint8_t len);

static uint8_t Channels_Softspan[16] = {0};

static bool init_done = false;


int LTC2668_Open(void)
{
    if (init_done) return 0;
    
    int ret;

    // DAC SPI Init---------------------------------------------------------
	dac_spi_fd = open(dac_spi_dev, O_RDWR);
	if (dac_spi_fd < 0)
    {
		printf("Can't open device dac\n");
        return dac_spi_fd;
    }

	ret = ioctl(dac_spi_fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
    {
		printf("Can't set spi mode for dac\n");
        return ret;
    }

    ret = ioctl(dac_spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
    {
		printf("Can't set bits per word for dac\n");
        return ret;
    }

    ret = ioctl(dac_spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
    {
		printf("Can't set max speed hz for dac\n");
        return ret;
    }

    // DAC GPIO Init----------------------------------------------------
    chip5 = gpiod_chip_open_by_name(gpiochip5);
    if (!chip5) 
    {
        printf("Open gpiochip6 failed\n");
        ret = -1;
        goto end;
    }

	dac_cs = gpiod_chip_get_line(chip5, dac_cs_line);
	if (!dac_cs) 
    {
		printf("Get dac_cs failed\n");
        ret = -1;
        goto close_chip;
	}

	ret = gpiod_line_request_output(dac_cs, CONSUMER, 1);
	if (ret < 0) 
    {
		printf("Request dac_cs as output failed\n");
        goto release_dac_cs;
	}

    adc_cs = gpiod_chip_get_line(chip5, adc_cs_line);
	if (!adc_cs) 
    {
		printf("Get adc_cs failed\n");
        ret = -1;
        goto release_dac_cs;
	}

	ret = gpiod_line_request_output(adc_cs, CONSUMER, 1);
	if (ret < 0) 
    {
		printf("Request adc_cs as output failed\n");
        goto release_adc_cs;
	}

    Update_Power_Up_Channel(ALL_CHANNELS);

    init_done = true;

    ret = LTC2668_Set_Softspan_Range(ALL_CHANNELS, LTC2668_SPAN_PLUS_MINUS_10V);
    if (ret < 0)
        printf("Setting softspan range failed\n");

    ret = LTC2668_Set_Reference_Mode(REF_EXTERNAL);
    if (ret < 0)
        printf("Setting reference mode failed\n");


    return 0;


release_adc_cs:
    gpiod_line_release(adc_cs);
release_dac_cs:
    gpiod_line_release(dac_cs);
close_chip:
    gpiod_chip_close(chip5);
end:
    init_done = false;
    return ret;
}


void LTC2668_Close(void)
{
    if (init_done)
    {
        Power_Down_Channel(ALL_CHANNELS);
        close(dac_spi_fd);
        gpiod_line_release(dac_cs);
        gpiod_line_release(adc_cs);
        gpiod_chip_close(chip5);

        init_done = false;
    }
}


int LTC2668_Set_Reference_Mode(uint8_t ref)
{
    if (!init_done)
    {
        printf("LTC2668_Open() function must be called first.\n");
        return -1;
    }

    if (ref != REF_EXTERNAL && ref != REF_INTERNAL)
    {
        printf("Invalid reference mode selected : %d\n", ref);
        return -1;
    }

    return LTC2668_write(LTC2668_CMD_CONFIG, 0, ref);
}


int LTC2668_Set_Softspan_Range(uint8_t channel, uint8_t softspan_range)
{
    if (!init_done)
    {
        printf("LTC2668_Open() function must be called first.\n");
        return -1;
    }

    if (softspan_range < 0 || softspan_range > 4)
    {
        printf("Invalid softspan range selected: %d\n", softspan_range);
        return -1;
    }

    if (channel < 16)
    {
        Channels_Softspan[channel] = softspan_range;
        return LTC2668_write(LTC2668_CMD_SPAN, channel, softspan_range);
    }
    else
    {
        for(int i = 0; i < 16; i++)
            Channels_Softspan[i] = softspan_range;
        return LTC2668_write(LTC2668_CMD_SPAN_ALL, 0, softspan_range);
    }
}


int LTC2668_Set_Output_Voltage(uint8_t channel, float voltage)
{
    if (!init_done)
    {
        printf("LTC2668_Open() function must be called first.\n");
        return -1;
    }

    float min, max, corrected_voltage;

    if (channel < 16)
    {    
        min = LTC2668_MIN_OUTPUT[Channels_Softspan[channel]];
        max = LTC2668_MAX_OUTPUT[Channels_Softspan[channel]];
        
        if (voltage < min) corrected_voltage = min;
        else if (voltage > max) corrected_voltage = max;
        else corrected_voltage = voltage;

        uint16_t code = Voltage_To_Code(corrected_voltage, min, max);

        return LTC2668_write(LTC2668_CMD_WRITE_N_UPDATE_N, channel, code);
        
    }
    else{

        if (!All_Softspan_Same())
        {
            printf("Some Channels have different sotfspan. Setting Voltage is applicable for a single channel only.\n");
            return -1;
        }
        else
        {
            min = LTC2668_MIN_OUTPUT[Channels_Softspan[0]];
            max = LTC2668_MAX_OUTPUT[Channels_Softspan[0]];
            
            if (voltage < min) corrected_voltage = min;
            else if (voltage > max) corrected_voltage = max;
            else corrected_voltage = voltage;

            uint16_t code = Voltage_To_Code(corrected_voltage, min, max);

            return LTC2668_write(LTC2668_CMD_WRITE_ALL_UPDATE_ALL, 0, code);
        }
    }
}


static uint16_t Voltage_To_Code(float dac_voltage, float min_output, float max_output)
{
    uint16_t dac_code;
    float float_code;
    float_code = 65535.0 * (dac_voltage - min_output) / (max_output - min_output);                    
    float_code = (float_code > (floor(float_code) + 0.5)) ? ceil(float_code) : floor(float_code);     
    if (float_code < 0.0) float_code = 0.0;
    if (float_code > 65535.0) float_code = 65535.0;
    dac_code = (uint16_t) (float_code);
    return dac_code;
}


static bool All_Softspan_Same(void) {
    for (int i = 1; i < 16; i++) 
    {
        if (Channels_Softspan[i] != Channels_Softspan[0]) 
            return false;
    }
    return true;
}


static int Update_Power_Up_Channel(uint8_t channel)
{
    if (channel < 16)
        return LTC2668_write(LTC2668_CMD_UPDATE_N, channel, 0);
    else
        return LTC2668_write(LTC2668_CMD_UPDATE_ALL, 0, 0);
}


static void Power_Down_Channel(uint8_t channel)
{
    if (channel < 16)
        LTC2668_write(LTC2668_CMD_POWER_DOWN_N, channel, 0);
    else
        LTC2668_write(LTC2668_CMD_POWER_DOWN_ALL, 0, 0);
}


static int LTC2668_write(uint8_t dac_command, uint8_t dac_address, uint16_t dac_code)
{
    static uint8_t last_data_array[4];
    uint8_t data_array[4], rx_array[4];
    int8_t ret;
    LT_union_int16_2bytes data;

    data.LT_int16 = dac_code;                     // Copy DAC code to union
    data_array[0] = 0;                            // Only required for 32 byte readback transaction
    data_array[1] = dac_command | dac_address;    // Build command / address byte
    data_array[2] = data.LT_byte[1];              // MS Byte
    data_array[3] = data.LT_byte[0];              // LS Byte


    gpiod_line_set_value(dac_cs, 0);
    ret = Transfer(data_array, rx_array, 4);
    gpiod_line_set_value(dac_cs, 1);
    
    if (ret < 0) return ret;

    // Compare data read back to data that was sent the previous
    // time this function was called
    if ((rx_array[2] == last_data_array[2]) && 
        (rx_array[1] == last_data_array[1]) && 
        (rx_array[0] == last_data_array[0]))

        ret = 0;
    else
        ret = -1;

    // Copy data array to a static array to compare
    // the next time the function is called
    last_data_array[0] = data_array[0]; 
    last_data_array[1] = data_array[1]; 
    last_data_array[2] = data_array[2];

    return(ret);
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

    
    ret = ioctl(dac_spi_fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        printf("dac: can't send spi message\n");
        return -1;
    }

    return 0;
}
