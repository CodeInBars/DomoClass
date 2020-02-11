#include "IRremote.h"


int receiver = 22; // Signal Pin of IR receiver to Arduino Digital Pin 11
int ledsMando[8] = {46, 47, 48, 49, 50, 51, 52};

IRrecv irrecv(receiver);     // create instance of 'irrecv'

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 8; i++) {
    pinMode(ledsMando[i], OUTPUT);
  }
}

void loop() {
  if (irrecv.decode(&results)) {
    translateIR();
    irrecv.resume(); // receive the next value
  }
}

void translateIR() {

  switch (results.value) {
    case 0xFFA25D:
      for (int i = 0; i < 8; i++) {
        digitalWrite(ledsMando[i], LOW);
      }
      break;
    case 0xFFE21D:
      Serial.println("FUNC/STOP");
      break;
    case 0xFF629D:
      Serial.println("VOL+");
      break;
    case 0xFF22DD:
      Serial.println("FAST BACK");
      break;
    case 0xFF02FD:
      Serial.println("PAUSE");
      break;
    case 0xFFC23D:
      Serial.println("FAST FORWARD");
      break;
    case 0xFFE01F:
      Serial.println("DOWN");
      break;
    case 0xFFA857:
      Serial.println("VOL-");
      break;
    case 0xFF906F:
      Serial.println("UP");
      break;
    case 0xFF9867:
      Serial.println("EQ");
      break;
    case 0xFFB04F:
      Serial.println("ST/REPT");
      break;
    case 0xFF6897://0
      digitalWrite(leds[0], HIGH);
      break;
    case 0xFF30CF://1
      digitalWrite(leds[1], HIGH);
      break;
    case 0xFF18E7://2
      digitalWrite(leds[2], HIGH);
      break;
    case 0xFF7A85://3
      digitalWrite(leds[3], HIGH);
      break;
    case 0xFF10EF://4
      digitalWrite(leds[4], HIGH);
      break;
    case 0xFF38C7://5
      digitalWrite(leds[5], HIGH);
      break;
    case 0xFF5AA5://6
      digitalWrite(leds[6], HIGH);
      break;
    case 0xFF42BD://7
      digitalWrite(leds[7], HIGH);
      break;
    case 0xFF4AB5://8
      
      break;
    case 0xFF52AD://9
      
      break;
    case 0xFFFFFFFF:
      Serial.println(" REPEAT");
      break;
    default:
      Serial.println(" other button   ");

  }
  delay(500);
}
