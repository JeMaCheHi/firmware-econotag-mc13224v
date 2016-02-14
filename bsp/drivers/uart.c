/*
 * Sistemas operativos empotrados
 * Driver de las uart
 */

#include <fcntl.h>
#include <errno.h>
#include "system.h"
#include "circular_buffer.h"

/*****************************************************************************/

/**
 * Acceso estructurado a los registros de control de las uart del MC1322x
 */

typedef struct
{
    //UART Control Register
    union
    {
        //Acceso directo
        uint32_t ucon;
        //acceso a flags con bitfields
        struct
        {
            uint32_t TxE        : 1;
            uint32_t RxE        : 1;
            uint32_t PEN        : 1;
            uint32_t EP         : 1;
            uint32_t ST2        : 1;
            uint32_t SB         : 1;
            uint32_t conTx      : 1;
            uint32_t Tx_oen_b   : 1;
            uint32_t            : 1;
            uint32_t Res        : 1;
            uint32_t xTIM       : 1;
            uint32_t FCp        : 1;
            uint32_t FCe        : 1;
            uint32_t MTxR       : 1;
            uint32_t MRxR       : 1;
            uint32_t TST        : 1;
        };
    };            
    //UART Status Register
    union
    {
        uint32_t ustat;
        struct
        {
            uint32_t SE         : 1;
            uint32_t PE         : 1;
            uint32_t FE         : 1;
            uint32_t TOE        : 1;
            uint32_t ROE        : 1;
            uint32_t RUE        : 1;
            uint32_t RxRdy      : 1;
            uint32_t TxRdy      : 1;
        };
    };
    //UART Data Register
    union{
        uint8_t Rx_data;
        uint8_t Tx_data;
        uint32_t udata;
    };
    //UART RxBuffer Control Register
    union
    {
        uint32_t RxLevel            : 5;
        uint32_t Rx_fifo_addr_diff  : 6;
        uint32_t urxcon;
    };
    //UART TxBuffer Control Register
    union
    {
        uint32_t TxLevel            : 5;
        uint32_t Tx_fifo_addr_diff  : 6;
        uint32_t utxcon;
    };
    //UART CTS Level Control Register
    uint32_t ucts;
    //UART Baud Rate Divider Register
    union
    {
        uint32_t ubr;
        struct
        {
            uint32_t ubrmod         : 16;
            uint32_t ubrinc         : 16;
        };
    };

} uart_regs_t;

/*****************************************************************************/

/**
 * Acceso estructurado a los pines de las uart del MC1322x
 */
typedef struct
{
	gpio_pin_t tx,rx,cts,rts;
} uart_pins_t;

/*****************************************************************************/

/**
 * Definición de las UARTS
 */
static volatile uart_regs_t* const uart_regs[uart_max] = {UART1_BASE, UART2_BASE};

static const uart_pins_t uart_pins[uart_max] = {
		{gpio_pin_14, gpio_pin_15, gpio_pin_16, gpio_pin_17},
		{gpio_pin_18, gpio_pin_19, gpio_pin_20, gpio_pin_21} };

static void uart_1_isr (void);
static void uart_2_isr (void);
static const itc_handler_t uart_irq_handlers[uart_max] = {uart_1_isr, uart_2_isr};

/*****************************************************************************/

/**
 * Tamaño de los búferes circulares
 */
#define __UART_BUFFER_SIZE__	256

static volatile uint8_t uart_rx_buffers[uart_max][__UART_BUFFER_SIZE__];
static volatile uint8_t uart_tx_buffers[uart_max][__UART_BUFFER_SIZE__];

static volatile circular_buffer_t uart_circular_rx_buffers[uart_max];
static volatile circular_buffer_t uart_circular_tx_buffers[uart_max];


/*****************************************************************************/

/**
 * Gestión de las callbacks
 */
typedef struct
{
	uart_callback_t tx_callback;
	uart_callback_t rx_callback;
} uart_callbacks_t;

static volatile uart_callbacks_t uart_callbacks[uart_max];

/*****************************************************************************/

/**
 * Inicializa una uart
 * @param uart	Identificador de la uart
 * @param br	Baudrate
 * @param name	Nombre del dispositivo
 * @return		Cero en caso de éxito o -1 en caso de error.
 * 				La condición de error se indica en la variable global errno
 */
