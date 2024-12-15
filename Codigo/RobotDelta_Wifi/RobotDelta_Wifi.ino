#include <WiFi.h>
#include <WiFiMulti.h>

#define paso1 15
#define dire1 2  
#define paso2 13    
#define dire2 12    
#define paso3 25   
#define dire3 26      
#define sensor_1 33
#define sensor_2 35
#define sensor_3 32

int s1, s2, s3;
int grado1;
int grado2;
int grado3;
int pasos1;
int pasos2;
int pasos3;

int R = 60;
int r = 35;
int L1 = 60;
int L2 = 90;
int retardo = 2000;

WiFiMulti wifiMulti;
WiFiServer servidor(80);

String modo = "cordenadas"; 
float X = 0.0, Y = 0.0, Z = -90.0; 
float q1 = 57.0, q2 = 57.0, q3 = 57.0;

bool bandera = false;

void calculo_de_cordenadas();
void calculo_de_q();
void pasopaso();
void control();
void calculo_de_s();

void setup() {
    pinMode(paso1, OUTPUT);
    pinMode(dire1, OUTPUT);
    pinMode(paso2, OUTPUT);
    pinMode(dire2, OUTPUT);
    pinMode(paso3, OUTPUT);
    pinMode(dire3, OUTPUT);

    Serial.begin(115200);
    Serial.println("\nIniciando WiFi Multi");

    wifiMulti.addAP("mazzocato", "leon6734");
    wifiMulti.addAP("MATIAS", "Emily123");

    WiFi.mode(WIFI_STA);
    Serial.print("Conectando a WiFi...");
    while (wifiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000); 
    }
    if (WiFi.localIP() == INADDR_NONE || WiFi.localIP().toString() == "0.0.0.0") {
        Serial.println("\nError: No se obtuvo una IP válida. Reiniciando...");
        ESP.restart(); // Reinicia el microcontrolador
    }
    Serial.println("\nConectado a WiFi");
    Serial.print("IP local: ");
    Serial.println(WiFi.localIP());
    servidor.begin();
    Serial.println("Servidor iniciado correctamente");
}

void loop() {
  WiFiClient cliente = servidor.available();
  if (cliente) {
    Serial.println("Nuevo cliente conectado");
    String peticion = cliente.readStringUntil('\r');
    cliente.flush();
    int q1Index = peticion.indexOf("q1=");
    int q2Index = peticion.indexOf("q2=");
    int q3Index = peticion.indexOf("q3=");
    int xIndex = peticion.indexOf("X=");
    int yIndex = peticion.indexOf("Y=");
    int zIndex = peticion.indexOf("Z=");
    if (q1Index!=q1 && q2Index!=q2 && q3Index!=q3 && xIndex!=X && yIndex!=Y && zIndex!=Z){
      if (peticion.indexOf("GET /modo/cordenadas") >= 0) {
        modo = "cordenadas";
      }else if (peticion.startsWith("GET /actualizar?")) {
        s1 = analogRead(sensor_1);
        s2 = analogRead(sensor_2);
        s3 = analogRead(sensor_3);
        
        if (q1Index >= 0) q1 = peticion.substring(q1Index + 3, peticion.indexOf('&', q1Index)).toFloat();
        if (q2Index >= 0) q2 = peticion.substring(q2Index + 3, peticion.indexOf('&', q2Index)).toFloat();
        if (q3Index >= 0) q3 = peticion.substring(q3Index + 3, peticion.indexOf('&', q3Index)).toFloat();
        if (xIndex >= 0) X = peticion.substring(xIndex + 2, peticion.indexOf('&', xIndex)).toFloat();
        if (yIndex >= 0) Y = peticion.substring(yIndex + 2, peticion.indexOf('&', yIndex)).toFloat();
        if (zIndex >= 0) Z = peticion.substring(zIndex + 2, peticion.indexOf('&', zIndex)).toFloat();

        bandera = true;
      }
      enviarPaginaWeb(cliente);
      if (modo == "cordenadas") {
        if (bandera == true) {
          calculo_de_s(); 
          control();
          bandera = false;
        }
      }
    }
    cliente.stop();
    Serial.println("Cliente desconectado");
  }
}

