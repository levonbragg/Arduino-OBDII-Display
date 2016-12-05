#include <doxygen.h>
#include <NexButton.h>
#include <NexConfig.h>
#include <NexCrop.h>
#include <NexDualStateButton.h>
#include <NexGauge.h>
#include <NexHardware.h>
#include <NexHotspot.h>
#include <NexNumber.h>
#include <NexObject.h>
#include <NexPage.h>
#include <NexPicture.h>
#include <NexProgressBar.h>
#include <NexSlider.h>
#include <NexText.h>
#include <NexTimer.h>
#include <Nextion.h>
#include <NexTouch.h>
#include <NexUpload.h>
#include <NexWaveform.h>
#include <SoftwareSerial.h>

/*
* OBD-II-UART Quickstart Sketch
* Written by Ryan Owens for SparkFun Electronics 7/5/2011
* Updates for Arduino 1.0+ by Toni Klopfenstein
* 
* Modified by Levon Bragg 
* Customized for my specific needs on a Diesel Ford F250 Truck
* Using the atMega1284p (MightyCore) https://github.com/MCUdude/MightyCore
* Modified Nextion Nexconfig.h to include software serial settings (Included in my github)
* My code is sloppy and inefficient. So what. it works for me.
*
* Released under the 'beerware' license
* (Do what you want with the code, but if we ever meet then you buy me a beer)
*
* This sketch will grab Exhaust Gas Temps, DFP Status, Regen Status, Engine Coolant,
* and Engine Oil Temp data from a vehicle with an OBD port  using the OBD-II-UART 
* board from SparkFun electronics. The data will be displayed
* on a Nextion 2.4" display. See the tutorial at https://www.sparkfun.com/tutorials/294
* to learn how to hook up the hardware:
*
*/

// Uncomment to print debud data to serial console
//#define debug

// Master RX, TX, connect to Nextion TX, RX
SoftwareSerial HMISerial(12,13); // RX, TX

// Nextion Pages
NexPage page0    = NexPage(0, 0, "page0");
NexPage page1    = NexPage(1, 0, "page1");
NexPage page2    = NexPage(2, 0, "page2");

// Page 1 objects
NexDSButton bt0 = NexDSButton(1, 9, "bt0");   // Page 1 "REGEN"
NexNumber n0 = NexNumber(1, 10, "n0");  // EGT 1
NexNumber n1 = NexNumber(1, 11, "n1");  // EGT 2
NexNumber n2 = NexNumber(1, 12, "n2");  // EGT 3
NexNumber n3 = NexNumber(1, 13, "n3");  // EGT4
NexNumber n4 = NexNumber(1, 14, "n4");  // DPF %
NexNumber n5 = NexNumber(1, 15, "n5");  // OIL Temp
NexNumber n6 = NexNumber(1, 16, "n6");  // H20 Temp
NexNumber n17 = NexNumber(1, 17, "n17");  // Page Holder
NexText tb0 = NexText(1, 1, "tb0");     // EGT 1


// Page 2 Objects
NexNumber n7 = NexNumber(2, 9, "n7");   // DEF in Liters
NexNumber n8 = NexNumber(2, 10, "n8");  // OIL Life Percent
NexNumber n9 = NexNumber(2, 11, "n9");  // Trans Temp
NexNumber n10 = NexNumber(2, 12, "n10");  // Ref Voltage
NexNumber n11 = NexNumber(2, 16, "n11");  // Driver Front Tire Pres
NexNumber n12 = NexNumber(2, 13, "n12");  // Driver Rear Tire Pres
NexNumber n13 = NexNumber(2, 14, "n13");  // Passenger Front Tire Pres
NexNumber n14 = NexNumber(2, 15, "n14");  // Passenger Rear Tire Pres
NexText tb8 = NexText(2, 1, "tb8");     // DEF (L)

NexTouch *nex_listen_list[] =
{
  &n0, &n1, &n2, &n3, &n4, &n5, &n6, &n7, &n8, &n9, &n10, &n11, &n12, &n13, &n14, 
  &page0, &page1, &page2, &bt0, &n17, &tb0, &tb8,
  NULL
};

//This is a character buffer that will store the data from the serial port
char rxData[32];
char rxWord1[32];
char rxWord2[32];

unsigned int interval = 100;
unsigned long currentMillis, previousMillis; 

char rxIndex=0;
int pageID = 1;

