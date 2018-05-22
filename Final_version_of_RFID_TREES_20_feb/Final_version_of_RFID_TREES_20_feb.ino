/////////////////////////////// Libraries /////////////////////////
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <MFRC522.h>     // RFID library for MFRC522
#include <SPI.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>




/////////////////////////////// define statements //////////////////
#define SS_pin 10                      // SPI Slave Select pin 
#define RST_pin 9
      
#define dataSize 4                   //RFID Card data lenght = 4byte
#define maxNum 6                    //Max. no. of options
#define shiftStart_add 900
#define shiftEnd_add 901
#define duration_add 902
#define miss_add 1000
#define masterNumber_add 904
#define nextAlarm_Minute_add 920
#define nextAlarm_Hour_add 921
#define missReseted_add 903


/////////////////////////////// Veriables /////////////////////////
byte readCard[dataSize];
byte storedCard[dataSize];
byte masterCard[dataSize];
boolean match = false;
boolean mode = false;
boolean verified = false;
boolean messageSent = false;
boolean missReseted = false;
int masterNumber[10] = {0};
int character = 0;
int i = 0;
int z = 0;
int miss = 0;
int select = 5;           //buttons connected to pin 5
int program = 6;          //buttons connected to pin 6
int progSelect = 0;
int modeSelect = 0;
int shiftStart = 0;
int shiftEnd = 0;
int duration = 0;
int buzzer = 3;           //buzzer connected to pin 3
boolean alarm = false;
boolean missInc = false;
int nextAlarm_Minute = 0;
int nextAlarm_Hour = 0;
int missCount = 0;
int presentHour;
int presentMinute;
int successRead = 0;


/////////////////////////////// objects /////////////////////////
MFRC522 mfrc522(SS_pin, RST_pin);
LiquidCrystal lcd(8, 7, A0, A1, A2, A3);
tmElements_t tm;


