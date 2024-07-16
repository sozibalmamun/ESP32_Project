
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
const char testdata[] = "-----BEGIN CERTIFICATE-----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-----END CERTIFICATE-----";


// VARIABLES
WebSocketsClient webSocket;

Stomp::StompClient stomper(webSocket, ws_host, ws_port, ws_baseurl, true);


void setup() {
    // USE_SERIAL.begin(921600);
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
    stomper.sendMessage("/app/cloud", "this is esp32");
    sample = false;
    //Serial.print("log: takeSample");
  }
}

void loop() {
  webSocket.loop(); 
  takeSample();
}
