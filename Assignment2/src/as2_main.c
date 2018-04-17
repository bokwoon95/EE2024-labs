/*****************************************************************************
 *
 *   EE2024 Assignment 2 Program
 *
 *   Tuesday Lab Group 28
 *   Ernest Ong (A0154968L)
 *   Chua Bok Woon (A0155737W)
 *   NOTE: some comments are inaccurate but the project's over and I'm too lazy
 *         to change them
 ******************************************************************************/

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"

#include <stdio.h>
#include <stdlib.h>

#include "led7seg.h"
#include "joystick.h"
#include "pca9532.h"
#include "light.h"
#include "acc.h"
#include "oled.h"
#include "temp.h"
#include "rgb.h"
#include "string.h"

// Stores the 3 possible states the rocket can be in
typedef enum {
    STATIONARY,
    LAUNCH,
    RETURN,
} RocketMode;
// Start the rocket off in Stationary mode
RocketMode mode = STATIONARY;

// msTicks tracks the number of milliseconds the program has been running
volatile uint32_t msTicks;

// ms_counter_x regulates the blinking of the red, blue, and red+blue LED
// 3 separate counters are needed so that they will not interfere with each other when the blinking changes from
//   (blinking red) or (blinking blue) to (blinking red+blue)
// We have decided that using only one ms_counter and resetting it to 0 whenever the rocket changes blinking status
//   would need extra flags and added complexity so using three counters instead isn't that much more complicated
volatile uint32_t ms_counter_rb;
volatile uint32_t ms_counter_r;
volatile uint32_t ms_counter_b;

// msg is a character array pointer that is used to store UART messages
static char* msg = NULL;

int MODE_TOGGLE = 0; // SW3 flag
int SW3_LastPressedTick = 0; // tracks the last time (in msticks) SW3 was pressed
int SW3_CurrentPressedTick = 0; // stores the current time (in msticks) where SW3 was pressed

// THRESHOLD VALUES FOR WARNINGS
//==============================
int OBSTACLE_NEAR_THRESHOLD = 3000; // in lux
float TEMP_HIGH_THRESHOLD = 26.0; // in degree Celsius
float ACC_THRESHOLD = 0.4; // in g

// WARNING FLAGS
int TEMP_WARNING = 0; //checks if TEMP_WARNING flag is active
int ACC_WARNING = 0; //checks if ACC_WARNING flag is active
int OBST_WARNING = 0; //checks if OBST_WARNING flag is active
int CLEAR_WARNING = 0; // checks if warning flags should be cleared

// These flags check whether a warning/sensor values has been sent to UART once
// This is useful to check so that you can print warnings/sensor values once and stop printing,
//   rather than printing it repeatedly in TeraTerm as the program loops repeatedly
int isTempWarningSent = 0;
int isAccWarningSent = 0;
// Set all these variables back to 0
int isObstNearWarningSent = 0;
int isObstAvoidedWarningSent = 0;
int areTempAndAccValuesSent = 0;
int isDistValueSent = 0;

// These variables track the last time (in msticks) these sensor values were sent to UART
int TempAndAcc_LastSentTick = 0;
int Dist_LastSentTick = 0;

// DECLARE VARIABLES TO STORE SENSOR VALUES
//=========================================
// distance stores the sensors values from the light sensor
int distance;
// my_temp_value stores the sensor values from the temperature sensor
uint32_t my_temp_value;
// x, y & z store the sensor values from the accelerometer
int8_t x;
int8_t y;
int8_t z;
// The offset variables store the initial values that x, y & z are set to when the program starts
// The actual x, y & z values are then found by adding the offset values to the sensor values of x, y & z
// Note that the offset variables do not change throughout the program's execution
int32_t xoff;
int32_t yoff;
int32_t zoff;
// display_x, display_y are character arrays that will store x & y accelerometer string readings to be displayed on the OLED
// display_temp is a character array that will store temperature string readings to be displayed on the OLED
// display_distance is a character array that will store light sensor string readings to be displayed on the OLED
// report_x, report_y are character arrays that will store x & y accelerometer string readings to be sent to UART
char display_x[40];
char display_y[40];
char display_temp[40];
char display_distance[40];
char report_x[40];
char report_y[40];

