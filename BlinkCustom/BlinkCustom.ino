#include <StandardCplusplus.h>
#include <serstream.h>

const byte interruptionPin(2);

volatile unsigned long fallingMillis;
volatile bool power(false);

namespace std
{
	ohserialstream cout(Serial);
}

void Interruption()
{
	if (millis() - fallingMillis > 1000)
	{
		std::cout << millis() - fallingMillis << std::endl;
		fallingMillis = millis();

		power = !power;
		digitalWrite(LED_BUILTIN, power);
	}
}

void setup() 
{
	Serial.begin(115200);

	fallingMillis = millis();
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(interruptionPin, INPUT);
	digitalWrite(interruptionPin, HIGH);
	attachInterrupt(0, Interruption, FALLING);
}

void loop() 
{
}
