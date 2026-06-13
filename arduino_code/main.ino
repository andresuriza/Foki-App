int lightPin = 2;

class Mode
{
  public:
    void light(void);
};

class Calm: public Mode
{
  public:
    void light(void)
    {
      Serial.print("Calma\n");
    }
};

class Overload: public Mode
{
  public:
    void light(void)
    {
      Serial.print("Sobrecarga\n");
    }
};

class Companion: public Mode
{
  public:
    void light(void)
    {
      Serial.print("Acompanamiento\n");
    }
};

class Custom: public Mode
{
  public:
    void light(String light)
    {
      Serial.print(light);
    }
};

void setup() {
  Serial.begin(9600);

  delay(1000);

  Calm calm1;
  calm1.light();

  Overload overload1;
  overload1.light();

  Companion companion1;
  companion1.light();

  Custom custom1;
  custom1.light("RGB###\n");

  // pinMode(lightPin, OUTPUT);
}

void loop() {


  // if (Serial.available() > 0)
  // {
  //   String receivedString = "";

  //   while(Serial.available() > 0) receivedString += char(Serial.read());

  //   if (receivedString == "1") digitalWrite(lightPin, HIGH);
  //   else digitalWrite(lightPin, LOW);
  // }

}
