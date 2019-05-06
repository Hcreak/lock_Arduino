int M_EN = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(5,INPUT_PULLUP);
  pinMode(14,OUTPUT);
  pinMode(12,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
    while (Serial.available() > 0) {
    int val = Serial.parseInt();
    Serial.println(val);
    if (val==1)
      {
        digitalWrite(14,HIGH);
        digitalWrite(12,LOW);
        delay(250);
        digitalWrite(14,LOW);
        digitalWrite(12,LOW);
      }
     if (val==0)
      {
        digitalWrite(14,LOW);
        digitalWrite(12,LOW);
      }
     if (val==2)
      {
        digitalWrite(14,LOW);
        digitalWrite(12,HIGH);
        delay(250);
        digitalWrite(14,LOW);
        digitalWrite(12,LOW);
        M_EN = 0;
      }
    }

    int Hall = 0;
    for (int i=0;i<5;i++) { 
      Hall+=digitalRead(5);
      delay(200);
    }

    if (Hall == 5) {
      Serial.println("1*5,UNLOCK");
      if (M_EN == 0) {
        Serial.println("UNLOCK");
        M_EN = 1;
      }
    }
    
    if (Hall == 0) {
      Serial.println("0*5,LOCK");
      if (M_EN == 1) {
        digitalWrite(14,HIGH);
        digitalWrite(12,LOW);
        delay(250);
        digitalWrite(14,LOW);
        digitalWrite(12,LOW);
        M_EN = 0;
      }
    }
    int val = digitalRead(5);
    Serial.println(val);
}