long EGT1, EGT2, EGT3, EGT4, REGEN, EngineCoolant, EngineOil, DEF, OILp, Trans, Volts, DFT, PFT, DRT, PRT;
double DPFpercent;
int DPFp;
int i=0;

int test = 1; 

void setup(){
  //Both the Serial console and the OBD-II-UART use 9600 bps.
//#ifdef debug
  Serial.begin(9600);  // Used for the USB Serial Console
  //Serial.println("Serial Up...");
//#endif 

  //Serial.println("Init Nextion");
  // Init the Nextion Display
  nexInit();
  delay(500);
  page1.show();
 // Serial.println("Nextion Done");
  
  Serial1.begin(9600); // Used to communicate with the OBD2 controller from SparkFun
 
  //Wait for a little while before sending the reset command to the OBD-II-UART
  delay(1500);
  
  //Reset the OBD-II-UART
  Serial1.println("ATWS");
  //Wait for a bit before starting to send commands after the reset.
  delay(1000);
 
  //Delete any data that may be in the serial port before we begin.
  while(Serial1.available()>0)
  Serial1.read();

  tb0.attachPop(tb0PopCallback, &tb0);  // EGT 1
  tb8.attachPop(tb8PopCallback, &tb8);  // DEF (L)

  //Serial.println("Setup Done");
}

void tb0PopCallback(void *ptr){
  pageID=2;
  page2.show();
}

void tb8PopCallback(void *ptr){
  pageID=1;
  page1.show();
}

void clear_rxWord(){
  //Fill rxdata array with zeros
  memset(rxWord1, 0, sizeof(rxWord1));
}

void clear_rxWords(){
  //Fill rxdata array with zeros
  memset(rxWord1, 0, sizeof(rxWord1));
  memset(rxWord2, 0, sizeof(rxWord2));
}

void loop(){
  nexLoop(nex_listen_list);
    
  //Fill rxdata array with zeros
  memset(rxData, 0, sizeof(rxData));
  
  //Delete any data that may be in the serial port before we begin.
  while(Serial1.available()>0)
  Serial1.read();

  switch (pageID) {
    case 1:
          nexLoop(nex_listen_list);
          
// Set header to 7E0
          Serial1.println("ATSH 7e0");
         
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }
          
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();
                       
          //++++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for the Engine OIL Temp
          clear_rxWord();
          Serial1.println("22f45c");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }
         
          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));

          // Calculate Engine Oil Temp
          EngineOil = (((strtol(&rxWord1[9],0,16)-40)*9)/5)+32;
          
          //Display Engine Oil Temp.
          HMISerial.print("page1.n5.val=" + String(EngineOil));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
        
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();

         nexLoop(nex_listen_list);
         
       
          //++++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for the Regen Status
          clear_rxWord();
          Serial1.println("22f48b");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }

          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ COUNTER? OR LENGHT? AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));

          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord2
          memcpy( rxWord2, rxData, sizeof(rxWord2));
          #ifdef debug
            for (i=0; i++; i<sizeof(rxWord1)){
              Serial.print(rxWord1[i]);
              }
          #endif 
          REGEN = bitRead(strtol(&rxWord1[15],0,16), 0);
        
          //Display Regen Status
          bt0.setValue(REGEN);
        
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();
         
          nexLoop(nex_listen_list);
         
          //++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for the DPF PERCENT
          clear_rxWord();
          Serial1.println("22042c");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }
 
          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));

          DPFpercent = strtol(&rxWord1[9],0,16)*256;
          //Serial.println(DPFpercent,4);
          DPFpercent += strtol(&rxWord1[12],0,16);
          //Serial.println(DPFpercent,4);
          DPFpercent *= 0.0015259021896696;
          //Serial.println(DPFpercent,4);
          DPFpercent -= 1;
          //Serial.println(DPFpercent,4);
          DPFpercent /= 1.75;
          //Serial.println(DPFpercent,4);
          DPFpercent *= 100;
          //Serial.println(DPFpercent,4);
          DPFp = DPFpercent;
        
          //Display DPF Percent on LCD
          //n4.setValue(DPFp);
          HMISerial.print("page1.n4.val=" + String(DPFp));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
         
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();

          nexLoop(nex_listen_list);
          
          //+++++++++++++++++++++++++++++++++++++++++
          // Query the OBD-II-UART for the Vehicle EGT
          // This memory location returns 
          // 1. the command echo
          // 2. count of bytes (00c) = 12 bytes
          // 3. first response
          // 4. second response
          
          clear_rxWords();
          Serial1.println("22f478");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }
         
          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ LENGHT AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxData));
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord2
          memcpy( rxWord2, rxData, sizeof(rxData));

          // Generate Display Data
          EGT1 = (((strtol(&rxWord1[15],0,16)*256)+strtol(&rxWord1[18],0,16))*0.18)-40;
          EGT2 = (((strtol(&rxWord2[3],0,16)*256)+strtol(&rxWord2[6],0,16))*0.18)-40;
          EGT3 = (((strtol(&rxWord2[9],0,16)*256)+strtol(&rxWord2[12],0,16))*0.18)-40;
          EGT4 = (((strtol(&rxWord2[15],0,16)*256)+strtol(&rxWord2[18],0,16))*0.18)-40;

          // Display the EGTs
          HMISerial.print("page1.n0.val=" + String(EGT1));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.print("page1.n1.val=" + String(EGT2));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.print("page1.n2.val=" + String(EGT3));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.print("page1.n3.val=" + String(EGT4));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);

          //++++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for the Engine Coolant
          