int32_t uart_init (uart_id_t uart, uint32_t br, const char *name)
{
//     	uint32_t inc, mod;
// 
// 	/* Sólo hay dos uarts */
// 	if (uart >= uart_max)
// 	{
// 		errno = ENODEV; /* El dispositivo no existe */
// 		return -1;
// 	}
// 
//         /* Compruebo el nombre */
// 	if (!name)
// 	{
// 		errno = EFAULT; /* Dirección no válida */
// 		return -1;
// 	}
// 
// 	/* La uart debe estar deshabilitada para fijar la frecuencia */
// 	uart_regs[uart]->ucon =	(1 << 13) |		/* MTxR = 1 - Enmascaramos las interrupciones */
// 							(1 << 14);		/* MRxR = 1 */
// 
// 	/* Una vez fijado xTIM, y con la UART desabilitada, fijamos la frecuencia */
// 	mod = 9999;
// 	inc = br*mod/(CPU_FREQ>>4);			/* Asumimos un oversampling de 8x */
// 	uart_regs[uart]->ubr = ( inc << 16 ) | mod;
// 
// 	/* Hay que habilitar el periférico antes fijar la función en GPIO_FUNC_SEL */
// 	/* Consultar la sección 11.5.1.2 Alternate Modes del datasheet: */
// 	/* "The peripheral function will control operation of the pad IF THE PERIPHERAL IS ENABLED." */
// 	uart_regs[uart]->ucon |=	(1 << 0) |		/* TxE = 1 - Habilitamos la transmisión */
// 				(1 << 1);		/* RxE = 1 - Habilitamos la recepción */
// 
// 	/* Cambiamos la función de los pines */
// 	gpio_set_pin_func (uart_pins[uart].tx, gpio_func_alternate_1);
// 	gpio_set_pin_func (uart_pins[uart].rx, gpio_func_alternate_1);
// 	gpio_set_pin_func (uart_pins[uart].cts, gpio_func_alternate_1);
// 	gpio_set_pin_func (uart_pins[uart].rts, gpio_func_alternate_1);
// 
// 	/* Fijamos TX y CTS como salidas y RX y RTS como entradas*/
// 	gpio_set_pin_dir_output (uart_pins[uart].tx);
// 	gpio_set_pin_dir_output (uart_pins[uart].cts);
// 	gpio_set_pin_dir_input (uart_pins[uart].rx);
// 	gpio_set_pin_dir_input (uart_pins[uart].rts);
// 
// 	/* Gestión de los búferes de rececepción y transmisión */
// 	circular_buffer_init (&uart_circular_rx_buffers[uart], (uint8_t *) uart_rx_buffers[uart], sizeof(uart_rx_buffers[uart]));
// 	circular_buffer_init (&uart_circular_tx_buffers[uart], (uint8_t *) uart_tx_buffers[uart], sizeof(uart_tx_buffers[uart]));
// 
// 	/* Programamos cuando se deben generar las interrupciones */
// 	uart_regs[uart]->TxLevel = 31;	/* Cuando la cola de envío esté vacía */
//         uart_regs[uart]->RxLevel = 1;	/* Cuando se haya recibido algún carácter */
// 
// 	/* Habilitamos las interrupciones de la uart en el ICT */
// 	itc_set_priority (itc_src_uart1 + uart, itc_priority_normal);
// 	itc_set_handler (itc_src_uart1 + uart, uart_irq_handlers[uart]);
// 	itc_enable_interrupt (itc_src_uart1 + uart);
// 
// 	/* Por defecto no hay funciones callback */
// 	uart_callbacks[uart].tx_callback = NULL;
// 	uart_callbacks[uart].rx_callback = NULL;
// 
// 	/* Habilitamos la generación de interrupciones por la uart en la recepción */
// 	uart_regs[uart]->MRxR =	0;
// 
// 	/* Registramos el dispositivo. Implementación del driver de nivel 2 */
// 	bsp_register_dev (name, uart, NULL, NULL, uart_receive, uart_send, NULL, NULL, NULL);
// 
// 	return 0;
    //Comprobacion de errores
    if(uart >= uart_max){
        errno = ENODEV;
        return -1;
    }
    if(name == NULL){
        errno = EFAULT;
        return -1;
    }
    //Desactivación de TxE, RxE, MTxR y MRxR
    uart_regs[uart]->ucon = 0x6000;
    
    //Se establecen los baudios de la UART
    uart_regs[uart]->ubrmod = 9999;
    uart_regs[uart]->ubrinc = (br * 9999) / (CPU_FREQ >> 4);
    
    //Rehabilitamos la uart
    uart_regs[uart]->TxE = 1;
    uart_regs[uart]->RxE = 1;

    //Fijamos la función de los pines
    gpio_set_pin_func(uart_pins[uart].tx, gpio_func_alternate_1);
    gpio_set_pin_func(uart_pins[uart].rx, gpio_func_alternate_1);
    gpio_set_pin_func(uart_pins[uart].cts, gpio_func_alternate_1);
    gpio_set_pin_func(uart_pins[uart].rts, gpio_func_alternate_1);
    
    //Y fijamos entradas/salidas
    gpio_set_pin_dir_output(uart_pins[uart].tx);
    gpio_set_pin_dir_output(uart_pins[uart].cts);
    gpio_set_pin_dir_input(uart_pins[uart].rx);
    gpio_set_pin_dir_input(uart_pins[uart].rts);
    
    //Inicializamos los bufferes circulares
    circular_buffer_init(&uart_circular_rx_buffers[uart], (uint8_t *) uart_rx_buffers[uart], sizeof(uart_rx_buffers[uart]));
    circular_buffer_init(&uart_circular_tx_buffers[uart], (uint8_t *) uart_tx_buffers[uart], sizeof(uart_tx_buffers[uart]));
    
    //Fijamos cuantos bytes deben haber en el buffer para que se activen las interrupciones
    uart_regs[uart]->RxLevel = 1;
    uart_regs[uart]->TxLevel = 31;
    
    /* Habilitamos las interrupciones de la uart en el ICT */
    
    //Configuramos las interrupciones en el ITC
    itc_set_handler(itc_src_uart1 + uart, uart_irq_handlers[uart]);
    itc_set_priority(itc_src_uart1 + uart, itc_priority_normal);
    itc_enable_interrupt(itc_src_uart1 + uart);
    
    //Configuramos los callbacks como vacíos
    uart_callbacks[uart].tx_callback = NULL;
    uart_callbacks[uart].rx_callback = NULL;
    
    //Y habilitamos las interrupciones de recepción
    uart_regs[uart]->MRxR = 0;
    
    bsp_register_dev (name, uart, NULL, NULL, uart_receive, uart_send, NULL, NULL, NULL);
    
    return 0;
}

