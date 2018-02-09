#include <StandardCplusplus.h>
#include <serstream.h>
#include <avr\sleep.h>

enum Command
{
	Nothing,
	Init,
	WaitBeforeSleep,
	Sleep
};

Command command;

volatile unsigned long ledMillis;
volatile unsigned long risingMillis;

namespace std
{
	ohserialstream cout(Serial);
}

void InterruptionRising()
{
	std::cout << "InterruptionRising()" << std::endl;
	risingMillis = millis();

	if (command == Command::Nothing)
		command = Command::WaitBeforeSleep;
}

// �������, ���������� �������� 2 ��� ���������� � ��������� �
ISR(TIMER2_COMPA_vect)
{
	// ������
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

void setup()
{
	Serial.begin(115200);
	command = Command::Init;
	risingMillis = millis();
}

void loop() 
{
	if (digitalRead(2) == HIGH &&
		millis() - risingMillis > 10)
	{
		switch (command)
		{
		case Command::Init:

			std::cout << "Command::Init" << std::endl;
			
			// ���������������� - ��� ������ � ����� ����� � ��������� ������������� ���������
			for (byte i = 0; i <= A5; i++)
			{
				pinMode(i, INPUT);
				digitalWrite(i, LOW);
			}
			// ���������� ���������� �������-��������� ��������������
			ADCSRA = 0;

			pinMode(LED_BUILTIN, OUTPUT);

			// ���������� �� �������� �������
			attachInterrupt(0, InterruptionRising, RISING);

			// ���������� �� �������
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

			command = Command::Nothing;
			
			break;

		case Command::WaitBeforeSleep:

			std::cout << "Command::WaitBeforeSleep" << std::endl;
			
			command = Command::Sleep;

			break;

		case Command::Sleep:

			std::cout << "Command::Sleep" << std::endl;

			command = Command::Init;

			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_enable();
			attachInterrupt(0, NULL, LOW);
			// ���������� Brown-out Detector(BOD) - ���������� � ATmega ����������, �������� �� ������� ������� ����
			MCUCR = bit(BODS) | bit(BODSE);
			MCUCR = bit(BODS);
			// ������ � ���
			sleep_cpu();
			// ����� ������ �� ��� - � ��� �������� ����� ������������ LOW ����������
			sleep_disable();

			break;
		}
	}
}
