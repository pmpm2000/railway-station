#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>
#include <Arduino.h>
#include <ZsutFeatures.h>

#define UDP_REMOTE_PORT 8696
#define PACKET_BUFFER_SIZE 32

#define TYPE0 "n" //numer
#define TYPE1 "r" //id relacji
#define TYPE2 "p" //id przewoznika
#define TYPE3 "t" //typ skladu: ezt (0) lub uni (1)

//adres IP BROKERA - DO ZMIANY
ZsutIPAddress address_ip = ZsutIPAddress(192, 168, 68, 142);

//adres MAC arduino
byte mac[] = {0x00, 0xaa, 0xbb, 0xcc, 0xde, 0xe7};

int sensor0Value;
int sensor0ValueOld = 1024;
int sensor1Value;
int sensor1ValueOld = 1024;
int sensor2Value;
int sensor2ValueOld = 1024;
int sensor3Value;
int sensor3ValueOld = 1024;

String TYPE0MESSAGE = "N";
String TYPE1MESSAGE = "R";
String TYPE2MESSAGE = "P";
String TYPE3MESSAGE0 = "Te";
String TYPE3MESSAGE1 = "Tu";

//uzupełnij numer peronu
String PERON = "P1 ";

ZsutEthernetUDP Udp;

void setup() {
  Serial.begin(9600);
  Serial.print("Konfiguracja publishera \n");
  ZsutEthernet.begin(mac);
  Serial.print("Adres IP publishera: ");
  Serial.println(ZsutEthernet.localIP());
  Udp.begin(UDP_REMOTE_PORT);
  Serial.println("Ustawiony adres IP brokera: ");
  Serial.println(address_ip);
  Serial.println("Konfiguracja publishera zakonczona. Rozpoczynam prace... \n");
}

void loop() {

  /// sensor 0 START
  int sensor0Value = ZsutAnalog0Read();
  if (sensor0Value != sensor0ValueOld) {
    String msg = "p" + String(",") + TYPE0 + String(",") + PERON + TYPE0MESSAGE + String(sensor0Value);
    Udp.beginPacket(address_ip, UDP_REMOTE_PORT);
    Udp.write(msg.c_str(), msg.length());
    Udp.endPacket();
    Serial.println(msg);
  }
  sensor0ValueOld = sensor0Value;
  /// sensor 0 END

  /// sensor 1 START
  int sensor1Value = ZsutAnalog1Read();
  if (sensor1Value != sensor1ValueOld) {
    String msg = "p" + String(",") + TYPE1 +  String(",")  + PERON + TYPE1MESSAGE + String(sensor1Value);
    Udp.beginPacket(address_ip, UDP_REMOTE_PORT);
    Udp.write(msg.c_str(), msg.length());
    Udp.endPacket();
    Serial.println(msg);
  }
  sensor1ValueOld = sensor1Value;
  /// sensor 1 END

  /// sensor 2 START
  int sensor2Value = ZsutAnalog2Read();
  if (sensor2Value != sensor2ValueOld) {
    String msg = "p" + String(",") + TYPE2 +  String(",") + PERON + TYPE2MESSAGE + String(sensor2Value);
    Udp.beginPacket(address_ip, UDP_REMOTE_PORT);
    Udp.write(msg.c_str(), msg.length());
    Udp.endPacket();
    Serial.println(msg);
  }
  sensor2ValueOld = sensor2Value;
  /// sensor 2 END

  /// sensor 3 START
  int sensor3Value = ZsutAnalog3Read();
  if (sensor3Value != sensor3ValueOld) {
    String msg;
    if(sensor3Value == 0)
    {
      msg = "p" + String(",") + TYPE3 + "," + PERON + TYPE3MESSAGE0;
    }
    else
    {
      msg = "p" + String(",") + TYPE3 + "," + PERON + TYPE3MESSAGE1;
    }
    Udp.beginPacket(address_ip, UDP_REMOTE_PORT);
    Udp.write(msg.c_str(), msg.length());
    Udp.endPacket();
    Serial.println(msg);
  }
  sensor3ValueOld = sensor3Value;
  /// sensor 3 END
}
