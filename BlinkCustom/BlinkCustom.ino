#include <StandardCplusplus.h>
#include <serstream.h>

const byte interruptionPin(2);

volatile unsigned long fallingMillis;
volatile unsigned long fallingConfirmMillis;
volatile unsigned long risingMillis;
volatile unsigned long risingConfirmMillis;
volatile bool power(false);
volatile unsigned long powerOnMillis;
volatile unsigned long ledMillis;

namespace std
{
	ohserialstream cout(Serial);
}

void InterruptionFalling()
{
	fallingMillis = millis();

	//if (millis() - fallingMillis > 1000)
	//{
	//	fallingMillis = millis();

	//	power = !power;
	//	digitalWrite(LED_BUILTIN, power);
	//}

}

void InterruptionRising()
{
	risingMillis = millis();
}
 
// �������, ���������� �������� 2 ��� ���������� � ��������� �
ISR(TIMER2_COMPA_vect)
{
	// fallingConfirmMillis
	if (digitalRead(2) == LOW &&
		(long)(risingConfirmMillis - fallingConfirmMillis) >= 0 &&
		(long)(fallingMillis - risingMillis) >= 0 &&
		millis() - fallingMillis > 10)
		fallingConfirmMillis = millis();

	// risingConfirmMillis
	if (digitalRead(2) == HIGH &&
		(long)(fallingConfirmMillis - risingConfirmMillis) >= 0 &&
		(long)(risingMillis - fallingMillis) >= 0 &&
		millis() - risingMillis > 10)
		risingConfirmMillis = millis();

	// ���������
	if (!power &&
		(long)(fallingConfirmMillis - risingConfirmMillis) > 0)
	{
		power = true;
		powerOnMillis = millis();
	}

	// ����������
	if (power &&
		(long)(risingConfirmMillis - fallingConfirmMillis) > 0 &&
		(long)(fallingConfirmMillis - powerOnMillis) > 0)
		power = false;

	// ������
	if (power)
	{
		unsigned int frequency = 1000 + analogRead(A0) * 3;
		unsigned int ratio = frequency * double(analogRead(A1)) / 1024 / 2;
		if (millis() - ledMillis > frequency)
		{
			ledMillis = millis();
			digitalWrite(LED_BUILTIN, HIGH);
		}
		if (millis() - ledMillis > ratio)
			digitalWrite(LED_BUILTIN, LOW);
	}
	else
		digitalWrite(LED_BUILTIN, LOW);
}

void setup()
{
	Serial.begin(115200);

	fallingMillis = fallingConfirmMillis = risingMillis = risingConfirmMillis = powerOnMillis = ledMillis = millis();

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(interruptionPin, INPUT);
	digitalWrite(interruptionPin, HIGH);

	pinMode(A0, INPUT);
	pinMode(A1, INPUT);

	// ������
	// ������������� ����� - ����� ��� ����������
	TCCR2A |= (1 << WGM21);
	// ������������� ����� �� �������� ������� (249 == 0xF9)
	OCR2A = 0xF9;
	// ��������� ���������� ��� ���������� � ��������� A
	TIMSK2 |= (1 << OCIE2A);
	// ������������� �������� ������� �� 64
	TCCR2B |= (1 << CS22) | (0 << CS21) | (0 << CS20);
	// ��������� ����������
	sei();

	// ���������� �� �������� �������
	attachInterrupt(0, InterruptionFalling, FALLING);
	attachInterrupt(1, InterruptionRising, RISING);
}

void loop() 
{
}
