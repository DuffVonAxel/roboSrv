#ifndef DISPLAY_H
#define DISPLAY_H
#include "freertos/FreeRTOS.h"													// Acesso aos termos.
#include "freertos/task.h"														// Acesso as prioridades da TASK.
#include "driver/gpio.h"														// Acesso ao uso das GPIOs.
#include <stdio.h>																// Acesso as operacoes de entradas e/ou saidas.
#include <string.h>                                                             // Acesso a manipulacao de strings.
#include <sys/param.h>          												// => MIN()

#define ks0073CS    23
#define ks0073CK    22
#define ks0073DI    21
// #define ks0073DO    04      // Pino numero. Nao sera utilzado o LCD Data Out.

/* GERAL */
#define bitX(valor,bit) (valor&(1<<bit))				                        // Testa e retorna o 'bit' de 'valor'.
#define bit1(valor,bit) valor |= (1<<bit)				                        // Faz o 'bit' de 'valor' =1.
#define bit0(valor,bit) valor &= ~(1<<bit)				                        // Faz o 'bit' de 'valor' =0.

char bat00Char[]={0x0E,0x1B,0x11,0x11,0x11,0x11,0x1F,0x00};
char bat02Char[]={0x0E,0x1B,0x11,0x11,0x11,0x1F,0x1F,0x00};
char bat04Char[]={0x0E,0x1B,0x11,0x11,0x1F,0x1F,0x1F,0x00};
char bat06Char[]={0x0E,0x1B,0x11,0x1F,0x1F,0x1F,0x1F,0x00};
char bat08Char[]={0x0E,0x1B,0x1F,0x1F,0x1F,0x1F,0x1F,0x00};
char bat10Char[]={0x0E,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x00};

void ks0073Pulso(void)
{
    gpio_set_level(ks0073CK,0);                         // Gerar borda...
    gpio_set_level(ks0073CK,1);                         // ...de subida.
}

void ks0073DC(char dado, bool rs)
{
    char cntValor=0;
    // Sequencia de sincronizacao.
    gpio_set_level(ks0073CS,0);	    // Habilita.
    gpio_set_level(ks0073DI,1);	    // Pino ativado para sincronizar.
    for(cntValor=0;cntValor<5;cntValor++) ks0073Pulso();
    gpio_set_level(ks0073DI,0);	    // Bit WR=0 (Modo escrita).
    ks0073Pulso();
    gpio_set_level(ks0073DI,rs);	// Bit RS=? (0=Comando, 1=Dado).
    ks0073Pulso();
    gpio_set_level(ks0073DI,0);	    // Fim do sincronismo.
    ks0073Pulso();
    for(cntValor=0;cntValor<4;cntValor++)
    {
        gpio_set_level(ks0073DI,bitX(dado,cntValor));
        ks0073Pulso();
    }
    gpio_set_level(ks0073DI,0);     // 4 bits restantes serao '0'.
    for(cntValor=0;cntValor<4;cntValor++) ks0073Pulso();
    for(cntValor=4;cntValor<8;cntValor++)
    {
        gpio_set_level(ks0073DI,bitX(dado,cntValor));
        ks0073Pulso();
    }
    gpio_set_level(ks0073DI,0);     // 4 bits restantes serao '0'.
    for(cntValor=0;cntValor<4;cntValor++) ks0073Pulso();
    gpio_set_level(ks0073CS,1);	    // Desabilitado
    gpio_set_level(ks0073CK,1);	    // Desabilitado
}

void ks0073Iniciar(void)
{
    gpio_reset_pin(ks0073CS);
    gpio_reset_pin(ks0073CK);
    gpio_reset_pin(ks0073DI);
    gpio_set_direction(ks0073CS,GPIO_MODE_OUTPUT);
    gpio_set_direction(ks0073CK,GPIO_MODE_OUTPUT);
    gpio_set_direction(ks0073DI,GPIO_MODE_OUTPUT);
    gpio_set_level(ks0073CS,1);	                        // Desabilitado
    gpio_set_level(ks0073CK,1);	                        // Desabilitado
    gpio_set_level(ks0073DI,0);	

    ks0073DC(0x34,0);   // 8 bit data length, extension bit RE=1
    ks0073DC(0x09,0);   // 4 line mode
    ks0073DC(0x30,0);   // 8 bit data length, extension bit RE=0
    ks0073DC(0x0F,0);   // display on, cursor on, cursor blink
    ks0073DC(0x01,0);   // clear display, cursor 1st. row, 1st. line
    ks0073DC(0x06,0);   // cursor will be automatically incremented
}

void ks0073Txt(char *palavra, char lin, char col)
{
    char tmp=0;
    // if(col<1)  col=1;
    // if(col>20) col=20;
    // if(lin<1) lin=1;
    // if(lin>4) lin=4;
    tmp=col-1;
    if(lin==1) ks0073DC(tmp,0);
    if(lin==2) ks0073DC(0x20+tmp,0);
    if(lin==3) ks0073DC(0x40+tmp,0);
    if(lin==4) ks0073DC(0x60+tmp,0);
    while(*palavra)
    {
        ks0073DC(*palavra,1);
        palavra++;
    }
}

/* Sub rotina
void ks0073Carga(void)
{
    // char tmp0=0,tmp1=0;
    char tmp0=0;
    // char pilha[]={bat00Char,bat02Char,bat04Char,bat06Char,bat08Char,bat10Char};
    ks0073DC(0x40,0);
    for(tmp0=0;tmp0<8;tmp0++) {ks0073DC(bat00Char[tmp0],1);}
    ks0073DC(0x48,0);
    for(tmp0=0;tmp0<8;tmp0++) {ks0073DC(bat02Char[tmp0],1);}
    ks0073DC(0x50,0);
    for(tmp0=0;tmp0<8;tmp0++) {ks0073DC(bat04Char[tmp0],1);}
    ks0073DC(0x58,0);
    for(tmp0=0;tmp0<8;tmp0++) {ks0073DC(bat06Char[tmp0],1);}
    ks0073DC(0x60,0);
    for(tmp0=0;tmp0<8;tmp0++) {ks0073DC(bat08Char[tmp0],1);}
    ks0073DC(0x68,0);
    for(tmp0=0;tmp0<8;tmp0++) 
    {
        ks0073DC(bat10Char[tmp0],1);
    }
    

    // for(tmp1=0;tmp1<6;tmp1++)
    // {
    //     ks0073DC(0x40+(tmp1*8),0);
    //     for (tmp0=0;tmp0<8;tmp0++)
    //     {
    //         ks0073DC(pilha[tmp1[tmp0]],1);
    //     }
    // }
}
*/

/* Endreco das Linhas
1st. line $00..$13
2nd. line $20..$33
3rd. line $40..$53
4th. line $60..$73
*/

/* LCD visto por baixo:
1 Vss  0V       18 K (0V)
2 Vdd +5V       17 A -K|--K|-+5V
3 Vee Adj       16 
4 CS            15
5 SID           14 
6 SCLK          13 
7 SOD           12 
8               11 
9               10 

*/

#endif