#include <WiFi.h>
#include <WiFiMulti.h>

// Definiciones de pines
#define paso1 15
#define dire1 2  
#define paso2 13    
#define dire2 12    
#define paso3 25   
#define dire3 26      
#define sensor_1 33
#define sensor_2 35
#define sensor_3 32

int s1, s2, s3; // Valores calculados de los sensores
int grado1;
int grado2;
int grado3;
int pasos1;
int pasos2;
int pasos3;

// Constantes del robot
int R = 60;
int r = 35;
int L1 = 60;
int L2 = 90;
int retardo = 2000;

float secuenciaX[10];
float secuenciaY[10]; 
float secuenciaZ[10]; 


WiFiMulti wifiMulti;
WiFiServer servidor(80);

// Variables globales
String modo = "cordenadas"; 
float X = 0.0, Y = 0.0, Z = -90.0; // Coordenadas cartesianas
float q1 = 50.0, q2 = 50.0, q3 = 50.0; // Ángulos de las juntas
bool bandera = false;

// Valores de seteo para 0° y 90°
int sensor1_0 = 3200, sensor1_90 = 1790;
int sensor2_0 = 3160, sensor2_90 = 1810;
int sensor3_0 = 3160, sensor3_90 = 2000;

// Declaración de funciones
void calculo_de_cordenadas();
void calculo_de_q();
void pasopaso();
void control();
void calculo_de_s();
void enviarPaginaWeb(WiFiClient &cliente);
void secuencia();

void setup() {
    // Configuración de pines como salida
    pinMode(paso1, OUTPUT);
    pinMode(dire1, OUTPUT);
    pinMode(paso2, OUTPUT);
    pinMode(dire2, OUTPUT);
    pinMode(paso3, OUTPUT);
    pinMode(dire3, OUTPUT);

    // Inicialización de la comunicación serie
    Serial.begin(115200);
    Serial.println("\nIniciando WiFi Multi");

    // Configuración de redes Wi-Fi
    wifiMulti.addAP("mazzocato", "leon6734");
    wifiMulti.addAP("MATIAS", "Emily123");
    //wifiMulti.addAP("SEBASTIAN", "12349876");

    // Configurar el modo WiFi como STA (Station)
    WiFi.mode(WIFI_STA);
    Serial.print("Conectando a WiFi...");
    while (wifiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000); // Espera hasta que se establezca la conexión
    }

    // Validar si se obtuvo una IP válida
    if (WiFi.localIP() == INADDR_NONE || WiFi.localIP().toString() == "0.0.0.0") {
        Serial.println("\nError: No se obtuvo una IP válida. Reiniciando...");
        ESP.restart(); // Reinicia el microcontrolador
    }

    Serial.println("\nConectado a WiFi");
    Serial.print("IP local: ");
    Serial.println(WiFi.localIP());

    // Iniciar el servidor
    servidor.begin();
    Serial.println("Servidor iniciado correctamente");
}