//Stationary mode variables
int countdown;
int currentTime;
int counter;
// Return mode variables
int obst_flag = 0;

// UART STUFF
//===========
uint8_t receiving_msg[5];    // Buffer to receive message (from TeraTerm)
uint32_t msg_len = 0;    // Receiving buffer's message length
uint32_t isReceived = 0; // Init to be not received
int rpt_status = 0;

// Every system tick, an interrupt kicks in to run the SysTick_Handler
// Since the blinking of the LEDs are extremely time-sensitive, their timely blinking is managed by the Systick_Handler
void SysTick_Handler (void) {
    msTicks++;

    if (mode == STATIONARY) {
        // If TEMP_WARNING flag is active, blink red LED every 333ms
        if (TEMP_WARNING) {
            if (ms_counter_r >= 0 && ms_counter_r < 333) {
                GPIO_ClearValue(0, (1<<26)); //clear blue led
                GPIO_SetValue(2, (1<<0)); //set red led
            } else if (ms_counter_r >= 333 && ms_counter_r < 666) {
                GPIO_ClearValue(0, (1<<26)); //clear blue led
                GPIO_ClearValue(2, (1<<0)); //clear red led
            } else if (ms_counter_r >= 666) {
                ms_counter_r = 0;
            }
            ms_counter_r++;
        }
    } else if (mode == LAUNCH) {
        // Every 10s, clear the areTempAndAccValuesSent flag
        // If that flag is cleared, launchMode will send current Temperature & Accelerometer Values to UART, 
        //   then set flag back to 1
        if (msTicks - TempAndAcc_LastSentTick >= 10000) {
            areTempAndAccValuesSent = 0;
            TempAndAcc_LastSentTick = msTicks;
        }

        // Blink the red and/or blue LEDs depending on TEMP_WARNING & ACC_WARNING flags
        // TEMP_WARNING                 : red every 333ms (ms_counter_r)
        // ACC_WARNING                  : blue every 333ms (ms_counter_b)
        // TEMP_WARNING & ACC_WARNING   : red followed by blue every 333ms (ms_counter_rb)
        // Each blinking pattern has its own ms_counter
        // If neither warnings are active, set all three counters back to zero
        if (TEMP_WARNING && ACC_WARNING) { 
            // blink red followed by blue every 333ms
            if (ms_counter_rb >= 0 && ms_counter_rb < 333) {
                GPIO_ClearValue(2, (1<<0)); //clear red led
                GPIO_SetValue(0, (1<<26)); //set blue led
            } else if (ms_counter_rb >= 333 && ms_counter_rb < 666) {
                GPIO_ClearValue(0, (1<<26)); //clear blue led
                GPIO_SetValue(2, (1<<0)); //set red led
            } else if (ms_counter_rb >= 666) {
                ms_counter_rb = 0;
            }
            ms_counter_rb++;
        } else if (TEMP_WARNING && !ACC_WARNING) {
            // blink red every 333ms, always clear blue
            if (ms_counter_r >= 0 && ms_counter_r < 333) {
                GPIO_ClearValue(0, (1<<26)); //clear blue led
                GPIO_SetValue(2, (1<<0)); //set red led
            } else if (ms_counter_r >= 333 && ms_counter_r < 666) {
                GPIO_ClearValue(0, (1<<26)); //clear blue led
                GPIO_ClearValue(2, (1<<0)); //clear red led
            } else if (ms_counter_r >= 666) {
                ms_counter_r = 0;
            }
            ms_counter_r++;
        } else if (!TEMP_WARNING && ACC_WARNING) {
            // blink blue every 333ms, always clear red
            if (ms_counter_b >= 0 && ms_counter_b < 333) {
                GPIO_ClearValue(2, (1<<0)); //clear red
                GPIO_SetValue(0, (1<<26)); //set blue
            } else if (ms_counter_b >= 333 && ms_counter_b < 666) {
                GPIO_ClearValue(2, (1<<0)); //clear red
                GPIO_ClearValue(0, (1<<26)); //clear blue
            } else if (ms_counter_b >= 666){
                ms_counter_b = 0;
            }
            ms_counter_b++;
        } else {
            ms_counter_rb = 0;
            ms_counter_r = 0;
            ms_counter_b = 0;
        }
    } else if (mode == RETURN) {
        // Every 10s, clear the isDistValueSent flag
        // If that flag is cleared, returnMode() will send current Distance Values to UART, then set flag back to 1
        if (msTicks - Dist_LastSentTick >= 10000) {
            isDistValueSent = 0;
            Dist_LastSentTick = msTicks;
        }
    }
}

