/* RFM69 library and code by Felix Rusu - felix@lowpowerlab.com
  // Get libraries at: https://github.com/LowPowerLab/
  // Make sure you adjust the settings in the configuration section below !!!
  // **********************************************************************************
  // Copyright Felix Rusu, LowPowerLab.com
  // Library and code by Felix Rusu - felix@lowpowerlab.com
  // **********************************************************************************
  // License
  // **********************************************************************************
  // This program is free software; you can redistribute it
  // and/or modify it under the terms of the GNU General
  // Public License as published by the Free Software
  // Foundation; either version 3 of the License, or
  // (at your option) any later version.
  //
  // This program is distributed in the hope that it will
  // be useful, but WITHOUT ANY WARRANTY; without even the
  // implied warranty of MERCHANTABILITY or FITNESS FOR A
  // PARTICULAR PURPOSE. See the GNU General Public
  // License for more details.
  //
  // You should have received a copy of the GNU General
  // Public License along with this program.
  // If not, see <http://www.gnu.org/licenses></http:>.
  //
  // Licence can be viewed at
  // http://www.gnu.org/licenses/gpl-3.0.txt
  //
  // Please maintain this license information along with authorship
  // and copyright notices in any redistribution of this code
  //..........................................................
  // **********************************************************************************/
//...........................
// Identify the version of this software
//...........................
const uint8_t firmware_version = 1;
#define DEBUG 1
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

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID     100  //the same on all nodes that talk to each other
#define NODEID        1    // the unique identifier for this node.

#define LED           13  // onboard blinky

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY      RF69_915MHZ
#define ENCRYPTKEY     "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW    true // set to 'true' if you are using an RFM69HCW module

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

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);
//***********************************************************
// BLINK on board LED
//***********************************************************
void Blink(byte PIN, byte DELAY_MS, byte loops)
{
  for (byte i=0; i<loops; i++)
  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}
//***********************************************************
// SETUP
//***********************************************************
void setup() {
  // The ATmega32u4 has to shut down the bootloader's USB connection and register a new one, and that takes a while.
  // This line will make the code wait until the USB registration has finished before trying to print anything.
  while ( ! Serial ) ;
  DEBUG_PRINT_BEGIN(SERIAL_BAUD);
  DEBUG_PRINTLNF("Feather RFM69HCW Receiver");
  // Hard Reset the RFM module
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

  DEBUG_PRINT("...Listening at ");
  DEBUG_PRINT(FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  DEBUG_PRINTLNF(" MHz");
  pinMode(LED, OUTPUT);
}
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
union rxUnion_t
{
  valuesStruct_t values;
  uint8_t b[sizeof(valuesStruct_t)];
};
//***********************************************************
// CONTROL LOOP
//***********************************************************
void loop() {
  //check if something was received (could be an interrupt from the radio)
  if (radio.receiveDone())
  {
    if (strlen((char *)radio.DATA)) {
      Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks
      if (radio.ACKRequested())
      {
        radio.sendACK();
      }
    }
    rxUnion_t rxData;
    memcpy(rxData.b, (char *)radio.DATA, sizeof(rxUnion_t));
    DEBUG_PRINTF("   [RX_RSSI:");DEBUG_PRINT(radio.RSSI);DEBUG_PRINTF("]");
    DEBUG_PRINTF("Version: ");
    DEBUG_PRINT(rxData.values.firmware_version);
    if (rxData.values.firmware_version == firmware_version) {
      DEBUG_PRINTF(" (firmware match)");
    }
    DEBUG_PRINTF("| Zone: ");
    DEBUG_PRINT(radio.SENDERID);
    DEBUG_PRINTF("| Reading number: ");
    DEBUG_PRINT(rxData.values.reading_number);
    DEBUG_PRINTF("| Battery Level: ");
    DEBUG_PRINT(rxData.values.battery_level);
    DEBUG_PRINTF(" | Moisture Reading: ");
    DEBUG_PRINT(rxData.values.moisture_reading);
    DEBUG_PRINTF(" | Time since last reading: ");
    DEBUG_PRINT(rxData.values.sleepMS);
    DEBUG_PRINTLNF(" ms");
  }
}
