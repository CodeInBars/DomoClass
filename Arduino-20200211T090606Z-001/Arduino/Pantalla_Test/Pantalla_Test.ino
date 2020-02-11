//#include <SD.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <RTClib.h>//Para RTC
#include <OneWire.h>
#include <DallasTemperature.h>
#include "IRremote.h" //Remote control
#include "SR04.h" // Sensor ultrasonido

#define TIME 60000//Tiempo
#define ONE_WIRE_BUS 53

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFD20
#define MAROON  0x7800
#define PINK    0xF81F
#define NAVY    0x000F
#define GREY    0xA4A4A4
#define DARKGREEN   0x0000

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define BOXSIZE 120
#define TEXTSIZE 9

#define MINPRESSURE 10
#define MAXPRESSURE 1000

//Pines ultrasonido
#define TRIG_PIN 50
#define ECHO_PIN 49

int ajusteX, ajusteY; //Ajustes de pantalla

boolean heaterState = true, fanState = true, luzState = true, flagAlarm = false;
boolean flagAuto = false, flagAutoHeater = false, flagAutoFan = false, statusAlarm = LOW;;

//Pines usados en la placa arduino
int receiver = 52; //Pin receptor señal mando
int ledsMando[8] = {25, 26, 27, 28, 29, 30, 31, 32}; //Leds
int heater = 22, fan = 23, luz = 24; //Controlador de calefactor y luz
int pir = 51; //Sensor de movimiento
int water = A10;//Sensor de nivel de agua
int pinAlarm = 33;//Led para la alarma

int opc = 0;
float temp;
int tempH = 20, tempF = 20;
long distancia;

unsigned long presentime;//Para contar el tiempo
unsigned long previoustime = 0;//Para contar el  tiempo
char buffer[128];
boolean flag = false;
//Objetos instanciados
RTC_DS3231 RTC;//Variable de RTC tipo 3231
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DateTime now;
IRrecv irrecv(receiver);
decode_results results;
SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN); 