// function to return current tick count
uint32_t getTicks(void){
    return msTicks;
}

// Function helper for init_uart()
// Sets the PINSEL Register for UART3 accordingly
void pinsel_uart3(void){
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 2;
    PinCfg.Pinnum = 0;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 1;
    PINSEL_ConfigPin(&PinCfg);
}

// Boilerplate for ssp
static void init_ssp(void) {
    SSP_CFG_Type SSP_ConfigStruct;
    PINSEL_CFG_Type PinCfg;

    /*
     * Initialize SPI pin connect
     * P0.7 - SCK;
     * P0.8 - MISO
     * P0.9 - MOSI
     * P2.2 - SSEL - used as GPIO
     */
    PinCfg.Funcnum = 2;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 7;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 8;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 9;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Funcnum = 0;
    PinCfg.Portnum = 2;
    PinCfg.Pinnum = 2;
    PINSEL_ConfigPin(&PinCfg);

    SSP_ConfigStructInit(&SSP_ConfigStruct);

    // Initialize SSP peripheral with parameter given in structure above
    SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

    // Enable SSP peripheral
    SSP_Cmd(LPC_SSP1, ENABLE);
}

// Boilerplate for i2c
static void init_i2c(void) {
    PINSEL_CFG_Type PinCfg;

    /* Initialize I2C2 pin connect */
    PinCfg.Funcnum = 2;
    PinCfg.Pinnum = 10;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 11;
    PINSEL_ConfigPin(&PinCfg);

    // Initialize I2C peripheral
    I2C_Init(LPC_I2C2, 100000);

    /* Enable I2C1 operation */
    I2C_Cmd(LPC_I2C2, ENABLE);
}

// Boilerplate for uart
void init_uart(void){
    UART_CFG_Type uartCfg;
    uartCfg.Baud_rate = 115200;
    uartCfg.Databits = UART_DATABIT_8;
    uartCfg.Parity = UART_PARITY_NONE;
    uartCfg.Stopbits = UART_STOPBIT_1;
    UART_Init(LPC_UART3, &uartCfg);
    //Pin select for uart3;
    pinsel_uart3();
    // Enable UART Transmit
    UART_TxCmd(LPC_UART3, ENABLE);
}

// Boilerplate for GPIO, plus initialize peripherals: SW3, rgb_red & rgb_blue
static void init_GPIO(void) {
    // Default values for funcnum, opendrain & pinmode
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 0; // Set to GPIO
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;

    // SW3
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 4;
    PINSEL_ConfigPin(&PinCfg);
    GPIO_SetDir(0, 1<<4, 0); // 0 means set as input

    // rgb red
    PinCfg.Portnum = 2;
    PinCfg.Pinnum = 0;
    PINSEL_ConfigPin(&PinCfg);

    // rgb blue
    PinCfg.Portnum = 0;
    PinCfg.Pinnum = 26;
    PINSEL_ConfigPin(&PinCfg);

    GPIO_ClearValue(0, 1<<27); //LM4811-clk
    GPIO_ClearValue(0, 1<<28); //LM4811-up/dn
    GPIO_ClearValue(2, 1<<13); //LM4811-shutdn
    /* <---- Speaker ------ */
}

