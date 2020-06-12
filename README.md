 REYAX RYLR869 example sketch with supporting role from a DHT22 Temperature and Humidity Sensor

More videos and tutorials http://www.youtube.com/thebreadboardca
my website                https://www.thebreadboard.ca
github                    https://https://github.com/thebreadboard
support me on Patrion     https://www.patreon.com/thebreadboard
or paypal me              https://paypal.me/thebreadboardca



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
