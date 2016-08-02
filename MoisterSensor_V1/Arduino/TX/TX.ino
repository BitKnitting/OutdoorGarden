
static uint8_t firmware_version =  1; //Version of packet (data structure) sent
#define DEBUG 0
#ifdef DEBUG
#define DEBUG_PRINT_BEGIN(x) Serial.begin(x)
#define DEBUG_PRINT(x)  Serial.print (x)
#define DEBUG_PRINTHEX(x) Serial.print(x,HEX)
#define DEBUG_PRINTF(x) Serial.print(F(x))
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTLNF(x) Serial.println(F(x))
#else
#define DEBUG_PRINT_BEGIN(x)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTF(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTLNF(x)
#define DEBUG_PRINTHEX(x)
#endif
#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <Adafruit_SleepyDog.h>
//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID     100  // The same on all nodes that talk to each other
#define NODEID        2    // The unique identifier of this node
#define RECEIVER      1    // The recipient of packets

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW   true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************
#define SERIAL_BAUD   115200

/* for Feather 32u4 */
#define RFM69_CS      8
#define RFM69_IRQ     7
#define RFM69_IRQN    4  // Pin 7 is IRQ 4!
#define RFM69_RST     4
/* for Feather M0
  #define RFM69_CS      8
  #define RFM69_IRQ     3
  #define RFM69_IRQN    3  // Pin 3 is IRQ 3!
  #define RFM69_RST     4
*/

unsigned int reading_number = 0;  // The reading number is incremented when a transmission happens.

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);


//***********************************************************
// void transmitReadings(int sleepMS)
//
// Get then transmit the battery level and value read for moisture
// From analog i/o.
//***********************************************************
//..........................
// Define a struct to hold the values
//..........................
struct valuesStruct_t
{
  uint8_t  firmware_version;
  unsigned int reading_number;
  float battery_level;
  int moisture_reading;
  unsigned int sleepMS;
};
//..........................
// Define a union type to map from the struct to a byte buffer
//..........................
union txUnion_t
{
  valuesStruct_t values;
  uint8_t b[sizeof(valuesStruct_t)];
};
//..........................
// finally, the code for transmitting the values.
//..........................

void transmitReadings(unsigned int sleepMS) {
  txUnion_t txData;
  // If the number of moisture readings is > unsigned int, the number rolls over
  reading_number += 1;
  txData.values.reading_number = reading_number;
  txData.values.battery_level = getBatteryLevel();
  txData.values.moisture_reading = getMoistureReading();
  txData.values.sleepMS = sleepMS;
  txData.values.firmware_version = firmware_version;
  DEBUG_PRINTF("Version: ");
  DEBUG_PRINT(txData.values.firmware_version);
  DEBUG_PRINTF("| Reading number: ");
  DEBUG_PRINT(txData.values.reading_number);
  DEBUG_PRINTF("| Battery Level: ");
  DEBUG_PRINT(txData.values.battery_level);
  DEBUG_PRINTF(" | Moisture Reading: ");
  DEBUG_PRINT(txData.values.moisture_reading);
  DEBUG_PRINTF(" | Time since last reading: ");
  DEBUG_PRINT(txData.values.sleepMS);
  DEBUG_PRINTLNF(" ms");
  if (radio.sendWithRetry(RECEIVER, txData.b, sizeof(txUnion_t))) { //target node Id, message as  byte array, message length
    // From p. 26 https://cdn-learn.adafruit.com/downloads/pdf/adafruit-feather-32u4-radio-with-rfm69hcw-module.pdf
    // If you put the radio to sleep after transmitting, rather than just sitting in receive mode, you can save more
    // current, after transmit is complete, the average current drops to ~10mA which is just for the microcontroller.
    radio.sleep();
  }
}
//***********************************************************
// int getMoistureReading()
//
// Get Moisture reading from the analog port reading the moisture sensor.
//***********************************************************
int getMoistureReading() {
  return analogRead(A0);
}
//***********************************************************
// float getBatteryLevel()
//
// Ask the feather how much battery is left.  If low, the user needs to recharge.
// Code from Adafruit's learn section: https://learn.adafruit.com/adafruit-feather-32u4-basic-proto/power-management
//***********************************************************
#define VBATPIN A9
float getBatteryLevel() {
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  return (measuredvbat);
}
//***********************************************************
// SETUP
//***********************************************************
void setup() {
  // The ATmega32u4 has to shut down the bootloader's USB connection and register a new one, and that takes a while.
  // This line will make the code wait until the USB registration has finished before trying to print anything.
  if (DEBUG) {
    while (!Serial);
  }
  DEBUG_PRINT_BEGIN(SERIAL_BAUD);
  DEBUG_PRINTLNF("Feather RFM69HCW Transmitter");

  //Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);

  // Initialize radio
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  if (IS_RFM69HCW) {
    radio.setHighPower();    // Only for RFM69HCW & HW!
  }
  radio.setPowerLevel(31); // power output ranges from 0 (5dBm) to 31 (20dBm)
  radio.encrypt(ENCRYPTKEY);
  reading_number = 0;
  //Not using the LED since the feather will be somewhere in my backyard
  //pinMode(LED, OUTPUT);
  DEBUG_PRINT(FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  DEBUG_PRINTLNF(" MHz");
}
// The watchdog timer's max amount of sleep time = 8 seconds.  If you set the time between readings to be
// >8000 and <= 32767 (the maximum positive value for an int), the watchdog timer is set to 8 seconds.
// A negative signed int will set the watchdog timer to 15ms.
void loop() {
  //if using the serial monitor, use the delay() function for time between readings.  If taking readings in the field,
  //use the watchdog timer to save on battery life.
  int time_Between_Readings_in_ms = 1000; //set for Debug.  Will change if using the watchdog timer.
  if (DEBUG) {
    // Debugging, so set a few seconds between sending.
    delay(time_Between_Readings_in_ms);
  } else {
    // Since the watchdog timer maxes out at 8 seconds....
    int number_of_sleeper_loops = 4; //i.e.: time between taking a moisture reading is 4 * 8 seconds = 32 seconds.
    for (int i = 0; i < number_of_sleeper_loops; i++) {
      time_Between_Readings_in_ms = Watchdog.sleep(8000);
    }
    time_Between_Readings_in_ms = time_Between_Readings_in_ms * number_of_sleeper_loops;
  }
  transmitReadings(time_Between_Readings_in_ms);
}
