///////////////////////////libraries//////////////////////////
#include <EEPROM.h>
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>


//////////////////////defined parameter///////////////////////
#define masterFinger_Refrence_add  1
#define locationTaken 0xAB
#define maxNum 6
#define shiftStart_add 900
#define shiftEnd_add 901
#define duration_add 902
#define miss_add 903
#define masterNumber_add 904
#define nextAlarm_Hour_add 930
#define nextAlarm_Minute_add 931


/////////////////////Objects/////////////////////////////
LiquidCrystal lcd(12, 11, A0, A1, A2, A3);
tmElements_t tm;
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//////////////////veriables///////////////////////////////
int character = 0;
int z = 0;
int verified = 0;
int masterNumber[10] = {0};
int p;
int program = 5;
int select = 6;
int buzzer = 7;
int progSelect = 0;
int modeSelect = 0;
int nextAlarm_Hour = 0;
int nextAlarm_Minute = 0;
int duration = 0;
int miss = 0;
int shiftStart = 0;
int shiftEnd = 0;
int presentHour = 0;
int presentMinute = 0;
boolean mode = false;


//////////////////////////Void Setup/////////////////////////
void setup()  
{
  lcd.begin(16, 2);
  Serial.begin(9600);

  fingerPrintInitialize();

  if (EEPROM.read(1) != locationTaken)
  {
    lcd.clear();
    lcd.print("Master fingr not defined");
    delay(1500);
    lcd.clear();
    lcd.print("Kindly define Master");
    delay(1500);
    fingerEnroll(1);
    EEPROM.write(1, locationTaken);
    lcd.clear();
    lcd.print("Master defined");
    delay(1500);
  }
  pinMode(buzzer, OUTPUT);
  pinMode(program, INPUT);
  pinMode(select, INPUT);
  digitalWrite(program, HIGH);
  digitalWrite(select, HIGH);
   shiftStart = EEPROM.read(shiftStart_add);
  shiftEnd = EEPROM.read(shiftEnd_add);
  duration = EEPROM.read(duration_add);
  miss = EEPROM.read(miss_add);

    for(int i = 0; i < 10; i++)
  {
    masterNumber[i] = EEPROM.read(904 + i);
  }

    if (RTC.read(tm)){
  presentHour = tm.Hour;
  presentMinute = tm.Minute;}
 if (shiftStart < shiftEnd){
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
}


////////////////////////////////Finger Print Initialize///////////////
void fingerPrintInitialize()
{
  finger.begin(57600);

  if (finger.verifyPassword()) {
    lcd.clear();
    lcd.print("Fngerprnt sensor!");
    delay(500);
  } else {
    lcd.clear();
    Serial.println("No fngerprnt sensor");
    while (1);
  }
}


///////////////////////// To send message /////////////////////
boolean sendMessage(int x[])
{
  Serial.println("AT+CMGF=1");
  delay(500);
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



////////////////////////////////Enroll///////////////////////////
uint8_t fingerEnroll(int id)
{
  int p = -1;
  lcd.print("Waiting for fingr");
  while(p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p){
      case FINGERPRINT_OK:{
      lcd.clear();
      lcd.print("Image taken :)");
      delay(1500);
      break;
      }

      case FINGERPRINT_NOFINGER:{
      lcd.clear();
      lcd.print(".");
      delay(1500);
      break;}
      
      case FINGERPRINT_PACKETRECIEVEERR:{
      lcd.clear();
      lcd.print("Communication Error");
      delay(1500);
      break;}

      case FINGERPRINT_IMAGEFAIL:{
      lcd.clear();
      lcd.print("Imaging Error");
      delay(1500);
      break;}

      default:{
      lcd.clear();
      lcd.print("Unknown error");
      delay(1500);
      break;}
      
      }
      
  }
  p = finger.image2Tz(1);
  switch (p){
    case FINGERPRINT_OK:{
    lcd.clear();
    lcd.print("Image Converter");
    delay(1500);
    break;}

    case FINGERPRINT_IMAGEMESS:{
    lcd.clear();
    lcd.print("Image too messy");
    delay(1500);
    return p;}

    case FINGERPRINT_PACKETRECIEVEERR:{
    lcd.clear();
    lcd.print("Communication Error");
    delay(1500);
    return p;}

    case FINGERPRINT_FEATUREFAIL:{
    lcd.clear();
    lcd.print("Fingerprint Fail");
    delay(1500);
    return p;}

    case FINGERPRINT_INVALIDIMAGE:{
    lcd.clear();
    lcd.print("FingerPrint Fail");
    delay(1500);
    return p;}

    default:{
    lcd.clear();
    lcd.print("Unknown Error");
    delay(1500);
    return p;}
  }

  lcd.clear();
  lcd.print("Remove Finger");
  delay(2000);
  p = 0;
  while(p != FINGERPRINT_NOFINGER){
    p = finger.getImage();
  }
  p = -1;
  lcd.clear();
  lcd.print("Plce same fingr");
  delay(2000);
  while (p != FINGERPRINT_OK){
    p = finger.getImage();
    switch (p){
      case FINGERPRINT_OK:{
      lcd.clear();
      lcd.print("Image Taken");
      delay(1500);
      break;}

      case FINGERPRINT_NOFINGER:{
      lcd.clear();
      lcd.print(".");
      delay(1500);
      break;}

      case FINGERPRINT_PACKETRECIEVEERR:{
      lcd.clear();
      lcd.print("Communication Error");
      delay(1500);
      break;}

      case FINGERPRINT_IMAGEFAIL:{
      lcd.clear();
      lcd.print("Imaging Error");
      delay(1500);
      break;}

      default:{
      lcd.clear();
      lcd.print("Unknown Error");
      delay(1500);
      break;}
    }
  }
  p = finger.image2Tz(2);
   switch (p) {
    case FINGERPRINT_OK:{
      lcd.clear();
      lcd.print("Image Converted");
      delay(1500);
      break;
      }

    case FINGERPRINT_IMAGEMESS:{
       lcd.clear();
       lcd.print("Image too messy");
       delay(1500);
       return p;}

       case FINGERPRINT_PACKETRECIEVEERR:{
       lcd.clear();
       lcd.print("Communication error");
       delay(1500);
       return p;}

       case FINGERPRINT_FEATUREFAIL:{
       lcd.clear();
       lcd.print("No Finger");
       delay(1500);
       return p;}

       case FINGERPRINT_INVALIDIMAGE:{
       lcd.clear();
       lcd.print("No Finger");
       delay(1500);
       return p;}

        default:{
          lcd.clear();
          lcd.print("Unknown Error");
          return p;}
    }
    lcd.clear();
    lcd.print("Creating Model");
    delay(500);

    p = finger.createModel();
    if (p == FINGERPRINT_OK) {lcd.clear();
    lcd.print("Prints matched!");
    delay(1500);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {lcd.clear();
    lcd.print("Communication error");
    delay(1500);
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {lcd.clear();
    lcd.print("Fingerprints did not match");
    delay(1500);
    return p;
  } else {lcd.clear();
    lcd.print("Unknown error");
    delay(1500);
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {lcd.clear();
  lcd.print("Stored!");
  delay(1500);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {lcd.clear();
    lcd.print("Communication error");
    delay(1500);
    return p;
    } else if (p == FINGERPRINT_BADLOCATION) {lcd.clear();
      lcd.print("Bad Location");
      delay(1500);
      return p;
      } else if (p == FINGERPRINT_FLASHERR) {lcd.clear();
    lcd.print("Error writing");
    delay(1500);
    return p;
    } else {lcd.clear();
    lcd.print("Unknown error");
    delay(1500);
    return p;
  }
}


///////////////////////////////delete Finger Print//////////////////
uint8_t deleteFingerprint(int id){
  uint8_t p = -1;
   p = finger.deleteModel(id);

   if (p == FINGERPRINT_OK){lcd.clear();
   lcd.print("Deleted");
   delay(1500);
   } else if (p == FINGERPRINT_PACKETRECIEVEERR) {lcd.clear();
   lcd.print("Communication Error");
   delay(1500);
   return p;
    } else if (p == FINGERPRINT_BADLOCATION) {lcd.clear();
    lcd.print("Bad Location");
    delay(1500);
    return p;
    } else if (p == FINGERPRINT_FLASHERR) {lcd.clear();
    lcd.print("Error Writing");
    delay(1500);
    return p;
    } else {lcd.clear();
    lcd.print("Unknown error");lcd.setCursor(0, 1); lcd.print(p, HEX);
    delay(1500);
    return p;
  }
}

////////////////////////////////find empty Location///////////////
int findEmptyLoc(void){
  int i = 1;
  for(i = 1; i <= 250; i++)
  {
    if(EEPROM.read(i) != locationTaken)
    {
      return i;
    }
  }
  return 0;
}


//////////////////////////////Find ID of scanned Finger//////////////
int getFingerprintId()
{
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  lcd.clear();
  lcd.print("Found ID #"); lcd.print(finger.fingerID);
  delay (1500);
  return finger.fingerID;

}


///////////////////////////for printing 2 digits atleast////////////////
void print2digits(int number) 
{
  if (number >= 0 && number < 10) {
    lcd.print('0');
  }
  lcd.print(number);
}


////////////////////////taking time from rtc//////////////////////
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
}}


//////////////////////for displaying contents of home screen////////////////////
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



///////////////////////////////void loop///////////////////////////
void loop (){
  if (digitalRead(program) == LOW && mode == false )
  {
    if (progSelect < maxNum && progSelect != 50){
    progSelect++;
    delay(200);}

    else
    {progSelect = 0;}
  }


  if (RTC.read(tm))
{
 if (tm.Hour == shiftEnd && tm.Minute == 0 && tm.Second == 5)
    {
      sendMessage(masterNumber);
      delay(1000);
      digitalWrite(buzzer, LOW);
      nextAlarm_Hour = shiftStart;
      nextAlarm_Minute = 0;
      EEPROM.write(miss_add, 00);
      miss = 0;
    }
    else 
    {
      
      
    }
}


if (RTC.read(tm)){
      presentHour = tm.Hour;
      presentMinute = tm.Minute;}





      if (shiftStart > shiftEnd)                                                                                 // Alarm Conditions
       {
        if(presentHour >= shiftStart || presentHour < shiftEnd)                                                  //condition if shift start is bigger than shift
        {
          
          if (nextAlarm_Minute < 58){                                                                                           // for next alarm minute smaller than 58
          if(presentMinute >= nextAlarm_Minute && presentMinute < nextAlarm_Minute + 2 && presentHour == nextAlarm_Hour)        // condition for buzzing the alarm
          {
            
            if (RTC.read(tm))
            {
              presentHour = tm.Hour;
              presentMinute = tm.Minute;
            }
            digitalWrite(buzzer, HIGH);
            while (finger.getImage() == FINGERPRINT_NOFINGER && presentMinute < nextAlarm_Minute + 2)                       // check if there is any finger print is detected
            {
               if (RTC.read(tm))
               {
                presentHour = tm.Hour;
                presentMinute = tm.Minute;
               }
                lcd.clear();
                lcd.print("Scan Your Finger");
                delay(200);
            }
           if (finger.getImage() == FINGERPRINT_OK){                             // scan for a fingerprint
           if (finger.image2Tz() == FINGERPRINT_OK){
            if(finger.fingerFastSearch() == FINGERPRINT_OK)
            {
              nextAlarm_Minute = nextAlarm_Minute + EEPROM.read(duration_add); 
              digitalWrite(buzzer, LOW);
            }}}

            else {}
          }
          if(presentMinute == nextAlarm_Minute + 2 && presentHour == nextAlarm_Hour && presentHour != shiftEnd)          // if no finger is detected in 2 minutes
          {
            miss++;
            EEPROM.write(miss_add, miss);
            digitalWrite(buzzer, LOW);
             if (RTC.read(tm))
            {
              presentHour = tm.Hour;
              presentMinute = tm.Minute;
            }
            nextAlarm_Minute = nextAlarm_Minute + duration ; 
            delay(1000);
          }
          else {}
         } 
         else
         {
          if (nextAlarm_Minute < 60 && nextAlarm_Minute >= 58)                                                    // if next alarm minute is in between 58 and 59
          {nextAlarm_Minute = 0;
          nextAlarm_Hour = nextAlarm_Hour + 1;
          if (nextAlarm_Hour > 23)
          {nextAlarm_Hour = 0;}
          }
          else if (nextAlarm_Minute >= 60)                                                                        // if next alarm minute is greater than 59
          {nextAlarm_Minute = nextAlarm_Minute - 60;
          nextAlarm_Hour = nextAlarm_Hour + 1;
          if (nextAlarm_Hour > 23)
          {nextAlarm_Hour = 0;}
          }
         
          digitalWrite(buzzer, LOW);
         }
         }
        }
       

       else if(shiftStart <= shiftEnd)
       {
        if(presentHour >= shiftStart && presentHour < shiftEnd)                                                                   // if shift start is smaller than shift end
        {
          if(nextAlarm_Minute < 58){
          if(presentMinute >= nextAlarm_Minute && presentMinute < nextAlarm_Minute + 2 && presentHour == nextAlarm_Hour)
          {
            
            if (RTC.read(tm))
            {
              presentHour = tm.Hour;
              presentMinute = tm.Minute;
            }
            digitalWrite(buzzer, HIGH);
            while (finger.getImage() == FINGERPRINT_NOFINGER && presentMinute < nextAlarm_Minute + 2)
            {
              lcd.clear();
              lcd.print("Scan Your finger");
               if (RTC.read(tm))
               {
                presentHour = tm.Hour;
                presentMinute = tm.Minute;
               }
            }
            if (finger.getImage() == FINGERPRINT_OK){
           if (finger.image2Tz() == FINGERPRINT_OK){
            if(finger.fingerFastSearch() == FINGERPRINT_OK)
            {
              nextAlarm_Minute = nextAlarm_Minute + EEPROM.read(duration_add); 
              digitalWrite(buzzer, LOW);
            }}}
          }
          if(presentMinute == nextAlarm_Minute + 2 && presentHour == nextAlarm_Hour && presentHour != shiftEnd)
          {
             if (RTC.read(tm))
            {
              presentHour = tm.Hour;
              presentMinute = tm.Minute;
            }
            else {}
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

  switch(progSelect)
  {
    case 0:                                                 // the main display
    {
      displayContent();
      break;
    }

    
    case 1:                                                // display for Time select mode
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time Select Mode");
      lcd.setCursor(0, 1);
      lcd.print("NEXT");
      lcd.setCursor(10, 1);
      lcd.print("SELECT");
      delay(200);
      if(digitalRead(select) == LOW)                        // if select button is pressed 
       {
         progSelect = 49;                                   // for jumping to case 50
         mode = true;
         
       }
       
       break;
    }


    case 2:                                                 // for selecting the card add mode
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Figer Add Mode");
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


    case 3:                                            // for selecting the card delete mode
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Finger Delete Mode");
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


    case 4:                                                  // for selecting the master number change mode
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


    case 5:                                                    // for resetting the miss
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


    case 6 :
    {                                                                // its a monitoring window for next alarm settings
      lcd.clear();
      lcd.print(nextAlarm_Hour);                                      // not a main feature of the system
      lcd.print(":");
      lcd.print(nextAlarm_Minute);
      delay(200);
      break ;
      
    }


    case 49:
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Scan the master");
      lcd.setCursor(0,1);
      lcd.print("Finger");
      p = 0;
      
      delay(3000);
      int checkMaster = getFingerprintId(); 
   
      if(checkMaster == 1)
      {
        progSelect = 50;
      break;}

      else
      {lcd.clear();
      lcd.print("INVALID FINGER");
      delay(1500);
        progSelect = 0;
        mode = false;
        break;}


      break;
    }
    case 50:                                                           // window for selecting the shift start opens when the time select mode is selected
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

      if(digitalRead(program) == LOW)                                   // loop for incrementing the shiftstart variable
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

      if(digitalRead(select) == LOW)                                    // selecting the current value to shift start
      {
        EEPROM.write(shiftStart_add, shiftStart);                       // Storing the data to the EEPROM
        progSelect = 51;                                                //  for chandging the switch case
        break;
      }break;

      
    }

    case 51:                                                            // comes after the shift start is selected
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

      if(digitalRead(program) == LOW)                                 // looping the time for shift end
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

      if(digitalRead(select) == LOW)                                // Selecting the current value as shift end
      {
        
        EEPROM.write(shiftEnd_add, shiftEnd);                       //Storing the value of EEPROM
        progSelect = 52;                                             // Changing the switch case for selecting the duration
      }
      break;

    }


    case 52:                                              // Switch case for duration selection
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

      if(digitalRead(program) == LOW)
      {
        if (duration > 59)                            // loop for duration
        {
          duration = 3;
      
        }
        else
        {
          duration++;
          delay(200);
        }
      }

      if(digitalRead(select) == LOW)                                                             // if select button is pressed
      {
        if (shiftStart < shiftEnd){if (RTC.read(tm)){ presentHour = tm.Hour;                      // from here it is checked if the prsent time comes in the active alarm duration
   if(presentHour >= shiftStart && presentHour < shiftEnd)                                         // if yes
  {
     if (RTC.read(tm)){
      nextAlarm_Hour = tm.Hour;
      nextAlarm_Minute = tm.Minute + duration;                                                      // then next alarm will buzz afer the duration minutes
      
  }
  
   
  }
  else
   {
      nextAlarm_Hour = EEPROM.read(shiftStart_add);                                                          // if no
      nextAlarm_Minute = 0;                                                                                  // next alarm will buzz when shift starts
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
        EEPROM.write(duration_add, duration);                                                // storing the duration in the EEPROM
        mode = false;                                                                        // selecting mode to false
        progSelect = 0;                                                                      // setting switch to 0
      }
      break;

    }
    case 60:                                                                                   // Opens when Card select mode
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Scan the master");
      lcd.setCursor(0,1);
      lcd.print("Finger");
      p = 0;
      
      delay(3000);
      int checkMaster = getFingerprintId();                                                                    //wait for a finger
      delay(1500);                                                                                             // check if master
      if(checkMaster == 1)                                                                                     //if true
      {
        
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Now scan finger");
        lcd.setCursor(2,1);
        lcd.print("to be added!");
        delay(3000);
        p = 0;
        while(p == FINGERPRINT_NOFINGER)                                                                    // wait for finger
        {p = finger.getImage();}
        if(getFingerprintId() <= 250 && getFingerprintId() >= 1)                                          // check if finger exist
        {
          lcd.clear();
          lcd.print("Finger Exist");
          delay(1500);
          progSelect = 0;
          mode = false;
          break;
        }
        int id = findEmptyLoc();                                                               // check for empty location
        fingerEnroll(id);                                                                       // enroll the finger
        EEPROM.write(id, locationTaken);
        mode = false;
        progSelect = 0;
      }

      else                                                                                 // if not a master finger
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("INVALID FINGER");
        delay(1000);
        mode = false;
        progSelect = 0;
        
      }
      break;
    }


    case 70:                                                                  // for deleting the finger id
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Scan the master");
      lcd.setCursor(0,1);
      lcd.print("finger");
      delay(3000);
      p = 0;
      while(p == FINGERPRINT_NOFINGER)
      {p = finger.getImage();}
      int checkMaster = getFingerprintId();
      if(checkMaster == 1)
      {
        
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("Now scan finger");
        lcd.setCursor(2,1);
        lcd.print("to be deleted!");
        delay(3000);
        p = 0;
        while(p == FINGERPRINT_NOFINGER)                                          // det the id number of scanned id
        {p = finger.getImage();}
        deleteFingerprint(getFingerprintId());                                   //delete the id
        EEPROM.write(getFingerprintId(), 0x02);                                 // for refrence that the id is empty
        lcd.clear();
        lcd.print("ID Removed");
        delay(500);
        mode = false;
        progSelect = 0;
        
      }

      else 
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("INVALID FINGER");
        delay(1000);
        mode = false;
        progSelect = 0;
        
      }
      break;
      
    }


    case 80:                                                               // for changing the master number
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Scan the master");
      lcd.setCursor(0,1);
      lcd.print("finger");
      delay(3000);
      int checkMaster = getFingerprintId();
      if(checkMaster == 1)
      {
        while(z < 10)
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
        delay(1000);
        mode = false;
        progSelect = 0;
          }
        }
        
      }}

      else 
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("INVALID FINGER");
        delay(1000);
        mode = false;
        progSelect = 0;
      }
      break;
      }


      case 90:                                                                              // for resetting miss
      {
        lcd.clear();
        lcd.print("Scan Master finger");
        lcd.setCursor(0,1);
        lcd.print("to rest the miss");
        delay(3000);
        p = 0;
        while(p == FINGERPRINT_NOFINGER)
        {p = finger.getImage();}
        if(getFingerprintId() == 1)
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
