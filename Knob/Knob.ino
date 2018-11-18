#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int val = 0;    // variable to read the value from the analog pin
int statu = -2; 

void EXPORT() {
  while(1){
    int Hall = 0;
    for (int i=0;i<5;i++) { 
      Hall+=digitalRead(4);
      delay(10);
    }
    if (Hall == 5) {
//      Serial.println("1*5,UNLOCK");
      if (statu != 0) {
        Serial.println("UNLOCK");
        statu = 0;
      }
      return ;
    }
    if (Hall == 0) {
//      Serial.println("0*5,LOCK");
      if (statu != 1) {
        Serial.println("LOCK");
        statu = 1;
      }
      return ;
    }
  }
}

void setup() {
  Serial.begin(9600);
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  myservo.write(val);
  pinMode(4,INPUT);
  EXPORT();
}

void loop() {
  while (Serial.available() > 0) {   // 串口收到字符数大于零。
    val = Serial.parseInt(); 
    Serial.println(val);
    myservo.write(val);
  }
  EXPORT();
  delay(1000);                        // waits for the servo to get there
}

