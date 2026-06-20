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
const char* password = "q3cvdacRzs2Z";

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
      if (sonido < 1100) {
        // Silencio
        tira->setBrightness(80); tira->fill(tira->Color(80, 140, 255));
        digitalWrite(pinVibrador, LOW);
      } else if (sonido < 1200) {
        tira->setBrightness(75); tira->fill(tira->Color(70, 120, 240));
        digitalWrite(pinVibrador, LOW);
      } else if (sonido < 1300) {
        tira->setBrightness(70); tira->fill(tira->Color(60, 100, 220));
        digitalWrite(pinVibrador, LOW);
      } else if (sonido < 1400) {
        // Ruido moderado
        tira->setBrightness(68); tira->fill(tira->Color(50, 80, 210));
        digitalWrite(pinVibrador, LOW);
      } else if (sonido < 1500) {
        tira->setBrightness(65); tira->fill(tira->Color(80, 60, 200));
        digitalWrite(pinVibrador, LOW);
      } else if (sonido < 1600) {
        // Ruido alto
        tira->setBrightness(60); tira->fill(tira->Color(100, 40, 185));
        digitalWrite(pinVibrador, LOW);
      } else if (sonido < 1700) {
        // Super alto
        tira->setBrightness(55); tira->fill(tira->Color(130, 20, 160));
        digitalWrite(pinVibrador, LOW);
      } else if (sonido < 1800) {
        tira->setBrightness(50); tira->fill(tira->Color(160, 10, 130));
        digitalWrite(pinVibrador, LOW);
      } else if (sonido < 1900) {
        tira->setBrightness(45); tira->fill(tira->Color(190, 5, 100));
        digitalWrite(pinVibrador, HIGH);
      } else if (sonido < 2000) {
        tira->setBrightness(40); tira->fill(tira->Color(210, 0, 80));
        digitalWrite(pinVibrador, HIGH);
      } else if (sonido < 2100) {
        tira->setBrightness(35); tira->fill(tira->Color(230, 0, 60));
        digitalWrite(pinVibrador, HIGH);
      } else if (sonido < 2200) {
        tira->setBrightness(30); tira->fill(tira->Color(245, 0, 40));
        digitalWrite(pinVibrador, HIGH);
      } else if (sonido < 2300) {
        tira->setBrightness(25); tira->fill(tira->Color(255, 0, 20));
        digitalWrite(pinVibrador, HIGH);
      } else if (sonido < 2400) {
        tira->setBrightness(23); tira->fill(tira->Color(255, 0, 10));
        digitalWrite(pinVibrador, HIGH);
      } else if (sonido < 2500) {
        tira->setBrightness(20); tira->fill(tira->Color(255, 0, 5));
        digitalWrite(pinVibrador, HIGH);
      } else if (sonido < 2600) {
        tira->setBrightness(17); tira->fill(tira->Color(255, 0, 0));
        digitalWrite(pinVibrador, HIGH);
      } else if (sonido < 2700) {
        tira->setBrightness(14); tira->fill(tira->Color(255, 0, 0));
        digitalWrite(pinVibrador, HIGH);
      } else if (sonido < 2800) {
        tira->setBrightness(11); tira->fill(tira->Color(255, 0, 0));
        digitalWrite(pinVibrador, HIGH);
      } else {
        // > 2800: límite máximo
        tira->setBrightness(8); tira->fill(tira->Color(255, 0, 0));
        digitalWrite(pinVibrador, HIGH);
      }
      tira->show();
    }
    
    String getNombre() override {
      return "Sobrecarga Ambiental";
    }
};

// ===================== MODO 3: PRESENCIA =====================
class ModoPresencia : public Modo {
  private:
    int brilloActual;
    unsigned long tiempoUltimoPaso;
    unsigned long tiempoPausaArriba;
    bool fadiendo;
    bool subiendo;
    bool pausandoArriba;

  public:
    ModoPresencia(Adafruit_NeoPixel* strip, int vibPin) : Modo(strip, vibPin) {
      brilloActual = 0;
      tiempoUltimoPaso = 0;
      tiempoPausaArriba = 0;
      fadiendo = false;
      subiendo = true;
      pausandoArriba = false;
    }

    void ejecutar(int luz, int sonido, int pir) override {
      if (pir == HIGH) {
        if (!fadiendo) {
          fadiendo = true;
          brilloActual = 0;
          subiendo = true;
          pausandoArriba = false;
        }
        if (pausandoArriba) {
          if (millis() - tiempoPausaArriba >= 500) {
            pausandoArriba = false;
            subiendo = false;
          }
        } else if (millis() - tiempoUltimoPaso >= 60) {
          tiempoUltimoPaso = millis();
          if (subiendo) {
            brilloActual += 5;
            if (brilloActual >= 150) {
              brilloActual = 150;
              pausandoArriba = true;
              tiempoPausaArriba = millis();
            }
          } else {
            brilloActual -= 7;
            if (brilloActual <= 10) { brilloActual = 10; subiendo = true; }
          }
        }
        tira->setBrightness(brilloActual);
        tira->fill(tira->Color(255, 130, 60));
        tira->show();
        digitalWrite(pinVibrador, HIGH);
      } else {
        fadiendo = false;
        brilloActual = 0;
        subiendo = true;
        pausandoArriba = false;
        tira->setBrightness(10);
        tira->fill(tira->Color(40, 20, 10));
        tira->show();
        digitalWrite(pinVibrador, LOW);
      }
    }

