#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 25
#define CALOR 30.00
#define FRIO 20.00
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int heater = 35;
int fan = 34;

void setup() {
  Serial.begin(9600);
  
  sensors.begin();
}
void loop() {
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  if(temp >= CALOR){
    digitalWrite(heater,LOW);
    digitalWrite(fan,HIGH);
  }
 
}