void loop() {
    WiFiClient cliente = servidor.available();
    if (cliente) {
        Serial.println("Nuevo cliente conectado");
        String peticion = cliente.readStringUntil('\r');
        cliente.flush();

        // Cambiar entre modos según la solicitud
        if (peticion.indexOf("GET /modo/cordenadas") >= 0) {
            modo = "cordenadas";
        } else if (peticion.indexOf("GET /modo/joins") >= 0) {
            modo = "joins";
        } else if (peticion.indexOf("GET /modo/seteo") >= 0) {
            modo = "seteo";
        } else if (peticion.indexOf("GET /modo/secuencia") >= 0) {
            modo = "secuencia";
        }  else if (peticion.indexOf("GET /guardar?sensor=") >= 0) {
    // Extraer parámetros de la solicitud
    int sensorIndex = peticion.indexOf("sensor=");
    int posicionIndex = peticion.indexOf("posicion=");

    String sensor = peticion.substring(sensorIndex + 7, peticion.indexOf('&', sensorIndex));
    String posicion = peticion.substring(posicionIndex + 9);

    int valor = 0;

    if (sensor == "1") {
        valor = analogRead(sensor_1);
        if (posicion == "0") sensor1_0 = valor;
        else if (posicion == "90") sensor1_90 = valor;
    } else if (sensor == "2") {
        valor = analogRead(sensor_2);
        if (posicion == "0") sensor2_0 = valor;
        else if (posicion == "90") sensor2_90 = valor;
    } else if (sensor == "3") {
        valor = analogRead(sensor_3);
        if (posicion == "0") sensor3_0 = valor;
        else if (posicion == "90") sensor3_90 = valor;
    }

    // Enviar una respuesta simple
    cliente.println("HTTP/1.1 200 OK");
    cliente.println("Content-Type: text/plain");
    cliente.println("Connection: close");
    cliente.println();
    cliente.println("OK");

    Serial.println("Valor guardado correctamente");
        } else if (peticion.startsWith("GET /actualizar?")) {

            // Extraer parámetros de la solicitud
            int q1Index = peticion.indexOf("q1=");
            int q2Index = peticion.indexOf("q2=");
            int q3Index = peticion.indexOf("q3=");
            int xIndex = peticion.indexOf("X=");
            int yIndex = peticion.indexOf("Y=");
            int zIndex = peticion.indexOf("Z=");
            int s1_0Index = peticion.indexOf("sensor1_0=");
            int s1_90Index = peticion.indexOf("sensor1_90=");
            int s2_0Index = peticion.indexOf("sensor2_0=");
            int s2_90Index = peticion.indexOf("sensor2_90=");
            int s3_0Index = peticion.indexOf("sensor3_0=");
            int s3_90Index = peticion.indexOf("sensor3_90=");

            if (q1Index >= 0) q1 = peticion.substring(q1Index + 3, peticion.indexOf('&', q1Index)).toFloat();
            if (q2Index >= 0) q2 = peticion.substring(q2Index + 3, peticion.indexOf('&', q2Index)).toFloat();
            if (q3Index >= 0) q3 = peticion.substring(q3Index + 3, peticion.indexOf('&', q3Index)).toFloat();
            if (xIndex >= 0) X = peticion.substring(xIndex + 2, peticion.indexOf('&', xIndex)).toFloat();
            if (yIndex >= 0) Y = peticion.substring(yIndex + 2, peticion.indexOf('&', yIndex)).toFloat();
            if (zIndex >= 0) Z = peticion.substring(zIndex + 2, peticion.indexOf('&', zIndex)).toFloat();
            bandera = false;
        }

        // Enviar la página web al cliente
        enviarPaginaWeb(cliente);

        // Control según el modo seleccionado
        if (!bandera) {
            calculo_de_s();
            control();
            bandera = true;
        }

        cliente.stop();
        Serial.println("Cliente desconectado");
    }
}

