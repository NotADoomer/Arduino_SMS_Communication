#include <SoftwareSerial.h> 
SoftwareSerial GPRS(7, 8); //Инициируем объект нашей библиотеки с портами 7,8 - они дефолтные для щилда, в мини версию уже меняете по желанию смотря как подключите его

String text; //Переменная для содержания сообщения
const int relay = 13; //Пин светодиода
String CellNumtemp,CellNum; //Переменные для хранения номера гостя
int zapros = -1; //Позже поймете для чего эта переменная, что бы не запутаться в запросах номера-гостя

void setup() {  
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH); // The current state of the light is ON

  Serial.begin(9600); 
  GPRS.begin(9600);  //Запускаем объект
  delay(5000);
  Serial.print("GPRS ready...\r\n");
  GPRS.print("AT+CMGF=1\r\n");  //AT команда для перевода СМС в текстовый режим
  delay(1000);
  GPRS.print("AT+CNMI=2,2,0,0,0\r\n"); //AT команда для получения СМС в реальном времени, то есть после этого он ожидает сообщение не читая прошлые
  delay(1000);
}

void loop(){
  
  if(GPRS.available()>0){ //Ждем как только СМС будет достпуно(приход сообщения), это решающее условие
    text = GPRS.readString(); //Сохраняем СМС полностью в переменную 
    Serial.print(text);    //Печатаем его
    delay(10);
  
    if(text.indexOf("+CMT:")>0){ //Проверка на то что это СМС, оно всегда так начинается, но так то это опциональный if 
      CellNumtemp = text.substring(text.indexOf("+77")); //находим сабстринг с номером, если еще межд. номера хотите то тут уже с кавычками надо пошаманить
      CellNum = CellNumtemp.substring(0,12); //сохраняем этот сабстринг с индекс 0 до 12 то есть полный номер
      Serial.println(CellNum); //Печатаем его
      
      if(CellNum == "+77073388601"){ //если это хозяин то выполнять запросы сразу же
        if(text.indexOf("ON")>=0){ //Запрос на включение
          digitalWrite(relay, HIGH); //Включаем светодиод
          SMS_send("Lamp set to ON\r","+77073388601"); //Отправляем сообщение с помощью моей функции. С начала пишем содержание СМС потом номер получателя
        }
      
        else if(text.indexOf("OFF")>=0){ //Запрос на выключение
          digitalWrite(relay, LOW);
          SMS_send("Lamp set to OFF\r","+77073388601");
        }
  
        if(text.indexOf("ALLOW")>=0){ //Это уже разрешения для другого номера
          digitalWrite(relay, zapros);  //ВКЛ или ВЫКЛ смотря на переменную
            switch (zapros) {//Смотрим на запрос гостя и в зависимости от него отправляем СМС хозяину
              case 0:
                SMS_send("Lamp set to OFF\r","+77073388601");//Отправляем сообщение с помощью моей функции дабы держать в курсе что светодиод выключен
                break;
              case 1:
                SMS_send("Lamp set to ON\r","+77073388601");
                break;
            }
        }
        else if(text.indexOf("NOT")>=0){ //Если хозяин не дал разрешения
          SMS_send("Ponyal\r",CellNum); //Ответное сообщение
        }
      }
      else{ //Если сообщение отправил не хозяин а гость
        String temp=""; //Просто переменная по идее можно без нее но с ней удобнее читать то что ниже
        if(text.indexOf("ON")>=0){ 
          temp="Allow " + CellNum + " to ON the LED?\r";
          SMS_send(temp,"+77073388601"); //Отправить хозяину запрос на разрешение включения светодиода от такого-то какого-то номера
          zapros=1; //записываем что его запрос 1 на след. цикле loop нам это пригодиться что бы после разрешения понять выключить ЛЕД или включить
        }
        else if(text.indexOf("OFF")>=0){ //тут тоже самое только на выключение
          temp="Allow " + CellNum + " to OFF the LED?\r";
          SMS_send(temp,"+77073388601");
          zapros=0;
        }
        else{//Если не корректная команда от гостя то ответить Лол что?
          SMS_send("lol what?\r",CellNum);
        }
      }
      CellNumtemp="";
      CellNum=""; //опусташаем номера для следующего цикла loop
    }
    text = ""; //опусташаем содержание смс для следующего цикла loop
  }
}

void SMS_send(String content,String receiver_number){ //функция для отправки СМС с параметрами содержания и номер получателя
    GPRS.println(String("AT+CMGS=\"") + receiver_number + "\""); //Вводим номер в нужную для нас AT команду
    delay(500);
    GPRS.print(content + "\r"); //Вводим содержание сообщения
    delay(500);
    GPRS.write( 0x1a ); //Вроде это нужно для уточнения формата отправки что ли, либо что-то связанное с байтами
    delay(1000);
}
