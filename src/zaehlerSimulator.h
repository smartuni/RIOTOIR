#ifndef __ZAEHLERSIMULATION__
#define __ZAEHLERSIMULATION__
 

#include "periph/gpio.h"

void init(gpio_t pin);

//   Simuliert die uebergebenen wh (Watt*Stunde) in minimaler Zeit (90 ms/wh)
void simConstFast(int wh);


// Simuliert die uebergebenen wh (Watt*Stunde) in der Geschwindigkeit (900 ms/wh)
void simConstSlow(int wh);

// Simuliert die uebergebenen wh (Watt*Stunde) von 90 ms/wh, erhöht pro Puls * die Zeit*1.2 bis zur haelfte der Pulse, dann veringert die Zeit pro Puls um  * Zeit*1.2
void simDecIncFunc(int wh);




#endif //__ZAEHLERSIMULATION__
