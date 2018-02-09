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
			// Отключение подсистемы аналого-цифрового преобразования
			ADCSRA = 0;

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
			// Выключение Brown-out Detector(BOD) - встроенная в ATmega подсистема, следящая за уровнем питания чипа
			MCUCR = bit(BODS) | bit(BODSE);
			MCUCR = bit(BODS);
			// Уходим в сон
			sleep_cpu();
			// Точка выхода из сна - в нее вернемся после срабатывания LOW прерывания
			sleep_disable();

			break;
		}
	}
}
