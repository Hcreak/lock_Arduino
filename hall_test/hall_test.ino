void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(5,INPUT_PULLUP);
//  pinMode(4,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
 
    int val = digitalRead(5);
    Serial.println(val);
    delay(500); 
}
