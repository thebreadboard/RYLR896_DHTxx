/*
REYAX RYLR869 example sketch with supporting role from a DHT22 Temperature and Humidity Sensor

This works with PlatformIO or Arduino IDE

More videos and tutorials http://www.youtube.com/thebreadboardca
my website https://www.thebreadboard.ca
github https://https://github.com/thebreadboard
support me on Patrion https://www.patreon.com/thebreadboard
or paypal me https://paypal.me/thebreadboardca

  This sketch shows how to use the REYAX RYLR869 LoRa transceiver
  as an IOT connection. it does not use end to end handshaking
  or specific addresses and Network IDs (this will be in the next version)
  it sends a message every "sendDelay" period to the RYLR896
  inbetween this time is listens for received data and calls apropriate 
  functions to handle the message received
  you can choose between sending data from a DHTxxx sensor or a simple 
  test message will mills() appended to keep each message unique.
  
  parts Used:-
  RYLR896             https://amzn.to/3fjBwgo
  DHT22               https://amzn.to/2MPaX6v
  Arduino Nano 33 IOT https://amzn.to/2UFwwuB


  the DHT22 is a common Temperature and Humidity sensor, the DHT22 measures Temp and Humidity
  quite accuratly compared to a DHT11, it is an easy thing to change to other sensors to suit budget
  or application needs

  Because Temperature and Humidity does not change very quickly in normal life
  you really should only need to sample say once every 10 minutes, this allows
  significant saving of battery power if battery drain is a problem
  You could even put the tranceiver to sleep between samples if the application allows for it

  Serial1 is used for the RYLR896 communications, if your arduino does not have a built in Serial1
  then simply add a softserial port of your choosing https://www.arduino.cc/en/tutorial/SoftwareSerialExample

  Hardware connections:

  NanoIOT33   RYLR896
  +3V3  to Pin 1 3.3V
  D3    to pin 2 NRST
  TXD   to pin 3 RXD
  RXD   to pin 4 TXD
         pin 5 Not connected
  GND   to pin 6 GND

  (WARNING: do not connect to 5V or the sensor will be damaged!)

  NanoIOT33   DHT22
  +3V3  to Pin 1 Vcc
  D21   to pin 2 data
         pin 3 Not connected
  GND   to pin 4 GND

  Connect a 10K pullup between the DHD22 Vcc (Pin 1) and DHT22 Data (pin 2)

  The external LED should be wired to a HIGH on the pin will illuminate the LED
  remember to place a current limit resistor in series with it of between 330R to 1KR

  V1.1 Peter Oakes, www.thebreadboard.ca 06/12/2020
*/

#include <Arduino.h>

// comment out the #define below if you dont have a DHT22 sensor connected
// and the code will use test data instead to transmit
//#define DHTSensor

//if you not using an external LED then use the built in one
//comment out if your using the external LED
#define useLED_Builtin

#ifdef DHTSensor
// the sensor code below needs all these libraries
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <avr/dtostrf.h>
#endif

// The SendDelay should reflect the LoRa duty cycle determined by the airtime calculator
// and yes I know I am way too fast, this is demo, not production :)
#define SendDelay  5000
#define flashSetupComplete  5
#define flashRcvMessage  3
#define flashSentCommand  1
#define flashDelay  50
#ifdef DHTSensor
#define DHTPIN 21
// change the #define line to reflect your type of DHTxx sensor
//  DHT11, DHT12, DHT2, DHT21, AM2301.
#define DHTTYPE DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);
#endif

// if you use a different pin for the reset line to the RYLR896 or external LED
// then simply change the two lines below to reflect you choices.
// if you want to use the built in led then comment out the #define statusLED 2 and un comment #define statusLED LED_BUILTIN
#define reset896 3 // Reset Pin Active Low
#ifdef useLED_Builtin
  #define statusLED LED_BUILTIN // the built in led
#else
  #define statusLED 2 // the blue external led
#endif
const String msgRecv = "+RCV=";
const String msgError = "+ERR=";
const String msgOK = "+OK";
const String sndMessage = "AT+SEND=0,";

