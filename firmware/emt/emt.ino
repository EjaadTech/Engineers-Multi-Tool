#include <avr/pgmspace.h>
////////////////// Auto Frequency////////////////////////
#define Crystal 16000000L

const int prescale[5] = { 1, 8, 64, 256, 1024 };
float Frequency[5], RegValue[5], InputFrequency = 0;
unsigned int  FinalReg;
long Test = 8000000;
unsigned long temp[5];
float duty = 5;
unsigned long time[2], ButtonTimer, PreTimer;
unsigned int ButtonRead, DutyBar;
float VoltageA, VoltageB;


#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include "menu.h"

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(6, 5, 4, 2, 3);


/* voltmeter.bmp: */
/* Width:  84 */
/* Height: 48 */
/* Format: Generic Bitmap 1BPP */
/*extern uint8_t voltmeter[];
extern uint8_t WaveOut[];
extern uint8_t SemiTester[];
extern uint8_t SerialOut[];*/


const uint8_t* menu[] = { (uint8_t*)voltmeter, (uint8_t*)WaveOut, (uint8_t*)SemiTester, (uint8_t*)SerialOut };

int count = 0;

int menutest;


void setup()
{
	Serial.begin(115200);
	pinMode(9, OUTPUT);
	calculation();
	display.begin();
	display.setContrast(63);
	display.clearDisplay();
	display.drawBitmap(0, 0, menu[0], 84, 48, 1);
	
	display.display();
}

void loop()
{
	ButtonRead = analogRead(A7);
	delay(20);
	Menu();
	if (count == 1 && ButtonRead > 700 && ButtonRead < 800)
	{
		VoltageRead();
		display.clearDisplay();

		display.drawBitmap(0, 0, menu[count-1], 84, 48, 1);
		display.display();
	}
	else if (count == 2 && ButtonRead > 700 && ButtonRead < 800)
	{
		FrequencySet();
		display.clearDisplay();
		
		display.drawBitmap(0, 0, menu[count-1], 84, 48, 1);
		display.display();
	
	}
	
}

void Menu()
{
	if (ButtonRead > 100 && ButtonRead < 200)
	{
		MenuUp();
		count++;
		if (count == 4)
			count = 0;
	}
	else if (ButtonRead > 400 && ButtonRead < 500)
	{
		count--;
		if (count < 0)
			count = 3;
		MenuDown();

	}

}

void MenuUp()
{
	for (int i = 0; i <=48; i += 4)
	{ 
		display.clearDisplay();
		display.drawBitmap(0, i, menu[count==0 ? 3 : (count-1) ], 84, 48, 1);
		display.drawBitmap(0, i-48, menu[count], 84, 48, 1);
		display.display();


	}
	
}

void MenuDown()
{
	
	for (int i = 48; i >= 0; i-=4)
	{
		display.clearDisplay();
		display.drawBitmap(0, i, menu[count == 0 ? 3 : (count - 1)], 84, 48, 1);
		display.drawBitmap(0, i - 48, menu[count], 84, 48, 1);
		display.display();
	}
}

void VoltageRead()
{
	delay(50);
	display.setTextSize(3);
	while (ButtonRead < 800)
	{
		ButtonRead = analogRead(A7);
		
		VoltageA = analogRead(A2)*5.0 / 1023.0;
		VoltageA = VoltageA * 11;
		VoltageB = analogRead(A3)*5.0 / 1023.0;
		VoltageB = VoltageB * 11;
		display.clearDisplay();
		if (VoltageA > 0)
		{
			VoltageA += 0.5;
		}
		if (VoltageB > 0)
		{
			VoltageB += 0.5;
		}
		if (VoltageA >= 10)
			display.print(VoltageA, 1);
		else
			display.print(VoltageA);
		display.println(VoltageB);
		display.display();
		display.setRotation(2);
		display.fillRoundRect(1, -2, 4, map(VoltageB,0,56,0,48), 1, 1);
		display.fillRoundRect(7, -2, 4, map(VoltageA, 0, 56, 0, 48), 1, 1);
		display.setRotation(0);
		display.display();
		delay(70);
	}
}
void calculation()
{
	Test = 8000000;

	for (int i = 4; i >= 0; i--)
	{

		RegValue[i] = (Crystal / (prescale[i] * InputFrequency)) - 1;


		if (InputFrequency<500)
			Frequency[i] = Crystal / (prescale[i] * RegValue[i]);
		else
			Frequency[i] = Crystal / (prescale[i] * ((int)RegValue[i] + 1));
		RegValue[i] = (int)RegValue[i];
		temp[i] = Frequency[i] - InputFrequency;
		if (temp[i]<0)
			temp[i] = InputFrequency - Frequency[i];


		if (temp[i] == 0)
		{
			FinalReg = i;
			break;
		}
		if (Test >= temp[i])
		{
			Test = temp[i];
			FinalReg = i;

		}

	}



	regsetting();
}

