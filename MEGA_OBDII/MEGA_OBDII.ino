/*
* OBD-II-UART Quickstart Sketch
* Written by Ryan Owens for SparkFun Electronics 7/5/2011
* Updates for Arduino 1.0+ by Toni Klopfenstein
* 
* Modified by Levon Bragg 11/2014 
* Customized for my specific needs on a Diesel Ford F250 Truck
*
* Released under the 'beerware' license
* (Do what you want with the code, but if we ever meet then you buy me a beer)
*
* This sketch will grab Exhaust Gas Temps, DFP Status, Regen Status, Engine Coolant,
* and Engine Oil Temp data from a vehicle with an OBD port  using the OBD-II-UART 
* board from SparkFun electronics. The data will be displayed
* on a serial 16x2 LCD. See the tutorial at https://www.sparkfun.com/tutorials/294
* to learn how to hook up the hardware:
*
*/

// Uncomment to print debud data to serial console
//#define debug

// include the LCD library code:
#include <genieArduino.h>
Genie genie;
#define RESETLINE 4  // Change this if you are not using an Arduino Adaptor Shield Version 2 (see code below)

//This is a character buffer that will store the data from the serial port
char rxData[32];
char rxWord1[32];
char rxWord2[32];

// variables used for lts (Long To String)
char b[8];   //declaring character array
String str;  //declaring string

char rxIndex=0;

// OBDII Variables
long EGT1, EGT2, EGT3, EGT4, REGEN, EngineCoolant, EngineOil;
double DPFpercent;
int DPFp;

int i=0;

void setup(){
  //the OBD-II-UART use 9600 bps.
  Serial1.begin(9600); // Used to communicate with the OBD2 controller from SparkFun

  // NOTE, the genieBegin function (e.g. genieBegin(GENIE_SERIAL_0, 115200)) no longer exists.  Use a Serial Begin and serial port of your choice in
  // your code and use the genie.Begin function to send it to the Genie library (see this example below)
  // 200K Baud is good for most Arduinos. Galileo should use 115200.  
  Serial.begin(115200);  // Serial0 @ 200000 (200K) Baud
  genie.Begin(Serial);   // Use Serial0 for talking to the Genie Library, and to the 4D Systems display

  // Attach the user function Event Handler for processing events
  genie.AttachEventHandler(myGenieEventHandler); 

  // Reset the Display (change D4 to D2 if you have original 4D Arduino Adaptor)
  // THIS IS IMPORTANT AND CAN PREVENT OUT OF SYNC ISSUES, SLOW SPEED RESPONSE ETC
  // If NOT using a 4D Arduino Adaptor, digitalWrites must be reversed as Display Reset is Active Low, and
  // the 4D Arduino Adaptors invert this signal so must be Active High.  
  pinMode(RESETLINE, OUTPUT);  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  digitalWrite(RESETLINE, 1);  // Reset the Display via D4
  delay(100);
  digitalWrite(RESETLINE, 0);  // unReset the Display via D4

  delay (3500); //let the display start up after the reset (This is important)

  //Turn the Display on (Contrast) - (Not needed but illustrates how)
  genie.WriteContrast(1); // 1 = Display ON, 0 = Display OFF.
  //For uLCD43, uLCD-70DT, and uLCD-35DT, use 0-15 for Brightness Control, where 0 = Display OFF, though to 15 = Max Brightness ON.

  //Reset the OBD-II-UART
  Serial1.println("ATZ");
  //Wait for a bit before starting to send commands after the reset.
  delay(2000);
  
  //Delete any data that may be in the serial port before we begin.
  while(Serial1.available())
  Serial1.read();
 
}

char lts(long _long){
   str=String(_long);
   str.toCharArray(b,8);
   return *b; 
}

