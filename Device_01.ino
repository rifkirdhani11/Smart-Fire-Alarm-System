/*
   Fire Alarm Detector
   Device 1/2
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <RF24.h>
#include <SoftwareSerial.h>

SoftwareSerial SIM800L(2, 3);   // RX, TX

#define flame1 A0
#define flame2 A1
#define flame3 A2
#define flame4 A3
#define flame5 A6
bool flameState = false;

#define gas 7
bool gasState = false;

#define relay 9

LiquidCrystal_I2C lcd (0x3F, 16, 2);  // SDA=A4, SCL=A5

RF24 radio (10, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};
bool fireState2 = false;
bool fireState = false;
bool sendSMS = false;

void setup() {
  // Setup GSM
  SIM800L.begin(9600);

  // Setup LCD
  lcd.begin();
  lcd.backlight();
  lcd.clear();

  // Setup Radio module
  radio.begin();
  radio.setChannel(1);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);
  radio.powerUp();

  // Setup PORT
  pinMode(flame1, INPUT);
  pinMode(flame2, INPUT);
  pinMode(flame3, INPUT);
  pinMode(flame4, INPUT);
  pinMode(flame5, INPUT);
  pinMode(gas, INPUT);

  pinMode(relay, OUTPUT);
}

void loop() {
  flameRead();
  gasRead();
  lcd.clear();

  delay(5);

  if (flameState == true && gasState == true) {
    fireState = true;
    lcd.setCursor(0, 0);
    lcd.print("Fire Detected    ");
    lcd.setCursor(0, 1);
    lcd.print("                 ");
    digitalWrite(relay, LOW);
    //sendMessage();
  }
  else {
    fireState = false;
    sendSMS = false;
    radio.startListening();
    if (radio.available() == 1) {
      radio.read(&fireState2, sizeof(fireState2));
      if (fireState2 == true) {
        lcd.setCursor(0, 0);
        lcd.print("Fire Detected ");
        lcd.setCursor(0, 1);
        lcd.print("Loc : Device 2");
        digitalWrite(relay, LOW);
        sendMessage();
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Standby       ");
        lcd.setCursor(0, 1);
        lcd.print("              ");
        digitalWrite(relay, HIGH);
      }
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Disconnected    ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      digitalWrite(relay, HIGH);
    }

    delay(5);
    radio.stopListening();
  }

  radio.write(&fireState, sizeof(fireState));
}

void flameRead () {
  int flameValue1 = analogRead(flame1);
  int flameValue2 = analogRead(flame2);
  int flameValue3 = analogRead(flame3);
  int flameValue4 = analogRead(flame4);
  int flameValue5 = analogRead(flame5);

  if (flameValue1 > 350 || flameValue2 > 350 || flameValue3 > 350 || flameValue4 > 350 || flameValue5 > 350) {
    flameState = true;
  } else flameState = false;
}

void gasRead() {
  if (!digitalRead(gas)) {
    gasState = true;
  } else gasState = false;
}

void sendMessage () {
  if (!sendSMS) {    
    SIM800L.println("AT+CMGF=1");                   // Set format SMS to ASCII
    delay(100);
    SIM800L.println("AT+CMGS=\"081334381097\"\r");
    delay(100);
    SIM800L.println("WARNING!! FIRE DETECTED AT DEVICE 2");
    delay(100);
    SIM800L.println((char)26);
    delay(100);
    SIM800L.write("AT+CMGF=1\r\n");                   // Set format SMS to ASCII
    delay(100);
    SIM800L.write("AT+CMGS=\"087729993399\"\r\n");
    delay(100);
    SIM800L.write("BAHAYA!! KEBAKARAN DI ALAMAT B");
    SIM800L.write((char)26);
    sendSMS = true;
  }
}

