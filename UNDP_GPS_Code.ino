///////////////////////////////////////////////////////////////////////////////////////
//////////////////////// UNDP GPS MODULE LOCAL CODE ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//  DATE CREATED  : 02 SEPTEMBER 2019
//  VERSION       : 12
//  PROGRAMMER    : MIRAN DABARE
///////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////   NOTES   /////////////////////////////////////////////
// * THIS CODE ONLY SUPPORTS NEO 6 GPS MODULE, THE DATA WILL BE LOGGED LOCALLY TO AN SD CARD
// * NO GPRS SUPPORT
// * SD CARD FILE NAME IS LIMITED TO 8 CHARACTERS
// * GPS BAUD RATE MUST BE 9600
// * EEPROM CAN ONLY HOLD A VALUE FROM 0 TO 255 IN ONE BYTE ADDRESS, AND ONLY HAS A LOFETIME OF 100,000 READS AND WRITES
// * LOW MEMORY WILL CAUSE ISSUES WITH THE SD CARD, MAKE SURE DYNAMIC MEMORY WONT GO OVER 80%



///////////////////////////// ASSIGNED ARDUINO PIN NUMBERS ////////////////////////////

/*~~~~~~~~ LEDS ~~~~~~~~
 * Red LED    - 5 // BLINK - RECORDING, SOLID - SD CARD ERROR
 * Green LED  - 6 // BLINK -  ON BATTARY, SOLID - POWER ON
 * Blue LED   - 7 // BLINK - GPS SEARCHING, SOLID - GPS LOCKED
 * ~~~~~ GPS MODULE ~~~~~~~
 * RX Pin     - 0
 * TX Pin     - 1
 * ~~~~~ SD CARD MODULE ~~~~~~
 * CS Pin     - 10
 * MOSI       - 11
 * MISO       - 12
 * SCLK       - 13
 * ~~~~ REC BUTTON ~~~
 * Button     - 2 // CONNECT THIS
 */



//////////////////////////////////// HEADERS ////////////////////////////////

#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>


////////////////////////// SD CARD FILES //////////////////////////////
/* This section initialize the file where the data is to be saved.
*/
 
File myFile;
char filename[16] = {'/0'};
int pinCS = 4; // Pin 10 on Arduino Uno / 4 ON NANO

//////////////////////// LED Lights /////////////////////////////////////
/* This section initialize the pins for the front 3 lights.
*/

int REDLED=5;
int GREENLED=6;
int BLUELED=7;

//////////////// LOG FILE NAMING /////////////
/* This section initialize the variable names to store the time and date to be saved later.
*/
uint8_t logtimeH=0;
uint8_t logtimeM=0;
uint8_t logtimeS=0;
uint8_t logdateY=0;
uint8_t logdateM=0;
uint8_t logdateD=0;

int logyear = 0;


///////////////////////////// VARIABLE INITIALIZATION /////////////////////////////////////

static const int RXPin = 0, TXPin = 1;
static const uint32_t GPSBaud = 9600; //BAUD RATE MUST BE 9600

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);


/////////////////////////// RANDOM //////////////////////////////
unsigned long time_1;
int looptime = 0;

float time_new =0;
float time_prev=0;

////////////////////////////// DATA BUTTON FILES ////////////////////////////////
int button=2;
int button_val=0;
int counter=0;

int HeaderCheck = 0;

/////////////////// GPS DISTANCE CALCULATION /////////////////////
/* This section initialize the variable used to calculate the distance from teh GPS coordinated.
*/
float dlong = 0.0;
float dlat = 0.0;

float lon1 =0.0;
float lon2 =0.0;

float lat1 =0.0;
float lat2 =0.0;

float totaldistance = 0.0;
float distanceR =0.0;

/////////////////////////////////////////// SETUP ////////////////////////////////////////////////

void setup() {

//////////////////////////////////// RANDOM STUFF //////////////////////////////////////
 
 pinMode(pinCS, OUTPUT);
 pinMode(button, INPUT);

//////////////////////////////////// GPS WRITING CODE//////////////////////////////////////
 
 Serial.begin(9600);
 ss.begin(GPSBaud);


////////////////////////////////////// HEADER PRINTS ////////////////////////////////////

Serial.println(F("Recording Time (s),Date(DDMMYYYY), Time (HHMMSSCC), Lat (deg), Long (deg), Lat (RAW), Long (RAW), Speed (km/s), Raw Course(deg),Course (deg), Altitude (m), Locked Satellites, Hori Dim Precision "));




 /////////////////////////////////////////// LEDS ///////////////////////////////////////////////////

 pinMode(REDLED, OUTPUT);
 pinMode(GREENLED, OUTPUT);
 pinMode(BLUELED, OUTPUT);
  
  //////////////////////// SD CARD INITIALIZATION ////////////////////////////////////////////
  /* Standard code to initialize the SD card module
*/
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
    digitalWrite(REDLED, LOW);
  } else
  {
    Serial.println("SD card initialization failed");
    digitalWrite(REDLED, HIGH);
    return;
  }

