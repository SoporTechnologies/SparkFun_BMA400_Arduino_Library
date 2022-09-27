#include <Wire.h>
#include "SparkFun_BMA400_Arduino_Library.h"

// Create a new sensor object
BMA400 accelerometer;

// I2C address selection
uint8_t i2cAddress = BMA400_I2C_ADDRESS_DEFAULT; // 0x14
//uint8_t i2cAddress = BMA400_I2C_ADDRESS_SECONDARY; // 0x15

// Pin used for interrupt detection
int interruptPin = 2;

// Flag to know when interrupts occur
volatile bool interruptOccurred = false;

void setup()
{
    // Start serial
    Serial.begin(115200);
    Serial.println("BMA400 Example9 begin!");

    // Initialize the I2C library
    Wire.begin();

    // Check if sensor is connected and initialize
    // Address is optional (defaults to 0x14)
    while(accelerometer.beginI2C(i2cAddress) != BMA400_OK)
    {
        // Not connected, inform user
        Serial.println("Error: BMA400 not connected, check wiring and I2C address!");

        // Wait a bit to see if connection is established
        delay(1000);
    }

    Serial.println("BMA400 connected!");

    // Variable to track errors returned by API calls
    int8_t err = BMA400_OK;

    // Here we configure the tap detection feature of the BMA400. It can detect
    // both single and double taps as a form of user input. There are a number
    // of parameters than can be configured to help distinguish taps from other
    // sources of noise.
    bma400_tap_conf config =
    {
        .axes_sel = BMA400_AXIS_Z_EN, // Which axes to evaluate for interrupts (X/Y/Z in any combination)
        .sensitivity = BMA400_TAP_SENSITIVITY_3, // Sensitivity threshold, up to 7
        .tics_th = BMA400_TICS_TH_12_DATA_SAMPLES, // Max time between top/bottom peaks of a single tap (helps with noise rejection)
        .quiet = BMA400_QUIET_60_DATA_SAMPLES, // Threshold time between taps to register as single or double taps
        .quiet_dt = BMA400_QUIET_DT_4_DATA_SAMPLES, // Minimum time between 2 taps (helps with noise rejection)
        .int_chan = BMA400_INT_CHANNEL_1 // Which pin to use for interrupts
    };
    err = accelerometer.setTapInterrupt(&config);
    if(err != BMA400_OK)
    {
        // Interrupt settings failed, most likely a communication error (code -2)
        Serial.print("Interrupt settings failed! Error code: ");
        Serial.println(err);
    }

    // Here we configure the INT1 pin to push/pull mode, active high
    err = accelerometer.setInterruptPinMode(BMA400_INT_CHANNEL_1, BMA400_INT_PUSH_PULL_ACTIVE_1);
    if(err != BMA400_OK)
    {
        // Interrupt pin mode failed, most likely a communication error (code -2)
        Serial.print("Interrupt pin mode failed! Error code: ");
        Serial.println(err);
    }

    // Enable single tap interrupt condition
    err = accelerometer.enableInterrupt(BMA400_SINGLE_TAP_INT_EN, true);
    if(err != BMA400_OK)
    {
        // Interrupt enable failed, most likely a communication error (code -2)
        Serial.print("Interrupt enable failed! Error code: ");
        Serial.println(err);
    }

    // Enable double tap interrupt condition
    err = accelerometer.enableInterrupt(BMA400_DOUBLE_TAP_INT_EN, true);
    if(err != BMA400_OK)
    {
        // Interrupt enable failed, most likely a communication error (code -2)
        Serial.print("Interrupt enable failed! Error code: ");
        Serial.println(err);
    }

    // Setup interrupt handler
    attachInterrupt(digitalPinToInterrupt(interruptPin), bma400InterruptHandler, RISING);
}

void loop()
{
    // Wait for interrupt to occur
    if(interruptOccurred)
    {
        // Reset flag for next interrupt
        interruptOccurred = false;

        Serial.print("Interrupt occurred!\t\t");

        // Variable to track errors returned by API calls
        int8_t err = BMA400_OK;

        // Get the interrupt status to know which condition triggered
        uint16_t interruptStatus = 0;
        err = accelerometer.getInterruptStatus(&interruptStatus);
        if(err != BMA400_OK)
        {
            // Status get failed, most likely a communication error (code -2)
            Serial.print("Get interrupt status failed! Error code: ");
            Serial.println(err);
            return;
        }

        // Check if this is the tap interrupt condition
        if(interruptStatus & BMA400_ASSERTED_S_TAP_INT)
        {
            Serial.println("Single tap!");
        }
        else if(interruptStatus & BMA400_ASSERTED_D_TAP_INT)
        {
            Serial.println("Double tap!");
        }
        else
        {
            Serial.println("Wrong interrupt condition!");
        }
    }
}

void bma400InterruptHandler()
{
    interruptOccurred = true;
}