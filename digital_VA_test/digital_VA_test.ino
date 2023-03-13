const int VA_PIN = 4;
void setup() {
  // put your setup code here, to run once:
  pinMode(VA_PIN, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(VA_PIN, HIGH);
  delay(1000);
  digitalWrite(VA_PIN, LOW);
  delay(2000);
  

}
