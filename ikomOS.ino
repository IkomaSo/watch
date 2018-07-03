#include <Wire.h>
#include <SPI.h>
#include <string.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <avr/sleep.h>
#include "RTClib.h"
#include "num.cpp"

#define OLED_DC     A3
#define OLED_CS     A5
#define OLED_RESET  A4
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10
Adafruit_BMP280 bme;

#define SW1  10
#define SW2  8
#define SW3  11

int S1 = 0;
int S2 = 0;
int S3 = 0;
uint8_t watchMode=0;
uint8_t hour,day,min,sec;
bool color = true;

static byte bPin = A11;
static byte battEn = 4;

// Voltage divider resistor values.
float R1 = 10000;
float R2 = 10000;

float vDivider;
float voltage;

RTC_DS3231 rtc;
char daysOfTheWeek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char daysOfTheMonth[12][10] = {"January","February","March","April","May","June","July","August","September","October","November","December"};
char Mode[3][7] = {"watch","adjust","status"};
uint8_t mn = 3;

void menu();

void watch();
void Adjust();
void Status();



ISR(PCINT0_vect) {
  sleep_disable();
}


void setup() {

  Serial.begin(9600);

  vDivider = (R2 / (R1 + R2));

  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  
  //setup for oled
  display.begin(SSD1306_SWITCHCAPVCC);
  display.fillRect(0, 0, display.width(), display.height(), 0);
  display.setCursor(20,25);
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.print("Hello");
  display.display();
  delay(2000);
  display.clearDisplay();

  display.fillRect(0, 0, display.width(), display.height(), 0);
  display.display();

  Serial.print("Display was prepared.\n");

  //time adjustment
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  PCICR = (1<<0);
  PCMSK0 = (1 <<4);

  bme.begin();

  //set sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

}

void loop() {
  sleep_enable();
  sleep_cpu();
  Serial.print("Wake up!");
  watch();
}

void watch() {
  hour = -1;
  day = -1;
  min = -1;
  for(uint8_t i = 0;i < 15 ; i++){
    S1 = digitalRead(SW1);
    S2 = digitalRead(SW2);
    S3 = digitalRead(SW3);
    DateTime now = rtc.now();
    if(day != now.day()){
      day = now.day();
      Serial.print("It is ");
      display.fillRect(0, 0, display.width(), 10, 0);
      char days[32];
      char com[] = ",";
      char sp[] = " ";
      char d[3];
      char y[5];
      snprintf(d, 3, "%d", now.day());
      snprintf(y, 5, "%d", now.year());
      strcpy(days, daysOfTheWeek[now.dayOfTheWeek()]);
      //strcat(days,com);
      strcat(days,sp);
      strcat(days,daysOfTheMonth[now.month()%12-1]);
      strcat(days,sp);
      strcat(days,d);
      //strcat(days,com);
      strcat(days,sp);
      strcat(days,y);
      Serial.print(days);
      Serial.print("\n");
      display.setCursor((display.width()-strlen(days)*6)/2,0);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.println(days);
    }
  
     if(hour != now.hour()){
      hour = now.hour();
      display.fillRect(0, 20, 44, 36, 0);
      if(now.hour() >= 10)display.drawBitmap(0, 20, L_num_bmp[now.hour()/10], 16, 24, WHITE);
      display.drawBitmap(16, 20, L_num_bmp[now.hour()%10], 16, 24, WHITE);
    }
  
     if(min != now.minute()){
       min = now.minute();
       display.fillRect(50, 20, 94, 36, 0);
       display.drawBitmap(50, 20, L_num_bmp[now.minute()/10], 16, 24, WHITE);
       display.drawBitmap(66, 20, L_num_bmp[now.minute()%10], 16, 24, WHITE);
     }
  
    display.fillRect(94, 32, 32, 24, 0); 
    display.drawBitmap(94, 32, num_bmp[now.second()/10], 8, 12, WHITE);
    display.drawBitmap(102, 32, num_bmp[now.second()%10], 8, 12, WHITE);
  
  
    digitalWrite(battEn, HIGH);
    voltage = analogRead(bPin);
    voltage = (voltage / 1024) * 3.35;
    voltage = voltage / vDivider;
    digitalWrite(battEn, LOW);
    display.fillRect(0, 55, display.width(), 10, 0);
    display.setCursor(0,55);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.print("Voltage: ");
    display.print(voltage);
    display.print("V");
    
    display.fillRect(42, 26, 4, 4, color);
    display.fillRect(42, 34, 4, 4, color);
    color = !color;
    display.display();
    delay(500);
    display.fillRect(42, 26, 4, 4, color);
    display.fillRect(42, 34, 4, 4, color);
    color = !color;
    display.display();
    delay(500);

    S1 = digitalRead(SW1);
    S2 = digitalRead(SW2);
    S3 = digitalRead(SW3);

    if(S2 == LOW) {
      display.clearDisplay();
      display.display();
      delay(500);
      menu();
    }
  }
  
  display.clearDisplay();
  display.display();

  return;
}

