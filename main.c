#include <LPC17xx.h>
#include <stdio.h>

/* =========================================================
   LPC1768 - Smart Greenhouse Management System
   Sensors:
     MQ-135  -> P0.24 (AD0.1)
     LM35    -> P0.25 (AD0.2)
     LDR     -> P0.26 (AD0.3)   (ADC input suggested)
     Moisture-> P0.23 (AD0.0)

   Actuators:
     Pump Relay -> P1.28
     Fan Relay  -> P1.27 (via BC547)
     Buzzer     -> P1.29
   ========================================================= */

/* ---------------- LCD Pin Definitions (GPIO0) ---------------- */
#define RS (1<<10)
#define EN (1<<11)
#define D4 (1<<19)
#define D5 (1<<20)
#define D6 (1<<21)
#define D7 (1<<22)

/* ---------------- Output Pins (GPIO1) ---------------- */
#define PUMP_RELAY_PIN   (1<<28)   // Pump Relay on P1.28
#define FAN_RELAY_PIN    (1<<27)   // Fan Relay on P1.27 (via transistor)
#define BUZZER_PIN       (1<<29)   // Buzzer on P1.29

/* NOTE:
   LED_PINS originally mentioned P1.19-P1.26.
   Only use this if you actually connected LEDs there.
*/
#define LED_PINS (0xFF<<19)        // P1.19 to P1.26

/* ---------------- Thresholds ---------------- */
#define LDR_THRESHOLD       30.0f
#define MOISTURE_THRESHOLD  25.0f
#define GAS_THRESHOLD       40.0f
#define TEMP_THRESHOLD      50.0f

/* ---------------- Delay ---------------- */
void wait_loop(volatile unsigned int t)
{
    while(t--)
    {
        volatile unsigned int i = 10000;
        while(i--) {}
    }
}

/* =========================================================
   LCD FUNCTIONS (16x2, 4-bit mode)
   ========================================================= */
void LCD_Pulse(void)
{
    LPC_GPIO0->FIOSET = EN;
    wait_loop(1);
    LPC_GPIO0->FIOCLR = EN;
}

void LCD_Send4(unsigned int nibble)
{
    LPC_GPIO0->FIOCLR = D4|D5|D6|D7;

    if(nibble & 0x1) LPC_GPIO0->FIOSET = D4;
    if(nibble & 0x2) LPC_GPIO0->FIOSET = D5;
    if(nibble & 0x4) LPC_GPIO0->FIOSET = D6;
    if(nibble & 0x8) LPC_GPIO0->FIOSET = D7;
}

void LCD_Command(unsigned char cmd)
{
    LPC_GPIO0->FIOCLR = RS;

    LCD_Send4((cmd >> 4) & 0x0F);
    LCD_Pulse();

    LCD_Send4(cmd & 0x0F);
    LCD_Pulse();

    wait_loop(10);
}

void LCD_Data(unsigned char data)
{
    LPC_GPIO0->FIOSET = RS;

    LCD_Send4((data >> 4) & 0x0F);
    LCD_Pulse();

    LCD_Send4(data & 0x0F);
    LCD_Pulse();

    wait_loop(5);
}

void LCD_Init(void)
{
    LPC_GPIO0->FIODIR |= RS|EN|D4|D5|D6|D7;

    wait_loop(200);

    LCD_Command(0x28);  // 4-bit, 2 line
    LCD_Command(0x0C);  // Display ON, Cursor OFF
    LCD_Command(0x06);  // Entry mode
    LCD_Command(0x01);  // Clear display

    wait_loop(50);
}

void LCD_String(const char *s)
{
    while(*s) LCD_Data(*s++);
}

void LCD_SetCursor(int row, int col)
{
    LCD_Command((row==0 ? 0x80 : 0xC0) + col);
}

/* =========================================================
   ADC FUNCTIONS
   Channels used:
     AD0.0 -> P0.23 -> Soil Moisture
     AD0.1 -> P0.24 -> MQ135 Gas
     AD0.2 -> P0.25 -> LM35 Temp
     AD0.3 -> P0.26 -> LDR
   ========================================================= */
void ADC_Init(void)
{
    LPC_SC->PCONP |= (1<<12);   // Power ON ADC

    /* Set ADC pins:
       P0.23 -> AD0.0
       P0.24 -> AD0.1
       P0.25 -> AD0.2
       P0.26 -> AD0.3
    */
    LPC_PINCON->PINSEL1 |= (1<<14); // P0.23 AD0.0
    LPC_PINCON->PINSEL1 |= (1<<16); // P0.24 AD0.1
    LPC_PINCON->PINSEL1 |= (1<<18); // P0.25 AD0.2
    LPC_PINCON->PINSEL1 |= (1<<20); // P0.26 AD0.3

    /* ADC Control:
       PDN = 1 (bit 21)
       CLKDIV = 7 (bits 8-15)
    */
    LPC_ADC->ADCR = (1<<21) | (7<<8);
}

unsigned int ADC_ReadChannel(unsigned int channel)
{
    LPC_ADC->ADCR &= ~0xFF;         // Clear channel selection
    LPC_ADC->ADCR |= (1<<channel);  // Select channel
    LPC_ADC->ADCR |= (1<<24);       // Start conversion

    while(!(LPC_ADC->ADGDR & (1U<<31)));  // Wait till DONE

    return (LPC_ADC->ADGDR >> 4) & 0xFFF; // 12-bit result
}