void enviarPaginaWeb(WiFiClient &cliente) {
    // Encabezados HTTP
    cliente.println("HTTP/1.1 200 OK");
    cliente.println("Content-Type: text/html; charset=UTF-8");
    cliente.println("Connection: close");
    cliente.println();

    // Página web
    cliente.println("<!DOCTYPE HTML>");
    cliente.println("<html>");
    cliente.println("<head>");
    cliente.println("<style>");
    cliente.println("button { background-color: #4CAF50; color: white; padding: 10px; margin: 5px; }");
    cliente.println("input { width: 50px; }");
    cliente.println("</style>");
    cliente.println("</head>");
    cliente.println("<body>");

    // Botones para cambiar de modo
    cliente.println("<button onclick=\"location.href='/modo/seteo'\">Modo: Seteo</button>");
    cliente.println("<button onclick=\"location.href='/modo/cordenadas'\">Modo: Coordenadas</button>");
    cliente.println("<button onclick=\"location.href='/modo/joins'\">Modo: Joins</button>");
    cliente.println("<button onclick=\"location.href='/modo/secuencia'\">Modo: Secuencia</button>");

    // Mostrar campos según el modo
    if (modo == "cordenadas") {
        cliente.println("<h2>Modo: Coordenadas</h2>");
        cliente.printf("<p>X=<input type='text' id='X' value='%.2f'></p>", X);
        cliente.printf("<p>Y=<input type='text' id='Y' value='%.2f'></p>", Y);
        cliente.printf("<p>Z=<input type='text' id='Z' value='%.2f'></p>", Z);
        cliente.printf("<p>q1=%.2f</p>", q1);
        cliente.printf("<p>q2=%.2f</p>", q2);
        cliente.printf("<p>q3=%.2f</p>", q3);
    } else if (modo == "joins") {
        cliente.println("<h2>Modo: Joins</h2>");
        cliente.printf("<p>q1=<input type='text' id='q1' value='%.2f'></p>", q1);
        cliente.printf("<p>q2=<input type='text' id='q2' value='%.2f'></p>", q2);
        cliente.printf("<p>q3=<input type='text' id='q3' value='%.2f'></p>", q3);
        cliente.printf("<p>S1=%.2f</p>", s1);
        cliente.printf("<p>S2=%.2f</p>", s2);
        cliente.printf("<p>S3=%.2f</p>", s3);
    } else if (modo == "seteo") {
        cliente.println("<h3>Seteo de Sensores</h3>");

        // Sensor 1
        cliente.println("<div>");
        cliente.printf("<p>Sensor 1 = <strong>%d</strong></p>", analogRead(sensor_1));
        cliente.println("<button onclick=\"guardarSensor(1, '0')\">0°</button>");
        cliente.println("<button onclick=\"guardarSensor(1, '90')\">90°</button>");
        cliente.printf("<p>S1 = %d &nbsp;&nbsp; 0° = <strong>%d</strong> &nbsp;&nbsp; 90° = <strong>%d</strong></p>", 
                       s1, sensor1_0, sensor1_90);
        cliente.println("</div>");

        // Sensor 2
        cliente.println("<div>");
        cliente.printf("<p>Sensor 2 = <strong>%d</strong></p>", analogRead(sensor_2));
        cliente.println("<button onclick=\"guardarSensor(2, '0')\">0°</button>");
        cliente.println("<button onclick=\"guardarSensor(2, '90')\">90°</button>");
        cliente.printf("<p>S2 = %d &nbsp;&nbsp; 0° = <strong>%d</strong> &nbsp;&nbsp; 90° = <strong>%d</strong></p>", 
                       s2, sensor2_0, sensor2_90);
        cliente.println("</div>");

        // Sensor 3
        cliente.println("<div>");
        cliente.printf("<p>Sensor 3 = <strong>%d</strong></p>", analogRead(sensor_3));
        cliente.println("<button onclick=\"guardarSensor(3, '0')\">0°</button>");
        cliente.println("<button onclick=\"guardarSensor(3, '90')\">90°</button>");
        cliente.printf("<p>S3 = %d &nbsp;&nbsp; 0° = <strong>%d</strong> &nbsp;&nbsp; 90° = <strong>%d</strong></p>", 
                       s3, sensor3_0, sensor3_90);
        cliente.println("</div>");

        // JavaScript para guardar valores
        cliente.println("<script>");
        cliente.println("function guardarSensor(sensor, posicion) {");
        cliente.println("  fetch(`/guardar?sensor=` + sensor + `&posicion=` + posicion)");
        cliente.println("    .then(response => {");
        cliente.println("      if (response.ok) {");
        cliente.println("        alert(`Valor guardado: Sensor ${sensor} en ${posicion}°.`);");
        cliente.println("        location.reload();");
        cliente.println("      } else {");
        cliente.println("        alert('Error al guardar el valor del sensor.');");
        cliente.println("      }");
        cliente.println("    });");
        cliente.println("}");
        cliente.println("</script>");
    } else if (modo == "secuencia") {
        cliente.println("<h2>Modo: Secuencia</h2>");
        cliente.println("<form>");
        cliente.println("<table>");
        cliente.println("<tr><th>Paso</th><th>X</th><th>Y</th><th>Z</th></tr>");
        for (int i = 0; i < 10; i++) {
            cliente.println("<tr>");
            cliente.printf("<td>%d</td>", i + 1);
            cliente.printf("<td><input type='text' id='X%d' value='%.2f'></td>", i, secuenciaX[i]);
            cliente.printf("<td><input type='text' id='Y%d' value='%.2f'></td>", i, secuenciaY[i]);
            cliente.printf("<td><input type='text' id='Z%d' value='%.2f'></td>", i, secuenciaZ[i]);
            cliente.println("</tr>");
        }
        cliente.println("</table>");
        cliente.println("<button type='button' onclick='actualizarSecuencia()'>Guardar Secuencia</button>");
        cliente.println("</form>");
    }

    // Botón de actualizar (excepto en modo seteo)
    if (modo != "seteo") {
        cliente.println("<button onclick=\"actualizar()\">Actualizar</button>");
    }

    // JavaScript para Actualizar
    cliente.println("<script>");
    cliente.println("function actualizar() {");
    cliente.println("  let params = '';");

    if (modo == "cordenadas") {
        cliente.println("  params += 'X=' + document.getElementById('X').value + '&';");
        cliente.println("  params += 'Y=' + document.getElementById('Y').value + '&';");
        cliente.println("  params += 'Z=' + document.getElementById('Z').value;");
    } else if (modo == "joins") {
        cliente.println("  params += 'q1=' + document.getElementById('q1').value + '&';");
        cliente.println("  params += 'q2=' + document.getElementById('q2').value + '&';");
        cliente.println("  params += 'q3=' + document.getElementById('q3').value;");
    }
    cliente.println("  fetch('/actualizar?' + params).then(response => location.reload());");
    cliente.println("}");
    cliente.println("</script>");

    cliente.println("</body>");
    cliente.println("</html>");
}


