/*
 * Racov feb.2021
 -- 20x4 LCD (paralel) display 
 -- clock with big numbers and I2C-DS3231 Real Time clock
 -- temperature and hunidity interior/exterior with I2C-BMP280
 -- rotary encoder for TIME set
 racov.ro

v.1 - Nicu FLORICA (niq_ro) changed t i2c interface for LCD2004
v.1.a - changed sensors to DHT22 
v.2 - added Doz (A.G.Doswell) mode to change the time with encoder: http://andydoz.blogspot.ro/2014_08_01_archive.html
*/

// Use: Pressing and holding the button will enter the clock set mode (on release of the button). Clock is set using the rotary encoder. 
// The clock must be set to UTC.
// Pressing and releasing the button quickly will display info about author (me)

#include <Wire.h>
#include "RTClib.h" // from https://github.com/adafruit/RTClib
//#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h> // used Adafruit library
#include <Wire.h>
#include <Encoder.h> // from http://www.pjrc.com/teensy/td_libs_Encoder.html

#define sw1 4
// usually the rotary encoders three pins have the ground pin in the middle
int PinA = 2;  // encoder right
int PinB = 3;  // encoder left

RTC_DS1307 RTC; // Tells the RTC library that we're using a DS1307 RTC
Encoder knob(PinB, PinA); //encoder connected to pins 2 and 3 (and ground)
//LiquidCrystal lcd(12, 13, 7, 6, 5, 4); // I used an odd pin combination because I need pin 2 and 3 for the interrupts.
LiquidCrystal_I2C lcd(0x27, 20, 4); // adress can be 0x3F

//the variables provide the holding values for the set clock routine
int setyeartemp; 
int setmonthtemp;
int setdaytemp;
int sethourstemp;
int setminstemp;
int setsecs = 0;
int maxday; // maximum number of days in the given month
int TimeMins; // number of seconds since midnight
int TimerMode = 2; //mode 0=Off 1=On 2=Auto
int TimeOut = 10;
int TimeOutCounter;

// These variables are for the push button routine
int buttonstate = 0; //flag to see if the button has been pressed, used internal on the subroutine only
int pushlengthset = 3000; // value for a long push in mS
int pushlength = pushlengthset; // set default pushlength
int pushstart = 0;// sets default push value for the button going low
int pushstop = 0;// sets the default value for when the button goes back high

int knobval; // value for the rotation of the knob
boolean buttonflag = false; // default value for the button flag


//const int TIMEZONE = 0; //UTC
const int TIMEZONE = 2; //UTC Craiova (Romania) - http://www.worldtimebuddy.com/utc-to-romania-craiova

#include "DHT.h"  // : https://github.com/adafruit/DHT-sensor-library
#define DHTPIN1 5     // Digital pin connected to the DHT sensor
#define DHTPIN2 6     // Digital pin connected to the DHT sensor
#define LED 9

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
//const int rs = 12, en = 11, d4 = 8, d5 = 7, d6 = 6, d7 = 5, LED = 9;
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//LiquidCrystal_I2C lcd(0x27, 20, 4); // adress can be 0x3F


// variables for automatic brightness
int light;

// interior environment variables
int intt = 24;
int inth = 45;
//int pres = 760;
// exterior environment variables
int exth = 51;
int extt = 12;

boolean intdht = true;
boolean extdht = true;

int h,m,s,yr,mt,dt,dy,olds;   // hous, minutes, seconds, year, month, date of the month, day, previous second
//char *DOW[]={"MAR","MIE","JOI","VIN","SAM","DUM","LUN"};  //define day of the week
//char *DOW[]={"Dum","Lun","Mar","Mie","Joi","Vin","Sam"};  //define day of the week
char *DOW[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};  //define day of the week
//char *MTH[]={"Ian","Feb","Mar","Apr","Mai","Iun","Iul","Aug","Sep","Oct","Noi","Dec"};  //define month
char *MTH[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};  //define month

