/*
 * Racov feb.2021
 -- 20x4 LCD (paralel) display 
 -- clock with big numbers and I2C-DS3231 Real Time clock
 -- temperature and hunidity interior/exterior with I2C-BMP280
 -- rotary encoder for TIME set
 racov.ro

v.1 - Nicu FLORICA (niq_ro) changed t i2c interface for LCD2004
v.1.a - changed sensors to DHT22 
*/

// include the library code:
// #include <LiquidCrystal.h>      //LCD
#include <LiquidCrystal_I2C.h> // used Adafruit library
#include <Wire.h>
#include <Adafruit_Sensor.h>
//#include <Adafruit_BME280.h>    //BME280 temp-hum-pres sensor,
#include <RTClib.h>             //DS3231 Real Time Clock
RTC_DS3231 rtc;                 //declare RTC                
//Adafruit_BME280 bme1;           //declare BME interior
//Adafruit_BME280 bme2;           //declare BME exterior

#include "DHT.h"  // : https://github.com/adafruit/DHT-sensor-library
#define DHTPIN1 5     // Digital pin connected to the DHT sensor
#define DHTPIN2 6     // Digital pin connected to the DHT sensor

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 3 (on the right) of the sensor to GROUND (if your sensor has 3 pins)
// Connect pin 4 (on the right) of the sensor to GROUND and leave the pin 3 EMPTY (if your sensor has 4 pins)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 8, d5 = 7, d6 = 6, d7 = 5, LED = 9;
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
LiquidCrystal_I2C lcd(0x27, 20, 4); // adress can be 0x3F

// usually the rotary encoders three pins have the ground pin in the middle
int encoderPinA = 2;  // encoder right
int encoderPinB = 3;  // encoder left
int clearButton = 4;  // encoder push

// variables for automatic brightness
int light;

// interior environment variables
int intt = 24;
int inth = 45;
//int pres = 760;
// exterior environment variables
int exth = 51;
int extt = 12;

int h,m,s,yr,mt,dt,dy,olds;   // hous, minutes, seconds, year, month, date of the month, day, previous second
//char *DOW[]={"MAR","MIE","JOI","VIN","SAM","DUM","LUN"};  //define day of the week
char *DOW[]={"Dum","Lun","Mar","Mie","Joi","Vin","Sam"};  //define day of the week
char *MTH[]={"Ian","Feb","Mar","apr","Mai","Iun","Iul","Aug","Sep","Oct","Nov","Dec"};  //define month
int EncoderMax;   // variabe for h, m, s, ... max value

int encoderPos = 0;  		    // a counter for the dial
unsigned int lastReportedPos = 1;   // change management
static boolean rotating = false;    // debounce management

// interrupt service routine vars
boolean A_set = false;
boolean B_set = false;

// varialbe to know if we are in time setting mode
boolean set_h = false;     //hours
boolean set_m = false;     //minutes
boolean set_s = false;     //seconds
boolean set_yr = false;    //year
boolean set_mt = false;    //month
boolean set_dt = false;    //date of the month

boolean intdht = true;
boolean extdht = true;

/////////////////////////////////////////////////////

