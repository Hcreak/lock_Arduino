void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(14,INPUT);
//  pinMode(4,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
 
    int val = digitalRead(14);
    Serial.println(val);
    delay(500); 
}
