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

//const byte interruptionPin(2);

//volatile unsigned long fallingMillis;
//volatile unsigned long fallingConfirmMillis;
//volatile unsigned long risingMillis;
//volatile unsigned long risingConfirmMillis;
//volatile unsigned long powerOffMillis;
volatile unsigned long ledMillis;
volatile unsigned long risingMillis;

namespace std
{
	ohserialstream cout(Serial);
}

//void InterruptionFalling()
//{
//	std::cout << "InterruptionFalling()" << std::endl;
//	clickBeginMillis = millis();
//
//	//fallingMillis = millis();
//
//	//sleep_disable();
//	//detachInterrupt(0);
//	//sleep = false;
//	//std::cout << "InterruptionLow()" << std::endl;
//}

void InterruptionRising()
{
	std::cout << "InterruptionRising()" << std::endl;
	risingMillis = millis();

	if (command == Command::Nothing)
		command = Command::WaitBeforeSleep;

	//fallingMillis = millis();

	//sleep_disable();
	//detachInterrupt(0);
	//sleep = false;
	//std::cout << "InterruptionLow()" << std::endl;
}

//void sleepNow()
//{
//	//powerOffMillis = 20;
//	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
//	sleep_enable();
//	attachInterrupt(0, InterruptionLow, LOW);
//	//MCUCR = bit(BODS) | bit(BODSE);
//	//MCUCR = bit(BODS);
//	//sleep_mode();
//	sleep_cpu();
//}

// Функция, вызываемая таймером 2 при совпадении с регистром А
ISR(TIMER2_COMPA_vect)
{
	// Мигаем
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
			
			// Энергосбережение - все выводы в режим входа и отключаем подтягивающие резисторы
			for (byte i = 0; i <= A5; i++)
			{
				pinMode(i, INPUT);
				digitalWrite(i, LOW);
			}

			pinMode(LED_BUILTIN, OUTPUT);

			// Прерывание по внешнему сигналу
			attachInterrupt(0, InterruptionRising, RISING);

			// Прерывание по таймеру
			// Устанавливаем режим - сброс при совпадении
			TCCR2A |= (1 << WGM21);
			// Устанавливаем число до которого считать (249 == 0xF9)
			OCR2A = 0xF9;
			// Разрешаем прерывание при совпадении с регистром A
			TIMSK2 |= (1 << OCIE2A);
			// Устанавливаем делитель частоты на 64
			TCCR2B |= (1 << CS22) | (0 << CS21) | (0 << CS20);
			// Разрешаем прерывание
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
			sleep_mode();
			sleep_disable();

			break;
		}
	}

	//if (fallingMillis != fallingMillisOld &&
	//	millis() - fallingMillis > 10)
	//{
	//	fallingMillisOld = fallingMillis;
	//	std::cout << "click" << std::endl;
	//	std::cout << "sleep" << std::endl;


	//	attachInterrupt(0, InterruptionFalling, FALLING);
	//}
}