void setup() {
 Serial.begin(9600);
 // set up the LCD's number of columns and rows:
//  lcd.begin(20, 4);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
 // set up the encoder
 pinMode(encoderPinA, INPUT);
 pinMode(encoderPinB, INPUT);
 pinMode(clearButton, INPUT);
 digitalWrite(clearButton, HIGH);  // not connected the encoder yet
 // encoder pin on interrupt 0 (pin 2)
 attachInterrupt(0, doEncoderA, CHANGE);
 // encoder pin on interrupt 1 (pin 3)
 attachInterrupt(1, doEncoderB, CHANGE);

 // *******DEFINE CUSTOM CHARACTERS FOR BIG FONT*****************
  byte A[8] =
  {
  B00011,
  B00111,
  B01111,
  B01111,
  B01111,
  B01111,
  B01111,
  B01111
  };
  byte B[8] =
  {
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111
  };
  byte C[8] =
  {
  B11000,
  B11100,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110
  };
  byte D[8] =
  {
  B01111,
  B01111,
  B01111,
  B01111,
  B01111,
  B01111,
  B00111,
  B00011
  };
  byte E[8] =
  {
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111
  };
  byte F[8] =
  {
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11110,
  B11100,
  B11000
  };
  byte G[8] =
  {
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
  };
  byte H[8] =
  {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
  };
  
  lcd.createChar(8,A);
  lcd.createChar(6,B);
  lcd.createChar(2,C);
  lcd.createChar(3,D);
  lcd.createChar(7,E);
  lcd.createChar(5,F);
  lcd.createChar(1,G);
  lcd.createChar(4,H);
 
  // Print a message to the LCD.
  // lcd.setCursor(col, row);
  // lcd.print("text");
  lcd.setCursor(6, 0);
  lcd.print("WEATHER");
  lcd.setCursor(7, 1);
  lcd.print("CLOCK");
  lcd.setCursor(0, 2);
  lcd.print("**RACOV** **niq_ro**");
  lcd.setCursor(0, 3);
  lcd.print("Feb.2021    Oct.2025");
  // slowly increase display backlighting brightness 
  for (int i = 0; i <= 4; i++) {
    analogWrite(LED, (i*50));
    delay(500);
    }
  delay(1000);
  
  // initializing the modules
  analogWrite(LED, 50);
  lcd.setCursor(0, 0);
  lcd.print("CHECKING MODULES....");
  delay(100);
  
  analogWrite(LED, 100);
  lcd.setCursor(0, 1);
  lcd.print("1.REAL CLOCK......OK");
  if (! rtc.begin()) {
    lcd.setCursor(16, 1);
    lcd.print("FAIL");
    delay(5000);
  }
  delay(500);

  dht1.begin();
  dht2.begin();

  analogWrite(LED, 150);
  lcd.setCursor(0, 2);
  lcd.print("2.INSIDE TEMP.....OK");
/*  
  if (!bme1.begin(0x76)) {
    lcd.setCursor(16, 2);
    lcd.print("FAIL");
    intbme = false;
  }
*/
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  inth = dht1.readHumidity();
  // Read temperature as Celsius (the default)
  intt = dht1.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(inth) || isnan(intt)) {
    Serial.println(F("Failed to read from DHT sensor 1!"));
    lcd.setCursor(16, 2);
    lcd.print("FAIL");
    intdht = false;
  }
  delay(500);
  
  analogWrite(LED, 200);
  lcd.setCursor(0, 3);
  lcd.print("3.OUTSIDE TEMP....OK");

 /* 
  if (!bme2.begin(0x77)) {
    lcd.setCursor(16, 3);
    lcd.print("FAIL");
    extbme = false;
  }
  */
 // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  exth = dht2.readHumidity();
  // Read temperature as Celsius (the default)
  extt = dht2.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(extt) || isnan(extt)) {
    Serial.println(F("Failed to read from DHT sensor 2!"));
    lcd.setCursor(16, 3);
    lcd.print("FAIL");
    extdht = false;
  }
  
  delay(2000);
  
  // print all the fix text - never changes
  lcd.setCursor(0, 0);
  lcd.print(" __ __"+String(char(165))+" __ __"+(char(165))+" __ __");  // char(165) is big point symbol
  lcd.setCursor(0, 1);
  lcd.print("      "+String(char(165))+"      "+(char(165))+"      ");
  lcd.setCursor(0, 2);
  lcd.print("Lun-01-Ian [--"+String(char(223))+" --%]");  // char(223) is degree symbol
  lcd.setCursor(0, 3);
//  lcd.print("  ----hPa   --"+String(char(223))+" --% ");
  lcd.print("              "+String(char(223))+" --% ");

// set a particular date and time if you want (yyyy,mm,dd,hh,mm,ss)
//   rtc.adjust(DateTime(2021, 2, 16, 11, 00, 00));
  
// read all time and date and display
  DateTime now = rtc.now();
  h  = now.hour();
  m  = now.minute();
  s  = now.second();
  yr = now.year();
  mt = now.month();
  dt = now.day();
  dy = now.dayOfTheWeek();
  olds = s;

// fill the display with all the data
  printbig((h%10),3);
  printbig((h/10),0);
  printbig((m%10),10);
  printbig((m/10),7);
  printbig((s%10),17);
  printbig((s/10),14);

  lcd.setCursor(0, 2);
  lcd.print(DOW[dy]);
  lcd.setCursor(4, 2);
  lcd.print(dt/10);
  lcd.print(dt%10);
  lcd.setCursor(7, 2);
  lcd.print(MTH[mt-1]);

}