void regsetting()
{
	if (prescale[FinalReg] == 1)
	{
		TCCR1B = (1 << CS10) | (1 << WGM13) | (1 << WGM12);
	}
	else if (prescale[FinalReg] == 8)
	{
		TCCR1B = (1 << CS11) | (1 << WGM13) | (1 << WGM12);
	}
	else if (prescale[FinalReg] == 64)
	{
		TCCR1B = (1 << CS11) | (1 << CS10) | (1 << WGM13) | (1 << WGM12);
	}
	else if (prescale[FinalReg] == 256)
	{
		TCCR1B = (1 << CS12) | (1 << WGM13) | (1 << WGM12);
	}
	else if (prescale[FinalReg] == 1024)
	{
		TCCR1B = (1 << CS12) | (1 << CS10) | (1 << WGM13) | (1 << WGM12);
	}
	else
		TCCR1B = 0;
	TCCR1A = 0x82;
	ICR1 = RegValue[FinalReg];
	OCR1A = RegValue[FinalReg] * duty / 100;




}
void FrequencySet()
{
	while (ButtonRead < 800)
	{
		display.clearDisplay();
		display.setTextSize(1);
		display.setTextColor(BLACK);
		display.setCursor(0, 0);
		ButtonRead = analogRead(A7);

		if (ButtonRead > 700 && ButtonRead < 800)
		{
			ButtonRead = analogRead(A7);
			delay(400);
			while (ButtonRead < 800)
			{

				SetFreq();
				if (ButtonRead > 700 && ButtonRead <800)
				{
					delay(400);
					break;
				}
				if (ButtonRead>400 && ButtonRead < 500)
				{
					while (ButtonRead < 800)
					{
						SetDuty();
						if ((ButtonRead>100 && ButtonRead < 200) || (ButtonRead > 700 && ButtonRead < 800))
							break;

					}
				}


			}



		}
		else
		{

			display.print("FREQ ");
			display.println(Frequency[FinalReg]);
			display.print("DUTY ");
			display.print(duty);
			display.println("%");
			DutyBar = map(duty, 0, 100, 0, 83);
			display.drawLine(DutyBar - 1, 47, 83, 47, BLACK);
			display.fillRect(-1, 31, DutyBar - 2, 18, BLACK);
			display.drawRect(-1, 29, DutyBar, 20, BLACK);
			display.display();
		}

	}
}

void SetFreq()
{
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(BLACK);
	display.setCursor(0, 0);
	ButtonRead = analogRead(A7);
	display.clearDisplay();
	display.println("  FREQ");
	display.print(Frequency[FinalReg],0);


	display.display();

	if (ButtonRead > 500 && ButtonRead < 600)
	{
		if (InputFrequency >= 0)
		{
			InputFrequency -= 100;

			calculation();
			delay(20);
		}

	}
	else if (ButtonRead > 200 && ButtonRead < 300)
	{
		if (InputFrequency <= 300)
		{
			InputFrequency++;

		}
		else if (InputFrequency <= 1000)
		{
			InputFrequency += 100;
		}
		else 
		{
			InputFrequency += 1000;
		}

		calculation();
		delay(50);
	}
}
void SetDuty()
{
	ButtonRead = analogRead(A7);
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(BLACK);
	display.println(" DUTY");
	display.print(duty);
	display.println("%");
	DutyBar = map(duty, 0, 100, 0, 83);
	display.drawLine(DutyBar - 1, 47, 83, 47, BLACK);
	display.fillRect(-1, 33, DutyBar - 2, 18, BLACK);
	display.drawRect(-1, 31, DutyBar, 19, BLACK);
	display.display();

	if (ButtonRead > 200 && ButtonRead < 300)
	{
		if (duty <= 100)
		{
			duty = duty + 1;
			calculation();
			delay(100);
		}

	}
	else if (ButtonRead>500 && ButtonRead < 600)
	{
		if (duty >= 0)
			duty = duty - 1;
		calculation();
		delay(100);
	}
}
