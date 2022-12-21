#ifndef ANALOG_H
#define ANALOG_H

#include <driver/adc.h>															// Modulo conversor ADC.

#define batMax  3958    // 14.5V
#define bat100  3822    // 14.0V
#define bat080  3685    // 13.5V
#define bat060  3549    // 13.0V
#define bat040  3412    // 12.5V
#define bat020  3276    // 12.0V
#define bat000  3139    // 11.5V
#define batMin  2730    // 10.0V

void adcIniciar(void)															// Inicializa o hardware do modulo conversor ADC.
{
	adc1_config_width(ADC_WIDTH_BIT_12); 										// Config. a resolucao do ADC para 12bits.
	adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11); 					// Config. a atenuacao do Canal 0 (GPIO36).
	// adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_11); 					// Config. a atenuacao do Canal 3 (GPIO39).
	/*
	Attenuation			Measurable input voltage range
	ADC_ATTEN_DB_0		100 mV ~ 950 mV
	ADC_ATTEN_DB_2_5	100 mV ~ 1250 mV
	ADC_ATTEN_DB_6		150 mV ~ 1750 mV
	ADC_ATTEN_DB_11 	150 mV ~ 2450 mV
	*/
}

void adcLer(void)
{
    unsigned int vlrAdc=0;
    vlrAdc = adc1_get_raw(ADC1_CHANNEL_0);								        // Le o pino GPIO36.

    if (vlrAdc>=batMax)
    {
        /* Gerar Alerta: Perigo */
    }
    if (vlrAdc<=batMin)
    {
        /* Gerar Alerta: Vai desligar */
    }
    if (vlrAdc>bat080 && vlrAdc<=bat100)
    {
        /* 100% */
    }
    if (vlrAdc>bat060 && vlrAdc<=bat080)
    {
        /* 080% */
    }
    if (vlrAdc>bat040 && vlrAdc<=bat060)
    {
        /* 060% */
    }
    if (vlrAdc>bat020 && vlrAdc<=bat040)
    {
        /* 040% */
    }
    if (vlrAdc>bat000 && vlrAdc<=bat020)
    {
        /* 020% */
    }
    if (vlrAdc>batMin && vlrAdc<=bat020)
    {
        /* Gerar Alerta: Recarregar */
    }    
}

#endif