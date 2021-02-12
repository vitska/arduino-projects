#include <avr/io.h>                    // adding header files
#include <util/delay.h>                // for _delay_ms()

int main(void){
    DDRD = 1 << 4;          // setting DDR of PORT D
    while(1)
    {
        // LED on
        PORTD = 1 << 4;
        _delay_ms(10);

        // LED off
        PORTD = 0;
        _delay_ms(90);

        // LED on
        PORTD = 1 << 4;
        _delay_ms(10);

        // LED off
        PORTD = 0;
        _delay_ms(90);

        //LED off
        PORTD = 0;          // PD4 = Low = LED attached on PD4 is OFF
        _delay_ms(800);     // wait 500 milliseconds
    }
}