    String getNombre() override {
      return "Presencia";
    }
};

// ===================== MODO 4: FOCUS =====================
class ModoFocus : public Modo {
  private:
    int estado; // 0=FOCUSING, 1=BREAK, 2=IDLE
    unsigned long tiempoBreak;
    bool vibrandoBreak;
    unsigned long tiempoParpadeo;
    bool luzEncendida;

  public:
    ModoFocus(Adafruit_NeoPixel* strip, int vibPin) : Modo(strip, vibPin) {
      estado = 2;
      tiempoBreak = 0;
      vibrandoBreak = false;
      tiempoParpadeo = 0;
      luzEncendida = true;
    }

    void setEstado(int e) {
      estado = e;
      if (e == 1) {
        tiempoBreak = millis();
        vibrandoBreak = true;
      } else {
        vibrandoBreak = false;
        digitalWrite(pinVibrador, LOW);
      }
    }

    void ejecutar(int luz, int sonido, int pir) override {
      if (estado == 2) {
        if (millis() - tiempoParpadeo >= 600) {
          tiempoParpadeo = millis();
          luzEncendida = !luzEncendida;
        }
        tira->setBrightness(luzEncendida ? 80 : 0);
        tira->fill(tira->Color(255, 120, 20));
        tira->show();
        digitalWrite(pinVibrador, LOW);
      } else {
        tira->setBrightness(80);
        tira->fill(tira->Color(255, 120, 20));
        tira->show();

        if (vibrandoBreak) {
          if (millis() - tiempoBreak < 3000) {
            digitalWrite(pinVibrador, HIGH);
          } else {
            digitalWrite(pinVibrador, LOW);
            vibrandoBreak = false;
          }
        } else {
          digitalWrite(pinVibrador, LOW);
        }
      }
    }

    String getNombre() override {
      return "Focus";
    }
};

// ===================== MODO 5: CUSTOM =====================
class ModoCustom : public Modo {
  private:
    uint8_t r, g, b, brillo;

  public:
    ModoCustom(Adafruit_NeoPixel* strip, int vibPin) : Modo(strip, vibPin) {
      r = 252; g = 216; b = 115; // amarillo por defecto (color de la app)
      brillo = 80;
    }

    void setColor(uint8_t nr, uint8_t ng, uint8_t nb) {
      r = nr; g = ng; b = nb;
    }

    void setBrillo(uint8_t val) {
      brillo = val;
    }

    void ejecutar(int luz, int sonido, int pir) override {
      tira->setBrightness(brillo);
      tira->fill(tira->Color(r, g, b));
      tira->show();
      digitalWrite(pinVibrador, LOW);
    }

    String getNombre() override {
      return "Custom";
    }
};

// ===================== VARIABLES GLOBALES =====================
Modo* modos[5];
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

// GET /modo?valor=0|1|2|3|4  -> cambia el modo activo
void handleSetModo() {
  if (server.hasArg("valor")) {
    int valor = server.arg("valor").toInt();
    if (valor >= 0 && valor < 5) {
      modoActualIndex = valor;
      Serial.print("Modo cambiado desde web a: ");
      Serial.println(modos[modoActualIndex]->getNombre());
      server.send(200, "text/plain", "OK: " + modos[modoActualIndex]->getNombre());
      return;
    }
  }
  server.send(400, "text/plain", "Error: parametro 'valor' invalido (usar 0-4)");
}

// GET /color?r=&g=&b=  -> setea color del ModoCustom
void handleSetColor() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    uint8_t r = server.arg("r").toInt();
    uint8_t g = server.arg("g").toInt();
    uint8_t b = server.arg("b").toInt();
    ((ModoCustom*)modos[4])->setColor(r, g, b);
    Serial.printf("Color: %d,%d,%d\n", r, g, b);
    server.send(200, "text/plain", "OK");
    return;
  }
  server.send(400, "text/plain", "Error: faltan parametros r, g, b");
}

// GET /brillo?valor=  -> setea brillo del ModoCustom (0-255)
void handleSetBrillo() {
  if (server.hasArg("valor")) {
    uint8_t val = server.arg("valor").toInt();
    ((ModoCustom*)modos[4])->setBrillo(val);
    Serial.printf("Brillo: %d\n", val);
    server.send(200, "text/plain", "OK");
    return;
  }
  server.send(400, "text/plain", "Error: falta parametro valor");
}

// GET /focus?estado=0|1|2  -> cambia el estado interno del ModoFocus
void handleSetFocusEstado() {
  if (server.hasArg("estado")) {
    int e = server.arg("estado").toInt();
    if (e >= 0 && e <= 2) {
      ((ModoFocus*)modos[3])->setEstado(e);
      Serial.print("Focus estado: ");
      Serial.println(e);
      server.send(200, "text/plain", "OK");
      return;
    }
  }
  server.send(400, "text/plain", "Error: parametro 'estado' invalido (usar 0, 1 o 2)");
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
  modos[3] = new ModoFocus(&strip, VIBRADOR_PIN);
  modos[4] = new ModoCustom(&strip, VIBRADOR_PIN);
  
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
  server.on("/focus", handleSetFocusEstado);
  server.on("/color", handleSetColor);
  server.on("/brillo", handleSetBrillo);
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