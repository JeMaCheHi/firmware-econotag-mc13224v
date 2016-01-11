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
 * Programa principal
 */
int main ()
{
    gpio_init();
    
    //Estado de los leds
    uint8_t led_rojo  = 0;
    uint8_t led_verde = 0;
    
    gpio_clear_pin(GREEN_LED);
    gpio_clear_pin(RED_LED);
    
    //Tecla recibida
    char c;
    
    //Mensaje de error
    char msg[50] = "Error: solo se pueden usar las teclas [g] y [r]\n";

    while (1)
    {
        c = uart_receive_byte(uart_1);
        if(c == 'g'){
            if(led_verde = !led_verde)
                gpio_set_pin(GREEN_LED);
            else 
                gpio_clear_pin(GREEN_LED);
        }
        else if( c == 'r'){
            if(led_rojo = !led_rojo)
                gpio_set_pin(RED_LED);
            else
                gpio_clear_pin(RED_LED);
        }
        else{
            for(i = 0; i < 50; ++i){
                uart_send_byte(uart_1, msg[i]);
            }
        }
    }   

    return 0;
}

/*****************************************************************************/