/*****************************************************************************/

/**
 * Transmite un byte por la uart
 * Implementación del driver de nivel 0. La llamada se bloquea hasta que transmite el byte
 * @param uart	Identificador de la uart
 * @param c		El carácter
 */
void uart_send_byte (uart_id_t uart, uint8_t c)
{
    
    //Deshabilitamos las interrupciones del transmisor
    uint32_t temp_interrupt = uart_regs[uart]->MTxR;
    uart_regs[uart]->MTxR = 1;
    
    //Volcamos todo lo que haya en el buffer circular en el FIFO.
    while(!circular_buffer_is_empty(&uart_circular_tx_buffers[uart])){
        while(uart_regs[uart]->Tx_fifo_addr_diff > 0)
            uart_regs[uart]->Tx_data = circular_buffer_read(&uart_circular_rx_buffers[uart]);
    }
    
    //Bloquear mientras no haya espacio,
    while(uart_regs[uart]->Tx_fifo_addr_diff == 0);
    uart_regs[uart]->Tx_data = c;
    
    //Volver a dejar las interrupciones como estaban
    uart_regs[uart]->MTxR = temp_interrupt;

}

/*****************************************************************************/

/**
 * Recibe un byte por la uart
 * Implementación del driver de nivel 0. La llamada se bloquea hasta que recibe el byte
 * @param uart	Identificador de la uart
 * @return		El byte recibido
 */
uint8_t uart_receive_byte (uart_id_t uart)
{
    //Deshabilitamos las interrupciones del receptor
    uint32_t temp_interrupt = uart_regs[uart]->MRxR;
    uint8_t ret;
    uart_regs[uart]->MRxR = 1;
    
    //Si hay algo pendiente en el buffer extraemos de ahi
    if(!circular_buffer_is_empty(&uart_circular_rx_buffers[uart])){
        ret = circular_buffer_read(&uart_circular_rx_buffers[uart]);
    }
    //Si no se extrae directamente del buffer de la uart, bloqueando si es necesario.
    else{
        while(uart_regs[uart]->Rx_fifo_addr_diff == 0);
        ret = uart_regs[uart]->Rx_data;
    }
    
    //Devolvemos las interrupciones a su estado anterior
    uart_regs[uart]->MRxR = temp_interrupt;
    
    return ret;
}

/*****************************************************************************/

/**
 * Transmisión de bytes
 * Implementación del driver de nivel 1. La llamada es no bloqueante y se realiza mediante interrupciones
 * @param uart	Identificador de la uart
 * @param buf	Búfer con los caracteres
 * @param count	Número de caracteres a escribir
 * @return	El número de bytes almacenados en el búfer de transmisión en caso de éxito o
 *              -1 en caso de error.
 * 		La condición de error se indica en la variable global errno
 */
ssize_t uart_send (uint32_t uart, char *buf, size_t count)
{
    //comprobación de errores
    if(uart >= uart_max){
        errno = ENODEV;
        return -1;
    }
    if(!buf || count < 0){
        errno = EFAULT;
        return -1;
    }
    
    
    //Deshabilitación de interrupciones
    uart_regs[uart]->MTxR = 1;
    ssize_t written_bytes = 0;
    
    while(!circular_buffer_is_full(&uart_circular_tx_buffers[uart]) && count > 0){
        circular_buffer_write(&uart_circular_tx_buffers[uart], *buf++);
        written_bytes++;
        count--;
    }
    
    uart_regs[uart]->MTxR = 0;
    
    return written_bytes;
}