// Look-Up Table that converts number of leds into its hexadecimal form of its binary representation
// You could make an algorithm to do this conversion, but you'd inevitably end up hardcoding
//   the conversion table for binary to hexadecimal anyway, so might as well hardcode everything to begin with
void ledDisplay (int leds){
	switch(leds){
		case 0:
			pca9532_setLeds(0x0000, 0xFFFF);
			break;
		case 1:
			pca9532_setLeds(0x0001, 0xFFFF);
			break;
		case 2:
			pca9532_setLeds(0x0003, 0xFFFF);
			break;
		case 3:
			pca9532_setLeds(0x0007, 0xFFFF);
			break;
		case 4:
			pca9532_setLeds(0x000F, 0xFFFF);
			break;
		case 5:
			pca9532_setLeds(0x001F, 0xFFFF);
			break;
		case 6:
			pca9532_setLeds(0x003F, 0xFFFF);
			break;
		case 7:
			pca9532_setLeds(0x007F, 0xFFFF);
			break;
		case 8:
			pca9532_setLeds(0x00FF, 0xFFFF);
			break;
		case 9:
			pca9532_setLeds(0x01FF, 0xFFFF);
			break;
		case 10:
			pca9532_setLeds(0x03FF, 0xFFFF);
			break;
		case 11:
			pca9532_setLeds(0x07FF, 0xFFFF);
			break;
		case 12:
			pca9532_setLeds(0x0FFF, 0xFFFF);
			break;
		case 13:
			pca9532_setLeds(0x1FFF, 0xFFFF);
			break;
		case 14:
			pca9532_setLeds(0x3FFF, 0xFFFF);
			break;
		case 15:
			pca9532_setLeds(0x7FFF, 0xFFFF);
			break;
		case 16:
			pca9532_setLeds(0xFFFF, 0xFFFF);
	}
}

// EINT3 interrupt handler
void EINT3_IRQHandler(void){
    // Determine whether GPIO Interrupt P0.4(SW3) has occurred
    if ((LPC_GPIOINT->IO0IntStatF>>4)& 0x1){
        SW3_CurrentPressedTick = getTicks();
        if (mode == STATIONARY) {
            MODE_TOGGLE = 1; // Set SW3 flag
        } else if (mode == LAUNCH) {
            // Compare the current time with the last time SW3 was pressed;
            // If both presses occurred within a second, change mode to RETURN
            if (SW3_CurrentPressedTick - SW3_LastPressedTick <= 1000) {
                mode = RETURN;
                printf("current: %d, last: %d\n", SW3_CurrentPressedTick, SW3_LastPressedTick);
            }
        } else if (mode == RETURN) {
            MODE_TOGGLE = 1; // Set SW3 flag
        }
        SW3_LastPressedTick = getTicks(); // Save current time as the lastest time SW3 was pressed

        // Clear GPIO Interrupt P0.4
        LPC_GPIOINT->IO0IntClr |= 1<<4;
    }
}

// UART3 interrupt handler
void UART3_IRQHandler(void){
    if(UART_Receive(LPC_UART3, &receiving_msg[0], 1, BLOCKING) == 1) {
        if(rpt_status == 0 && receiving_msg[0] == 'R'){
            rpt_status = 1;
            /* printf("1: %c, %d\n", receiving_msg[0], rpt_status); */
        } else if(rpt_status == 1 && receiving_msg[0] == 'P'){
            rpt_status = 2;
            /* printf("2: %c, %d\n", receiving_msg[0], rpt_status); */
        } else if(rpt_status == 2 && receiving_msg[0] == 'T'){
            rpt_status = 3;
            /* printf("3: %c, %d\n", receiving_msg[0], rpt_status); */
        } else if(rpt_status == 3 && receiving_msg[0] == '\r'){
            rpt_status = 4;
            /* printf("4: %c, %d\n", receiving_msg[0], rpt_status); */
        } else if(rpt_status == 4 && receiving_msg[0] == '\n'){
            rpt_status = 4;
            /* printf("4: %c, %d\n", receiving_msg[0], rpt_status); */
        } else { 
            rpt_status = 0;
            /* printf("else: %c, %d\n", receiving_msg[0], rpt_status); */
        }
    }
}