void translateIR() {

  switch (results.value) {
    case 0xFFA25D: //Boton Apagar
      for (int i = 0; i < 8; i++) {
        digitalWrite(ledsMando[i], LOW);
      }
      break;
    case 0xFFE21D:
      Serial.println("FUNC/STOP");
      flagAlarm = false;
      statusAlarm = LOW;
      digitalWrite(pinAlarm,statusAlarm);
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
      for (int i = 0; i < 8; i++) {
        digitalWrite(ledsMando[i], LOW);
      }
      break;
    case 0xFF30CF://1
      for (int i = 0; i < 8; i++) {
        digitalWrite(ledsMando[i], LOW);
      }
      digitalWrite(ledsMando[0], HIGH);
      break;
    case 0xFF18E7://2
      for (int i = 0; i < 8; i++) {
        digitalWrite(ledsMando[i], LOW);
      }
      digitalWrite(ledsMando[1], HIGH);
      break;
    case 0xFF7A85://3
      for (int i = 0; i < 8; i++) {
        digitalWrite(ledsMando[i], LOW);
      }
      digitalWrite(ledsMando[2], HIGH);
      break;
    case 0xFF10EF://4
      for (int i = 0; i < 8; i++) {
        digitalWrite(ledsMando[i], LOW);
      }
      digitalWrite(ledsMando[3], HIGH);
      break;
    case 0xFF38C7://5
      for (int i = 0; i < 8; i++) {
        digitalWrite(ledsMando[i], LOW);
      }
      digitalWrite(ledsMando[4], HIGH);
      break;
    case 0xFF5AA5://6
      for (int i = 0; i < 8; i++) {
        digitalWrite(ledsMando[i], LOW);
      }
      digitalWrite(ledsMando[5], HIGH);
      break;
    case 0xFF42BD://7
      for (int i = 0; i < 8; i++) {
        digitalWrite(ledsMando[i], LOW);
      }
      digitalWrite(ledsMando[6], HIGH);
      break;
    case 0xFF4AB5://8
      for (int i = 0; i < 8; i++) {
        digitalWrite(ledsMando[i], LOW);
      }
      digitalWrite(ledsMando[7], HIGH);
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

void setup() {
  Serial.begin(19200);
  for (int i = 0; i < 8; i++) {
    pinMode(ledsMando[i], OUTPUT);
  }
  irrecv.enableIRIn();
  sensors.begin();
  RTC.begin();
  if (!RTC.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }
  //RTC.adjust(DateTime(__DATE__, __TIME__));//Para ajustar fecha y hora

  tft.reset();
  uint16_t identifier = 0x9341;
  tft.begin(identifier);

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  pinMode(heater, OUTPUT);
  pinMode(luz, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(pinAlarm, OUTPUT); //Alarma Distancia
  digitalWrite(heater, HIGH);
  digitalWrite(fan, HIGH);
  digitalWrite(luz, HIGH);
  ajusteX = 5;
  ajusteY = 5;
  tft.setRotation(2);
  tft.fillScreen(DARKGREEN);

  now = RTC.now();//Variable para RTC
  sprintf(buffer, "%02d/%02d/%04d %02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute());

  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);

  menu();
}

void loop() {
  now = RTC.now();//Variable para RTC
  sprintf(buffer, "%02d/%02d/%04d %02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute());

  TSPoint p = ts.getPoint();

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE and p.z < MAXPRESSURE) {

    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0 );
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

    p.x += 5;
    p.y -= 5;

    switch (opc) {
      case 0://Menu
        if (p.x >= 10 and p.x <= 220 and p.y >= 1 and p.y <= 33) {
          tft.fillScreen(DARKGREEN);
          botonesHeater(heaterState);
          opc = 1;
        }
        if (p.x >= 10 and p.x <= 220 and p.y >= 33 and p.y <= 33 + 49) {
          tft.fillScreen(DARKGREEN);
          botonesFan(fanState);
          opc = 2;
        }
        if (p.x >= 10 and p.x <= 220 and p.y >= 33 + 49 and p.y <= 33 + 49 + 49) {
          tft.fillScreen(DARKGREEN);
          botonesLuz(luzState);
          opc = 3;
        }
        if (p.x >= 10 and p.x <= 230 and p.y >= 180 and p.y <= 220) {
          tft.drawRect(10, 170, 220, 70, GREEN);
          tft.fillScreen(DARKGREEN);
          waterSensor();
          opc = 4;
          flag = true;
        }
        break;
      case 1://Heater
        if (flagAutoHeater == false) {
          if (p.x >= 130 and p.x <= 230 and p.y >= 10 and p.y <= 150 and heaterState == true) {
            encender(0);
            botonesHeater(heaterState);
          }
          if (p.x >= 10 and p.x <= 110 and p.y >= 10 and p.y <= 150 and heaterState == false) {
            apagar(0);
            botonesHeater(heaterState);

          }
        }
        if (p.x >= 190 and p.x <= 220 and p.y >= 250 and p.y <= 290) {
          flagAutoHeater = !flagAutoHeater;
          if (temp < tempH) {
            botonesHeater(false);
          } else {
            botonesHeater(true);
          }
        }
        if (p.x >= 1 and p.x <= 71 and p.y >= 260 and p.y <= 310) {
          tft.drawRect(7, 275, 60, 30, WHITE);
          menu();
          opc = 0;
        }
        if (p.x >= 180 and p.x <= 210 and p.y >= 200 and p.y <= 220) {
          if (tempH >= 33) {
            tft.drawRect(175, 215, 30, 30, YELLOW);
            tempH = 33;
          } else {
            tempH++;
            tft.drawRect(175, 215, 30, 30, RED);
            tft.drawRect(47, 215, 30, 30, BLACK);
            tft.fillRect(100, 215, 45, 30, BLACK);
          }
          if (flagAutoHeater) {
            if (temp < tempH) {
              botonesHeater(false);
            } else {
              botonesHeater(true);
            }
          } else {
            botonesHeater(heaterState);
          }
        }
        if (p.x >= 45 and p.x <= 80 and p.y >= 200 and p.y <= 220) {
          if (tempH <= 15) {
            tft.drawRect(47, 215, 30, 30, YELLOW);
            tempH = 15;
          } else {
            tempH--;
            tft.drawRect(47, 215, 30, 30, RED);
            tft.drawRect(175, 215, 30, 30, BLACK);
            tft.fillRect(100, 215, 45, 30, BLACK);
          }
          if (flagAutoHeater) {
            if (temp < tempH) {
              botonesHeater(false);
            } else {
              botonesHeater(true);
            }
          } else {
            botonesHeater(heaterState);
          }
        }
        break;
      case 2://Fan
        if (flagAutoFan == false) {
          if (p.x >= 130 and p.x <= 230 and p.y >= 10 and p.y <= 150 and fanState == true) {
            encender(1);
            botonesFan(fanState);
          }
          if (p.x >= 10 and p.x <= 110 and p.y >= 10 and p.y <= 150 and fanState == false) {
            apagar(1);
            botonesFan(fanState);
          }
        }
        if (p.x >= 190 and p.x <= 220 and p.y >= 250 and p.y <= 290) {
          flagAutoFan = !flagAutoFan;
          if (temp > tempF) {
            botonesFan(false);
          } else {
            botonesFan(true);
          }
        }
        if (p.x >= 1 and p.x <= 71 and p.y >= 260 and p.y <= 310) {
          tft.drawRect(7, 275, 60, 30, WHITE);
          menu();
          opc = 0;
        }
        if (p.x >= 180 and p.x <= 210 and p.y >= 200 and p.y <= 220) {
          if (tempF >= 33) {
            tft.drawRect(175, 215, 30, 30, YELLOW);
            tempF = 33;
          } else {
            tempF++;
            tft.drawRect(175, 215, 30, 30, RED);
            tft.drawRect(47, 215, 30, 30, BLACK);
            tft.fillRect(100, 215, 45, 30, BLACK);
          }
          if (flagAutoFan) {
            if (temp > tempF) {
              botonesFan(false);
            } else {
              botonesFan(true);
            }
          } else {
            botonesFan(fanState);
          }
        }
        if (p.x >= 45 and p.x <= 80 and p.y >= 200 and p.y <= 220) {
          if (tempF <= 15) {
            tft.drawRect(47, 215, 30, 30, YELLOW);
            tempF = 15;
          } else {
            tempF--;
            tft.drawRect(47, 215, 30, 30, RED);
            tft.drawRect(175, 215, 30, 30, BLACK);
            tft.fillRect(100, 215, 45, 30, BLACK);
          }
          if (flagAutoFan) {
            if (temp < tempF) {
              botonesFan(false);
            } else {
              botonesFan(true);
            }
          } else {
            botonesFan(fanState);
          }
        }
        break;
      case 3://Luces
        if (flagAuto == false) {
          if (p.x >= 130 and p.x <= 230 and p.y >= 10 and p.y <= 150 and luzState == true) {
            encender(2);
            botonesLuz(luzState);
          }
          if (p.x >= 10 and p.x <= 110 and p.y >= 10 and p.y <= 150 and luzState == false) {
            apagar(2);
            botonesLuz(luzState);
          }
        }
        if (p.x >= 140 and p.x <= 190 and p.y >= 250 and p.y <= 290) {
          flagAuto = !flagAuto;
          botonesLuz(!digitalRead(pir));
        }
        if (p.x >= 10 and p.x <= 230 and p.y >= 180 and p.y <= 220) {
          tft.drawRect(10, 170, 220, 70, GREEN);
          menu();
          opc = 0;
        }
        break;
      case 4://WaterSensor
        if (p.x >= 1 and p.x <= 71 and p.y >= 260 and p.y <= 310) {
          tft.drawRect(7, 275, 60, 30, WHITE);
          menu();
          opc = 0;
          flag = false;
        }
        break;
    }
  }
  presentime = millis();
  if (presentime - previoustime >= TIME) {
    previoustime = presentime;
    //For temp
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
    flag = false;
    //Refresh menu
    menu();
    opc = 0;
  }
  automatico();//Auto
  if (irrecv.decode(&results)) {
    translateIR();
    irrecv.resume(); // receive the next value
  }
  if(flag){
    waterSensor();
  }
  distanciaSensor();
}

void distanciaSensor(){
  distancia = sr04.Distance();
  if(distancia < 30){
      flagAlarm = true;
    }
  if(flagAlarm){
    statusAlarm = ! statusAlarm;
    digitalWrite(pinAlarm,statusAlarm);
  }
  Serial.print(distancia);
  Serial.println("cm");
}
  
void menu() {
  //Refresh the menu
  now = RTC.now();//Variable para RTC
  sprintf(buffer, "%02d/%02d/%04d %02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute());
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);
  //Fill screen
  tft.fillScreen(DARKGREEN);
  //For fan
  tft.drawRect(1, 1, 239, 50, WHITE);
  tft.fillRect(11, 11, 220, 30, WHITE);
  //For heater
  tft.drawRect(1, 1 + 49, 239, 50, WHITE);
  tft.fillRect(11, 11 + 49, 220, 30, WHITE);
  //For light
  tft.drawRect(1, 1 + 49 + 49, 239, 50, WHITE);
  tft.fillRect(11, 11 + 49 + 49, 220, 30, GREY);
  //For temp and time
  tft.fillRect(0, 273, 240, 20, WHITE);
  tft.fillRect(0, 296, 240, 20, RED);
  //For menu
  tft.setCursor(45, 180);
  tft.setTextSize(7);
  tft.setTextColor(YELLOW);
  tft.print("MENU");
  //For fan
  tft.setTextColor(RED);
  tft.setCursor(35, 15);
  tft.setTextSize(3);
  tft.print("Calefactor");
  //For heater
  tft.setTextColor(BLUE);
  tft.setCursor(40, 15 + 49);
  tft.setTextSize(3);
  tft.print("Ventilador");
  //For light
  tft.setTextColor(WHITE);
  tft.setCursor(25, 15 + 49 + 49);
  tft.setTextSize(3);
  tft.print("Iluminacion");
  //For time
  tft.setCursor(25, 275);
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.print(buffer);
  //For temp
  tft.setCursor(90, 300);
  tft.setTextSize(2);
  tft.setTextColor(GREEN);
  tft.print(temp);
  tft.print(" C");
}
void botonesHeater(boolean modo) {
  tft.setCursor(50, 170);
  tft.setTextSize(3);
  tft.setTextColor(YELLOW);
  tft.print("Consigna");

  tft.setCursor(55, 220);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.print("-  ");
  tft.print(tempH);
  tft.print("  +");

  tft.setCursor(10, 280);
  tft.setTextSize(3);
  tft.setTextColor(BLUE);
  tft.drawRect(1, 270, 70, 40, GREEN);
  tft.print("<--");

  tft.setCursor(90, 280);
  tft.setTextSize(3);
  tft.setTextColor(RED);
  tft.print("Auto: ");

  if (flagAutoHeater) {
    tft.fillRect(180, 270, 40, 40, GREEN);
  } else {
    tft.fillRect(180, 270, 40, 40, BLACK);
    tft.drawRect(180, 270, 40, 40, GREEN);
  }

  if (modo == false) {
    tft.fillRect(11, 11, 99, 140, GREY);
    tft.fillRect(130, 11, 99, 140, GREEN);

    tft.setCursor(30, 70);
    tft.setTextSize(3);
    tft.setTextColor(WHITE);
    tft.print("OFF");
    tft.setCursor(165, 70);
    tft.print("ON");
  } else {
    tft.fillRect(11, 11, 99, 140, RED);
    tft.fillRect(130, 11, 99, 140, GREY);

    tft.setCursor(30, 70);
    tft.setTextSize(3);
    tft.setTextColor(WHITE);
    tft.print("OFF");
    tft.setCursor(165, 70);
    tft.print("ON");
  }
}
void botonesFan(boolean modo) {
  tft.setCursor(50, 170);
  tft.setTextSize(3);
  tft.setTextColor(YELLOW);
  tft.print("Consigna");

  tft.setCursor(55, 220);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.print("-  ");
  tft.print(tempF);
  tft.print("  +");

  tft.setCursor(10, 280);
  tft.setTextSize(3);
  tft.setTextColor(BLUE);
  tft.drawRect(1, 270, 70, 40, GREEN);
  tft.print("<--");

  tft.setCursor(90, 280);
  tft.setTextSize(3);
  tft.setTextColor(RED);
  tft.print("Auto: ");

  if (flagAutoFan) {
    tft.fillRect(180, 270, 40, 40, GREEN);
  } else {
    tft.fillRect(180, 270, 40, 40, BLACK);
    tft.drawRect(180, 270, 40, 40, GREEN);
  }

  if (modo == false) {
    tft.fillRect(11, 11, 99, 140, GREY);
    tft.fillRect(130, 11, 99, 140, GREEN);

    tft.setCursor(30, 70);
    tft.setTextSize(3);
    tft.setTextColor(WHITE);
    tft.print("OFF");
    tft.setCursor(165, 70);
    tft.print("ON");
  } else {
    tft.fillRect(11, 11, 99, 140, RED);
    tft.fillRect(130, 11, 99, 140, GREY);

    tft.setCursor(30, 70);
    tft.setTextSize(3);
    tft.setTextColor(WHITE);
    tft.print("OFF");
    tft.setCursor(165, 70);
    tft.print("ON");
  }
}
void botonesLuz(boolean modo) {
  tft.setCursor(20, 180);
  tft.setTextSize(7);
  tft.setTextColor(YELLOW);
  tft.print("LUCES");

  tft.setCursor(50, 280);
  tft.setTextSize(3);
  tft.setTextColor(RED);
  tft.print("Auto: ");
  if (flagAuto) {
    tft.fillRect(140, 270, 40, 40, GREEN);
  } else {
    tft.fillRect(140, 270, 40, 40, BLACK);
    tft.drawRect(140, 270, 40, 40, GREEN);
  }

  if (modo == false) {
    tft.fillRect(11, 11, 99, 140, GREY);
    tft.fillRect(130, 11, 99, 140, GREEN);

    tft.setCursor(30, 70);
    tft.setTextSize(3);
    tft.setTextColor(WHITE);
    tft.print("OFF");
    tft.setCursor(165, 70);
    tft.print("ON");
  } else {
    tft.fillRect(11, 11, 99, 140, RED);
    tft.fillRect(130, 11, 99, 140, GREY);

    tft.setCursor(30, 70);
    tft.setTextSize(3);
    tft.setTextColor(WHITE);
    tft.print("OFF");
    tft.setCursor(165, 70);
    tft.print("ON");
  }
}

void waterSensor() {
  Serial.println(analogRead(water));
  int sensor = map(analogRead(water),0,320,299,11);
  tft.setCursor(10, 280);
  tft.setTextSize(3);
  tft.setTextColor(BLUE);
  tft.drawRect(1, 270, 70, 40, GREEN);
  tft.print("<--");
  //x --> 100 - 230 y --> 10 - 300
  tft.drawRect(100,10,130,290,GREEN);//xMax = 230; yMax = 300
  tft.fillRect(101,sensor,128,299-sensor,BLUE);
  sensor = map(analogRead(water),0,320,11,299);
  tft.fillRect(101,11,128,299-sensor,DARKGREEN);

  tft.setCursor(1,1);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.print("Nivel\nde\nhumedad");
}

void apagar(int modo) {
  switch (modo) {
    case 0:
      digitalWrite(heater, HIGH);
      heaterState = true;
      break;
    case 1:
      digitalWrite(fan, HIGH);
      fanState = true;
      break;
    case 2:
      digitalWrite(luz, HIGH);
      luzState = true;
      break;
  }
}

void encender(int modo) {
  switch (modo) {
    case 0:
      digitalWrite(heater, LOW);
      heaterState = false;
      break;
    case 1:
      digitalWrite(fan, LOW);
      fanState = false;
      break;
    case 2:
      digitalWrite(luz, LOW);
      luzState = false;
      break;
  }
}
void automatico() {
  if (flagAuto) {
    if (digitalRead(pir)) {
      encender(2);
    } else {
      apagar(2);
    }
  }
  if (flagAutoHeater) {
    if (temp < tempH) {
      encender(0);
    } else {
      apagar(0);
    }
  }
  if (flagAutoFan) {
    if (temp > tempF) {
      encender(1);
    } else {
      apagar(1);
    }
  }
}