/////////////////////////////// Void Setup /////////////////////////
void setup()
{
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("INITIALIZING");
  
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  
  pinMode(select, INPUT);
  pinMode(program, INPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(select, HIGH);
  digitalWrite(program, HIGH);
  
  shiftStart = EEPROM.read(shiftStart_add);                   // To Initialize the variables with previous value from EEPROM
  shiftEnd = EEPROM.read(shiftEnd_add);
  duration = EEPROM.read(duration_add);
  miss = EEPROM.read(miss_add);
  
  digitalWrite(buzzer, LOW);
  
  if (RTC.read(tm)){                                     // present value of hour and minute taken from RTC to compare
  presentHour = tm.Hour;
  presentMinute = tm.Minute;}
  
  flushCard(readCard);                                                    // for removing garbage value
  
  for(int i = 0; i < 10; i++)                                            //to initialize master mobile number
  {
    masterNumber[i] = EEPROM.read(904 + i);
  }
  
 if (shiftStart < shiftEnd){                                            // to set the alarm when system is resetted
  if(presentHour >= shiftStart && presentHour < shiftEnd)
  {
    if (RTC.read(tm)){
      nextAlarm_Hour = tm.Hour;
      nextAlarm_Minute = tm.Minute + duration;
      
  }

  }
    else                               
    {
      nextAlarm_Hour = shiftStart;
      nextAlarm_Minute = 0;
    }
  }

  if(shiftStart > shiftEnd)
  {
    if(presentHour >= shiftStart || presentHour < shiftEnd)
    {
      if (RTC.read(tm))
      {
        nextAlarm_Hour = tm.Hour;
        nextAlarm_Minute = tm.Minute + duration;
      }
    
  }
    else
    {
      nextAlarm_Hour = shiftStart;
      nextAlarm_Minute = 0;
    }
    }
  
  if (EEPROM.read(1) != 143) {                          // to check if there is a master defined or not
    lcd.print(F("No Master Defined"));
    delay(1500);
    do {
      successRead = getID();            
      lcd.clear();   
      lcd.print("Scan Card to be master");
      delay(200);
    }
    while (!successRead);                  
    for ( int j = 0; j < 4; j++ ) {        
      EEPROM.write( 2 + j, readCard[j] );  
    }
    EEPROM.write(1, 143);
    lcd.clear();                  
    lcd.print("Master Card Defined");
    delay(1500);
  }
  
  for ( int i = 0; i < 4; i++ ) {          
    masterCard[i] = EEPROM.read(2 + i);
    
  }
  


  
}


///////////////////////// To send message /////////////////////
boolean sendMessage(int x[])
{
  Serial.println("AT+CMGS=1");
  delay(1500);
  Serial.print("AT+CMGS=\"");
  for (int i; i < 10; i++)
  {
    Serial.print(x[i]);
  }
  Serial.println("\"");
  while (Serial.read()!= '>');
  {
    Serial.print("Total miss = ");
    Serial.print(EEPROM.read(miss_add));
    delay(500);
    Serial.write(0x1A);  // sends ctrl+z end of message
    Serial.write(0x0D);  // Carriage Return in Hex
    Serial.write(0x0A);  // Line feed in Hex
     //The 0D0A pair of characters is the signal for the end of a line and beginning of another.
    delay(500);
    return true;
  }
}


/////////////////////////////// Time from RTC to be displayed////////////////////////////////
void displayTime()
{
   if (RTC.read(tm)){
  lcd.clear();
  lcd.setCursor(0,0);
  print2digits(tm.Hour);
  lcd.print(":");
  print2digits(tm.Minute);
  lcd.print(":");
  print2digits(tm.Second);
  
   }
}


/////////////////////////////Default Display content /////////////////////////////////////
void displayContent()
{
  displayTime();
  lcd.setCursor(9,0);
  lcd.print("Mis=");
  lcd.print(miss);
  lcd.setCursor(0,1);
  lcd.print("PROGRAM");
  lcd.setCursor(10,1);
  lcd.print("SELECT");
  delay(200);
}




///////////////////////// Get PICC's UID ////////////////////////
int getID()
{
  if(! mfrc522.PICC_IsNewCardPresent())  return 0;

  if ( ! mfrc522.PICC_ReadCardSerial())  return 0;

  for (int i = 0; i < dataSize; i++)                          //4 is to be changed according to the type of card 
  readCard[i] = mfrc522.uid.uidByte[i];

  delay(1500);
  mfrc522.PICC_HaltA();
  return 1;
   
}


/////////////////////////////// Read an ID from EEPROM /////////////////////////
void readID(int number)
{
  int start = (number * dataSize) + 2;
  for (int i = 0; i<dataSize; i++)
  {
    storedCard[i] = EEPROM.read(start + i);
  }
}


///////////////////////// Check the value of two cards /////////////////////
boolean checkTwo(byte a[], byte b[])
{
  if(a[0] != NULL) {match = true;}

  for (int k = 0; k < dataSize; k++)
  {
    if (a[k] != b[k])
    match = false;
  }

  if (match)
  {
    return true;
  }
  else
  return false;
}


/////////////////////////////////for flushing the card variables ///////////////////// 
void flushCard(byte x [])
{
  for (int i = 0; i < dataSize; i++)
  {
    x[i] = 0x00;
  }
}


/////////////////////////////////Verification///////////////////////////////
boolean verification(byte a[])
{
  for(int i = 0; i < 4; i++)
  {
    if(storedCard[i] == a[i])
    {
      lcd.clear();
      lcd.print("Verified");
      delay(1000);
      return true;
     }

    else
    {
     
    return false;
    }
  }
}

///////////////////////// Check readCard is masterCard /////////////////////
boolean isMaster(byte test[])
{
  if (checkTwo(test, masterCard ))
   return true;
   
  else 
   return false;
}


///////////////////////// Write Success to EEPROM /////////////////////
void successWrite()
{
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Write Success!");
  delay(500);
  lcd.clear();
}



///////////////////////// Write Unsuccessfull to EEPROM /////////////////////
void failedWrite()
{
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Write Failed!");
  delay(500);
  lcd.clear();
}




///////////////////////// Add ID to EEPROM /////////////////////
void writeID(byte a[])
{
  if (!findID (a))
  {
    int num = EEPROM.read(0);
    int start = (num * dataSize) + 6;
    num++;
    EEPROM.write(0, num);
    for(int j = 0; j < dataSize; j++)
    {
      EEPROM.write(start + j, a[j]);
    }
    successWrite();
    mode = false;
    progSelect = 0;
  }

  else
  {
  }
  
}




///////////////////////// Find ID slot from EEPROM /////////////////////
int findIDSLOT (byte find[]){
  int count = EEPROM.read(0);
  for (int i = 1; i <= count; i++)
  {
    readID(i);
    if(checkTwo(find, storedCard)){
      return i;
      break;
      }
  }
}


///////////////////////// Delete ID from EEPROM /////////////////////
void deleteID (byte a[])
{
  if(!findID(a))
  {
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("No card found");
  }
  else
  {
    int num = EEPROM.read(0);
    int slot;
    int start;
    int looping;
    int j = 0;
    int count = EEPROM.read(0);
    slot = findIDSLOT(a);
    start = (slot * dataSize) + 2;
    looping = (num - slot) * dataSize;
    num--;
    EEPROM.write(0, num);
    for (j = 0; j< looping; j++)
    {
      EEPROM.write(start + j, EEPROM.read(start +  dataSize + j));
    }

    for (int k = 0; k < dataSize; k++)
    {
      EEPROM.write(start + j + k, 0);
    }
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("Card ID deleted!");
    delay(500);
    lcd.clear();
    
  }
}


///////////////////////// Find ID from EEPROM /////////////////////
boolean findID ( byte findc[] )
{
  int count = EEPROM.read(0);
  for (int i = 1; i<= count; i++)
  {
    readID(i);
    if(checkTwo(findc, storedCard))
    {
     return true;
     break;
    }

    else
    {
     return false;
    }
  }
}




/////////////////////////////for printing 2difits/////////////////
void print2digits(int number) 
{
  if (number >= 0 && number < 10) {
    lcd.print('0');
  }
  lcd.print(number);
}



/////////////////////////////// Void loop /////////////////////////
void loop() 
{
  if (digitalRead(program) == LOW && mode == false )                     // This loop is used for changing the different options
  {
    if (progSelect < maxNum && progSelect != 50){
    progSelect++;
    delay(200);}

    else
    {progSelect = 0;}
  }

 
  if (RTC.read(tm)){                                               // alarm condition starts here
      presentHour = tm.Hour;
      presentMinute = tm.Minute;}


       if (shiftStart > shiftEnd)
       {
        if(presentHour >= shiftStart || presentHour < shiftEnd)    //Condition when shift start time is greater then shift end
        {
          
          flushCard(readCard);
          verified = checkTwo(storedCard, readCard);
          if (nextAlarm_Minute < 58){
          if(presentMinute >= nextAlarm_Minute && presentMinute < nextAlarm_Minute + 2 && presentHour == nextAlarm_Hour)                 //2 minute alarm raising loop
          {
            
            if (RTC.read(tm))
            {
              presentHour = tm.Hour;
              presentMinute = tm.Minute;
            }
            digitalWrite(buzzer, HIGH);
            while (getID() != 1 && presentMinute < nextAlarm_Minute + 2)                      //check if 2 minutes are over or there is a card available
            {
               if (RTC.read(tm))
               {
                presentHour = tm.Hour;
                presentMinute = tm.Minute;
               }
                lcd.clear();
                lcd.print("Scan Your Card");
                delay(200);
            }
            int slot = findIDSLOT(readCard);                                                 // check if swiped card is stored card
            readID(slot);
            verified = checkTwo(storedCard, readCard);
            if(verified == true)
            {
              nextAlarm_Minute = nextAlarm_Minute + EEPROM.read(duration_add);              // if card is swiped increment the next alarm and lower the buzzer
              digitalWrite(buzzer, LOW);
            }

            else {}
          }
          if(presentMinute == nextAlarm_Minute + 2 && verified == false && presentHour == nextAlarm_Hour && presentHour != shiftEnd)       // condition for miss
          {
            miss++;                                                                                                                                              //increase the miss and store into EEPROM
            EEPROM.write(miss_add, miss);
            digitalWrite(buzzer, LOW);                                                                                                                           //Turn off the buzzer
             if (RTC.read(tm))
            {
              presentHour = tm.Hour;
              presentMinute = tm.Minute;
            }
            nextAlarm_Minute = nextAlarm_Minute + duration ;                                                                                                     //Increase the nextalarm
            delay(1000);
          }
          else {}
         } 
         else
         {
          if (nextAlarm_Minute < 60 && nextAlarm_Minute >= 58)                                                           // If next alarm minute is in the range 58 and 60
          {nextAlarm_Minute = 0;                                                                                         // Set next alarm minute to 0 and increase the nextAlarm hour
          nextAlarm_Hour = nextAlarm_Hour + 1;
          if (nextAlarm_Hour > 23)
          {nextAlarm_Hour = 0;}
          }
          else if (nextAlarm_Minute >= 60)                                                                              // If next alarm minute is greater than 59
          {nextAlarm_Minute = nextAlarm_Minute - 60;                                                                    //Subtract nextalarm Minute by 60
          nextAlarm_Hour = nextAlarm_Hour + 1;                                                                          //Increase the next alarm hour by 1
          if (nextAlarm_Hour > 23)                                                                                      //If next alarm hour is greater than 23 make it 0
          {nextAlarm_Hour = 0;}
          }
         
          digitalWrite(buzzer, LOW);
         }
         }
        }
       

       else if(shiftStart <= shiftEnd)
       {
        if(presentHour >= shiftStart && presentHour < shiftEnd)                                    // Condition for shift start is smaller than shift end
        {
          
          flushCard(readCard);
          verified = checkTwo(storedCard, readCard);
          if(nextAlarm_Minute < 58){
          if(presentMinute >= nextAlarm_Minute && presentMinute < nextAlarm_Minute + 2 && presentHour == nextAlarm_Hour)       // From here every thing is as same as the above
          {
            
            if (RTC.read(tm))
            {
              presentHour = tm.Hour;
              presentMinute = tm.Minute;
            }
            digitalWrite(buzzer, HIGH);
            while (getID() != 1 && presentMinute < nextAlarm_Minute + 2)
            {
              lcd.clear();
              lcd.print("Scan Your Card!");
               if (RTC.read(tm))
               {
                presentHour = tm.Hour;
                presentMinute = tm.Minute;
               }
            }
            int slot = findIDSLOT(readCard);
            readID(slot);
            verified = checkTwo(storedCard, readCard);
            if(verified == true)
            {
              nextAlarm_Minute = nextAlarm_Minute + EEPROM.read(duration_add); 
              digitalWrite(buzzer, LOW);
            }
          }
          if(presentMinute == nextAlarm_Minute + 2 && verified == false && presentHour == nextAlarm_Hour && presentHour != shiftEnd)
          {
             if (RTC.read(tm))
            {
              presentHour = tm.Hour;
              presentMinute = tm.Minute;
            }
            miss++;
            EEPROM.write(miss_add, miss);
            digitalWrite(buzzer, LOW);
            nextAlarm_Minute = nextAlarm_Minute + duration ; 
            delay(1000);
          }
          else {}
        }
        else
         {
          if (nextAlarm_Minute < 60 && nextAlarm_Minute >= 58)
          {nextAlarm_Minute = 0;
          nextAlarm_Hour = nextAlarm_Hour + 1;
          if (nextAlarm_Hour > 23)
          {nextAlarm_Hour = 0;}
          }
          else if (nextAlarm_Minute >= 60) 
          {nextAlarm_Minute = nextAlarm_Minute - 60;
          nextAlarm_Hour = nextAlarm_Hour + 1;
          if (nextAlarm_Hour > 23)
          {nextAlarm_Hour = 0;}
          }
         
          digitalWrite(buzzer, LOW);
         }
        }
       } 
       
       
if (RTC.read(tm))
{
 if (tm.Hour == shiftEnd && tm.Minute == 0 && tm.Second == 5)                                   //for sending the message
    {
      sendMessage(masterNumber);
      delay(1000);
      digitalWrite(buzzer, LOW);                                                                // just in any case
      nextAlarm_Hour = shiftStart;                                                              //setting the next alarm variables
      nextAlarm_Minute = 0;
      EEPROM.write(miss_add, 00);                                                                // resetting the miss
      miss = 0;
    }
    else 
    {
      
      
    }
}

    
  switch(progSelect)                               //  Switch case for the UI
  {
    case 0:                                        // Main screen case
    {
      displayContent();
      
       
      break;
    }

    

    case 1:                                    // Time change case
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time Select Mode");
      lcd.setCursor(0, 1);
      lcd.print("NEXT");
      lcd.setCursor(10, 1);
      lcd.print("SELECT");
      delay(200);
      if(digitalRead(select) == LOW)
       {
         progSelect = 49;
         mode = true;                    // for changing the working of second button
         
       }
       
       break;
    }


    case 2:                           // Card add mode
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Card Add Mode");
      lcd.setCursor(0, 1);
      lcd.print("NEXT");
      lcd.setCursor(10, 1);
      lcd.print("SELECT");
      delay(200);

      

      if(digitalRead(select) == LOW)
       {
         progSelect = 60;
         mode = true;
         
       }
       break;
    }

    case 3:                                  // card delete case
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Card Delete Mode");
      lcd.setCursor(0, 1);
      lcd.print("NEXT");
      lcd.setCursor(10, 1);
      lcd.print("SELECT");
      delay(200);

      if(digitalRead(select) == LOW)
       {
         progSelect = 70;
         mode = true;
         
       }
       break;
    }


    case 4:                              // change number case
    {
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Change Number");
      lcd.setCursor(0, 1);
      lcd.print("NEXT");
      lcd.setCursor(10, 1);
      lcd.print("SELECT");
      delay(200);

      if(digitalRead(select) == LOW)
       {
         progSelect = 80;
         mode = true;
         
       }
       break;
    }
    case 5:                                // Miss reset case
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("To reset the miss");
      lcd.setCursor(0, 1);
      lcd.print("NEXT");
      lcd.setCursor(10, 1);
      lcd.print("SELECT");
      delay(200);

      if(digitalRead(select) == LOW)
       {
         progSelect = 90;
         mode = true;
         
       }
       break;
    }

     case 6 :                                               // its a monitoring window for next alarm settings
    {                                                      // not a main feature of the system
      lcd.clear();
      lcd.print(nextAlarm_Hour);
      lcd.print(":");
      lcd.print(nextAlarm_Minute);
      delay(200);
      break ;
      
    }

    case 49:                                             // Check if master is present
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Scan the master");
      lcd.setCursor(0,1);
      lcd.print("card");

      while(!getID());
      boolean checkMaster = isMaster(readCard);
      delay(1500);
      if(checkMaster == true)
      {
        progSelect = 50;
        break;
      }

      else
      {
        progSelect = 0;
        mode = false;
        break;
      }

      break;
    }
    

    case 50:                                     //For changing the chift start time
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Select start time");
      lcd.setCursor(0,1);
      lcd.print("INC");
      lcd.setCursor(5, 1);
      lcd.print(shiftStart);
      lcd.setCursor(10, 1);
      lcd.print("SELECT");
      delay(200);

      if(digitalRead(program) == LOW)
      {
        if (shiftStart > 22)
        {
          shiftStart = 0;
      
        }
        else
        {
          shiftStart++;
          delay(300);
        }
      }

      if(digitalRead(select) == LOW)
      {
        EEPROM.write(shiftStart_add, shiftStart);
        progSelect = 51;
      }
      break;

      
    }

    case 51:                                     //for changing the shift end
    {
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Select end time");
      lcd.setCursor(0,1);
      lcd.print("INC");
      lcd.setCursor(5, 1);
      lcd.print(shiftEnd);
      lcd.setCursor(10, 1);
      lcd.print("SELECT");
      delay(200);

      if(digitalRead(program) == LOW)
      {
        if (shiftEnd > 22)
        {
          shiftEnd = 0;
      
        }
        else
        {
          shiftEnd++;
          delay(300);
        }
      }

      if(digitalRead(select) == LOW)
      {
        
        EEPROM.write(shiftEnd_add, shiftEnd);
        progSelect = 52;
      }
      break;

    }


    


    case 52:                                               // for changing the time duration
    {
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Select duration time ");
      lcd.setCursor(0,1);
      lcd.print("INC");
      lcd.setCursor(5, 1);
      lcd.print(duration);
      lcd.setCursor(10, 1);
      lcd.print("SELECT");
      delay(200);

      if(digitalRead(program) == LOW)                                                     // loop for duration
      {
        if (duration > 59)
        {
          duration = 3;
      
        }
        else
        {
          duration++;
          delay(200);
        }
      }

      if(digitalRead(select) == LOW)                                                              // if select button is pressed
      {
        if (shiftStart < shiftEnd){if (RTC.read(tm)){ presentHour = tm.Hour;                      // from here it is checked if the prsent time comes in the active alarm duration
   if(presentHour >= shiftStart && presentHour < shiftEnd)                                        // if yes
  {
     if (RTC.read(tm)){                                                                           // then next alarm will buzz afer the duration minutes
      nextAlarm_Hour = tm.Hour;
      nextAlarm_Minute = tm.Minute + duration;
      
  }
  
   
  }
  else                                                                                            // if no
   {
      nextAlarm_Hour = EEPROM.read(shiftStart_add);                                               // next alarm will buzz when shift starts
      nextAlarm_Minute = 0;
    }
  }}
  if(shiftStart > shiftEnd)
  {if (RTC.read(tm)){ presentHour = tm.Hour;
   if(presentHour >= shiftStart || presentHour < shiftEnd)
    {
      if (RTC.read(tm))
      {
        nextAlarm_Hour = tm.Hour;
        nextAlarm_Minute = tm.Minute + duration;
      }
     
  }
    else
    {
      nextAlarm_Hour = EEPROM.read(shiftStart_add);
      nextAlarm_Minute = 0;
    }
     
    }}

    
    
        lcd.clear();
        lcd.setCursor(4,0);
        lcd.print("SUCCESS!");
        delay(1000);
        EEPROM.write(duration_add, duration);                                               // storing the duration in the EEPROM
        mode = false;                                                                       // selecting mode to false
        progSelect = 0;                                                                      // setting switch to 0
      }
      break;

      
    }

    case 60:                                                           //  for adding the card
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Scan the master");
      lcd.setCursor(0,1);
      lcd.print("card");

      while(!getID());
      boolean checkMaster = isMaster(readCard);
      delay(1500);
      if(checkMaster == true)
      {
        
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("Now scan card");
        lcd.setCursor(2,1);
        lcd.print("to be added!");
        while(getID() != 1);
        if(findID(readCard) == true)
        {
          lcd.clear();
          lcd.print("Card Exist");
          delay(1500);
          progSelect = 0;
          mode = false;
          break;
        }
        writeID(readCard);
        mode = false;
        progSelect = 0;
      }

      else 
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("INVALID CARD");
        delay(1000);
        mode = false;
        progSelect = 0;
        
      }
      break;
    }

    case 70:                                                 // for deleting the card
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Scan the master");
      lcd.setCursor(0,1);
      lcd.print("card");

      while(!getID());
      boolean checkMaster = isMaster(readCard);
      if(checkMaster == true)
      {
        
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("Now scan card");
        lcd.setCursor(2,1);
        lcd.print("to be deleted!");
        while(getID() != 1);
        deleteID(readCard);
        delay(500);
        mode = false;
        progSelect = 0;
        
      }

      else 
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("INVALID CARD");
        delay(1000);
        mode = false;
        progSelect = 0;
        
      }
      break;
      
    }


    case 80:                                                   //for changing the master number
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Scan the master");
      lcd.setCursor(0,1);
      lcd.print("card");

      while(!getID());
      boolean checkMaster = isMaster(readCard);
      if(checkMaster == true)
      {
        while(z < 10)                                 // for a 10 digit number add 
        {
        if(digitalRead(program) == LOW)
        {
          if(character > 8)
          {
            character = 0;
          }

          else
          {
            character++;
            delay(500);
          }
        }
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print ("Enter char no.");
        lcd.print(z+1);
        lcd.setCursor(5,1);
        lcd.print(character);
        
        lcd.setCursor(0,1);
        lcd.print("INC");
        lcd.setCursor(10, 1);
        lcd.print("SELECT");
        delay(200);

        if(digitalRead(select) == LOW)
        {
          masterNumber[z] = character;
          EEPROM.write(masterNumber_add + z, character);
          z++;
          delay(500);
          if(z >= 10)
          {
            lcd.clear();
            for(int i = 0; i < 10; i++)
            {
              lcd.print(masterNumber[i]);
            }
        lcd.setCursor(5,1);
        lcd.print("DONE!");
        delay(3000);
        mode = false;
        progSelect = 0;
          }
        }
        
      }}

      else 
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("INVALID CARD");
        delay(1000);
        mode = false;
        progSelect = 0;
      }
      break;
      }

      case 90:                                              //for resetting the miss
      {
        flushCard(readCard);
        lcd.clear();
        lcd.print("Scan Master card");
        lcd.setCursor(0,1);
        lcd.print("to reset the miss");
        while(!getID());
        if(isMaster(readCard) == true)
        {
          EEPROM.write(miss_add, 0);
          miss = EEPROM.read(miss_add);
          mode = false;
          progSelect = 0;
        }

        else
        {
          lcd.clear();
          lcd.print("Invalid Card");
          mode = false;
          progSelect = 0;
          
        }
      }
  }
}




