#include <SPI.h>
#include <LoRa.h>

#define RainGatePin 10
const int csPin = 8;
const int resetPin = 4;
const int irqPin = 7;

String outgoing;              // outgoing message
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
bool state = false;
bool laststate = true;

void setup() {
  Serial.begin(9600);
  pinMode(RainGatePin, INPUT_PULLUP);
  while (!Serial);
  Serial.println("LoRa Duplex");
  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }

  Serial.println("LoRa init succeeded.");
}

void loop() {
  static uint16_t countRain = 0;
  int readRain = digitalRead(RainGatePin);
  if (readRain == HIGH && laststate == false) {
    countRain++;
    Serial.println(countRain);
  }
  if (state) {
    String message = "Count : " + (String)countRain;
    sendMessage(message, countRain);
    Serial.println("Sending " + message);
    state = !state;
  }
  laststate = readRain;
  onReceive(LoRa.parsePacket());
}

void sendMessage(String outgoing, uint8_t value) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(value);
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  // read packet header bytes:
  byte recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {
    Serial.println("error: message length does not match length");
    return;
  }

  if (recipient != localAddress || sender != destination) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  } else {
    state = !state;
  }

  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}
