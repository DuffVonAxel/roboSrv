#include "wifiAP.h"
#include "analog.h"
#include "display.h"
#include "mpasso.h"

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