static void stationaryMode() {
    oled_clearScreen(OLED_COLOR_BLACK); // clear screen first

    // STATIONARY MODE INITIALIZATION
    //===============================
    // Initialize these flags to 0
    isTempWarningSent = 0;
    isAccWarningSent = 0;
    isObstNearWarningSent = 0;
    isObstAvoidedWarningSent = 0;

    // Set 7seg display
    led7seg_setChar(15 + 55, 0); //ASCII for F

    // Send STATIONARY mode alert once to UART
    msg = "Entering Stationary Mode \r\n";
    UART_Send(LPC_UART3, (uint8_t *)msg , strlen(msg), BLOCKING);

    // Entering STATIONARY mode from RETURN mode will clear 16-LED display
	pca9532_setLeds(0x0000, 0xFFFF);
    // Entering STATIONARY mode from RETURN mode will clear OBST_WARNING flag
	OBST_WARNING = 0;

    // set counter as a non-zero value to prevent transition to LAUNCH immediately
    counter = -1; // may not be necessary 

    // MAIN STATIONARY LOOP
    //=====================
    // This will keep looping until the mode is changed from STATIONARY
    while (mode == STATIONARY) {
        // Print "STATIONARY" on OLED
        oled_putString(0, 0, (uint8_t *)"STATIONARY", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

        // Read temperature values
        my_temp_value = temp_read();
        // Display temperature values
        sprintf(display_temp, "Temp: %2.2f   ", (my_temp_value/10.0));
        oled_putString(0, 10, (uint8_t *)display_temp, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        // Check temperature values; if too high set TEMP_WARNING flag and print OLED warning
        if ((my_temp_value/10.0) > TEMP_HIGH_THRESHOLD) {
            if (!isTempWarningSent) {
                msg = "Temp. too high \r\n";
                UART_Send(LPC_UART3, (uint8_t *)msg , strlen(msg), BLOCKING);
                isTempWarningSent = 1;
            }
            TEMP_WARNING = 1;
            oled_putString(0, 20, (uint8_t *)"Temp. too high ", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        }

        // Check if SW4 (CLEAR_WARNING) is being pressed; if yes, clear all warnings flags
        CLEAR_WARNING = !((GPIO_ReadValue(1) >> 31) & 0x01);
        if (TEMP_WARNING && CLEAR_WARNING) {

            TEMP_WARNING = 0; //clear temp flag
            isTempWarningSent = 0; //clear send flag so that the next temp warning can trigger another UART send

            GPIO_ClearValue(2, (1<<0)); //clear red led
            GPIO_ClearValue(0, (1<<26)); //clear blue led
            oled_clearScreen(OLED_COLOR_BLACK); //clear OLED so that entire screen will be refreshed
        }

        // currentTime is the time elapsed (in seconds) since the program started.
        currentTime = getTicks() / 1000;

        if(MODE_TOGGLE){
            /* do { */
            while (counter != 0) {
                // If Rocket is in TEMP_WARNING state, deactivate MODE_TOGGLE whenever it is activated
                // So that it will not countdown while TEMP_WARNING is active
                if(TEMP_WARNING){
                    MODE_TOGGLE = 0;
                    break;
                }

                // Read temperature values
                my_temp_value = temp_read();
                // Display temperature values
                sprintf(display_temp, "Temp: %2.2f   ", (my_temp_value/10.0));
                oled_putString(0, 10, (uint8_t *)display_temp, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                // Check temperature values; if too high set TEMP_WARNING flag and print OLED warning.
                //   Also set 7 segment back to F and exit the while loop
                if ((my_temp_value/10.0) > TEMP_HIGH_THRESHOLD) {
                    // If temp warning has not already been sent to the UART, send it
                    if (!isTempWarningSent) {
                        msg = "Temp. too high \r\n";
                        UART_Send(LPC_UART3, (uint8_t *)msg , strlen(msg), BLOCKING);
                        isTempWarningSent = 1;
                    }
                    led7seg_setChar(15 + 55, 0);
                    MODE_TOGGLE = 0;
                    TEMP_WARNING = 1;
                    oled_putString(0, 20, (uint8_t *)"Temp. too high ", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                    break;
                }

                // countdown is the time elapsed (in seconds) after SW3 was pressed.
                countdown = (getTicks() / 1000) - currentTime;

                // while countdown counts up from 0, counter counts down from 15
                // When counter hits 0, it stays as 0
                counter = (countdown <= 15) ? 15 - countdown : 0;

                // If counter is between 0 to 9, show decimal 0 to 9; else show hexadecimal A to F
                // note that counter only takes values between 0 to 15
                if (counter < 10) {
                    led7seg_setChar(counter + 48, 0); // number + 48 = 0-9 ascii value
                } else {
                    led7seg_setChar(counter + 55, 0); // number + 55 = A-F ascii value
                }

                // continue printing temperature values while counting down
                sprintf(display_temp, "Temp: %2.2f   ", (my_temp_value/10.0));
                oled_putString(0, 10, (uint8_t *)display_temp, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            } //while (counter != 0);

            // When counter hits 0, change mode to LAUNCH and clear MODE_TOGGLE (SW3) flag
            if (counter == 0) {
                mode = LAUNCH;
                MODE_TOGGLE = 0;
            }
        }
    }
}

static void launchMode(){
    oled_clearScreen(OLED_COLOR_BLACK); // clear screen first

    // LAUNCH MODE INITIALIZATION
    //===========================
    // Initialize these flags to 0
    isTempWarningSent = 0;
    isAccWarningSent = 0;
    isObstNearWarningSent = 0;
    isObstAvoidedWarningSent = 0;

    // Set 7seg display
    led7seg_setChar(0 + 48, 0); //ASCII for 0

    // Send LAUNCH mode alert once to UART
    msg = "Entering Launch Mode \r\n";
    UART_Send(LPC_UART3, (uint8_t *)msg , strlen(msg), BLOCKING);

    // MAIN LAUNCH LOOP
    //=================
    // This will keep looping until the mode is changed from LAUNCH
    while (mode == LAUNCH) {
        // Print "LAUNCH" on OLED
        oled_putString(0, 0, (uint8_t *)"LAUNCH", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

        // Read accelerometer values
        acc_read(&x, &y, &z);
        x = x-xoff;
        y = y-yoff;
        z = z-zoff;
        // Display accelerometer values
        sprintf(display_x, "Acc: %2.2f   ", (float) x/9.8);
        sprintf(display_y, "     %2.2f   ", (float) y/9.8);
        oled_putString(0, 20, (uint8_t *)display_x, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        oled_putString(0, 30, (uint8_t *)display_y, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        // Check accelerometer values; if too high set ACC_WARNING flag and print OLED warning
        if((abs(x)/9.8 > ACC_THRESHOLD) || (abs(y)/9.8 > ACC_THRESHOLD)){
            // If Accelerometer warning has not already been sent to the UART, send it
            if (!isAccWarningSent) {
                msg = "Veer off course \r\n";
                UART_Send(LPC_UART3, (uint8_t *)msg , strlen(msg), BLOCKING);
                isAccWarningSent = 1;
            }
            ACC_WARNING = 1;
            oled_putString(0, 40, (uint8_t *)"Veer off course ", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        }

        // Read temperature values
        my_temp_value = temp_read();
        // Display temperature values
        sprintf(display_temp, "Temp: %2.2f   ", (my_temp_value/10.0));
        oled_putString(0, 10, (uint8_t *)display_temp, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        // Check temperature values; if too high set TEMP_WARNING flag and print OLED warning
        if ((my_temp_value/10.0) > TEMP_HIGH_THRESHOLD) {
            // If temp warning has not already been sent to the UART, send it
            if (!isTempWarningSent) {
                msg = "Temp. too high \r\n";
                UART_Send(LPC_UART3, (uint8_t *)msg , strlen(msg), BLOCKING);
                isTempWarningSent = 1;
            }
            TEMP_WARNING = 1;
            oled_putString(0, 50, (uint8_t *)"Temp. too high ", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        }

        // Check if SW4 (CLEAR_WARNING) is being pressed; if yes, clear all warnings flags
        CLEAR_WARNING = !((GPIO_ReadValue(1) >> 31) & 0x01);
        if ((TEMP_WARNING || ACC_WARNING) && CLEAR_WARNING) {

            TEMP_WARNING = 0; //clear temp flag
            isTempWarningSent = 0; //clear send flag so that the next temp warning can trigger another UART send

            ACC_WARNING = 0; //clear acc flag
            isAccWarningSent = 0; //clear send flag so that the next acc warning can trigger another UART send

            GPIO_ClearValue(2, (1<<0)); //clear red led
            GPIO_ClearValue(0, (1<<26)); //clear blue led
            oled_clearScreen(OLED_COLOR_BLACK); //clear OLED so that entire screen will be refreshed
        }

        // Every 10s the "areTempAndAccValuesSent" flag will be cleared to 0 by the SysTick_Handler and cause 
        //   this code to send a accelerometer report to the UART
        sprintf(report_x, "Acc: x- \"%2.2f\", " , (float) x/9.8);
        sprintf(report_y, "y- \"%2.2f\"", (float) y/9.8);
        if (!areTempAndAccValuesSent) {
            UART_SendString(LPC_UART3, display_temp);
            UART_SendString(LPC_UART3, report_x);
            UART_SendString(LPC_UART3, report_y);
            UART_SendString(LPC_UART3, "\r\n");
            areTempAndAccValuesSent = 1;
        }

        // If UART IRQ handler detects an "RPT<Enter>", "rpt_status" flag will be set to "4" and this code will run,
        //   sending an accelerometer report to the UART
        if (rpt_status == 4) {
            UART_SendString(LPC_UART3, display_temp);
            UART_SendString(LPC_UART3, report_x);
            UART_SendString(LPC_UART3, report_y);
            UART_SendString(LPC_UART3, "\r\n");
            UART_SendString(LPC_UART3, "Reported. \r\n");
            rpt_status = 0;
        }
    }
}

static void returnMode(){
    oled_clearScreen(OLED_COLOR_BLACK); // clear screen first

    // RETURN MODE INITIALIZATION
    //===========================
    // Initialize these flags to 0
    isTempWarningSent = 0;
    isAccWarningSent = 0;
    isObstNearWarningSent = 0;
    isObstAvoidedWarningSent = 0;

    // Set 7seg display
    led7seg_setChar(0 + 48, 0); //ASCII for 0

    // Send RETURN mode alert once to UART
    msg = "Entering Return Mode \r\n";
    UART_Send(LPC_UART3, (uint8_t *)msg , strlen(msg), BLOCKING);

    // Entering RETURN mode from LAUNCH mode will clear TEMP_WARNING flag
    TEMP_WARNING = 0;
    // Entering RETURN mode from LAUNCH mode will clear ACC_WARNING flag
    ACC_WARNING = 0;
    // Entering RETURN mode from LAUNCH mode will clear both red & blue LEDs
    GPIO_ClearValue(2, (1<<0)); //clear red
    GPIO_ClearValue(0, (1<<26)); //clear blue

    // clear obst_flag
    obst_flag = 0;

    // MAIN RETURN LOOP
    //=================
    // This will keep looping until the mode is changed from RETURN
    while (mode == RETURN){
        // Print "RETURN" on OLED
        oled_putString(0, 0, (uint8_t *)"RETURN", OLED_COLOR_WHITE, OLED_COLOR_BLACK);

        // Read light sensor values
        // distance here is synonymous with light sensor value
    	distance = (int) light_read();
        // Display light sensor values
        sprintf(display_distance, "Distance: %dm  ", distance);
        oled_putString(0, 10, (uint8_t *)display_distance, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        // Light up 16-led display depending on distance
        ledDisplay(distance/250);
        // Check distance; if too high set OBST_WARNING flag and print OLED WARNING
        // else clear OBST_WARNING flag
        if(distance > OBSTACLE_NEAR_THRESHOLD){
            // If onstacle near warning has not already been sent to the UART, send it
            if (!isObstNearWarningSent) {
                msg = "Obstacle Near \r\n";
                UART_Send(LPC_UART3, (uint8_t *)msg , strlen(msg), BLOCKING);
                isObstNearWarningSent = 1;
                isObstAvoidedWarningSent = 0;
            }
        	OBST_WARNING = 1;
        	obst_flag = 1;
            oled_putString(0, 20, (uint8_t *)"Obstacle Near ", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        } else {
        	OBST_WARNING = 0;
            isObstAvoidedWarningSent = 0;
        }

        // If OBST_WARNING is cleared but obst_flag is still active, it means the rocket just avoided an obstacle.
        // Clear obst_flag and display "Obst. Avoided" on OLED
        if(obst_flag == 1 && OBST_WARNING == 0){
            // If onstacle avoided warning has not already been sent to the UART, send it
            if (!isObstAvoidedWarningSent) {
                msg = "Obst. Avoided \r\n";
                UART_Send(LPC_UART3, (uint8_t *)msg , strlen(msg), BLOCKING);
                isObstAvoidedWarningSent = 1;
                isObstNearWarningSent = 0;
            }
        	obst_flag = 0;
            oled_clearScreen(OLED_COLOR_BLACK);
            oled_putString(0, 20, (uint8_t *)"Obst. Avoided ", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        }

        // Every 10s, the SysTick Handler clears the isDistValueSent flag
        // If that flag is cleared, the below code will send current Distance Values to UART, then set flag back to 1
        if (!isDistValueSent) {
            UART_SendString(LPC_UART3, display_distance);
            UART_SendString(LPC_UART3, "\r\n");
            isDistValueSent = 1;
        }

        // If UART IRQ handler detects an "RPT<Enter>", "rpt_status" flag will be set to "4" and this code will run,
        //   sending a distance report to the UART
        if (rpt_status == 4) {
            UART_SendString(LPC_UART3, display_distance);
            UART_SendString(LPC_UART3, "\r\n");
            UART_SendString(LPC_UART3, "Reported. \r\n");
            rpt_status = 0;
        }

        if (MODE_TOGGLE == 1) {
            MODE_TOGGLE = 0;
            Timer0_Wait(1000); // Wait 1 second
            mode = STATIONARY;
        }
    }
}

// Called first when entering main function to initialize everything
static void init_all() {
    SysTick_Config(SystemCoreClock/1000);

    //Interfaces
    init_GPIO();
    init_ssp();
    init_i2c();

    //Peripherals
    rgb_init();
    led7seg_init();
    acc_init();
    oled_init();
    temp_init(getTicks);
    init_uart();

    // Light Sensor
    light_enable();
    light_setRange(LIGHT_RANGE_4000);

    // Initialize initial x, y, z values to 0.
    acc_read(&x, &y, &z);
    xoff = x;
    yoff = y;
    zoff = z;

    // Enable GPIO Interrupt for P0.4 (SW3)
    LPC_GPIOINT->IO0IntEnF |= 1 << 4;
    // Enable Interrupt for EINT3
    NVIC_EnableIRQ(EINT3_IRQn);
    // Enable UART Rx interrupt
    UART_IntConfig(LPC_UART3, UART_INTCFG_RBR, ENABLE);
    // Enable Interrupt for UART3
    NVIC_EnableIRQ(UART3_IRQn);

    // Clearing the OLED before the program starts
    oled_clearScreen(OLED_COLOR_BLACK);
}

int main (void) {
    init_all();

    while(1) {
        if(mode == STATIONARY){
            stationaryMode();
        } else if(mode == LAUNCH) {
            launchMode();
        } else if (mode == RETURN) {
            returnMode();
        }
    }
}
