/*****************************************************************************/
/*                                                                           */
/* Sistemas Empotrados                                                       */
/* Programa para testear el driver de las UART                               */
/*                                                                           */
/*****************************************************************************/

#include <stdint.h>
#include "system.h"

/*
 * Constantes relativas a la plataforma
 */

// El led rojo está en el GPIO 44
#define RED_LED gpio_pin_44

// El led verde está en el GPIO 45
#define GREEN_LED gpio_pin_45

// Pin de salida del switch S3
#define KBI0            gpio_pin_22

// Pin de salida del switch S2
#define KBI1            gpio_pin_23

// Pin de entrada del switch S3
#define KBI4            gpio_pin_26

// Pin de entrada del switch S2
#define KBI5            gpio_pin_27

/*
 * Constantes relativas a la aplicacion
 */
uint32_t const delay = 0x10000;
 
//Estado de los leds en variables globales
uint8_t blink_red_led = 1;
uint8_t blink_green_led = 1;

/*****************************************************************************/

/*
 * Inicialización de los pines de E/S
 */
void gpio_init(void)
{
    // Configuramos el GPIO44 y GPIO45 para que sea de salida
    gpio_set_port_dir_output(gpio_port_1, 1 << (RED_LED - 32) | 1 << (GREEN_LED - 32));

    //Configuramos los pines de los switches
    gpio_set_port_dir_output(gpio_port_0, 1 << KBI0 | 1 << KBI1);
    gpio_set_port_dir_input(gpio_port_0, 1 << KBI4 | 1 << KBI5);

    //Ponemos un 1 en KBI0 y KBI1 para leer la pulsacion de los switches
    gpio_set_port(gpio_port_0, 1 << KBI0 | 1 << KBI1);
}

/*****************************************************************************/

/*
 * Retardo para el parpedeo
 */
void pause(void)
{
        uint32_t i;
	for (i=0 ; i<delay ; i++);
}

/*****************************************************************************/

/*
 * Callback de recepción
 */

void my_rx_callback(void){
    
    //Mensaje de error
    char c;
    char msg[50] = "Error: solo se pueden usar las teclas [g] y [r]:\r\n";
    uart_receive(uart_1, &c, 1);
    if(c == 'g'){
        blink_green_led= !blink_green_led;
    }
    else if( c == 'r'){
        blink_red_led = !blink_red_led;
    }
    else{
        uart_send(uart_1, msg, 50);
    }
}

/*****************************************************************************/

/*
 * Programa principal
 */
int main ()
{
    gpio_init();
    
    uart_set_receive_callback(uart_1, (uart_callback_t) my_rx_callback);

    while (1)
    {
        //Hacer el parpadeo de los leds
        if(blink_green_led){
            gpio_set_pin(GREEN_LED);
        }
        if(blink_red_led){
            gpio_set_pin(RED_LED);
        }
        pause();
        
        //Apagar los leds, si el parpadeo esta desactivado, se quedaran apagados.
        gpio_clear_pin(GREEN_LED);
        gpio_clear_pin(RED_LED);
        
        pause();
    
    }   

    return 0;
}

/*****************************************************************************/