unsigned long lastSend = SendDelay;
bool cmdComplete = true; //so we know when a command is done so we can do another

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  // Initialise the ports were using
  pinMode(reset896, OUTPUT);
  pinMode(statusLED, OUTPUT);

  // RESET RYLR896
  digitalWrite(reset896, LOW); // Reset radio start
  delay(100);
  digitalWrite(reset896, HIGH); // Reset radio complete

#ifdef DHTSensor
  // initialise the Sensor library
  dht.begin();
#endif

  if (Serial1.available() > 0) {
    // read the incoming data:
    Serial.println ("RLYL896 Status = " + Serial1.readString());
  }
  // FLASH EXTERNAL LED
  flashLED(flashSetupComplete, statusLED); // done setup
}

void loop() {
  // if we exceeded SendDelay AND the previous command completed then send the data again
  if ((millis() > lastSend + SendDelay) && cmdComplete) {
    sendSensorData();
    lastSend = millis();
  }
  recvMsg();
}

// see if a message from RYLR896 was received
// if so figure out what type it is
// and dispatch it to specific handler
void recvMsg()
{
  String rcvMsg;
  if (Serial1.available() > 0) { // Did we get a message
    rcvMsg  = Serial1.readString();
    if (rcvMsg.startsWith(msgRecv)) { // starts with "+RCV"
      ProcessMSG(rcvMsg);
    }
    else if (rcvMsg.startsWith(msgError)) { // Startes with "+ERR"
      ProcessError(rcvMsg);
    }
    else if (rcvMsg.startsWith(msgOK)) { // Starts with "+OK"
      ProcessOK(rcvMsg);
    }
    else { // found none of the above
      Serial.println (rcvMsg);
    }
  }
}
#ifdef DHTSensor
// We have a DHT22 defined so use this code
void sendSensorData() {
  bool tempGood = false; // if true we got a good temperature reading
  bool humiGood = false; // if true we got a good humdity reading

  String t = "\r\n";
  String h = "\r\n";

  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    char tmpbuffer[5];
    t = dtostrf(event.temperature, 2, 1, tmpbuffer);
    tempGood = true;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    char humbuffer[5];
    h = dtostrf(event.relative_humidity, 2, 1, humbuffer);
    humiGood = true;
  }
  if (tempGood && humiGood)
  {
    sendCmd(sndMessage + "13,T=" + t + "|H=" + h + "\r\n") ;
    Serial.print(sndMessage + "13,T=" + t + "|H=" + h + "\r\n");
  }
}
#else
// if #define DHTSensor is commented out we will use this version of sendSensorData()
// needs to be improved but for now just send test messages
// with a forever changing component so we know things
// are working at the remote end
void sendSensorData()
{
  String payload = "Hello There! " + String(millis());
  String myMessage = sndMessage + payload.length() + "," + payload + "\r\n";
  Serial.print(myMessage );
  sendCmd(myMessage);
}
#endif

/**** Message handlers for the RYLR896 ****/
// A message with +RCV in the header
void ProcessMSG(String rcvMsg) {
  flashLED(flashRcvMessage, statusLED);// FLASH  LED to indicate a message received
  Serial.print(rcvMsg); // diagnostics print
  // of course this is useless without actually doing something with the message
}

// A message with +ERR in the header
void ProcessError(String errMsg) {
  Serial.print("Receved Error " + errMsg); // diagnostics print
  cmdComplete = true; // done last command even if an error happened
}

// A message with +OK in the header
void ProcessOK(String okMsg) {
  Serial.print(okMsg); // diagnostics print
  cmdComplete = true; // sucesfully completed last command
}

// Send a command to RYLR896 or transmit a message
void sendCmd(const String& messageToSend)
{
  Serial1.print(messageToSend);
  cmdComplete = false; // to allow for complete of command/transmit
  flashLED(flashSentCommand, statusLED);// FLASH  LED to indicate command processed
}

//just flash an led n number of times
void flashLED(uint flashCount, uint32_t pinID) {
  // FLASH EXTERNAL LED
  for (uint x = 0; x < flashCount; x++) {
    digitalWrite(pinID, HIGH);
    delay(flashDelay);
    digitalWrite(pinID, LOW);
    delay(flashDelay);
  }
}
