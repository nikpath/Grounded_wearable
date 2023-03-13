void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("36: " + String(analogRead(36)));
  Serial.println("39: " + String(analogRead(39)));
  Serial.println("34: " + String(analogRead(34)));
  Serial.println("35: " + String(analogRead(35)));
  Serial.println("32: " + String(analogRead(32)));
  Serial.println("33: " + String(analogRead(33)));
  Serial.println("25: " + String(analogRead(25)));
  Serial.println("26: " + String(analogRead(26)));
  Serial.println("27: " + String(analogRead(27)));
  Serial.println("14: " + String(analogRead(14)));
  Serial.println("12: " + String(analogRead(12)));
  Serial.println("13: " + String(analogRead(13)));
  Serial.println("9: " + String(analogRead(9)));
  Serial.println("10: " + String(analogRead(10)));
  Serial.println("11: " + String(analogRead(11)));
  Serial.println("23: " + String(analogRead(23)));
  Serial.println("22: " + String(analogRead(22)));
  Serial.println("1: " + String(analogRead(1)));
  Serial.println("3: " + String(analogRead(3)));
  Serial.println("21: " + String(analogRead(21)));
  Serial.println("19: " + String(analogRead(19)));
  Serial.println("18: " + String(analogRead(18)));
  Serial.println("5: " + String(analogRead(5)));
  Serial.println("4: " + String(analogRead(4)));
  Serial.println("0: " + String(analogRead(0)));
  Serial.println("2: " + String(analogRead(2)));
  Serial.println("15: " + String(analogRead(15)));
  Serial.println("8: " + String(analogRead(8)));
  Serial.println("7: " + String(analogRead(7)));
  Serial.println("6: " + String(analogRead(6)));
  Serial.println("l");
  delay(2000);

}