/*****************************************************************************/

/**
 * Recepción de bytes
 * Implementación del driver de nivel 1. La llamada es no bloqueante y se realiza mediante interrupciones
 * @param uart	Identificador de la uart
 * @param buf	Búfer para almacenar los bytes
 * @param count	Número de bytes a leer
 * @return	El número de bytes realmente leídos en caso de éxito o
 *              -1 en caso de error.
 * 		La condición de error se indica en la variable global errno
 */
ssize_t uart_receive (uint32_t uart, char *buf, size_t count)
{
    //comprobación de errores
    if(uart >= uart_max){
        errno = ENODEV;
        return -1;
    }
    if(!buf || count < 0){
        errno = EFAULT;
        return -1;
    }
    
    ssize_t read_bytes = 0;
    
    //Deshabilitación de interrupciones
    uart_regs[uart]->MRxR = 1;
    
    while(!circular_buffer_is_empty(&uart_circular_rx_buffers[uart]) && count > 0){
        *buf++ = circular_buffer_read(&uart_circular_rx_buffers[uart]);
        read_bytes++;
        count--;
    }
        
    uart_regs[uart]->MRxR = 0;
    
    return read_bytes;
}

/*****************************************************************************/

/**
 * Fija la función callback de recepción de una uart
 * @param uart	Identificador de la uart
 * @param func	Función callback. NULL para anular una selección anterior
 * @return	Cero en caso de éxito o -1 en caso de error.
 * 		La condición de error se indica en la variable global errno
 */
int32_t uart_set_receive_callback (uart_id_t uart, uart_callback_t func)
{
    //comprobación de errores
    if(uart >= uart_max){
        errno = ENODEV;
        return -1;
    }
    
    uart_callbacks[uart].rx_callback = func;
    
    return 0;
}

/*****************************************************************************/

/**
 * Fija la función callback de transmisión de una uart
 * @param uart	Identificador de la uart
 * @param func	Función callback. NULL para anular una selección anterior
 * @return	Cero en caso de éxito o -1 en caso de error.
 * 		La condición de error se indica en la variable global errno
 */
int32_t uart_set_send_callback (uart_id_t uart, uart_callback_t func)
{
    //comprobación de errores
    if(uart >= uart_max){
        errno = ENODEV;
        return -1;
    }
    
    uart_callbacks[uart].tx_callback = func;
    
    return 0;
}

/*****************************************************************************/

/**
 * Manejador genérico de interrupciones para las uart.
 * Cada isr llamará a este manejador indicando la uart en la que se ha
 * producido la interrupción.
 * Lo declaramos inline para reducir la latencia de la isr
 * @param uart	Identificador de la uart
 */
static inline void uart_isr (uart_id_t uart)
{
    uint32_t status = uart_regs[uart]->ustat;
    
    //Gestión de interrupciones de recepción
    if(uart_regs[uart]->RxRdy){
        while(!circular_buffer_is_full(&uart_circular_rx_buffers[uart]) && (uart_regs[uart]->Rx_fifo_addr_diff > 0)){
            circular_buffer_write(&uart_circular_rx_buffers[uart], uart_regs[uart]->Rx_data);
        }
        
        if(uart_callbacks[uart].rx_callback) 
            uart_callbacks[uart].rx_callback();
        
        if(circular_buffer_is_full(&uart_circular_rx_buffers[uart]))
            uart_regs[uart]->MRxR = 1;
    }

    //Gestión de interrupciones de transmisión
    if(uart_regs[uart]->TxRdy){
        while(!circular_buffer_is_empty(&uart_circular_tx_buffers[uart]) && (uart_regs[uart]->Tx_fifo_addr_diff > 0)){
            uart_regs[uart]->Tx_data = circular_buffer_read(&uart_circular_tx_buffers[uart]);
        }
        
        if(uart_callbacks[uart].tx_callback) 
            uart_callbacks[uart].tx_callback();
        
        if(circular_buffer_is_empty(&uart_circular_tx_buffers[uart]))
            uart_regs[uart]->MTxR = 1;
    }
    
}

/*****************************************************************************/

/**
 * Manejador de interrupciones para la uart1
 */
static void uart_1_isr (void)
{
	uart_isr(uart_1);
}

/*****************************************************************************/

/**
 * Manejador de interrupciones para la uart2
 */
static void uart_2_isr (void)
{
	uart_isr(uart_2);
}

/*****************************************************************************/