// Set header to 7E1
          Serial1.println("ATSH 7e1");         
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
            Serial1.read();

          while(Serial1.available()>0)
            Serial1.read();
          
          clear_rxWord();  
          Serial1.println("0105");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }
         
          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));

          EngineCoolant = (((strtol(&rxWord1[6],0,16)-40)*9)/5)+32;
        
          //Display Coolant Temp.
          HMISerial.print("page1.n6.val=" + String(EngineCoolant));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
        
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();
        
         nexLoop(nex_listen_list);

         //++++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for Trans Temp
          
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
            Serial1.read();
          
          clear_rxWord();    
          Serial1.println("221e1c");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }
         
          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));
       
          // Calculate 
          Trans = strtol(&rxWord1[9],0,16)*256;
          Trans += strtol(&rxWord1[12],0,16);
          Trans *= 0.0625;
          Trans *= 1.8;
          Trans += 32;

          //Display 
          //n9.setValue(Trans);
          HMISerial.print("page1.n15.val=" + String(Trans));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
                
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();

          nexLoop(nex_listen_list);
         
    case 2:
        nexLoop(nex_listen_list);

// Set header to 7E0
          Serial1.println("ATSH 7e0");         
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
            Serial1.read();

          while(Serial1.available()>0)
            Serial1.read();
            
          //++++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for      DEF Level
          clear_rxWord();
          Serial1.println("220487");
          
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }

          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));
          
          // Calculate 
          DEF = (strtol(&rxWord1[9],0,16)*256);
          DEF += strtol(&rxWord1[12],0,16);
          DEF = DEF/1000;
        
          //Display 
          //n7.setValue(DEF);
          HMISerial.print("page2.n7.val=" + String(DEF));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
                
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();

          nexLoop(nex_listen_list);
          
          //++++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for           OIL %

          clear_rxWord();
          Serial1.println("22054b");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }
         
          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));

          // Calculate 
          OILp = (strtol(&rxWord1[9],0,16));
         
          //Display 
          //n8.setValue(OILp);
          HMISerial.print("page2.n8.val=" + String(OILp));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
                
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();

          nexLoop(nex_listen_list);
          
          //++++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for          Voltage

          clear_rxWord();
          Serial1.println("22d127");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }
         
          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));

          // Calculate 
          Volts = strtol(&rxWord1[9],0,16)*256;
          Volts += strtol(&rxWord1[12],0,16);
          Volts *= 0.0009765625;
         
          //Display 
          //n10.setValue(Volts);
          HMISerial.print("page2.n10.val=" + String(Volts));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
                
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();

          nexLoop(nex_listen_list);

          //++++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for Trans Temp
// Set header to 7E1
          Serial1.println("ATSH 7e1");
          
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + 500) > millis() ){
            nexLoop(nex_listen_list);
          }
          
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
            Serial1.read();

          while(Serial1.available()>0)
            Serial1.read();

          clear_rxWord();              
          Serial1.println("221e1c");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + interval) > millis() ){
            nexLoop(nex_listen_list);
          }
         
          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));
       
          // Calculate 
          Trans = strtol(&rxWord1[9],0,16)*256;
          Trans += strtol(&rxWord1[12],0,16);
          Trans *= 0.0625;
          Trans *= 1.8;
          Trans += 32;

          //Display 
          //n9.setValue(Trans);
          HMISerial.print("page2.n9.val=" + String(Trans));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
                
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();

          nexLoop(nex_listen_list);
          
          //++++++++++++++++++++++++++++++++++++++++++