void menu(){
  hour = -1;
  day = -1;
  min = -1;
  display.setCursor(0,0);
  display.print("Ikoma@watchX \n~$");
  display.display();
  while(1){
    S1 = digitalRead(SW1);
    S2 = digitalRead(SW2);
    S3 = digitalRead(SW3);
    if(S3 == LOW && watchMode != mn-1){
      watchMode++;
      display.clearDisplay();
      display.setCursor(0,0);
      display.print("Ikoma@watchX \n~$");
      display.display();
      delay(100);
      for(uint8_t i=0;i < sizeof(Mode[watchMode]);i++){
         display.print(Mode[watchMode][i]);
         display.display();
         delay(50);
      }
    }else if(S3 == LOW && watchMode == mn-1 ){
      watchMode = 0;
      display.clearDisplay();
      display.setCursor(0,0);
      display.print("Ikoma@watchX \n~$");
      display.display();
      delay(100);
      for(uint8_t i=0;i < sizeof(Mode[watchMode]);i++){
         display.print(Mode[watchMode][i]);
         display.display();
         delay(50);
      }
    }else if(S1 == LOW && watchMode != 0){
      watchMode--;
      display.clearDisplay();
      display.setCursor(0,0);
      display.print("Ikoma@watchX \n~$");
      display.display();
      delay(100);
      for(uint8_t i=0;i < sizeof(Mode[watchMode]);i++){
         display.print(Mode[watchMode][i]);
         display.display();
         delay(50);
      }
    }else if(S1 == LOW && watchMode == 0 ){
      watchMode = mn-1;
      display.clearDisplay();
      display.setCursor(0,0);
      display.print("Ikoma@watchX \n~$");
      display.display();
      delay(100);
      for(uint8_t i=0;i < sizeof(Mode[watchMode]);i++){
         display.print(Mode[watchMode][i]);
         display.display();
         delay(50);
      }
    }else if(S2 == LOW){
      display.setCursor(0,20);
      display.print("calling ");
      display.print(Mode[watchMode]);
      display.print("...");
      display.display();
      delay(1000);
      display.clearDisplay();
      display.display();
      if(watchMode == 0){return;}
      else if(watchMode == 1){
         DateTime now = rtc.now();
         hour = now.hour();
         day = now.day();
         min = now.minute();
         sec = 0;
         Adjust();
         return;
       }
      else if(watchMode == 2){Status();return;}
    }
    delay(500);
  }
}

void Status(){
  while(1){
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("<Status>-------------");
    S2 = digitalRead(SW2);
    if(S2 == LOW) return;
    display.print("Battery Voltage:\n               ");
    digitalWrite(battEn, HIGH);
    voltage = analogRead(bPin);
    voltage = (voltage / 1024) * 3.35;
    voltage = voltage / vDivider;
    digitalWrite(battEn, LOW);
    display.print(voltage);
    display.println(" V");
    display.print("Temperature: ");
    display.print(bme.readTemperature());
    display.println(" *C");
    display.print("Pressure: ");
    display.print(bme.readPressure());
    display.println(" Pa");
    display.print("Approx altitude:\n             ");
    display.print(bme.readAltitude(1013.25)); 
    display.println(" m");
    display.display();
    delay(500);
  }
  
}

void Adjust(){
  
  display.fillRect(0, 0, display.width(), display.height(), 0);
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("<Time adjustment>----");
  display.display();

  delay(1000);
  
  S2 = digitalRead(SW2);

  DateTime now = rtc.now();

  while(S2 == HIGH){
    S1 = digitalRead(SW1);
    S2 = digitalRead(SW2);
    S3 = digitalRead(SW3);
    if(S3 == LOW && min != 59) min++;
    else if(S1 == LOW && min != 0) min--;
    else if(S3 == LOW && min == 59 && hour != 24) {min = 0;hour++;}
    else if(S1 == LOW && min == 0 && hour != 0) {min = 59;hour--;}
    if(S2 == LOW) break;
    display.fillRect(0, 20, 44, 36, 0);
    if(hour >= 10)display.drawBitmap(0, 20, L_num_bmp[hour/10], 16, 24, WHITE);
    display.drawBitmap(16, 20, L_num_bmp[hour%10], 16, 24, WHITE);
    display.fillRect(50, 20, 94, 36, 0);
    display.drawBitmap(50, 20, L_num_bmp[min/10], 16, 24, WHITE);
    display.drawBitmap(66, 20, L_num_bmp[min%10], 16, 24, WHITE);
    display.fillRect(94, 32, 32, 24, 0); 
    display.drawBitmap(94, 32, num_bmp[sec/10], 8, 12, WHITE);
    display.drawBitmap(102, 32, num_bmp[sec%10], 8, 12, WHITE);
    display.fillRect(42, 26, 4, 4, 1);
    display.fillRect(42, 34, 4, 4, 1);
    display.display();
    delay(500);
  }
  rtc.adjust(DateTime(now.year(),now.month(),now.day(),hour,min,sec));
  display.clearDisplay();
  display.display();
  hour = -1;
  day = -1;
  min = -1;
  return;
}
