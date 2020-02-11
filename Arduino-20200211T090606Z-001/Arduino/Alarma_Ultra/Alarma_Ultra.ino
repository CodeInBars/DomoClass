#include "SR04.h"
#define TRIG_PIN 12
#define ECHO_PIN 11
#define ALARM 3
#define CONSIGNA 30.00

SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);
long a;

void setup() {
   Serial.begin(9600);
   pinMode(3,OUTPUT);
   delay(1000);
}

void loop() {
   a=sr04.Distance();
   Serial.print(a);
   Serial.println("cm");
   if(a < CONSIGNA){
    tone(3,400);
    delay(500);
    tone(3,200);
    delay(500);
   }else{
    noTone(3);
   }
}