void loop() {
  DateTime now = rtc.now();
  s  = now.second();                        // read seconds
  if (olds != s) {                          // if seconds changed
/*    Serial.println(dy);
    Serial.println(mt);
    Serial.println(dt);
    Serial.println(yr); */
      printbig((s%10),17);                  // display seconds
      printbig((s/10),14);
      olds = s;
      timeset();              		    // verify if user wants to set time
      if ( s == 0 ) {                       // minutes change
          m  = now.minute();                // read minutes
          printbig((m%10),10);              // display minutes
          printbig((m/10),7);
          if (m == 0) {                     // hours change
            h  = now.hour();                // read hours
            printbig((h%10),3);             // dislay hours
            printbig((h/10),0);
            if (h == 0) {                   // day change
              dt = now.day();               // read day
              dy = now.dayOfTheWeek();
              mt = now.month();             // read month
              yr = now.year();              // read year
              lcd.setCursor(0, 2);          // display date full
              lcd.print(DOW[dy]);
              lcd.setCursor(4, 2);
              lcd.print(dt/10);
              lcd.print(dt%10);
              lcd.setCursor(7, 2);
              lcd.print(MTH[mt-1]);  // niq_ corrected
          }
      }
    }
        // set screen brightness
        light = (analogRead(A6));
        analogWrite(LED, 2+(light/4.2));
                
        // interior temperature/humidity
   /*     if (intbme){   
          intemp = round(bme1.readTemperature() - 1);
          inhum = round(bme1.readHumidity());
          pres = round(bme1.readPressure() / 100.0F);
   */
         if (intdht){
          intt = round(dht1.readTemperature() - 1);
          inth = round(dht1.readHumidity());
          
          lcd.setCursor(11,2);
        if (intt>=0) {
          lcd.print("[");
          if (intt<10) {lcd.print(" ");}
          }
          else {
            if (intt>(-10)) {lcd.print("[");}
          }
        lcd.print(intt);
        lcd.setCursor(15,2);
        if (inth<100) {lcd.print(" ");}
        lcd.print(inth);
        lcd.setCursor(2,3);
  /*
        if (pres < 1000) {lcd.print(" ");
          if (pres < 100) {lcd.print("  ");}
          }
        lcd.print(pres);
  */
        }
      // exterior temperature/humidity
        if (extdht){
            extt = round(dht2.readTemperature() - 1);
            exth = round(dht2.readHumidity());
            lcd.setCursor(11,3);
          if (extt>=0) {
          lcd.print(" ");
          if (extt<10) {lcd.print(" ");}
          }
          else {
            if (extt>(-10)) {lcd.print(" ");}
            }
         lcd.print(extt);
        lcd.setCursor(15,3);
              if (exth<100) {lcd.print(" ");}
              if (exth<10) {lcd.print(" ");}
              lcd.print(exth);
        }
        
    // do other things every second
  }
  delay (100);
}  // end main loop

