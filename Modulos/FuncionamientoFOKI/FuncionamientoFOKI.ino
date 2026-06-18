#include <Adafruit_NeoPixel.h>

#define PIR_PIN 14
#define VIBRADOR_PIN 27
#define LDR_PIN 34
#define SONIDO_PIN 35
#define BOTON_PIN 23
#define LED_PIN 4
#define NUM_LEDS 30

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

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
  
  Serial.println("Calibrando PIR... (30 segundos)");
  delay(30000);
  Serial.println("Listo!");
}

void loop() {
  bool botonPresionado = (digitalRead(BOTON_PIN) == LOW);
  
  if (botonPresionado && !botonPresionadoAntes) {
    modoActualIndex = (modoActualIndex + 1) % 3;
    Serial.print("Cambiando a modo: ");
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