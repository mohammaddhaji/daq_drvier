#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sched.h>
#include <string.h>
#include <termios.h>
#include <errno.h>

#include <ad7616.h>
#include <ltc2668.h>
#include <mcp23s17.h>
#include <rttimer.h>
#include <uart.h>


#define PACKET_SIZE 6

#define HEADER_1 1
#define HEADER_2 2
#define IN_MESSAGE 3
int state = HEADER_1;

unsigned char tx_packet[PACKET_SIZE];
unsigned int counter = 0;

uint8_t buffer[1024];
uint8_t rx_packet[PACKET_SIZE - 2];
int buffer_idx = 0;

int serial_fd;



void my_job(void){

    static int firsttime = 1;

    if (firsttime){

        firsttime = 0;

        int ret = 0;

        //////////////////////////////////////////////////////////////////////
        //////////////////////////////// UART ////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        ret = UART_Open(UART_1_4_6_8);
        if (ret < 0){
            printf("UART Open faild\n");
            Stop_Job();
            return;
        }

        UART_Change_Mode(UART_1_5_6_9);
        
        
        struct termios serial_settings;

        // Open the serial port
        serial_fd = open(UART1_485_422, O_RDWR | O_NOCTTY);
        if (serial_fd == -1) {
            perror("Error opening serial port");
            Stop_Job();
            return;
        }

        // Configure the serial port
        tcgetattr(serial_fd, &serial_settings);
        cfsetispeed(&serial_settings, B19200);
        cfsetospeed(&serial_settings, B19200);
        serial_settings.c_cflag &= ~PARENB; // Disable parity bit
        serial_settings.c_cflag &= ~CSTOPB; // Set one stop bit
        serial_settings.c_cflag &= ~CSIZE;  // Clear data size bits
        serial_settings.c_cflag |= CS8;     // Set 8 data bits
        serial_settings.c_cflag &= ~CRTSCTS; // Disable hardware flow control
        serial_settings.c_cflag |= CREAD | CLOCAL; // Enable receiver and ignore modem control lines
        tcsetattr(serial_fd, TCSANOW, &serial_settings);

        // Set the read operation to non-blocking mode
        int flags = fcntl(serial_fd, F_GETFL, 0);
        flags |= O_NONBLOCK;
        fcntl(serial_fd, F_SETFL, flags); 
        

        //////////////////////////////////////////////////////////////////////
        ///////////////////////////////// IOE ////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        ret = MCP23S17_Open();
        if (ret < 0){
            printf("MCP23S17 Open faild\n");
            Stop_Job();
            return;
        }

        ret = MCP23S17_Set_Pullup_Mode(IOE_1, ALL_PINS, PULLUP_ENABLED);
        if (ret < 0)
            printf("Setting all pins of ioe1 pullup enabled failed\n");

        ret = MCP23S17_Set_Pullup_Mode(IOE_2, PIN_5, PULLUP_DISABLED);
        if (ret < 0)
            printf("Setting pin 5 of ioe2 pullup disabled failed\n");

        ret = MCP23S17_Set_Direction(IOE_1, PIN_0, DIR_OUTPUT);
        if (ret < 0)
            printf("Setting pin 0 of ioe1 direction output failed\n");

        ret = MCP23S17_Set_Direction(IOE_2, ALL_PINS, DIR_INPUT);
        if (ret < 0)
            printf("Setting all pins of ioe2 direction input failed\n");

        ret = MCP23S17_Get_Level(IOE_1, PIN_0);
        if (ret < 0)
            printf("Digital read of pin 0 of ioe1 failed\n");

        ret = MCP23S17_Get_Level(IOE_2, PIN_8);
        if (ret < 0)
            printf("Digital read of pin 8 of ioe2 failed\n");

        ret = MCP23S17_Set_Level(IOE_1, PIN_0, LEVEL_HIGH);
        if (ret < 0)
            printf("Digital write on pin 0 of ioe1 failed\n");

        ret = MCP23S17_Set_Level(IOE_2, PIN_8, LEVEL_LOW);
        if (ret < 0)
            printf("Digital write on pin 8 of ioe2 failed\n");

        ret = MCP23S17_Set_Data(IOE_1, 0x85);
        if (ret < 0)
            printf("Write GPIO of ioe1 failed\n");

        uint16_t data = MCP23S17_Get_Data(IOE_2);

        //////////////////////////////////////////////////////////////////////
        ///////////////////////////////// DAC ////////////////////////////////
        //////////////////////////////////////////////////////////////////////       

        ret = LTC2668_Open();
        if (ret < 0){
            printf("LTC2668 Open faild\n");
            Stop_Job();
            return;
        }

        ret = LTC2668_Set_Softspan_Range(ALL_CHANNELS, LTC2668_SPAN_PLUS_MINUS_10V);
        if (ret < 0)
            printf("Setting softspan range failed\n");

        ret = LTC2668_Set_Reference_Mode(REF_EXTERNAL);
        if (ret < 0)
            printf("Setting reference mode failed\n");

        ret = LTC2668_Set_Output_Voltage(ALL_CHANNELS, 0);
        if (ret < 0)
            printf("Setting output voltage failed\n");

        //////////////////////////////////////////////////////////////////////
        ///////////////////////////////// ADC ////////////////////////////////
        //////////////////////////////////////////////////////////////////////       

        ret = AD7616_Open();
        if (ret < 0){
            printf("AD7616 Open faild\n");
            Stop_Job();
            return;
        }

        ret = AD7616_Set_Input_Range_All(ADC_1, AD7616_InputRange_10V_PN);
        if (ret < 0)
            printf("Setting adc1 input range failed\n");

        ret = AD7616_Set_Oversampling(ADC_1, AD7616_OverSampling_8);
        if (ret < 0)
            printf("Setting adc1 oversampling failed\n");


        ret = AD7616_Set_Input_Range_All(ADC_2, AD7616_InputRange_10V_PN);
        if (ret < 0)
            printf("Setting adc2 input range failed\n");

        ret = AD7616_Set_Oversampling(ADC_2, AD7616_OverSampling_8);
        if (ret < 0)
            printf("Setting adc2 oversampling failed\n");

    }
    else{

        ADC_Channels ADC1_Channels, ADC2_Channels;
        int ret;

        ret = AD7616_Read_All_Voltages(ADC_2, &ADC2_Channels);
        printf("V2A: %f\n", ADC2_Channels.VA[2]);

        //////////////////////////////////////////////////////////////////////
        //////////////////////////////// UART ////////////////////////////////
        //////////////////////////////////////////////////////////////////////
        
        tx_packet[0] = 0xAA;
        tx_packet[1] = 0xBB;
        tx_packet[2] = (counter >> 24) & 0xFF;
        tx_packet[3] = (counter >> 16) & 0xFF;
        tx_packet[4] = (counter >> 8) & 0xFF;
        tx_packet[5] = counter & 0xFF; 
        // Send the packet
        ssize_t bytes_written = write(serial_fd, tx_packet, PACKET_SIZE);
        if (bytes_written != PACKET_SIZE) 
            perror("Error writing to serial port");
        
        counter++;

        // Receive the packet
        ssize_t bytes_read = read(serial_fd, buffer, PACKET_SIZE);
        if (bytes_read == -1) 
            if (!(errno == EAGAIN || errno == EWOULDBLOCK)) 
                perror("Error reading from serial port"); 
        
        buffer_idx = 0;
        
        if (bytes_read > 0) {
            while (buffer_idx < bytes_read) {
                if (state == HEADER_1) {
                    if (buffer[buffer_idx] == 0xAA) {
                        state = HEADER_2;
                    }
                } else if (state == HEADER_2) {
                    if (buffer[buffer_idx] == 0xBB) {
                        state = IN_MESSAGE;
                        memset(rx_packet, 0, sizeof(rx_packet));
                    }
                } else if (state == IN_MESSAGE) {
                    if (buffer_idx == PACKET_SIZE - 2) {
                        buffer_idx--;
                        state = HEADER_1;
                        uint32_t receivedValue = 0;
                        memcpy(&receivedValue, rx_packet, sizeof(receivedValue));
                        printf("received counter: %u\n", receivedValue);
                    } else {
                        rx_packet[buffer_idx] = buffer[buffer_idx];
                    }
                }

                buffer_idx++;
            }
        }
    }
}


void on_ctrl_c(int sig) {
    Stop_Job();
    printf("\nTimer Stoped. Releasing Resources...\n");
    AD7616_Close();
    LTC2668_Close();
    MCP23S17_Close();
    UART_Close();
}


int main(int argc, char *argv[]){

    if (argc != 2) {
        printf("Usage: %s <interval in ms>\n", argv[0]);
        return 1;
    }

    // Running this program on linux realtime scheduler with highest priority.
    pid_t pid = getpid();
    struct sched_param param;
    param.sched_priority = 99;
    int result = sched_setscheduler(pid, SCHED_FIFO, &param);
    if (result != 0) {
        perror("sched_setscheduler");
        return -1;
    }
    mlockall(MCL_CURRENT | MCL_FUTURE);
    signal(SIGINT, on_ctrl_c);
 


    int interval_ms = atoi(argv[1]);
    Start_Job(my_job, interval_ms*1000);


    return 0;
}