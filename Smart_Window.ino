#include <DHT11.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>
#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

//함수선언
void IR_Remote();//리모컨
void i2C_lcd();//i2c lcd
void rain();//빗물감지
void DC_motor(); //모터

//i2C 연결설정
LiquidCrystal_I2C lcd(0x27,16,2);
  
//적외선 연결설정
int RECV_PIN = 11;
boolean automode = false;
boolean window_opened = false;

//온습도센서 연결설정
int pin=A2;
DHT11 dht11(pin);
int err;
float temp, humi;

// DC모터 설정
  int a = 3;
  int b = 6;
  int speed = A9;  //모터속도조절

//빗물감지센서 연결설정
int readingSensor = A0;

IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  irrecv.enableIRIn();
  pinMode(12,OUTPUT);
  pinMode(a,OUTPUT);
  pinMode(b,OUTPUT);  

 //xTaskCreate( DC_motor, NULL, configMINIMAL_STACK_SIZE, NULL, 2, NULL );
  xTaskCreate( rain, NULL, configMINIMAL_STACK_SIZE, NULL, 1, NULL );
  xTaskCreate( IR_Remote, NULL, configMINIMAL_STACK_SIZE, NULL, 1, NULL ); 
  xTaskCreate( i2C_lcd, NULL, configMINIMAL_STACK_SIZE, NULL, 2, NULL );
  
  vTaskStartScheduler();
  while(1) ;
}

//////////////////////////////////////////////////////////
/*
void DC_motor(){

  while(1){
    analogWrite(speed, 150); //9
    digitalWrite(a,HIGH);
    digitalWrite(b,LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(a,LOW);
    digitalWrite(b,HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
*/
//////////////////////////////////////////////////////////
void rain(){
  while(1){
    Serial.println(analogRead(A0));
    vTaskDelay(1000 / portTICK_PERIOD_MS );
  }
}

//////////////////////////////////////////////////////////
void i2C_lcd() {
  while(1) {        
  if((err=dht11.read(humi, temp))==0)
  {
    
    lcd.display();
    lcd.print("TEMP:     ");
    lcd.print(temp,1);
    lcd.print(" C deg");
    lcd.setCursor(0,1);
    lcd.print("HUMIDITY: ");
    lcd.print(humi,0);
    lcd.print(" %   ");
    lcd.println();
 
    Serial.print("temp: ");
    Serial.print(temp);
    Serial.print("humi: ");
    Serial.print(humi);
    Serial.println();
   }
  else
  {
    
    lcd.display();
    lcd.print("ERROR NO.: ");
    lcd.print(err);
  }
  vTaskDelay(1500 / portTICK_PERIOD_MS );
  lcd.clear();

}
}

//////////////////////////////////////////////////////////
void IR_Remote(){
   digitalWrite(12,automode);
  while(1){
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    //led 출력//
    if(automode == false){
      if(results.value==0xFF629D){
        automode=true;
         digitalWrite(12,automode);
      } else if(results.value==0xFF906F){
        if(window_opened == false){
            window_open();
            Serial.println("window open");
         }
      } else if(results.value==0xFFA857){
          if(window_opened == true){
            window_close();
            Serial.println("window close");
          }
      } else{}
    } else{
      if(results.value==0xFF629D){
        automode = false;
        digitalWrite(12,automode);
      }
    }
    int a;
    if(automode ==true){
      a=1;
    }else{
      a=2;
    }
    Serial.println(a);
    irrecv.resume();  //Receive the next value
  }

  autoFunction();
}
}

void autoFunction(){
    if(automode == true){
      if(analogRead(A0)<400&&window_opened==true){
        window_close();
        Serial.println("close window.");
      }
      if(temp >=27&&window_opened==false){
        window_open();
      }     
    }  
}

void window_open(){
   analogWrite(speed, 150); //9
          digitalWrite(a,HIGH);
          digitalWrite(b,LOW);
          vTaskDelay(535 / portTICK_PERIOD_MS );
          digitalWrite(a,HIGH);
          digitalWrite(b,HIGH);
          window_opened = true;
}

void window_close(){
   analogWrite(speed, 150); //9
          digitalWrite(a,LOW);
          digitalWrite(b,HIGH);
          vTaskDelay(535 / portTICK_PERIOD_MS );
          digitalWrite(a,HIGH);
          digitalWrite(b,HIGH);
          window_opened = false;
}

void loop() {}