Serial.println("Setup Completed");


////////////////////////// POWER ON LED /////////////////////////

digitalWrite(GREENLED,HIGH);

}


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// LOOP ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////



void loop()
{
 

  button_val=digitalRead(button); // CHECKS THE RECORD BUTTON (DEFAULT ITS PINNED TO HIGH IN THE BOARD, REMOVE AND FIX IT TO A SWITCH IF NEEDED
  

  while (ss.available() > 0)
    gps.encode(ss.read());

    
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// DATA ACQUISITION /////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

// CHECKS THE TIME AND DATE TO WRITE TO THE FILE NAME, THE DATA ACQ WILL NOT START UNTIL THIS IS OBTAINED BY GPS
 logtimeH  = gps.time.hour();
 logtimeM  = gps.time.minute();
 logtimeS  = gps.time.second();
   
 logdateM = gps.date.month();
 logdateD = gps.date.day();
  
/* This if condition checks whether the record button is pressed as well as to start the recording only once a good GPS fix is obtained (ie: when the time is fixed to 2020)
 *  
 */
if (button_val == HIGH && gps.date.year()>2019 )
{
    while (ss.available() > 0)
      gps.encode(ss.read());
      

    sprintf(filename, "%u%u_%u%u.csv", logdateM,logdateD,logtimeH,logtimeM); // WRITE THE DATE AND TIME TO THE LOG FILENAME
    
    Serial.print(filename);
    
  logyear = gps.date.year(); // THIS PREVENTS THE ACQUISITION LOOP FROM EXITING IF GPS IS LOST DURING THE RECORDING
 
  myFile = SD.open(filename, FILE_WRITE);

 ////////////////////////////////////////// DATA LOG HEADERS ///////////////////////
/* This writes the first header line of the file in the Sd card 
 *  
 */
  if (myFile) {   

 Serial.println("Data Headers Writing");
    
  myFile.println("Recording Time (s),Date(DDMMYYYY), Time (HHMMSSCC), Lat (deg), Long (deg), Distance Between 2 Points (m), Distance Travelled (m), Speed (km/s), Raw Course(deg),Course (deg), Altitude (m), Locked Satellites, Hori Dim Precision (m), Fix Age (ms)");

   myFile.close(); // close the file
  }
    HeaderCheck=1; // THIS MAKES SURE THE HEADERS ARE WRITTEN BEFORE GOING TO THE MAIN WHILE LOOP.
}
  while (ss.available() > 0)
   gps.encode(ss.read());


 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////// MAIN RECORDING LOOP ///////////////////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   /* The main GPS cordinate recordings take place only if the record button is pressed, a good GPS fix is obtained, and the previous header lines have been printed.
    *  
    */
   
while (button_val == HIGH &&  logyear>2018 && HeaderCheck ==1)
{

  while (ss.available() > 0)
 gps.encode(ss.read());
  
///////////////////////////// GPS LOCK INDICATORS ////////////////////////////
/* This section controls the Blue GPS lock LED light blinking sequence. Check the instruction manual.
 *  
 */
if (gps.satellites.value() > 3)
{

digitalWrite(BLUELED,HIGH);
}

 else {
 digitalWrite(BLUELED,HIGH);
 delay(100);
 digitalWrite(BLUELED,LOW); 
 delay(100);
}

  digitalWrite(REDLED,HIGH); // Turns on the Record LED light to a solid colour.

// THIS FIXES THE INITAL DISTANCE CALCULATION INFINITE ERROR
  if (totaldistance > 5000) {
    totaldistance = 0.0;
  }
  
button_val=digitalRead(button);
time_new = millis()- time_prev;

int loopstarttime = millis();

////////////////////////////// GPS DATA COLLECTION /////////////////////////////////////

// Recording time, "Recording Time (s),Date(DDMMYYYY), Time (HHMMSSCC), Lat (deg), Long (deg), Distance Travelled (m),Speed (m/s), Speed (km/s), Raw Course(deg),Course (deg), Altitude (m), Locked Satellites, Hori Dim Precision, Fix Age (ms)"

////////////////////////// SERIAL PRINTING //////////////////////////////

Serial.print(time_new); Serial.print(",");
  
while (ss.available() > 0)
gps.encode(ss.read());

// INITIALIZATION OF THE DISTANCE CALCULATION VARIABLES
lat1 = 0.0;
lon1 = 0.0;

//////////////// STORING THE LON LAT VALUES FOR CALCULATION ///////////////////
/* This section calculates the linear distance traveled by the module. The equation for this calculation is available online and in the manual.
 *  
 */

lon1 = (gps.location.rawLng().deg);
float lon1_billi = (gps.location.rawLng().billionths);
lon1 = lon1 + (lon1_billi/1000000000);

lat1 = (gps.location.rawLat().deg);
float lat1_billi = (gps.location.rawLat().billionths);
lat1 = lat1 + (lat1_billi/1000000000);

dlong = lon2 - lon1;
dlat = lat2 - lat1;
float del_lat = 40030170 * dlat / 360;
float del_long = 40030170 * dlong * cos(radians((lat1+lat2)/2)) / 360;
float distanceR = sqrt(pow(del_lat,2) + pow(del_long,2));
totaldistance = distanceR + totaldistance;

float vehiclespeed = distanceR/looptime;


    Serial.print(gps.date.value());       Serial.print(",");
    Serial.print(gps.time.value());       Serial.print(",");
    Serial.print(gps.location.lat(), 6);  Serial.print(",");
    Serial.print(gps.location.lng(), 6);  Serial.print(",");
    Serial.print(vehiclespeed);           Serial.print(",");
    Serial.print(distanceR, 3);           Serial.print(",");
    Serial.print(totaldistance, 3);       Serial.print(",");
    Serial.print(gps.speed.kmph());       Serial.print(",");
    Serial.print(gps.course.value());     Serial.print(",");
    Serial.print(gps.course.deg());       Serial.print(",");
    Serial.print(gps.altitude.meters());  Serial.print(",");
    Serial.print(gps.satellites.value()); Serial.print(",");
    Serial.print(gps.hdop.hdop());        Serial.print(",");
    Serial.println(gps.location.age());

 
/////////////////////// SD CARD PRINTNG ///////////////////////////////

   myFile = SD.open(filename, FILE_WRITE);
  if (myFile) {    
    myFile.print(time_new);               myFile.print(",");
    myFile.print(gps.date.value());       myFile.print(",");
    myFile.print(gps.time.value());       myFile.print(",");
    myFile.print(gps.location.lat(), 6);  myFile.print(",");
    myFile.print(gps.location.lng(), 6);  myFile.print(",");
    myFile.print(vehiclespeed);           myFile.print(",");
    myFile.print(distanceR, 3);           myFile.print(",");
    myFile.print(totaldistance, 3);       myFile.print(",");
    myFile.print(gps.speed.kmph());       myFile.print(",");
    myFile.print(gps.course.value());     myFile.print(",");
    myFile.print(gps.course.deg());       myFile.print(",");
    myFile.print(gps.altitude.meters());  myFile.print(",");
    myFile.print(gps.satellites.value()); myFile.print(",");
    myFile.print(gps.hdop.hdop());        myFile.print(",");
    myFile.println(gps.location.age());

    myFile.close(); // close the file
  }

  
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening file");
    digitalWrite(REDLED, HIGH);

while(1);    // never gets here. NOT NEEDED OF COURSE 

  }

    digitalWrite(GREENLED, HIGH);
    digitalWrite(REDLED,LOW);

    lon2=lon1; // SAVES THE PREVIOUS LONG LAT VALUES TO THIS
    lat2=lat1;

    looptime = millis() - loopstarttime;
    delay(100);
}

Serial.println("No GPS Fix"); // GPS LED FIX BLINK LIGHT 

 digitalWrite(BLUELED,HIGH);
 delay(100);
 digitalWrite(BLUELED,LOW); 
 delay(100);


//////////////////////// IDLE STATE ////////////////////////////

while (button_val == LOW) // THE CODE WILL STAY HERE IF THE REC BUTTON IS LOW
{
button_val=digitalRead(button);
time_prev=millis();

// CHECK BATTERY MODE (NOT IMPLIMENETED YET)


  digitalWrite(GREENLED, HIGH);
      delay(500);
   digitalWrite(GREENLED, LOW); 
      delay(500);
}

////////// FILE NAME COUNTER //////////////

delay(50);

}



  
