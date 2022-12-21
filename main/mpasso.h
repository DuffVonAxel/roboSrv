#ifndef MPASSO_H
#define MPASSO_H

// #include "wifi.h"

// #include <stdio.h>																// Acesso as operacoes de entradas e/ou saidas.
// #include <string.h>                                                             // Acesso a manipulacao de strings.
// #include <sys/param.h>          												// => MIN()
// #include <driver/adc.h>															// Modulo conversor ADC.
#include "freertos/FreeRTOS.h"													// Acesso aos termos.
#include "freertos/task.h"														// Acesso as prioridades da TASK.
#include "driver/gpio.h"														// Acesso ao uso das GPIOs.
// #include "esp_err.h"
// #include "esp_mac.h"
// #include "esp_wifi.h"
// #include "esp_event.h"          												// => esp_event_base_t
// #include "esp_log.h"															// Acesso ao uso dos LOGs.
// #include "nvs_flash.h"          												// => nvs_flash_init
// #include "esp_http_server.h"    												// => httpd_uri_t
// #include "lwip/err.h"
// #include "lwip/sys.h"

#define MPC_STP 32      // Pino(10e) comum do:    Step
#define MPC_SLP 33      // Pino(09e) comum do:    Sleep
#define MP0_DIR 25      // Pino(08e) do modulo 0: direction
#define MP1_DIR 26      // Pino(07e) do modulo 1: direction
#define BAT_LVL 36      // Pino 14. Nivel da bateria ADC0.

/* Dados: Motor de Passo */
#define ___angPP            1.8     // Angulo por passo do motor.
#define ___redEixo          3       // Redutor no eixo do mp.

/* Dados: Robo */
#define ___distEixos        220     // Distancia entre os eixos em mm.
#define ___diamRoda         101.6   // Diametro da roda em mm.
#define ___periRoda         319.19  // Perimetro da roda = 2PI * (___diamRoda/2)

unsigned int __totalPasso;          // Numero total de passos para uma volta.
float __passoPormm;                 // Var para calcular 1 passo = ? mm.
float __mmPorPasso;                 // Var para calcular 1 mm = ? passo.
float __arcCte;                     // Var para calcular a constatnte do arco.

/* Bloco Inicio: Modulo Motor de Passo */
void mpCalc(void)
{
    __totalPasso = 360/(___angPP / ___redEixo);
    __passoPormm = ___periRoda / __totalPasso;
    __mmPorPasso = __totalPasso / ___periRoda;
    __arcCte = (3.14159 * (___distEixos/2))/180;
}

void mpIniciar(void)															// Inicializa o hardware do modulo.
{
	gpio_reset_pin(MPC_STP);													// Limpa configuracoes anteriores.
	gpio_reset_pin(MPC_SLP);													// Limpa configuracoes anteriores.
	gpio_reset_pin(MP0_DIR);													// Limpa configuracoes anteriores.
    gpio_reset_pin(MP1_DIR);													// Limpa configuracoes anteriores.
    gpio_set_direction(MPC_STP, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
    gpio_set_direction(MPC_SLP, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
    gpio_set_direction(MP0_DIR, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
    gpio_set_direction(MP1_DIR, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
	gpio_set_level(MPC_STP,0);													// Limpa o pino.
	gpio_set_level(MPC_SLP,0);													// Limpa o pino.
	gpio_set_level(MP0_DIR,0);													// Limpa o pino.
    gpio_set_level(MP1_DIR,0);													// Limpa o pino.
    mpCalc();
}

void mpPulso(void)																// Gera um pulso no modulo.
{
	gpio_set_level(MPC_STP,1);													// Ativa o pino.
	gpio_set_level(MPC_STP,0);													// Desliga o pino.
}

void mpMover(uint16_t qtdPassos, bool setEsq, bool setDir)
{
    uint16_t cntPulsos = qtdPassos;         // Carrega o valor de passos.
    gpio_set_level(MPC_SLP,1);              // Sai do modo Sleep.
    gpio_set_level(MP0_DIR,setEsq);         // Ativa/Desativa o pino da direcao do motor MP0.
    gpio_set_level(MP1_DIR,setDir);         // Ativa/Desativa o pino da direcao do motor MP1.

    while (cntPulsos!=0)                    // Laco de contagem de passos.
    {
        mpPulso();							// Gera um pulso para o modulo.
		cntPulsos--;						// Decrementa o valor...
		vTaskDelay(1); 						// Aguarda estabilizar a saida.
    }

    gpio_set_level(MPC_SLP,0);              // Entra no modo Sleep.
    gpio_set_level(MP0_DIR,0);              // Desativa o pino da direcao do motor MP0.
    gpio_set_level(MP1_DIR,0);              // Desativa o pino da direcao do motor MP1.
}

void roboFrente(uint16_t dista)
{
    uint16_t qtdPassosFrente=0;             // Variavel iniciada.
    qtdPassosFrente=dista*__mmPorPasso;     // Converte valor para passos.
    mpMover(qtdPassosFrente,0,0);           // Chama funcao de movimento.
}

void roboTraz(uint16_t dista)
{
    uint16_t qtdPassosTraz=0;               // Variavel iniciada.
    qtdPassosTraz = dista * __mmPorPasso;   // Converte valor para passos.
    mpMover(qtdPassosTraz,1,1);             // Chama funcao de movimento.
}

void roboEsq(uint16_t angulo)
{
    uint16_t arcoEsq=0;                     // Variavel iniciada.
    uint16_t qtdPassosEsq=0;                // Variavel iniciada.
    arcoEsq = angulo * __arcCte;            // Converte angulo para valor.
    qtdPassosEsq = arcoEsq * __mmPorPasso;  // Converte valor para passos.
    mpMover(qtdPassosEsq,1,0);              // Chama funcao de movimento.
}

void roboDir(uint16_t angulo)
{
    uint16_t arcoDir=0;                     // Variavel iniciada.
    uint16_t qtdPassosDir=0;                // Variavel iniciada.
    arcoDir = angulo * __arcCte;            // Converte angulo para valor.
    qtdPassosDir = arcoDir * __mmPorPasso;  // Converte valor para passos.
    mpMover(qtdPassosDir,0,1);              // Chama funcao de movimento.
}
/* Bloco Fim: Modulo Motor de Passo */


#endif