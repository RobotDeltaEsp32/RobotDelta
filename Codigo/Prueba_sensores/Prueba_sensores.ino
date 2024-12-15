#define sensor_1 33
#define sensor_2 35
#define sensor_3 32
int s1;
int s2;
int s3;

void setup() {
  Serial.begin(115200);
}

void loop() {
  s1 = analogRead(sensor_1);
  s2 = analogRead(sensor_2);
  s3 = analogRead(sensor_3);
  
  int grado1 = map(s1, 3200, 1790, 0, 90);
  int grado2 = map(s2, 3160, 1810, 0, 90);
  int grado3 = map(s3, 3160, 2000, 0, 90);
  Serial.print("sensor1: "); Serial.println(s1);
  Serial.print("sensor2: "); Serial.println(s2);
  Serial.print("sensor3: "); Serial.println(s3);
  Serial.println("---------------------------------");
  
  delay(2000);
}