void timeset() 
{
  // set year
   if (digitalRead(clearButton) == LOW )  {
    delay (10);
    analogWrite(LED, 150);             
    if (digitalRead(clearButton) == LOW )  {  // debounce switch set year
      set_yr = true;                             // set year
      set_mt = false;
      set_dt = false;
      set_h = false;
      set_m = false;
      set_s = false;
      encoderPos = yr;
  // blanks the time munbers
        lcd.setCursor(0, 0);
        lcd.print(" __ __"+String(char(165))+" __ __"+(char(165))+" __ __");  // char(165) is big point symbol
        lcd.setCursor(0, 1);
        lcd.print("      "+String(char(165))+"      "+(char(165))+"      ");
        lcd.setCursor(0, 2);
        lcd.print("Year: ");
      delay (500);
      while (digitalRead(clearButton) == HIGH ) {
        yr = encoderPos;
        lcd.setCursor(6, 2);
        lcd.print(yr);
        rotating = true;  // reset the debouncer
        if (lastReportedPos != encoderPos) {
          lastReportedPos = encoderPos;
        }
        delay (100);                              // debounce again
      }
  //  rtc.adjust(DateTime(yr, mt, dt, h, m, s));    //write the RTC memory (YYYY, M, D, H, M, s)
    analogWrite(LED, 0);                          // blink screen
    delay(200);
    analogWrite(LED, 150);
    }
   }

  // set month
   if (digitalRead(clearButton) == LOW )  {
    delay (10);                               
    if (digitalRead(clearButton) == LOW )  {  // debounce switch set month
      set_yr = false;                             // set month
      set_mt = true;
      set_dt = false;
      set_h = false;
      set_m = false;
      set_s = false;
      lcd.setCursor(0, 2);
      lcd.print("___-__-___");
      encoderPos = mt;
      delay (500);
      while (digitalRead(clearButton) == HIGH ) {
        mt = encoderPos;
        lcd.setCursor(7, 2);
        lcd.print(MTH[mt-1]);
        rotating = true;  // reset the debouncer
        if (lastReportedPos != encoderPos) {
          lastReportedPos = encoderPos;
        }
        delay (100);                              // debounce again
      }
 //   rtc.adjust(DateTime(yr, mt, dt, h, m, s));
    analogWrite(LED, 0);
    delay(200);
    analogWrite(LED, 150);
    }
   }

   // set day of month
   if (digitalRead(clearButton) == LOW )  {
    delay (10);                               
    if (digitalRead(clearButton) == LOW )  {  // debounce switch set day of month
      set_yr = false;                             // set day of month
      set_mt = false;
      set_dt = true;
      set_h = false;
      set_m = false;
      set_s = false;
      encoderPos = dt;        
      lcd.setCursor(0, 2);
      lcd.print("___-__-___");
      delay (500);
      while (digitalRead(clearButton) == HIGH ) {
        dt = encoderPos+1;
        lcd.setCursor(4, 2);
        lcd.print(dt/10);
        lcd.print(dt%10);
        rotating = true;  // reset the debouncer
        if (lastReportedPos != encoderPos) {
          lastReportedPos = encoderPos;
        }
        delay (100);                              // debounce again
      }
 //   rtc.adjust(DateTime(yr, mt, dt, h, m, s));
    analogWrite(LED, 0);
    delay(200);
    // fill the date
    analogWrite(LED, 150);
    lcd.setCursor(0, 2);
    lcd.print(DOW[dy]);
    lcd.setCursor(4, 2);
    lcd.print(dt/10);
    lcd.print(dt%10);
    lcd.setCursor(7, 2);
    lcd.print(MTH[mt-1]);
    }
   }
   
  // set hours
  if (digitalRead(clearButton) == LOW )  {
    delay (10);                               
    if (digitalRead(clearButton) == LOW )  {  // debounce switch set hours
      set_h = true;                             // set hours
      set_m = false;
      set_s = false;
      encoderPos = h;
      delay (500);
      while (digitalRead(clearButton) == HIGH ) {
        h = encoderPos;
        lcd.setCursor(0, 0);
        lcd.print(" __ __"+String(char(165))+" __ __"+(char(165))+" __ __");  // char(165) is big point symbol
        lcd.setCursor(0, 1);
        lcd.print("      "+String(char(165))+"      "+(char(165))+"      ");
         printbig((h/10),0);
         printbig((h%10),3);
        rotating = true;  // reset the debouncer
        if (lastReportedPos != encoderPos) {
          lastReportedPos = encoderPos;
        }
        delay (100);                              // debounce again
      }
  //  rtc.adjust(DateTime(yr, mt, dt, h, m, s));
    analogWrite(LED, 0);
    delay(200);
    analogWrite(LED, 150);
    }
  }

  // set minutes
  if (digitalRead(clearButton) == LOW )  {
    delay (10);                               
    if (digitalRead(clearButton) == LOW )  {  // debounce switch set minutes
      set_h = false;                             // set minutes
      set_m = true;
      set_s = false;
      encoderPos = m;
      delay (500);
      while (digitalRead(clearButton) == HIGH ) {
        m = encoderPos;
        lcd.setCursor(0, 0);
        lcd.print(" __ __"+String(char(165))+" __ __"+(char(165))+" __ __");  // char(165) is big point symbol
        lcd.setCursor(0, 1);
        lcd.print("      "+String(char(165))+"      "+(char(165))+"      ");
        printbig(m/10,7);
        printbig(m%10,10);
        rotating = true;  // reset the debouncer
        if (lastReportedPos != encoderPos) {
          lastReportedPos = encoderPos;
        }
        delay (100);                              // debounce again
      }
  //  rtc.adjust(DateTime(yr, mt, dt, h, m, s));
    analogWrite(LED, 0);
    delay(200);
    analogWrite(LED, 150);
    }
  }
  
  // set seconds
  if (digitalRead(clearButton) == LOW )  {
    delay (10);                               
    if (digitalRead(clearButton) == LOW )  {  // debounce switch set seconds
      set_h = false;                             // set seconds
      set_m = false;
      set_s = true;
      encoderPos = s;
      delay (500);
      while (digitalRead(clearButton) == HIGH ) {
        s = encoderPos;
        lcd.setCursor(0, 0);
        lcd.print(" __ __"+String(char(165))+" __ __"+(char(165))+" __ __");  // char(165) is big point symbol
        lcd.setCursor(0, 1);
        lcd.print("      "+String(char(165))+"      "+(char(165))+"      ");
        printbig(s/10,14);
        printbig(s%10,17);
        rotating = true;  // reset the debouncer
        if (lastReportedPos != encoderPos) {
          lastReportedPos = encoderPos;
        }
        delay (100);                              // debounce again
      }
    rtc.adjust(DateTime(yr, mt, dt, h, m, s));
    analogWrite(LED, 0);
    delay(200);
    analogWrite(LED, 150);
    }
  }
  
  //fill in the clock numbers
  printbig((h%10),3);
  printbig((h/10),0);
  printbig((m%10),10);
  printbig((m/10),7);
  printbig((s%10),17);
  printbig((s/10),14);

  // fill in the date
  lcd.setCursor(0, 2);
  lcd.print(DOW[dy]);
  lcd.setCursor(4, 2);
  lcd.print(dt/10);
  lcd.print(dt%10);
  lcd.setCursor(7, 2);
  lcd.print(MTH[mt-1]);

  DateTime now = rtc.now();   // read again the time
}

