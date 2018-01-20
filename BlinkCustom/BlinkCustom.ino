#include <StandardCplusplus.h>
#include <serstream.h>

const byte interruptionPin(2);

volatile unsigned long fallingMillis;
volatile unsigned long risingMillis;
volatile bool power(false);

namespace std
{
	ohserialstream cout(Serial);
}

void InterruptionFalling()
{
	if (millis() - fallingMillis > 1000)
	{
		fallingMillis = millis();

		power = !power;
		digitalWrite(LED_BUILTIN, power);
	}
}

void setup() 
{
	Serial.begin(115200);

	fallingMillis = risingMillis = millis();
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(interruptionPin, INPUT);
	digitalWrite(interruptionPin, HIGH);
	attachInterrupt(0, InterruptionFalling, FALLING);
}

void loop() 
{
}