void enviarPaginaWeb(WiFiClient &cliente) {
  cliente.println("HTTP/1.1 200 OK");
  cliente.println("Content-Type: text/html; charset=UTF-8");
  cliente.println("Connection: close");
  cliente.println();

  cliente.println("<!DOCTYPE HTML>");
  cliente.println("<html>");
  cliente.println("<head>");
  cliente.println("<style>");
  cliente.println("button { background-color: #4CAF50; color: white; padding: 10px; margin: 5px; }");
  cliente.println("input { width: 50px; }");
  cliente.println("</style>");
  cliente.println("</head>");
  cliente.println("<body>");

  if (modo == "cordenadas") {
    cliente.println("<h2>Modo: Coordenadas</h2>");
    cliente.println("<form>");
    cliente.println("<p>X=<input type='text' id='X' value='" + String(X, 2) + "'></p>");
    cliente.println("<p>Y=<input type='text' id='Y' value='" + String(Y, 2) + "'></p>");
    cliente.println("<p>Z=<input type='text' id='Z' value='" + String(Z, 2) + "'></p>");
    cliente.println("<p>q1=" + String(q1, 2) + "</p>");
    cliente.println("<p>q2=" + String(q2, 2) + "</p>");
    cliente.println("<p>q3=" + String(q3, 2) + "</p>");
    cliente.println("</form>");
  } else {
    cliente.println("<h2>Error: Modo no soportado</h2>");
  }
  cliente.println("<button onclick=\"actualizar()\">Actualizar</button>");
  cliente.println("<script>");
  cliente.println("function actualizar() {");
  cliente.println("  let params = '';");

  cliente.println("  if (!document.getElementById('X').value || !document.getElementById('Y').value || !document.getElementById('Z').value) {");
  cliente.println("    alert('Por favor, ingrese todos los valores.');");
  cliente.println("    return;");
  cliente.println("  }");

  cliente.println("  params += 'X=' + document.getElementById('X').value + '&';");
  cliente.println("  params += 'Y=' + document.getElementById('Y').value + '&';");
  cliente.println("  params += 'Z=' + document.getElementById('Z').value + '&';");

  cliente.println("  fetch('/actualizar?' + params).then(response => location.reload());");
  cliente.println("}");
  cliente.println("</script>");

  cliente.println("</body>");
  cliente.println("</html>");
}


void calculo_de_q() {
  float a1 = -2*Z*L1;
  float b1 = 2*L1*(X*cos(0)+Y*sin(0)+r-R);
  float c1 = pow(X+cos(0)*(r-R),2)+pow(Y+sin(0)*(r-R),2)+pow(Z,2)+pow(L1,2)-pow(L2,2);

  float a2 = -2*Z*L1;
  float b2 = 2*L1*(X*cos(radians(120))+Y*sin(radians(120))+r-R);
  float c2 = pow(X+cos(radians(120))*(r-R),2)+pow(Y+sin(radians(120))*(r-R),2)+pow(Z,2)+pow(L1,2)-pow(L2,2);

  float a3 = -2*Z*L1;
  float b3 = 2*L1*(X*cos(radians(240))+Y*sin(radians(240))+r-R);
  float c3 = pow(X+cos(radians(240))*(r-R),2)+pow(Y+sin(radians(240))*(r-R),2)+pow(Z,2)+pow(L1,2)-pow(L2,2);

  float q1rad = asin(c1/sqrt(pow(a1,2)+pow(b1,2)))-atan(b1/a1);
  float q2rad = asin(c2/sqrt(pow(a2,2)+pow(b2,2)))-atan(b2/a2);
  float q3rad = asin(c3/sqrt(pow(a3,2)+pow(b3,2)))-atan(b3/a3);

  q1 = (int)(q1rad*(180/PI));
  q2 = (int)(q2rad*(180/PI));
  q3 = (int)(q3rad*(180/PI));
}

void pasopaso() {
  pasos1 = floor((abs(q1-grado1)/1.8)*16);
  pasos2 = floor((abs(q2-grado2)/1.8)*16);
  pasos3 = floor((abs(q3-grado3)/1.8)*16);
  Serial.print("pasos1: "); Serial.print(pasos1);
  Serial.print(" pasos2: "); Serial.print(pasos2);
  Serial.print(" pasos3: "); Serial.println(pasos3);
  if(q1>grado1 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire1, HIGH);
    for(int i1=0; i1<=pasos1; i1++){
      digitalWrite(paso1, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso1, LOW);       
      delayMicroseconds(retardo);
    }
  }
  if(q2>grado2 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire2, HIGH);
    for(int i2=0; i2<=pasos2; i2++){
      digitalWrite(paso2, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso2, LOW);       
      delayMicroseconds(retardo);
    }
  }
  if(q3>grado3 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire3, HIGH); 
    for(int i3=0; i3<=pasos3; i3++){
      digitalWrite(paso3, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso3, LOW);       
      delayMicroseconds(retardo);
    }
  }
  if(q1<grado1 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire1, LOW);
    for(int j1=0; j1<=pasos1; j1++){
      digitalWrite(paso1, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso1, LOW);       
      delayMicroseconds(retardo);
    }
  }
  if(q2<grado2 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire2, LOW);  
    for(int j2=0; j2<=pasos2; j2++){
      digitalWrite(paso2, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso2, LOW);       
      delayMicroseconds(retardo);
    }
  }
  if(q3<grado3 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire3, LOW); 
    for(int j3=0; j3<=pasos3; j3++){
      digitalWrite(paso3, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso3, LOW);       
      delayMicroseconds(retardo);
    }
  }
  Serial.print("s1: "); Serial.print(grado1);
  Serial.print(" s2: "); Serial.print(grado2);
  Serial.print(" s3: "); Serial.println(grado3);
}

void calculo_de_s() {
  grado1 = map(s1, 3200, 1790, 0, 90);
  grado2 = map(s2, 3160, 1810, 0, 90);
  grado3 = map(s3, 3160, 2000, 0, 90);

  Serial.print("s1: "); Serial.print(grado1);
  Serial.print(" s2: "); Serial.print(grado2);
  Serial.print(" s3: "); Serial.println(grado3);
}
void control() {
  if (modo == "cordenadas") {
    calculo_de_q();  
    pasopaso();
  }
}