/* =========================================================
   SENSOR FUNCTIONS
   ========================================================= */
float LM35_GetTempC(void)
{
    unsigned int adc = ADC_ReadChannel(2);  // AD0.2
    return ((float)adc * 3.3f / 4095.0f) * 100.0f;   // 10mV/°C
}

/* Moisture mapping */
#define DRY_VOLTAGE 3.0f
#define WET_VOLTAGE 1.0f

float Moisture_GetPercent(void)
{
    unsigned int adc = ADC_ReadChannel(0); // AD0.0
    float voltage = ((float)adc * 3.3f) / 4095.0f;

    float moisture = (DRY_VOLTAGE - voltage) / (DRY_VOLTAGE - WET_VOLTAGE) * 100.0f;

    if(moisture < 0) moisture = 0;
    if(moisture > 100) moisture = 100;

    return moisture;
}

float LDR_GetValue(void)
{
    unsigned int adc = ADC_ReadChannel(3); // AD0.3
    return ((float)adc / 4095.0f) * 100.0f;
}

float MQ135_GetPercent(void)
{
    unsigned int sum = 0;

    for(int i = 0; i < 5; i++)
        sum += ADC_ReadChannel(1); // AD0.1

    float avg_adc = (float)sum / 5.0f;
    float voltage = (avg_adc / 4095.0f) * 3.3f;

    float airQuality = (voltage / 3.3f) * 100.0f;

    if(airQuality > 100) airQuality = 100;
    if(airQuality < 0)   airQuality = 0;

    return airQuality;
}

/* =========================================================
   OUTPUT INIT (Relays, Buzzer, LEDs)
   ========================================================= */
void Outputs_Init(void)
{
    LPC_GPIO1->FIODIR |= PUMP_RELAY_PIN | FAN_RELAY_PIN | BUZZER_PIN | LED_PINS;

    /* Set default OFF state:
       - Relays OFF (Assuming Active LOW relay module)
       - Buzzer OFF (Assuming Active LOW)
       - LEDs OFF
    */
    LPC_GPIO1->FIOSET = PUMP_RELAY_PIN;     // Pump OFF
    LPC_GPIO1->FIOSET = FAN_RELAY_PIN;      // Fan OFF
    LPC_GPIO1->FIOSET = BUZZER_PIN;         // Buzzer OFF
    LPC_GPIO1->FIOCLR = LED_PINS;           // LEDs OFF (change if active high)
}

/* =========================================================
   LCD UPDATE
   ========================================================= */
void LCD_UpdateSensors(float temp, float moisture, float airQuality)
{
    char line[17];

    LCD_SetCursor(0,0);
    sprintf(line, "T:%2.1fC M:%2.0f%%", temp, moisture);
    LCD_String(line);

    LCD_SetCursor(1,0);
    sprintf(line, "AirQ:%2.0f%%        ", airQuality);
    LCD_String(line);
}

/* =========================================================
   MAIN
   ========================================================= */
int main(void)
{
    ADC_Init();
    LCD_Init();
    Outputs_Init();

    LCD_SetCursor(0,0);
    LCD_String("Green House");
    LCD_SetCursor(1,0);
    LCD_String("Management");
    wait_loop(500);

    float temp_avg = 0;
    float moisture_avg = 0;

    while(1)
    {
        float temp      = LM35_GetTempC();
        float moisture  = Moisture_GetPercent();
        float airQ      = MQ135_GetPercent();
        float light     = LDR_GetValue();

        /* Simple smoothing */
        temp_avg     = 0.2f*temp + 0.8f*temp_avg;
        moisture_avg = 0.2f*moisture + 0.8f*moisture_avg;

        /* LCD Update */
        LCD_UpdateSensors(temp_avg, moisture_avg, airQ);

        /* ---------------- Pump Control (Moisture) ----------------
           Active LOW Relay Module:
           FIOCLR -> ON
           FIOSET -> OFF
        */
        if(moisture_avg < MOISTURE_THRESHOLD)
            LPC_GPIO1->FIOCLR = PUMP_RELAY_PIN; // Pump ON
        else
            LPC_GPIO1->FIOSET = PUMP_RELAY_PIN; // Pump OFF

        /* ---------------- LED Control (LDR) ----------------
           If light is low -> LEDs ON
           (Assuming LEDs Active HIGH, change if needed)
        */
        if(light < LDR_THRESHOLD)
            LPC_GPIO1->FIOSET = LED_PINS;  // LEDs ON
        else
            LPC_GPIO1->FIOCLR = LED_PINS;  // LEDs OFF

        /* ---------------- Fan + Buzzer Control ---------------- */
        if(airQ > GAS_THRESHOLD || temp_avg > TEMP_THRESHOLD)
        {
            LPC_GPIO1->FIOCLR = FAN_RELAY_PIN;  // Fan ON (Active LOW)
            LPC_GPIO1->FIOCLR = BUZZER_PIN;     // Buzzer ON (Active LOW)
        }
        else
        {
            LPC_GPIO1->FIOSET = FAN_RELAY_PIN;  // Fan OFF
            LPC_GPIO1->FIOSET = BUZZER_PIN;     // Buzzer OFF
        }

        wait_loop(100);  // ~200ms delay
    }
}
