#define PIR_PIN 14
#define VIBRADOR_PIN 27

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(VIBRADOR_PIN, OUTPUT);
  delay(30000); // calibración PIR
  Serial.println("Listo!");
}

void testVibrador() {
  digitalWrite(VIBRADOR_PIN, HIGH);;
}

void loop() {
  int estado = digitalRead(PIR_PIN);
  Serial.print("PIN value: ");
  Serial.println(estado);
  
  if (estado == HIGH) {
    testVibrador(); // vibra cuando detecta presencia
  }
  else{
     digitalWrite(VIBRADOR_PIN, LOW);
  }
 
  delay(500);
}