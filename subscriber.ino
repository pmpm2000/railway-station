#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>
#include <Arduino.h>
#include <ZsutFeatures.h>   

#define UDP_REMOTE_PORT 8696
#define PACKET_BUFFER_SIZE 32

//adres IP BROKERA - DO ZMIANY
ZsutIPAddress address_ip = ZsutIPAddress(192,168,68,142);

//adres MAC arduino
byte mac[] = {0x00, 0xaa, 0xbb, 0xcc, 0xde, 0xe7};
ZsutEthernetUDP Udp;

char packetBuffer[255];

void setup() {
  Serial.begin(9600);
  Serial.print("Konfiguracja subscribera \n");
  ZsutEthernet.begin(mac);
  Serial.print("Adres IP subscribera: ");
  Serial.println(ZsutEthernet.localIP());
  Udp.begin(UDP_REMOTE_PORT);
  Serial.println("Ustawiony adres IP brokera: ");
  Serial.println(address_ip);
  Serial.println("Konfiguracja subscibera zakonczona. Rozpoczynam prace... \n");
  
}

void loop() {
  String message;
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(packetBuffer,255);
    if (len > 0) {
        packetBuffer[len] = 0;
    }
    
    if (packetBuffer[0] == 'w' && packetBuffer[1] == ',')
    {
      Serial.println("Otrzymalem wiadomosc: \n");
      for(int i=2;i<packetSize;i++)
      {
        message = message + String(packetBuffer[i]);
      }
      Serial.println(message);
    }
    
    else if ((packetBuffer[0] == 's' && packetBuffer[1] == ',') || (packetBuffer[0] == 'u' && packetBuffer[1] == ','))
    {
      Serial.println("Otrzymalem polecenie: \n");
      String msg = packetBuffer;
      Serial.println(msg);
      Udp.beginPacket(address_ip, UDP_REMOTE_PORT);
      Udp.write(msg.c_str(), msg.length());
      Udp.endPacket();
    }
    message = "";
  }
}