// Interrupt on A changing state
void doEncoderA() {
  // debounce
  if (set_h) EncoderMax = 24;
    else if (set_m) EncoderMax = 60;
    else if (set_s) EncoderMax = 60;
    else if (set_yr) EncoderMax = 3000;
    else if (set_mt) EncoderMax = 12;
    else if (set_dt) EncoderMax = 31;
  if ( rotating ) delay (1);  // wait a little until the bouncing is done
  // Test transition, did things really change?
  if ( digitalRead(encoderPinA) != A_set ) { // debounce once more
    A_set = !A_set;
    // adjust counter + if A leads B
    if ( A_set && !B_set )
      encoderPos += 1;
      encoderPos = (EncoderMax+encoderPos) % EncoderMax;
    rotating = false;  // no more debouncing until loop() hits again
  }
}

// Interrupt on B changing state, same as A above
void doEncoderB() {
  if (set_h) EncoderMax = 24;
    else if (set_m) EncoderMax = 60;
    else if (set_s) EncoderMax = 60;
    else if (set_yr) EncoderMax = 3000;
    else if (set_mt) EncoderMax = 12;
    else if (set_dt) EncoderMax = 31;
  if ( rotating ) delay (1);
  if ( digitalRead(encoderPinB) != B_set ) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if ( B_set && !A_set )
      encoderPos -= 1;
      encoderPos = (EncoderMax+encoderPos) % EncoderMax;
    rotating = false;
  }
}


void printbig(int i, int x)
{
  //  prints each segment of the big numbers

  if (i == 0) {
      lcd.setCursor(x,0);
      lcd.write(8);  
      lcd.write(1); 
      lcd.write(2);
      lcd.setCursor(x, 1);
      lcd.write(3);  
      lcd.write(4);  
      lcd.write(5);
  }
    else if  (i == 1) {
      lcd.setCursor(x,0);
      lcd.write(8);
      lcd.write(255);
      lcd.print(" ");
      lcd.setCursor(x, 1);
      lcd.print(" ");
      lcd.write(255);
      lcd.print(" ");
    }

    else if  (i == 2) {
      lcd.setCursor(x,0);
      lcd.write(1);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(4);
    }

    else if (i == 3) {
      lcd.setCursor(x,0);
      lcd.write(1);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, 1);
      lcd.write(4);
      lcd.write(7);
      lcd.write(5); 
    }

    else if (i == 4) {
      lcd.setCursor(x,0);
      lcd.write(3);
      lcd.write(4);
      lcd.write(2);
      lcd.setCursor(x, 1);
      lcd.print("  ");
      lcd.write(5);
    }

    else if (i == 5) {
      lcd.setCursor(x,0);
      lcd.write(255);
      lcd.write(6);
      lcd.write(1);
      lcd.setCursor(x, 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5);
    }

    else if (i == 6) {
      lcd.setCursor(x,0);
      lcd.write(8);
      lcd.write(6);
      lcd.print(" ");
      lcd.setCursor(x, 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
    }

    else if (i == 7) {
      lcd.setCursor(x,0);
      lcd.write(1);
      lcd.write(1);
      lcd.write(5);
      lcd.setCursor(x, 1);
      lcd.print(" ");
      lcd.write(8);
      lcd.print(" ");
    }
    
    else if (i == 8) {
      lcd.setCursor(x,0);
      lcd.write(8);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
    }

    else if (i == 9) {
      lcd.setCursor(x,0);
      lcd.write(8);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, 1);
      lcd.print(" ");
      lcd.write(4);
      lcd.write(5);
    }

  } 