unsigned long tpcitit = 30000;
unsigned long tpcitire;


  
void setup () {
    //Serial.begin(57600); //start debug serial interface
    Wire.begin(); //start I2C interface
    //RTC.begin(); //start RTC interface
  //dht.begin();  // DHT init 
  
  //  lcd.begin(16,2); //Start LCD (defined as 16 x 2 characters)
    lcd.begin();
 //   lcd.createChar(0, grad);  // create custom symbol
  lcd.backlight();
    lcd.clear(); 
    pinMode(sw1,INPUT);//push button on encoder connected to A0 (and GND)
    digitalWrite(sw1,HIGH); //Pull A0 high
    pinMode(A3,OUTPUT); //Relay connected to A3
    digitalWrite (A3, HIGH); //sets relay off (default condition)

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
  if (! RTC.begin()) {
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

  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  inth = dht1.readHumidity();
  delay(500);
  // Read temperature as Celsius (the default)
  intt = dht1.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(inth) || isnan(intt)) 
   {
    Serial.println(F("Failed to read from DHT sensor 1!"));
    lcd.setCursor(16, 2);
    lcd.print("FAIL");
    intdht = false;
   }

  delay(500);
  
  analogWrite(LED, 200);
  lcd.setCursor(0, 3);
  lcd.print("3.OUTSIDE TEMP....OK");

 // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  exth = dht2.readHumidity();
  delay(500);
  // Read temperature as Celsius (the default)
  extt = dht2.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(extt) || isnan(extt)) 
  {
    Serial.println(F("Failed to read from DHT sensor 2!"));
    lcd.setCursor(16, 3);
    lcd.print("FAIL");
    extdht = false;
  }
  
  delay(2000);
    
    //Checks to see if the RTC is runnning, and if not, sets the time to the time this sketch was compiled.
    if (! RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
lcd.clear();
promo();
tpcitire = tpcitit + millis();
}
           

void loop () {

    pushlength = pushlengthset;
    pushlength = getpushlength ();
    delay (10);
    
    if (pushlength <pushlengthset) 
    {
      ShortPush ();   
      promo();
    }  
       //This runs the setclock routine if the knob is pushed for a long time
       if (pushlength >pushlengthset) 
       {
         lcd.clear();
         DateTime now = RTC.now();
         setyeartemp=now.year(),DEC;
         setmonthtemp=now.month(),DEC;
         setdaytemp=now.day(),DEC;
         sethourstemp=now.hour(),DEC;
         setminstemp=now.minute(),DEC;
         setclock();
         pushlength = pushlengthset;
       };

  DateTime now = RTC.now();
  s  = now.second();                        // read seconds
  if (olds != s) {                          // if seconds changed
/*    Serial.println(dy);
    Serial.println(mt);
    Serial.println(dt);
    Serial.println(yr); */
      printbig((s%10),17);                  // display seconds
      printbig((s/10),14);
      olds = s;
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

    if (millis() - tpcitire > tpcitit)
     {
        // set screen brightness
        light = (analogRead(A6));
        analogWrite(LED, 2+(light/4.2));
                
        // interior temperature/humidity
         if (intdht){
          intt = round(dht1.readTemperature() - 1);
           delay(500);
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
        }
       delay(100); 
      // exterior temperature/humidity
        if (extdht){
            extt = round(dht2.readTemperature() - 1);
             delay(500);
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
     tpcitire = millis();
     } 
    // do other things every second
  }


}  // end main loop

//sets the clock
void setclock (){
   setyear ();
   lcd.clear ();
   setmonth ();
   lcd.clear ();
   setday ();
   lcd.clear ();
   sethours ();
   lcd.clear ();
   setmins ();
   lcd.clear();
   
   RTC.adjust(DateTime(setyeartemp,setmonthtemp,setdaytemp,sethourstemp,setminstemp,setsecs));
   delay (1000);
   promo();
}

// subroutine to return the length of the button push.
int getpushlength () {
  buttonstate = digitalRead(sw1);  
       if(buttonstate == LOW && buttonflag==false) {     
              pushstart = millis();
              buttonflag = true;
          };
          
       if (buttonstate == HIGH && buttonflag==true) {
         pushstop = millis ();
         pushlength = pushstop - pushstart;
         buttonflag = false;
       };
       return pushlength;
}
// The following subroutines set the individual clock parameters
int setyear () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Year");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setyeartemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setyeartemp=setyeartemp + knobval;
    if (setyeartemp < 2014) { //Year can't be older than currently, it's not a time machine.
      setyeartemp = 2014;
    }
    lcd.print (setyeartemp);
    lcd.print("  "); 
    setyear();
}
  
