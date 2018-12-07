#include <Servo.h>
#include <SPI.h>
#include "Adafruit_BLE_UART.h"

#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RST 9
#define ADAFRUITBLE_RDY 2

Adafruit_BLE_UART uart = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

boolean checkCRC(uint8_t *buffer);

unsigned long time = 0l;
boolean connection = false;
uint8_t btm = 65;
uint8_t out = btm;
uint8_t cap = 90;

#define persec 30
#define sendat (1000/persec)

Servo panservo;
Servo tiltservo;


const int maxpan = 270;
const int minpan = 0;
const int maxtilt = 270;
const int mintilt = 0;
char input;

int pan = 100;
int tilt = 200;
int step = 10;

/**************************************************************************/
/*!
  This function is called whenever select ACI events happen
*/
/**************************************************************************/
void aciCallback(aci_evt_opcode_t event)
{
  switch (event)
  {
    case ACI_EVT_DEVICE_STARTED:
      Serial.println(F("Advertising started"));
      break;
    case ACI_EVT_CONNECTED:
      Serial.println(F("Connected!"));
      connection = true;
      break;
    case ACI_EVT_DISCONNECTED:
      Serial.println(F("Disconnected or advertising timed out"));
      connection = false;
      break;
    default:
      break;
  }
}

/**************************************************************************/
/*!
  This function is called whenever data arrives on the RX channel
*/
/**************************************************************************/
void rxCallback(uint8_t *buffer, uint8_t len)
{          digitalWrite(LED_BUILTIN, HIGH);

  //  char pfx[2] = { (char)buffer[0], (char)buffer[1] };
  if ((char)buffer[0] == '!') {  //Sensor data flag
    switch ((char)buffer[1]) {
      case 'B':
        if (checkCRC(buffer) == false) {
          break;
        }
        if ((char)buffer[2] == '5') {
          if ((pan + step) < maxpan) pan += step;
        }
        if ((char)buffer[2] == '6') {
          if ((pan - step) < maxpan) pan += step;
        }
        if ((char)buffer[2] == '7') {
          if ((tilt + step) < maxtilt) tilt += step;
        }
        if ((char)buffer[2] == '8') {
          if ((tilt - step) < maxtilt) tilt += step;
        }

        panservo.write(pan);
        tiltservo.write(tilt);
        //printButtonState((char)buffer[3]);
        Serial.println("ok");
        break;
     
      default:
        Serial.println("Unknown Data Type");
        break;
    }
    Serial.println("");
  }

  /* Echo the same data back! */
    uart.write(buffer, len);
}





/**************************************************************************/
/*!
  Configure the Arduino and start advertising with the radio
*/
/**************************************************************************/
void setup(void)
{
  Serial.begin(9600);
  Serial.println(F("Arduino setup"));

  pinMode(LED_BUILTIN, OUTPUT);

  uart.setRXcallback(rxCallback);
  uart.setACIcallback(aciCallback);
  uart.begin();


  panservo.attach(5);
  tiltservo.attach(11);
  panservo.write(pan);
  tiltservo.write(tilt);

}

/**************************************************************************/
/*!
  Constantly checks for new events on the nRF8001
*/
/**************************************************************************/
void loop()
{
  uart.pollACI();


}


boolean checkCRC(uint8_t *buffer) {

  uint8_t len = sizeof(buffer);
  uint8_t crc = buffer[len - 2];
  uint8_t sum = 0;

  for (int i = 0; i < (len - 1); i++) {

    sum += buffer[i];

  }

  Serial.print("CRC ");

  if ((crc & ~sum) == 0) {
    Serial.println("PASS");
    return true;
  }

  else {
    Serial.println("FAIL");
    return false;
  }

}


//this code was adapted from an example by brad zdanivsky found on http://verticalchallenge.org/archives/2823 & CollinCunningham's BLE_UART_Controller_Test.ino
