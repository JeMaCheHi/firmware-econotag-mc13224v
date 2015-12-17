/*
 * Sistemas operativos empotrados
 * Driver para el GPIO del MC1322x
 */

#include "system.h"

/*****************************************************************************/

/**
 * Acceso estructurado a los registros de control del gpio del MC1322x
 */
typedef struct
{
    uint32_t pad_dir[2];
    uint32_t data[2];
    uint32_t pad_pu_en[2];
    uint32_t func_sel[4];
    uint32_t data_sel[2];
    uint32_t pad_pu_sel[2];
    uint32_t pad_hyst_en[2];
    uint32_t pad_keep[2];
    uint32_t data_set[2];
    uint32_t data_reset[2];
    uint32_t pad_dir_set[2];
    uint32_t pad_dir_reset[2];
} gpio_regs_t;

static volatile gpio_regs_t* const gpio_regs = GPIO_BASE;

/*****************************************************************************/

/**
 * Fija la dirección los pines seleccionados en la máscara como de entrada
 *
 * @param 	port 	Puerto
 * @param 	mask 	Máscara para seleccionar los pines
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_set_port_dir_input (gpio_port_t port, uint32_t mask)
{
    if(port >= gpio_port_max) 
        return gpio_invalid_parameter;
    gpio_regs->pad_dir_reset[port] = mask;
    return gpio_no_error;
}

/*****************************************************************************/

/**
 * Fija la dirección los pines seleccionados en la máscara como de salida
 *
 * @param	port 	Puerto
 * @param	mask 	Máscara para seleccionar los pines
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_set_port_dir_output (gpio_port_t port, uint32_t mask)
{
    if(port >= gpio_port_max) 
        return gpio_invalid_parameter;
    gpio_regs->pad_dir_set[port] = mask;
    return gpio_no_error;
}

/*****************************************************************************/

/**
 * Fija la dirección del pin indicado como de entrada
 *
 * @param	pin 	Número de pin
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_set_pin_dir_input (gpio_pin_t pin)
{
    if(pin >= gpio_pin_max) 
        return gpio_invalid_parameter;
    //Dividimos el numero de pin por 32 para saber el puerto y hacemos
    //el módulo 32 para que todos los pines queden en el rango [0,31]
    gpio_regs->pad_dir_reset[pin >> 5] = ( 1 << (pin & 0x1f) );
    return gpio_no_error;
}

/*****************************************************************************/

/**
 * Fija la dirección del pin indicado como de salida
 *
 * @param	pin 	Número de pin
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_set_pin_dir_output (gpio_pin_t pin)
{
    if(pin >= gpio_pin_max) 
        return gpio_invalid_parameter;
    //Dividimos el numero de pin por 32 para saber el puerto y hacemos
    //el módulo 32 para que todos los pines queden en el rango [0,31]
    gpio_regs->pad_dir_set[pin >> 5] = ( 1 << (pin & 0x1f) );
    return gpio_no_error;
}

/*****************************************************************************/

/**
 * Escribe unos en los pines seleccionados en la máscara
 *
 * @param	port 	Puerto
 * @param	mask 	Máscara para seleccionar los pines
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_set_port (gpio_port_t port, uint32_t mask)
{
    if(port >= gpio_port_max) 
        return gpio_invalid_parameter;
    gpio_regs->data_set[port] = mask;
    return gpio_no_error;
}

/*****************************************************************************/

/**
 * Escribe ceros en los pines seleccionados en la máscara
 *
 * @param	port 	Puerto
 * @param	mask 	Máscara para seleccionar los pines
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_clear_port (gpio_port_t port, uint32_t mask)
{
    if(port >= gpio_port_max) 
        return gpio_invalid_parameter;
    gpio_regs->data_reset[port] = mask;
    return gpio_no_error;
}

/*****************************************************************************/

/**
 * Escribe un uno en el pin indicado
 *
 * @param	pin 	Número de pin
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_set_pin (gpio_pin_t pin)
{
    if(pin >= gpio_pin_max) 
        return gpio_invalid_parameter;
    //Dividimos el numero de pin por 32 para saber el puerto y hacemos
    //el módulo 32 para que todos los pines queden en el rango [0,31]
    gpio_regs->data_set[pin >> 5] = ( 1 << (pin & 0x1f) );
    return gpio_no_error;
}

/*****************************************************************************/

/**
 * Escribe un cero en el pin indicado
 *
 * @param	pin 	Número de pin
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_clear_pin (gpio_pin_t pin)
{
    if(pin >= gpio_pin_max) 
        return gpio_invalid_parameter;
    //Dividimos el numero de pin por 32 para saber el puerto y hacemos
    //el módulo 32 para que todos los pines queden en el rango [0,31]
    gpio_regs->data_reset[pin >> 5] = ( 1 << (pin & 0x1f) );
    return gpio_no_error;
}

/*****************************************************************************/

/**
 * Lee el valor de los pines de un puerto
 *
 * @param	port	  Puerto
 * @param	port_data Valor de los pines del puerto
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			  gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_get_port (gpio_port_t port, uint32_t *port_data)
{
    if(port >= gpio_port_max) 
        return gpio_invalid_parameter;
    *port_data = gpio_regs->data[port];
    return gpio_no_error;
}

/*****************************************************************************/

/**
 * Lee el valor del pin indicado
 *
 * @param	pin	  Número de pin
 * @param       pin_data  Cero si el pin está a cero, distinto de cero en otro caso
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			  gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_get_pin (gpio_pin_t pin, uint32_t *pin_data)
{
    if(pin >= gpio_pin_max || pin < gpio_pin_0) 
        return gpio_invalid_parameter;
    *pin_data = gpio_regs->data[pin >> 5] & ( 1 << (pin & 0x1f) );
    return gpio_no_error;
}

/*****************************************************************************/

/**
 * Fija los pines seleccionados a una función
 *
 * @param	port 	Puerto
 * @param	func	Función
 * @param	mask	Máscara para seleccionar los pines
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_set_port_func (gpio_port_t port, gpio_func_t func, uint32_t mask)
{
    if(port >= gpio_port_max || func >= gpio_func_max) 
        return gpio_invalid_parameter;
    
    uint32_t i, pin, reg, offset;

    for (i=0; i<32; ++i){
        if (mask & (1<<i)){	    // Si el pin se modifica
            pin = i + (port << 5);
            reg = pin >> 4;                 //se divide el pin entre 16 para saber que registro le corresponde
            offset = (pin & 0xf) << 1;      //(pin % 16) * 2
            gpio_regs->func_sel[reg] &= (~(3 << offset));   //bit clear
            gpio_regs->func_sel[reg] |= (func << offset);   //se escribe la funcion
        }
    }

    return gpio_no_error;
}

/*****************************************************************************/

/**
 * Fija el pin seleccionado a una función
 *
 * @param	pin 	Pin
 * @param	func	Función
 * @return	gpio_no_error si los parámetros de entrada son corectos o
 *			gpio_invalid_parameter en otro caso
 */
inline gpio_err_t gpio_set_pin_func (gpio_pin_t pin, gpio_func_t func)
{
    if(pin >= gpio_pin_max || func >= gpio_func_max) 
        return gpio_invalid_parameter;

    uint32_t  reg = pin >> 4,   //se divide el pin entre 16 para saber que registro le corresponde
              offset = (pin & 0xf) << 1; //(pin % 16) * 2

    gpio_regs->func_sel[reg] &= ~( 3 << offset );   //bit clear
    gpio_regs->func_sel[reg] |= ( func << offset ); //se escribe la funcion
    return gpio_no_error;
}

/*****************************************************************************/
