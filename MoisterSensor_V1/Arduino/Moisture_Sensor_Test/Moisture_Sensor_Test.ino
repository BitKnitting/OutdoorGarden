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
#define SERIAL_BAUD   115200

void setup() {
  // put your setup code here, to run once:
  DEBUG_PRINT_BEGIN(SERIAL_BAUD);
  DEBUG_PRINTLNF("Get Readings from Moisture Sensor");

}

void loop() {
  // put your main code here, to run repeatedly:
  int moisture_reading = analogRead(A0);
  if (DEBUG) {
    DEBUG_PRINTF("...Moisture Reading: ");
    Serial.println(moisture_reading);
  }

}
