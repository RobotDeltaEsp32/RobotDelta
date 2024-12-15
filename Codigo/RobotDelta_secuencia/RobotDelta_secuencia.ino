#define paso_1 15
#define dire_1 2  
#define paso_2 13    
#define dire_2 12    
#define paso_3 25   
#define dire_3 26      
#define sensor_1 33
#define sensor_2 35
#define sensor_3 32

int R = 60;
int r = 35;
int L1 = 60;
int L2 = 90;
int retardo = 2000; 

int secuenciaX[4] = {-30, -30, 30, 30};
int secuenciaY[4] = {-30,  30, 30,-30};
int secuenciaZ[4] = {-90, -90,-90,-90};

void secuencia();
void cinematica(int x, int y, int z, int grado1, int grado2, int grado3);

void setup() {
  pinMode(paso_1, OUTPUT); pinMode(dire_1, OUTPUT);
  pinMode(paso_2, OUTPUT); pinMode(dire_2, OUTPUT);
  pinMode(paso_3, OUTPUT); pinMode(dire_3, OUTPUT);

  Serial.begin(115200);
  Serial.println("Iniciando secuencia...");
}

void loop() {
  secuencia();
  delay(4000);
}

void secuencia() {
  for (int i = 0; i < 4; i++) { 
    int s1 = analogRead(sensor_1);
    int s2 = analogRead(sensor_2);
    int s3 = analogRead(sensor_3);

    int grado1 = map(s1, 3200, 1790, 0, 90);
    int grado2 = map(s2, 3160, 1810, 0, 90);
    int grado3 = map(s3, 3160, 2000, 0, 90);

    Serial.print("Ejecutando paso "); Serial.println(i + 1);
    Serial.print("X: "); Serial.print(secuenciaX[i]);
    Serial.print(", Y: "); Serial.print(secuenciaY[i]);
    Serial.print(", Z: "); Serial.println(secuenciaZ[i]);

    cinematica(secuenciaX[i], secuenciaY[i], secuenciaZ[i], grado1, grado2, grado3);
    delay(4000);
  }
}

void cinematica(int x, int y, int z, int grado1, int grado2, int grado3) {
  float a1 = -2*z*L1;
  float b1 = 2*L1*(x*cos(0)+y*sin(0)+r-R);
  float c1 = pow(x+cos(0)*(r-R),2)+pow(y+sin(0)*(r-R),2)+pow(z,2)+pow(L1,2)-pow(L2,2);

  float a2 = -2*z*L1;
  float b2 = 2*L1*(x*cos(radians(120))+y*sin(radians(120))+r-R);
  float c2 = pow(x+cos(radians(120))*(r-R),2)+pow(y+sin(radians(120))*(r-R),2)+pow(z,2)+pow(L1,2)-pow(L2,2);

  float a3 = -2*z*L1;
  float b3 = 2*L1*(x*cos(radians(240))+y*sin(radians(240))+r-R);
  float c3 = pow(x+cos(radians(240))*(r-R),2)+pow(y+sin(radians(240))*(r-R),2)+pow(z,2)+pow(L1,2)-pow(L2,2);

  float q1rad = asin(c1/sqrt(pow(a1,2)+pow(b1,2)))-atan(b1/a1);
  float q2rad = asin(c2/sqrt(pow(a2,2)+pow(b2,2)))-atan(b2/a2);
  float q3rad = asin(c3/sqrt(pow(a3,2)+pow(b3,2)))-atan(b3/a3);

  int q1 = q1rad*(180/PI);
  int q2 = q2rad*(180/PI);
  int q3 = q3rad*(180/PI);
  int pasos1 = floor((abs(q1-grado1)/1.8)*16);
  int pasos2 = floor((abs(q2-grado2)/1.8)*16);
  int pasos3 = floor((abs(q3-grado3)/1.8)*16);

  Serial.print("q1: "); Serial.println(q1);
  Serial.print("sensor1: "); Serial.println(grado1);
  Serial.print("q2: "); Serial.println(q2);
  Serial.print("sensor2: "); Serial.println(grado2);
  Serial.print("q3: "); Serial.println(q3);
  Serial.print("sensor3: "); Serial.println(grado3);
  Serial.println("---------------------------------");

  if(q1>grado1 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90) {
    digitalWrite(dire_1, HIGH);
    for(int i1=0; i1<=pasos1; i1++){
      digitalWrite(paso_1, HIGH);
      delayMicroseconds(retardo);
      digitalWrite(paso_1, LOW);
      delayMicroseconds(retardo);
    }
  } 
  if(q1<grado1 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire_1, LOW);
    for(int j1=0; j1<=pasos1; j1++){
      digitalWrite(paso_1, HIGH);
      delayMicroseconds(retardo);
      digitalWrite(paso_1, LOW);
      delayMicroseconds(retardo);
    }
  }
  if(q2>grado2 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire_2, HIGH);
    for(int i2=0; i2<=pasos2; i2++) {
      digitalWrite(paso_2, HIGH);
      delayMicroseconds(retardo);
      digitalWrite(paso_2, LOW);
      delayMicroseconds(retardo);
    }
  } 
  if(q2<grado2 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire_2, LOW);
    for(int j2=0; j2<=pasos2; j2++){
      digitalWrite(paso_2, HIGH);
      delayMicroseconds(retardo);
      digitalWrite(paso_2, LOW);
      delayMicroseconds(retardo);
    }
  }
  if(q3>grado3 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire_3, HIGH);
    for(int i3=0; i3<=pasos3; i3++){
      digitalWrite(paso_3, HIGH);
      delayMicroseconds(retardo);
      digitalWrite(paso_3, LOW);
      delayMicroseconds(retardo);
    }
  } 
  if(q3<grado3 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire_3, LOW);
    for(int j3=0; j3<=pasos3; j3++){
      digitalWrite(paso_3, HIGH);
      delayMicroseconds(retardo);
      digitalWrite(paso_3, LOW);
      delayMicroseconds(retardo);
    }
  }
}