void control() {
    if (modo == "cordenadas") {
        calculo_de_q();
        pasopaso();
    } else if (modo == "joins") {
        //calculo_de_cordenadas();
        pasopaso();
    } else if (modo == "secuencia"){
        //secuencia();
    }
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
    digitalWrite(dire1, HIGH);   // direccion de giro 0
    for(int i1=0; i1<=pasos1; i1++){
      digitalWrite(paso1, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso1, LOW);       
      delayMicroseconds(retardo);
    }
  }
  if(q2>grado2 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire2, HIGH);   // direccion de giro 0
    for(int i2=0; i2<=pasos2; i2++){
      digitalWrite(paso2, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso2, LOW);       
      delayMicroseconds(retardo);
    }
  }
  if(q3>grado3 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire3, HIGH);   // direccion de giro 0
    for(int i3=0; i3<=pasos3; i3++){
      //Serial.println("Direccion giro 1");
      digitalWrite(paso3, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso3, LOW);       
      delayMicroseconds(retardo);
    }
  }
  if(q1<grado1 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire1, LOW);   // direccion de giro 0
    for(int j1=0; j1<=pasos1; j1++){
      digitalWrite(paso1, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso1, LOW);       
      delayMicroseconds(retardo);
    }
  }
  if(q2<grado2 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire2, LOW);   // direccion de giro 0
    for(int j2=0; j2<=pasos2; j2++){
      digitalWrite(paso2, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso2, LOW);       
      delayMicroseconds(retardo);
    }
  }
  if(q3<grado3 && q1>=0 && q1<=90 && q2>=0 && q2<=90 && q3>=0 && q3<=90){
    digitalWrite(dire3, LOW);   // direccion de giro 0
    for(int j3=0; j3<=pasos3; j3++){
      //Serial.println("Direccion giro 2");
      digitalWrite(paso3, HIGH);      
      delayMicroseconds(retardo);          
      digitalWrite(paso3, LOW);       
      delayMicroseconds(retardo);
    }
  }
  Serial.print("s1: "); Serial.print(s1);
  Serial.print(" s2: "); Serial.print(s2);
  Serial.print(" s3: "); Serial.println(s3);
}

void calculo_de_s() {
  grado1 = map(s1, 3200, 1790, 0, 90);
  grado2 = map(s2, 3160, 1810, 0, 90);
  grado3 = map(s3, 3160, 2000, 0, 90);

  Serial.print("s1: "); Serial.print(grado1);
  Serial.print(" s2: "); Serial.print(grado2);
  Serial.print(" s3: "); Serial.println(grado3);
}

void secuencia() {
  for (int i = 0; i < 10; i++) {
    // Actualizar coordenadas para este paso
    X = secuenciaX[i];
    Y = secuenciaY[i];
    Z = secuenciaZ[i];

    // Calcular ángulos y mover motores
    calculo_de_q();
    pasopaso();

    // Agregar un retardo opcional entre pasos
    delay(1000);
  }
}