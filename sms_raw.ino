#include <SoftwareSerial.h> 
SoftwareSerial GPRS(7, 8); 
String text; 
const int relay = 13; 
String CellNumtemp,CellNum;
int zapros = -1; 

void setup() {  
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH); 

  Serial.begin(9600); 
  GPRS.begin(9600);  
  delay(5000);
  Serial.print("GPRS ready...\r\n");
  GPRS.print("AT+CMGF=1\r\n"); 
  delay(1000);
  GPRS.print("AT+CNMI=2,2,0,0,0\r\n"); 
  delay(1000);
}

void loop(){
  
  if(GPRS.available()>0){ 
    text = GPRS.readString();
    Serial.print(text);    
    delay(10);
  
    if(text.indexOf("+CMT:")>0){ 
      CellNumtemp = text.substring(text.indexOf("+77")); 
      CellNum = CellNumtemp.substring(0,12); 
      Serial.println(CellNum); 
      
      if(CellNum == "+XXXXXXXXXXX"){ 
        if(text.indexOf("ON")>=0){ 
          digitalWrite(relay, HIGH); 
          SMS_send("Lamp set to ON\r","+XXXXXXXXXXX"); 
        }
      
        else if(text.indexOf("OFF")>=0){ 
          digitalWrite(relay, LOW);
          SMS_send("Lamp set to OFF\r","+XXXXXXXXXXX");
        }
  
        if(text.indexOf("ALLOW")>=0){ 
          digitalWrite(relay, zapros);  
            switch (zapros) {
              case 0:
                SMS_send("Lamp set to OFF\r","+XXXXXXXXXXX");
                break;
              case 1:
                SMS_send("Lamp set to ON\r","+XXXXXXXXXXX");
                break;
            }
        }
        else if(text.indexOf("NOT")>=0){ 
          SMS_send("Ponyal\r",CellNum); 
        }
      }
      else{
        String temp=""; 
        if(text.indexOf("ON")>=0){ 
          temp="Allow " + CellNum + " to ON the LED?\r";
          SMS_send(temp,"+XXXXXXXXXXX"); 
          zapros=1; 
        }
        else if(text.indexOf("OFF")>=0){ 
          temp="Allow " + CellNum + " to OFF the LED?\r";
          SMS_send(temp,"+XXXXXXXXXXX");
          zapros=0;
        }
        else{
          SMS_send("lol what?\r",CellNum);
        }
      }
      CellNumtemp="";
      CellNum=""; 
    }
    text = ""; 
  }
}

void SMS_send(String content,String receiver_number){ 
    GPRS.println(String("AT+CMGS=\"") + receiver_number + "\""); 
    delay(500);
    GPRS.print(content + "\r"); 
    delay(500);
    GPRS.write( 0x1a ); 
    delay(1000);
}
