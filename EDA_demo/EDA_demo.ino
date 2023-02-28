const int GSR=34;
int threshold=0;
int sensorValue;

void setup(){
  long sum=0;
  Serial.begin(9600);
  delay(1000);
}

void loop(){
  long a =0;
  sensorValue=analogRead(GSR);
  // a = ((1024+2*sensorValue)*10000)/(512-sensorValue);
  Serial.println(sensorValue);
  delay(4);
 }
