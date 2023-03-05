#include "wifiAP.h"
#include "analog.h"
#include "display.h"
#include "mpasso.h"

static ipCliente1[]={"000.000.000.000"};
static ipCliente2[]={"000.000.000.000"};
static ipCliente3[]={"000.000.000.000"};
static ipCliente4[]={"000.000.000.000"};

static ipRoboSrv0[]={"000.000.000.000"};

void app_main(void)
{
    ks0073Iniciar();
    adcIniciar();
    mpIniciar();
    
    wifi_init();

    while (1) 
    {
        adcLer();
        vTaskDelay(10);
    }
}
