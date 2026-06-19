#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WebServer.h>

#define PIR_PIN 14
#define VIBRADOR_PIN 27
#define LDR_PIN 34
#define SONIDO_PIN 35
#define BOTON_PIN 23
#define LED_PIN 4
#define NUM_LEDS 30

const char* ssid = "LIB-7095038";
const char* password = "TU_PASSWORD";

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);

// ===================== CLASE BASE (abstracta) =====================
class Modo {
  protected:
    Adafruit_NeoPixel* tira;
    int pinVibrador;
    
  public:
    Modo(Adafruit_NeoPixel* strip, int vibPin) {
      tira = strip;
      pinVibrador = vibPin;
    }
    
    virtual void ejecutar(int luz, int sonido, int pir) = 0;
    virtual String getNombre() = 0;
};

// ===================== MODO 1: AMBIENTE TRANQUILO =====================
class ModoTranquilo : public Modo {
  public:
    ModoTranquilo(Adafruit_NeoPixel* strip, int vibPin) : Modo(strip, vibPin) {}
    
    void ejecutar(int luz, int sonido, int pir) override {
      int brillo = map(luz, 200, 4095, 0, 255);
      brillo = constrain(brillo, 0, 255);
      tira->setBrightness(brillo);
      tira->fill(tira->Color(255, 120, 20));
      tira->show();
      digitalWrite(pinVibrador, LOW);
    }
    
    String getNombre() override {
      return "Ambiente Tranquilo";
    }
};

// ===================== MODO 2: SOBRECARGA AMBIENTAL =====================
class ModoSobrecarga : public Modo {
  public:
    ModoSobrecarga(Adafruit_NeoPixel* strip, int vibPin) : Modo(strip, vibPin) {}
    
    void ejecutar(int luz, int sonido, int pir) override {
      if (sonido > 1500) {
        tira->setBrightness(40);
        tira->fill(tira->Color(50, 0, 100));
        digitalWrite(pinVibrador, HIGH);
      } else if (sonido > 1300) {
        tira->setBrightness(60);
        tira->fill(tira->Color(100, 50, 150));
        digitalWrite(pinVibrador, LOW);
      } else if (sonido > 1000) {
        tira->setBrightness(70);
        tira->fill(tira->Color(100, 100, 200));
        digitalWrite(pinVibrador, LOW);
      } else {
        tira->setBrightness(80);
        tira->fill(tira->Color(100, 150, 255));
        digitalWrite(pinVibrador, LOW);
      }
      tira->show();
    }
    
    String getNombre() override {
      return "Sobrecarga Ambiental";
    }
};

// ===================== MODO 3: PRESENCIA =====================
class ModoPresencia : public Modo {
  public:
    ModoPresencia(Adafruit_NeoPixel* strip, int vibPin) : Modo(strip, vibPin) {}
    
    void ejecutar(int luz, int sonido, int pir) override {
      if (pir == HIGH) {
        for (int b = 0; b <= 150; b += 10) {
          tira->setBrightness(b);
          tira->fill(tira->Color(255, 130, 60));
          tira->show();
          delay(20);
        }
        digitalWrite(pinVibrador, HIGH);
      } else {
        tira->setBrightness(20);
        tira->fill(tira->Color(40, 20, 10));
        tira->show();
        digitalWrite(pinVibrador, LOW);
      }
    }
    
    String getNombre() override {
      return "Presencia";
    }
};

// ===================== VARIABLES GLOBALES =====================
Modo* modos[3];
int modoActualIndex = 0;
bool botonPresionadoAntes = false;

// ===================== FUNCIONES DE LECTURA =====================
int leerLuz() {
  return analogRead(LDR_PIN);
}

int leerSonido() {
  int maximo = 0;
  for (int i = 0; i < 30; i++) {
    int muestra = analogRead(SONIDO_PIN);
    if (muestra > maximo) maximo = muestra;
  }
  return maximo;
}

// ===================== ENDPOINTS DEL SERVIDOR WEB =====================
// Tu compañero puede llamar a estos endpoints desde la página web (fetch/AJAX)

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>LumaNest</h1>";
  html += "<p>Modo actual: " + modos[modoActualIndex]->getNombre() + "</p>";
  html += "<button onclick=\"fetch('/modo?valor=0')\">Tranquilo</button>";
  html += "<button onclick=\"fetch('/modo?valor=1')\">Sobrecarga</button>";
  html += "<button onclick=\"fetch('/modo?valor=2')\">Presencia</button>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// GET /modo?valor=0|1|2  -> cambia el modo activo
void handleSetModo() {
  if (server.hasArg("valor")) {
    int valor = server.arg("valor").toInt();
    if (valor >= 0 && valor < 3) {
      modoActualIndex = valor;
      Serial.print("Modo cambiado desde web a: ");
      Serial.println(modos[modoActualIndex]->getNombre());
      server.send(200, "text/plain", "OK: " + modos[modoActualIndex]->getNombre());
      return;
    }
  }
  server.send(400, "text/plain", "Error: parametro 'valor' invalido (usar 0, 1 o 2)");
}

// GET /estado  -> devuelve JSON con los datos actuales (para que la pagina los muestre)
void handleEstado() {
  int luz = leerLuz();
  int sonido = leerSonido();
  int pir = digitalRead(PIR_PIN);
  
  String json = "{";
  json += "\"modo\":" + String(modoActualIndex) + ",";
  json += "\"nombreModo\":\"" + modos[modoActualIndex]->getNombre() + "\",";
  json += "\"luz\":" + String(luz) + ",";
  json += "\"sonido\":" + String(sonido) + ",";
  json += "\"pir\":" + String(pir);
  json += "}";
  
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(VIBRADOR_PIN, OUTPUT);
  pinMode(BOTON_PIN, INPUT_PULLUP);
  
  strip.begin();
  strip.setBrightness(80);
  strip.show();
  
  modos[0] = new ModoTranquilo(&strip, VIBRADOR_PIN);
  modos[1] = new ModoSobrecarga(&strip, VIBRADOR_PIN);
  modos[2] = new ModoPresencia(&strip, VIBRADOR_PIN);
  
  // ---- Conexión WiFi ----
  WiFi.begin(ssid, password);
  Serial.print("Conectando a ");
  Serial.println(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // ---- Endpoints ----
  server.on("/", handleRoot);
  server.on("/modo", handleSetModo);
  server.on("/estado", handleEstado);
  server.begin();
  Serial.println("Servidor iniciado");
  
  Serial.println("Calibrando PIR... (30 segundos)");
  delay(30000);
  Serial.println("Listo!");
}

void loop() {
  server.handleClient(); // atiende peticiones web
  
  bool botonPresionado = (digitalRead(BOTON_PIN) == LOW);
  
  if (botonPresionado && !botonPresionadoAntes) {
    modoActualIndex = (modoActualIndex + 1) % 3;
    Serial.print("Cambiando a modo (boton): ");
    Serial.println(modos[modoActualIndex]->getNombre());
    delay(200);
  }
  botonPresionadoAntes = botonPresionado;
  
  int luz = leerLuz();
  int sonido = leerSonido();
  int pir = digitalRead(PIR_PIN);
  
  modos[modoActualIndex]->ejecutar(luz, sonido, pir);
  
  Serial.print("Modo: ");
  Serial.print(modos[modoActualIndex]->getNombre());
  Serial.print(" | Luz: ");
  Serial.print(luz);
  Serial.print(" | Sonido: ");
  Serial.println(sonido);
}