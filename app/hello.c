/*****************************************************************************/
/*                                                                           */
/* Sistemas Empotrados                                                       */
/* El "hola mundo" en la Redwire EconoTAG en C                               */
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
	gpio_set_port_dir_output(gpio_port_1, 1 << RED_LED | 1 << GREEN_LED);
        
        //Configuramos los pines de los switches
        gpio_set_port_dir_output(gpio_port_0, KBI0 | KBI1);
        gpio_set_port_dir_input(gpio_port_0, KBI4 | KBI5);
        
        //Ponemos un 1 en KBI0 y KBI1 para leer la pulsacion de los switches
        gpio_set_port(gpio_port_0, 1 << KBI0 | 1 << KBI1);
}

/*****************************************************************************/

/*
 * Enciende el led indicado en el pin
 * @param pin Pin para seleccionar leds
 */
void led_on (uint32_t pin)
{
	// Encendemos los leds indicados por el pin
	gpio_set_pin(pin);
}

/*****************************************************************************/

/*
 * Apaga el led indicado en el pin
 * @param pin Pin para seleccionar leds
 */
void led_off (uint32_t pin)
{
	// Apagamos los leds indicado por el pin
	gpio_clear_pin(pin);
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

/**
 * Funcion para comprobar si se ha pulsado un switch
 * @param led_green_mask
 */

void test_buttons(uint32_t * led){
    uint32_t port_data;
    gpio_get_port(gpio_port_0, &port_data);
    if(port_data & (1 << KBI4)){
        *led = GREEN_LED;
    }
    else if(port_data & (1 << KBI5)){
        *led = RED_LED;
    }    
}

/*****************************************************************************/

/*
 * Manejador de instrucciones no definidas
 */
__attribute__ ((interrupt("UNDEF")))
void undef_handler(void){
    gpio_set_pin(GREEN_LED);
}

/*
 * Manejador de interrupciones ASM 
 */

void asm_handler(void){
    gpio_set_pin(GREEN_LED);
    itc_unforce_interrupt(itc_src_asm);
}

/*****************************************************************************/

/*
 * Programa principal
 */
int main ()
{
    gpio_init();

    itc_set_handler(itc_src_asm, asm_handler);
    excep_set_handler(excep_undef, undef_handler);
    
//     itc_enable_interrupt(itc_src_asm);
//     itc_force_interrupt(itc_src_asm);
    
    uint32_t led = RED_LED;
    gpio_set_pin(led);

    while (1)
    {
        test_buttons(&led);
        gpio_set_pin(led);
        pause();
        
        gpio_clear_pin(led);
        test_buttons(&led);
        pause();
        
	
	
	
                
    }   

    return 0;
}

/*****************************************************************************/