int setmonth () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Set Month");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setmonthtemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setmonthtemp=setmonthtemp + knobval;
    if (setmonthtemp < 1) {// month must be between 1 and 12
      setmonthtemp = 1;
    }
    if (setmonthtemp > 12) {
      setmonthtemp=12;
    }
    lcd.print (setmonthtemp);
    lcd.print("  "); 
    setmonth();
}

int setday () {
  if (setmonthtemp == 4 || setmonthtemp == 5 || setmonthtemp == 9 || setmonthtemp == 11) { //30 days hath September, April June and November
    maxday = 30;
  }
  else {
  maxday = 31; //... all the others have 31
  }
  if (setmonthtemp ==2 && setyeartemp % 4 ==0) { //... Except February alone, and that has 28 days clear, and 29 in a leap year.
    maxday = 29;
  }
  if (setmonthtemp ==2 && setyeartemp % 4 !=0) {
    maxday = 28;
  }
//lcd.clear();  
   lcd.setCursor (0,0);
    lcd.print ("Set Day");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setdaytemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setdaytemp=setdaytemp+ knobval;
    if (setdaytemp < 1) {
      setdaytemp = 1;
    }
    if (setdaytemp > maxday) {
      setdaytemp = maxday;
    }
    lcd.print (setdaytemp);
    lcd.print("  "); 
    setday();
}

int sethours () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Hours");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return sethourstemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    sethourstemp=sethourstemp + knobval;
    if (sethourstemp < 1) {
      sethourstemp = 1;
    }
    if (sethourstemp > 23) {
      sethourstemp=23;
    }
    lcd.print (sethourstemp);
    lcd.print("  "); 
    sethours();
}

int setmins () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Set Mins");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setminstemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setminstemp=setminstemp + knobval;
    if (setminstemp < 0) {
      setminstemp = 0;
    }
    if (setminstemp > 59) {
      setminstemp=59;
    }
    lcd.print (setminstemp);
    lcd.print("  "); 
    setmins();
}

int setmode () { //Sets the mode of the timer. Auto, On or Off
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Mode");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return TimerMode;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    TimerMode=TimerMode + knobval;
    if (TimerMode < 0) {
      TimerMode = 0;
    }
    if (TimerMode > 2) {
      TimerMode=2;
    }
    if (TimerMode == 0) {
    lcd.print("Off (!)");
    lcd.print("  "); 
    }
    if (TimerMode == 1) {
    lcd.print("On (*)");
    lcd.print("  "); 
    }
    if (TimerMode == 2) {
    lcd.print("Auto");
    lcd.print("  "); 
    }
    setmode ();
}


void ShortPush () {
  //This displays the calculated sunrise and sunset times when the knob is pushed for a short time.
lcd.clear();
for (long Counter = 0; Counter < 100 ; Counter ++) { //returns to the main loop if it's been run 604 times 
                                                     //(don't ask me why I've set 604,it seemed like a good number)
  lcd.setCursor (0,0);
  lcd.print ("Clock by niq_ro");

  //If the knob is pushed again, enter the mode set menu
  pushlength = pushlengthset;
  pushlength = getpushlength ();
  if (pushlength != pushlengthset) {
    lcd.clear ();
  } 
}
lcd.clear();
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

void promo()
{
   // print all the fix text - never changes
  lcd.setCursor(0, 0);
  lcd.print(" __ __"+String(char(165))+" __ __"+(char(165))+" __ __");  // char(165) is big point symbol
  lcd.setCursor(0, 1);
  lcd.print("      "+String(char(165))+"      "+(char(165))+"      ");
  lcd.setCursor(0, 2);
  lcd.print("Lun,01-Ian [--"+String(char(223))+" --%]");  // char(223) is degree symbol
  lcd.setCursor(0, 3);
//  lcd.print("  ----hPa   --"+String(char(223))+" --% ");
  lcd.print("              "+String(char(223))+" --% ");

// read all time and date and display
  DateTime now = RTC.now();
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
