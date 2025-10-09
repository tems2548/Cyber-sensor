#define LED 33
#define sensor 32

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(sensor, INPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  int val = digitalRead(sensor);
  digitalWrite(LED,val);
  delay(100);
}