#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "INFINITUM19E4"; // Reemplaza con tu SSID
const char* password = "A9S6kGA3Qd"; // Reemplaza con tu contraseña
//const char* ssid = "BUAP_Estudiantes";  // Enter SSID here
//const char* password = "f85ac21de4";  //Enter Password here

WebServer server(80);

const int ledPin1 = 18; // Pin para el LED 1
const int ledPin2 = 5;  // Pin para el LED 2
const int pwmPin = 19;  // Pin para PWM control del brillo del LED
const int analogPin = 34; // Pin para la entrada ADC
const int ipButtonPin = 15; // Pin para el botón que cambia entre IP estática y dinámica

IPAddress staticIP(192, 168, 1, 184); // Dirección IP estática
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

bool useStaticIP = false; // Variable para alternar entre IP estática y dinámica
int lastButtonState = HIGH; // Estado anterior del botón

void setup() {
  Serial.begin(115200);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(ipButtonPin, INPUT_PULLUP); // Configuración del pin del botón

  connectWiFi();

  // Manejo de peticiones
  server.on("/", HTTP_GET, handleRoot);
  server.on("/led", HTTP_GET, handleLedControl);
  server.on("/analog", HTTP_GET, handleAnalogRead);
  server.on("/pwm", HTTP_GET, handlePwmControl); // Control para el brillo del LED por PWM
  server.begin();
}

void loop() {
  server.handleClient();
  
  int buttonState = digitalRead(ipButtonPin);
  if (buttonState == LOW && lastButtonState == HIGH) { // Detecta el cambio de estado del botón
    delay(50); // Anti-rebote
    useStaticIP = !useStaticIP; // Cambia entre IP estática y dinámica
    connectWiFi(); // Reconecta con la nueva configuración de IP
  }
  lastButtonState = buttonState;
}

void connectWiFi() {
  WiFi.disconnect(true); // Desconecta de cualquier conexión previa
  delay(100);

  if (useStaticIP) {
    WiFi.config(staticIP, gateway, subnet);
    Serial.println("Configurando IP estática...");
  } else {
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); // Configuración para IP dinámica
    Serial.println("Configurando IP dinámica...");
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }

  Serial.print("Conectado a WiFi con ");
  Serial.println(useStaticIP ? "IP estática: " : "IP dinámica: ");
  Serial.println(WiFi.localIP());
}

void handleRoot() {
  String html = "<!DOCTYPE html><html lang='es'><head>"
                "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                "<title>Control de LEDs y Entrada Analógica</title>"
                "<style>"
                "body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f0f0f0; text-align: center; }"
                ".container { max-width: 900px; margin: 50px auto; padding: 20px; background-color: #fff; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }"
                "h1 { color: #333; }"
                "button, input[type='text'] { margin: 10px; padding: 10px 20px; font-size: 16px; border-radius: 5px; border: none; background-color: #007BFF; color: white; cursor: pointer; }"
                "input[type='text'] { width: 100px; }"
                "button:hover { background-color: #0056b3; }"
                "img { margin-top: 20px; max-width: 100%; height: auto; border-radius: 10px; }"
                "</style>"
                "</head><body>"
                "<div class='container'>"
                "<h1>Control de LEDs y Entrada Analógica</h1>"

                // Image added here
                "<img src='https://www.talent-network.org/comunidades/wp-content/uploads/2019/07/TN-comunidades-FCE-BUAP.png' alt='BUAP Logo' />"

                "<h3>Control de LED en Pin 18</h3>"
                "<button onclick=\"fetch('/led?pin=18&state=on')\">LED On</button>"
                "<button onclick=\"fetch('/led?pin=18&state=off')\">LED Off</button>"

                "<h3>Control de LED en Pin 5</h3>"
                "<button onclick=\"fetch('/led?pin=5&state=on')\">LED On</button>"
                "<button onclick=\"fetch('/led?pin=5&state=off')\">LED Off</button>"

                "<h3>Control de Brillo (PWM) en Pin 19</h3>"
                "<input type='text' id='pwmInput' placeholder='0-255' />"
                "<button onclick='setPwm()'>Aplicar PWM</button>"
                "<p>Brillo PWM: <span id='pwmValue'>128</span></p>"

                "<h3>Entrada Analógica</h3>"
                "<button onclick=\"getAnalogValue()\">Mostrar Valor</button>"
                "<p id='analogValue'>Valor: -</p>"

                // Muestra la IP y el tipo en la interfaz
                "<h3>Conexión de Red</h3>"
                "<p>IP: " + WiFi.localIP().toString() + "</p>"
                "<p>Tipo de IP: " + String(useStaticIP ? "Estática" : "Dinámica") + "</p>"

                "</div>"
                "<script>"
                "function setPwm() {"
                "  let pwmValue = document.getElementById('pwmInput').value;"
                "  if (!isNaN(pwmValue) && pwmValue >= 0 && pwmValue <= 255) {"
                "    document.getElementById('pwmValue').innerText = pwmValue;"
                "    fetch('/pwm?value=' + pwmValue);"
                "  } else {"
                "    alert('Por favor, ingresa un valor entre 0 y 255');"
                "  }"
                "}"
                "function getAnalogValue() {"
                "  fetch('/analog')"
                "    .then(response => response.text())"
                "    .then(data => {"
                "      document.getElementById('analogValue').innerText = 'Valor: ' + data;"
                "    });"
                "}"
                "</script>"
                "</body></html>";
                
  server.send(200, "text/html", html);
}

void handleLedControl() {
  String pinStr = server.arg("pin");
  String state = server.arg("state");
  
  if (pinStr == "18") {
    digitalWrite(ledPin1, state == "on" ? HIGH : LOW);
    Serial.println("LED en pin 18 " + String(state == "on" ? "encendido" : "apagado"));
  } else if (pinStr == "5") {
    digitalWrite(ledPin2, state == "on" ? HIGH : LOW);
    Serial.println("LED en pin 5 " + String(state == "on" ? "encendido" : "apagado"));
  }
  server.send(200, "text/plain", "LED " + state);
}

void handlePwmControl() {
  String pwmValue = server.arg("value");
  int pwm = pwmValue.toInt();
  analogWrite(pwmPin, pwm); // Ajusta el brillo del LED con PWM
  Serial.println("PWM Value: " + pwmValue);
  server.send(200, "text/plain", "PWM ajustado a " + pwmValue);
}

void handleAnalogRead() {
  int analogValue = analogRead(analogPin);  // Lectura del pin ADC
  Serial.println(analogValue); 
  server.send(200, "text/plain", String(analogValue));
}