void loop(){
  //Fill rxdata array with zeros
  memset(rxData, 0, sizeof(rxData));
  //Delete any data that may be in the serial port before we begin.
  while(Serial1.available()>0)
  Serial1.read();
  
//+++++++++++ Engine Oil ++++++++++++++++++++
#ifdef debug
  Serial.println("PID: 22f45c");
  Serial.println("Response: ");
#endif 

  //Query the OBD-II-UART for the Engine OIL Temp
  Serial1.println("22f45c");
  delay(200);
  getResponse(); // READ ECHO AND DISCARD
  getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
  memcpy( rxWord1, rxData, sizeof(rxWord1));
  // Calculate Engine Oil Temp
  EngineOil = (((strtol(&rxWord1[9],0,16)-40)*9)/5)+32;
  //Display Engine Oil Temp.
  lts(EngineOil);
  genie.WriteStr(0x05, b);  

#ifdef debug
    for (i=0; i++; i<sizeof(rxWord1)){
    Serial.print(rxWord1[i]);
    }
  // Print to serial console for debug
  Serial.println();
  Serial.print("Oil Temp: ");
  Serial.println(EngineOil);
  Serial.println("DONE PID: 22f45c");
  Serial.println();
#endif 
 
  //Delete any data that may be in the serial port before we begin.
  while(Serial1.available()>0)
  Serial1.read();
 
//+++++++++++ Engine Coolant +++++++++++++
#ifdef debug
  Serial.println("PID: 0105");
  Serial.println("Response: ");
#endif  

  //Query the OBD-II-UART for the Engine Coolant
  Serial1.println("0105");
  delay(200);
  getResponse(); // READ ECHO AND DISCARD
  getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
  memcpy( rxWord1, rxData, sizeof(rxWord1));
  EngineCoolant = (((strtol(&rxWord1[6],0,16)-40)*9)/5)+32;
  //Display Coolant Temp.
  lts(EngineCoolant);
  genie.WriteStr(0x06, b); 

#ifdef debug
  for (i=0; i++; i<sizeof(rxWord1)){
    Serial.print(rxWord1[i]);
  }
  Serial.println();
  Serial.print("Engine Coolant: ");
  Serial.println(EngineCoolant);
  Serial.println("DONE PID: 0105");
  Serial.println();
#endif 

//Delete any data that may be in the serial port before we begin.
  while(Serial1.available()>0)
  Serial1.read();

 
//++++++++++++ Regen Status +++++++++++++++++
#ifdef debug 
  Serial.println("PID: 22f48b");
  Serial.println("Response: ");
#endif  

  //Query the OBD-II-UART for the Regen Status
  Serial1.println("22f48b");
  delay(200);
  getResponse(); // READ ECHO AND DISCARD
  getResponse(); // READ COUNTER? OR LENGHT? AND DISCARD
  getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
  memcpy( rxWord1, rxData, sizeof(rxWord1));
  getResponse(); // READ REAL DATA AND THEN STORE IN rxWord2
  memcpy( rxWord2, rxData, sizeof(rxWord2));
  REGEN = bitRead(strtol(&rxWord1[15],0,16), 0);
  //Display Regen Status
  genie.WriteObject(GENIE_OBJ_USER_LED, 0x00,REGEN);  
  
#ifdef debug
    for (i=0; i++; i<sizeof(rxWord1)){
    Serial.print(rxWord1[i]);
    }
    for (i=0; i++; i<sizeof(rxWord2)){
    Serial.print(rxWord2[i]);
    }
  Serial.println();
  Serial.print("Regen Status: ");
  Serial.println(REGEN);
  Serial.println("DONE PID: 22f48b");
  Serial.println();
#endif  

  //Delete any data that may be in the serial port before we begin.
  while(Serial1.available()>0)
  Serial1.read();
 
 
//++++++++++++ DPF Percent ++++++++++++++++++
#ifdef debug 
  Serial.println("PID: 22042c");
  Serial.println("Response: ");
#endif  

  // Query the OBD-II-UART for the DPF PERCENT
  Serial1.println("22042c");
  delay(200);
  getResponse(); // READ ECHO AND DISCARD
  getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
  memcpy( rxWord1, rxData, sizeof(rxWord1));
  DPFpercent = strtol(&rxWord1[9],0,16)*256;
  // Serial.println(DPFpercent,4);
  DPFpercent += strtol(&rxWord1[12],0,16);
  // Serial.println(DPFpercent,4);
  DPFpercent *= 0.0015259021896696;
  // Serial.println(DPFpercent,4);
  DPFpercent -= 1;
  // Serial.println(DPFpercent,4);
  DPFpercent /= 1.75;
  // Serial.println(DPFpercent,4);
  DPFpercent *= 100;
  // Serial.println(DPFpercent,4);
  DPFp = DPFpercent;

  // Display DPF Percent on LCD
  if (DPFp < 0){
    DPFp=0;
    }
  if (DPFp >100){
   DPFp=100;   
  }
    
  genie.WriteObject(GENIE_OBJ_GAUGE, 0x00, DPFp);
 
#ifdef debug
  for (i=0; i++; i<sizeof(rxWord1)){
   Serial.print(rxWord1[i]);
  }
  Serial.println();
  Serial.print("DPF %: ");
  Serial.println(DPFp);
  Serial.println("DONE PID: 22042c");
  Serial.println();
#endif  

  // Delete any data that may be in the serial port before we begin.
  while(Serial1.available()>0)
  Serial1.read();


//++++++++++++++ Exhaust Gas Temp ++++++++++++++++

#ifdef debug
  Serial.println("PID: 22f478");
#endif   

  //Query the OBD-II-UART for the Vehicle EGT
  Serial1.println("22f478");
  delay(200);
  getResponse(); // READ ECHO AND DISCARD
  getResponse(); // READ COUNTER? OR LENGHT? AND DISCARD
  getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
  memcpy( rxWord1, rxData, sizeof(rxData));
  getResponse(); // READ REAL DATA AND THEN STORE IN rxWord2
  memcpy( rxWord2, rxData, sizeof(rxData));
    // Generate Display Data
  EGT1 = (((strtol(&rxWord1[15],0,16)*256)+strtol(&rxWord1[18],0,16))*0.18)-40;
  EGT2 = (((strtol(&rxWord2[3],0,16)*256)+strtol(&rxWord2[6],0,16))*0.18)-40;
  EGT3 = (((strtol(&rxWord2[9],0,16)*256)+strtol(&rxWord2[12],0,16))*0.18)-40;
  EGT4 = (((strtol(&rxWord2[15],0,16)*256)+strtol(&rxWord2[18],0,16))*0.18)-40;

  lts(EGT1);
  genie.WriteStr(0x00, b); 

  lts(EGT2);
  genie.WriteStr(0x01, b); 

  lts(EGT3);
  genie.WriteStr(0x02, b); 

  lts(EGT4);
  genie.WriteStr(0x03, b); 
    
#ifdef debug
    for (i=0; i++; i<sizeof(rxWord1)){
    Serial.print(rxWord1[i]);
    }
    for (i=0; i++; i<sizeof(rxWord2)){
    Serial.print(rxWord2[i]);
    }
  Serial.println("DONE PID: 22f478");
  Serial.println();
#endif  


}

