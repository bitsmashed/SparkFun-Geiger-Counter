 /*
   Cosm sensor client using the SparkFun Geiger Counter
   SparkFun Electronics 2013, aaron@sparkfun.com
   2-25-13
   beerware licsense

 
 This sketch connects the SparkFun Geiger Counter to Cosm (http://www.cosm.com)
 using a Arduino Ethernet board. You can use the Arduino Ethernet shield, or
 ,anything with a Wiznet Ethernet module on board.
 
 This example has been updated to use version 2.0 of the Cosm.com API. 
 To make it work, create a feed with a datastream, and give it the ID
 sensor1. Or change the code below to match your feed.
 
 Be sure to:
 * add your API key
 * add your FEEDID
 * add the MAC address of your Arduino ethernet 
 * verify the IPAdress server() for the Cosm servers: https://cosm.com/docs/v2/ip_addresses.html
 
 Circuit connections:
 * 5V and GND on Arduino to 5V and GND on Geiger counter
 * TX on Geiger counter board attached to Arduino D2 over newsoftserial
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>

                        
#define APIKEY         "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" // your cosm api key, should be a random alpha numeric set
#define FEEDID         xxxxx // your feed ID, the SparkFun Geiger Counter feed id is 22279, https://cosm.com/feeds/22279
#define USERAGENT      "SparkFun Geiger Counter (22279)" // user agent is the project name

// assign a MAC address for the ethernet controller.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
// fill in your address here:
byte mac[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// fill in an available IP address on your network here,
// for manual configuration:
IPAddress ip(192,168,0,1); //type ipconfig in a terminal to find your IP
// initialize the library instance:
EthernetClient client;

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(173,203,98,29);    // numeric IP for api.cosm.com
//char server[] = "api.cosm.com";   // name address for cosm API, for some reason this doesn't work

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop

SoftwareSerial mySerial(2, 3); // rx, tx

unsigned int count = 0;
unsigned int countStartTime = 0;
int countsPerMinute;

float coefficientOfConversion = 140.0 / 60000.0;
float sV = 0;

void setup() {
  // start serial port:
  Serial.begin(9600);
  mySerial.begin(9600);
 // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // DHCP failed, so use a fixed IP address:
    Ethernet.begin(mac, ip);
  }
}

void loop() {
  
  geiger_CPM();
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println("disconnecting.");
    client.stop();
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

void geiger_CPM()
{  
  while (mySerial.available() > 0) {
     int inChar = mySerial.read();
     if (inChar == '0' || inChar == '1') {
       count++;
      }
  }

  unsigned int now = millis();
  unsigned int elapsedTime = now - countStartTime;
  //Serial.println(elapsedTime); //prints timer
  if (elapsedTime >= 60000) {
    countsPerMinute = count;
    sV = (float) countsPerMinute * coefficientOfConversion;
    count = 0;
    countStartTime = now;
    // if you're not connected, then connect again and send data:
    if(!client.connected()) {
      sendData(countsPerMinute);
    }
    Serial.print("CPM: ");
    Serial.println(countsPerMinute);
    Serial.print("uSv/hr: ");
    Serial.println(sV, 5);
  }
}

// this method makes a HTTP connection to the server:
void sendData(int thisData) {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.print("PUT /v2/feeds/");
    client.print(FEEDID);
    client.println(".csv HTTP/1.1");
    client.println("Host: api.cosm.com");
    client.print("X-ApiKey: ");
    client.println(APIKEY);
    client.print("User-Agent: ");
    client.println(USERAGENT);
    client.print("Content-Length: ");

    // calculate the length of the sensor reading in bytes:
    // 8 bytes for "sensor1," + number of digits of the data:
    int thisLength = 2 + getLength(thisData);// + 2 + getLength(thisData2);
    client.println(thisLength);

    // last pieces of the HTTP PUT request:
    client.println("Content-Type: text/csv");
    client.println("Connection: close");
    client.println();

    // here's the actual content of the PUT request:
    client.print("0,");
    client.println(thisData);
    //client.print("1,");
    //client.println(thisData2);
  
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
   // note the time that the connection was made or attempted:
  lastConnectionTime = millis();
}


// This method calculates the number of digits in the
// sensor reading.  Since each digit of the ASCII decimal
// representation is a byte, the number of digits equals
// the number of bytes:

int getLength(int someValue) {
  // there's at least one byte:
  int digits = 1;
  // continually divide the value by ten, 
  // adding one to the digit count for each
  // time you divide, until you're at 0:
  int dividend = someValue /10;
  while (dividend > 0) {
    dividend = dividend /10;
    digits++;
  }
  // return the number of digits:
  return digits;
}

    
