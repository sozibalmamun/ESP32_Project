
#include <WiFi.h>
#include <WebSocketsClient.h>
#include "StompClient.h"


/**
* Stomp server settings
**/
//wss://grozziieget.zjweiting.com:3091/CloudSocket-Dev/websocket
bool useWSS                       = true; //false;
const char* ws_host               = "grozziieget.zjweiting.com"; //"192.168.1.35";
const int ws_port                 = 3091;//8080;
const char* ws_baseurl            = "/CloudSocket-Dev/websocket/"; //"/websocket/"; // don't forget leading and trailing "/" !!!

bool sample = false;


// VARIABLES
WebSocketsClient webSocket;

Stomp::StompClient stomper(webSocket, ws_host, ws_port, ws_baseurl, true);


void setup() {
  
    Serial.begin(115200);

    // Connect to wifi
    WiFi.begin("myssid", "mypassword");

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // Check if connected to wifi
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("No Wifi!");
        return;
    }

    Serial.println("Connected to Wifi, Connecting to server.");
  // Start the StompClient
  stomper.onConnect(subscribe);
  stomper.onError(error);



  if (useWSS) {
    stomper.beginSSL();
  } else {
    stomper.begin();
  }
  
  //stomper.sendMessage("/app/cloud", "hello");
  //takeSample();
}

// Once the Stomp connection has been made, subscribe to a topic
void subscribe(Stomp::StompCommand cmd) {
  Serial.println("Connected to STOMP broker");
  stomper.subscribe("/topic/cloud", Stomp::CLIENT, handleSampleMessage);
}

void error(const Stomp::StompCommand cmd) {
  Serial.println("ERROR: " + cmd.body);
}

Stomp::Stomp_Ack_t handleBlinkMessage(const Stomp::StompCommand cmd) {
  Serial.println("Got a message!");
  Serial.println(cmd.body);

  return Stomp::CONTINUE;
}

Stomp::Stomp_Ack_t handleSampleMessage(const Stomp::StompCommand cmd) {
  Serial.println("Got a message!");
  Serial.println(cmd.body);

  sample = true;
  return Stomp::CONTINUE;
}

void takeSample() {
  if (sample) {
    stomper.sendMessage("/app/cloud", "ESP");
    sample = false;
    //Serial.print("log: takeSample");
  }
}

void loop() {
  webSocket.loop(); 
  takeSample();
}