//The getResponse function collects incoming data from the UART into the rxData buffer
// and only exits when a carriage return character is seen. Once the carriage return
// string is detected, the rxData buffer is null terminated (so we can treat it as a string)
// and the rxData index is reset to 0 so that the next string can be copied.
void getResponse(void){
  char inChar=0;
  //Keep reading characters until we get a carriage return
  while(inChar != '\r'){
    //If a character comes in on the serial port, we need to act on it.
    if(Serial1.available()>0){
      //Start by checking if we've received the end of message character ('\r').
      if(Serial1.peek() == '\r'){
        //Clear the Serial buffer
        inChar=Serial1.read();
        //Put the end of string character on our data string
        rxData[rxIndex]='\0';
        //Reset the buffer index so that the next character goes back at the beginning of the string.
        rxIndex=0;
      }
      //If we didn't get the end of message character, just add the new character to the string.
      else{
        //Get the new character from the Serial port.
        inChar = Serial1.read();
        //Serial.print(inChar);
        //Add the new character to the string, and increment the index variable.
        rxData[rxIndex++]=inChar;
      }
    }
    else
      inChar='\r';
  }
  //Serial.println("GetResponseDone");
}


// LONG HAND VERSION, MAYBE MORE VISIBLE AND MORE LIKE VERSION 1 OF THE LIBRARY
void myGenieEventHandler(void)
{
  genieFrame Event;
  genie.DequeueEvent(&Event);

  int slider_val = 0;

  //If the cmd received is from a Reported Event (Events triggered from the Events tab of Workshop4 objects)
  if (Event.reportObject.cmd == GENIE_REPORT_EVENT)
  {
    if (Event.reportObject.object == GENIE_OBJ_SLIDER)                // If the Reported Message was from a Slider
    {
      if (Event.reportObject.index == 0)                              // If Slider0
      {
        slider_val = genie.GetEventData(&Event);                      // Receive the event data from the Slider0
        genie.WriteObject(GENIE_OBJ_LED_DIGITS, 0x00, slider_val);    // Write Slider0 value to to LED Digits 0
      }
    }
  }

  //If the cmd received is from a Reported Object, which occurs if a Read Object (genie.ReadOject) is requested in the main code, reply processed here.
  if (Event.reportObject.cmd == GENIE_REPORT_OBJ)
  {
    if (Event.reportObject.object == GENIE_OBJ_USER_LED)              // If the Reported Message was from a User LED
    {
      if (Event.reportObject.index == 0)                              // If UserLed0
      {
        bool UserLed0_val = genie.GetEventData(&Event);               // Receive the event data from the UserLed0
        UserLed0_val = !UserLed0_val;                                 // Toggle the state of the User LED Variable
        genie.WriteObject(GENIE_OBJ_USER_LED, 0x00, UserLed0_val);    // Write UserLed0_val value back to to UserLed0
      }
    }
  }

  //This can be expanded as more objects are added that need to be captured
  //Event.reportObject.cmd is used to determine the command of that event, such as an reported event
  //Event.reportObject.object is used to determine the object type, such as a Slider
  //Event.reportObject.index is used to determine the index of the object, such as Slider0
  //genie.GetEventData(&Event) us used to save the data from the Event, into a variable.
}