// Set header to 726
          Serial1.println("ATSH 726");
          
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + 500) > millis() ){
            nexLoop(nex_listen_list);
          }
          
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
            Serial1.read();

          while(Serial1.available()>0)
            Serial1.read();
            
          //Query the OBD-II-UART for DFT
          clear_rxWord();
          Serial1.println("222813");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + 500) > millis() ){
            nexLoop(nex_listen_list);
          }
         
          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));

          // Calculate 
          //(((256*A)+B)/3+22/3)*0.145
          DFT = strtol(&rxWord1[9],0,16)*256;
          DFT += strtol(&rxWord1[12],0,16);
          DFT = (DFT/3+22/3)*0.145;

          //Display 
          //n11.setValue(DFT);
          HMISerial.print("page2.n11.val=" + String(DFT));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
                
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();

          nexLoop(nex_listen_list);

          //++++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for PFT
          clear_rxWord();
          Serial1.println("222814");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + 500) > millis() ){
            nexLoop(nex_listen_list);
          }

          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));

          // Calculate 
          //(((256*A)+B)/3+22/3)*0.145
          PFT = strtol(&rxWord1[9],0,16)*256;
          PFT += strtol(&rxWord1[12],0,16);
          PFT = (PFT/3+22/3)*0.145;

          //Display 
          HMISerial.print("page2.n12.val=" + String(PFT));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
                
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();

          nexLoop(nex_listen_list);
          
          //++++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for PRT
          clear_rxWord();
          Serial1.println("222815");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + 500) > millis() ){
            nexLoop(nex_listen_list);
          }

          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));

          // Calculate 
          //(((256*A)+B)/3+22/3)*0.145
          PRT = strtol(&rxWord1[9],0,16)*256;
          PRT += strtol(&rxWord1[12],0,16);
          PRT = (PRT/3+22/3)*0.145;

          //Display 
          //n14.setValue(PRT);
          HMISerial.print("page2.n14.val=" + String(PRT));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
                
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
          Serial1.read();

          nexLoop(nex_listen_list);
          
          //++++++++++++++++++++++++++++++++++++++++++
          //Query the OBD-II-UART for DRT
          clear_rxWord();
          Serial1.println("222816");
          //delay(200);
          currentMillis = millis();
          while ((currentMillis + 500) > millis() ){
            nexLoop(nex_listen_list);
          }

          getResponse(); // READ ECHO AND DISCARD
          getResponse(); // READ REAL DATA AND THEN STORE IN rxWord1
          memcpy( rxWord1, rxData, sizeof(rxWord1));

          // Calculate 
          //(((256*A)+B)/3+22/3)*0.145
          DRT = strtol(&rxWord1[9],0,16)*256;
          DRT += strtol(&rxWord1[12],0,16);
          DRT = (DRT/3+22/3)*0.145;

          //Display 
          HMISerial.print("page2.n13.val=" + String(DRT));
          HMISerial.write(0xff);
          HMISerial.write(0xff);
          HMISerial.write(0xff);
                
          //Delete any data that may be in the serial port before we begin.
          while(Serial1.available()>0)
            Serial1.read();
          
      break;
    
    default:
      // Do nothing...
      nexLoop(nex_listen_list);
      break;
  }

  //delay(200);
  currentMillis = millis();
  while ((currentMillis + 500) > millis() ){
      nexLoop(nex_listen_list);
    }

}

//The getResponse function collects incoming data from the UART into the rxData buffer
// and only exits when a carriage return character is seen. Once the carriage return
// string is detected, the rxData buffer is null terminated (so we can treat it as a string)
// and the rxData index is reset to 0 so that the next string can be copied.
void getResponse(void){
  char inChar=0;
  //Fill rxdata array with zeros
  memset(rxData, 0, sizeof(rxData));
  
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
  if (strcmp(rxData,"NO DATA") == 0){
    memset(rxData, 0, sizeof(rxData));
  }
  //Serial.println("GetResponseDone");
}

