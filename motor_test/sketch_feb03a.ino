void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
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
      }
  }
}
