#include <SPI.h>
#include <LoRa.h>
#define node 2
const int csPin = 8;          // LoRa radio chip select
const int resetPin = 4;       // LoRa radio reset
const int irqPin = 7;         // change for your board; must be a hardware interrupt pin

String outgoing;              // outgoing message
byte localAddress = 0xFF;     // address of this device
byte destination[node] = {0xBA, 0xBB};
uint8_t value[node];
bool laststate = true;
bool state[node] = {false, false};
uint16_t interval = 5000;
uint32_t pretime = 0;
uint32_t currenttime = 0;
uint16_t i = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Duplex");
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(433E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  Serial.println("LoRa init succeeded.");
}

void loop() {
  currenttime = millis();
  if (currenttime - pretime >= interval) {
    i = 0;
    state[i] = !state[i];
    pretime = currenttime;
  }
  if (state[i] && i < node) {
    Serial.print("i state = ");
    Serial.println(i);
    sendMessage(i);
    state[i] = !state[i];
    i++;
  }
  onReceive(LoRa.parsePacket());
}

void sendMessage(int x) {
  String outgoing = "Hello LoRa: " + String(x);
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination[x]);           // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  Serial.print("Sending");
  Serial.println(outgoing);
}

void onReceive(int packetSize) {
  if (packetSize == 0) {
    //Serial.println("No packet");
    return;
  }
  // read packet header bytes:
  byte recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingValue = LoRa.read();     // incoming value of RainGate
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";                 // payload of packet

  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming += (char)LoRa.read();      // add bytes one by one
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress) {
    Serial.println("This message is not for me.");
    return;
  } else {
    if (sender == destination[i - 1]) {
      value[i - 1] = incomingValue;
    }
    sprint(sender, recipient, incomingValue, incomingLength, incoming);
    state[i] = !state[i];
  }
}

void sprint(byte sender, int recipient, byte incomingValue, byte incomingLength, String incoming) {
  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Value of Raingate: " + String(incomingValue));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}
