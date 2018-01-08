/*
* This sketch sends data via HTTP PUT requests to a Hue Bridge.
*
*
*/

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// Wifi Settings

const char* ssid = "OUR_HOME";
const char* password = "A987654321";

// Hue Settings

const char* bridge_ip = "192.168.0.100"; // Hue Bridge IP
const int port = 80;
String user="drHb0HaMrDu7g-ReyNCtSuujvS9i8eJvvEXUg8oH"; // Hue Bridge user -To create a user check this page (http://support.intuilab.com/kb/connecte ... hue-lights)
String light="0";

// Commands

String hue_on="{\"on\":true}";
String hue_off="{\"on\":false}";

const int buttonPin = D0;
int buttonState = 0;
const int pinAdc = A0;

int threshold = 250; //Change This
int volume;
int clap = false;
int clapInterval = 0;


void setup() {
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    hue_control();
    //check_light_status();
    delay(1000);
 }

  long sum = 0;
  for(int i=0; i<32; i++)
  {
      sum += analogRead(pinAdc);
  }
  sum >>= 5;

  //Serial.println(sum);
  
  if(sum>=threshold){
    // If first clap was made, then 
    if (!clap) {
      Serial.println("Clap interval TRUE");
      clap = true;
    } else if (clapInterval > 100) {
      hue_control();
      Serial.println(sum);
      clap = false;
      clapInterval = 0;
    }
  }

  if (clap) {
   clapInterval++;
   
   if (clapInterval >= 500) {
     clapInterval = 0;
     Serial.println("Clap interval FALSE");
     clap = false;
   }
 }
 
 delay(1);
  
    
}

const char* check_light_status() {
 
 Serial.print("Connecting to ");
 Serial.println(bridge_ip);

 // Use WiFiClient class to create TCP connections
 WiFiClient client;

 if (!client.connect(bridge_ip, port)) {
 Serial.println("Connection failed");
 //return;
 }


 // We now create a URI for the request
 String url = "/api/";
 url += user;
 url += "/groups/";
 url += light;
 url += "/";

 Serial.print("Requesting URL: ");
 Serial.println(url);

 // This will send the request to the server
 client.print(String("GET ") + url + " HTTP/1.1\r\n" +
             "Host: " + String(bridge_ip) + ":" + String(port) + "\r\n" +
             "Connection: close\r\n\r\n");
 unsigned long timeout = millis();
 while (client.available() == 0) {
  if (millis() - timeout > 5000) {
    Serial.println(">>> Client Timeout !");
    client.stop();
    //return;
  }
 }


 // Read all the lines of the reply from server and print them to Serial
 String line;
 while(client.available()){
  line = client.readStringUntil('\r');
  Serial.print(line);

 }

 StaticJsonBuffer<1024> jsonBuffer;
 
 JsonObject& root = jsonBuffer.parseObject(line);

  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    //eturn;
  }
 const char* state = root["state"]["any_on"];
  Serial.println(state);



 Serial.println();
 Serial.println("closing connection");

 return state;
}



void hue_control() {
  String status = check_light_status();
  Serial.println(status);
  Serial.print("Connecting to ");
  Serial.println(bridge_ip);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  if (!client.connect(bridge_ip, port)) {
  Serial.println("Connection failed");
  return;
  }

  // This will send the request to the server
  client.println("PUT /api/" + user + "/groups/" + light + "/action");
  client.println("Host: " + String(bridge_ip) + ":" + String(port));
  client.println("User-Agent: ESP8266/1.0");
  client.println("Connection: close");
  client.println("Content-type: text/xml; charset=\"utf-8\"");
  client.print("Content-Length: ");

  if(status == "true"){
  client.println(hue_off.length()); // PUT COMMAND HERE
  client.println();
  client.println(hue_off); // PUT COMMAND HERE
  } else {
    client.println(hue_on.length()); // PUT COMMAND HERE
    client.println();
    client.println(hue_on); // PUT COMMAND HERE
  }
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
  String line = client.readStringUntil('\r');
  Serial.print(line);
  }

  Serial.println();
  Serial.println("Closing connection");
}


 
