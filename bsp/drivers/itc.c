/*
 * Sistemas operativos empotrados
 * Driver para el controlador de interrupciones del MC1322x
 */

#include "system.h"

/*****************************************************************************/

/**
 * Acceso estructurado a los registros de control del ITC del MC1322x
 */
typedef struct
{
    volatile uint32_t intcntl;          //0x80020000
    volatile uint32_t nimask;           //0x80020004
    volatile uint32_t intennum;         //0x80020008
    volatile uint32_t intdisnum;        //0x8002000C
    volatile uint32_t intenable;        //0x80020010
    volatile uint32_t inttype;          //0x80020014
    const uint32_t reserved[4];         //0x80020018-0x80020024
    volatile uint32_t const nivector;   //0x80020028;
    volatile uint32_t const fivector;   //0x8002002C;
    volatile uint32_t const intsrc;     //0x80020030;
    volatile uint32_t intfrc;           //0x80020034;
    volatile uint32_t const nipend;     //0x80020038;
    volatile uint32_t const fipend;     //0x8002003C;
} itc_regs_t;

static volatile itc_regs_t* const itc_regs = ITC_BASE;

/**
 * Tabla de manejadores de interrupción.
 */
static itc_handler_t itc_handlers[itc_src_max];

/**
 * Estado de las interrupciones
 */

static uint32_t itc_ints_status;

/*****************************************************************************/

/**
 * Inicializa el controlador de interrupciones.
 * Deshabilita los bits I y F de la CPU, inicializa la tabla de manejadores a NULL,
 * y habilita el arbitraje de interrupciones Normales y rápidas en el controlador
 * de interupciones.
 */
inline void itc_init ()
{
        uint8_t i;
        for(i = 0; i < itc_src_max; ++i)
            itc_handlers[i] = (uint32_t) 0x0;
        itc_regs->intfrc = (uint32_t) 0x0;
        itc_regs->intenable = (uint32_t) 0x0;
        //ponemos un 1 en las posiciones 19 y 20
        itc_regs->intcntl &= (~( 3 << 19 ));
}

/*****************************************************************************/

/**
 * Deshabilita el envío de peticiones de interrupción a la CPU
 * Permite implementar regiones críticas en modo USER
 */
inline void itc_disable_ints ()
{
        itc_ints_status = itc_regs->intenable;
        itc_regs->intenable = (uint32_t) 0;
}

/*****************************************************************************/

/**
 * Vuelve a habilitar el envío de peticiones de interrupción a la CPU
 * Permite implementar regiones críticas en modo USER
 */
inline void itc_restore_ints ()
{
        itc_regs->intenable = itc_ints_status;
}

/*****************************************************************************/

/**
 * Asigna un manejador de interrupción
 * @param src		Identificador de la fuente
 * @param handler	Manejador
 */
inline void itc_set_handler (itc_src_t src, itc_handler_t handler)
{
	itc_handlers[src] = handler;
}

/*****************************************************************************/

/**
 * Asigna una prioridad (normal o fast) a una fuente de interrupción
 * @param src		Identificador de la fuente
 * @param priority	Tipo de prioridad
 */
inline void itc_set_priority (itc_src_t src, itc_priority_t priority)	
{
        if(priority)
            /*Si hay alguna interrupcion mapeada como FIQ escribe el bit
            encima y la pone como normal */
            itc_regs->inttype = (uint32_t) (1 << src);
        else
            itc_regs->inttype &= (uint32_t) ~(1 << src);
}

/*****************************************************************************/

/**
 * Habilita las interrupciones de una determinda fuente
 * @param src		Identificador de la fuente
 */
inline void itc_enable_interrupt (itc_src_t src)
{
        itc_regs->intenable |= (uint32_t) (1 << src);
}

/*****************************************************************************/

/**
 * Deshabilita las interrupciones de una determinda fuente
 * @param src		Identificador de la fuente
 */
inline void itc_disable_interrupt (itc_src_t src)
{
        uint32_t tmp = (1 << src);
        itc_regs->intenable &= ~tmp;
        
}

/*****************************************************************************/

/**
 * Fuerza una interrupción con propósitos de depuración
 * @param src		Identificador de la fuente
 */
inline void itc_force_interrupt (itc_src_t src)
{
        itc_regs->intfrc |= (1 << src);
}

/*****************************************************************************/

/**
 * Desfuerza una interrupción con propósitos de depuración
 * @param src		Identificador de la fuente
 */
inline void itc_unforce_interrupt (itc_src_t src)
{
	/* ESTA FUNCIÓN SE DEFINIRÁ EN LA PRÁCTICA 6 */
        itc_regs->intfrc &= ~(1 << src);
}

/*****************************************************************************/

/**
 * Da servicio a la interrupción normal pendiente de más prioridad.
 * Deshabilia las IRQ de menor prioridad hasta que se haya completado el servicio
 * de la IRQ para evitar inversiones de prioridad
 */
void itc_service_normal_interrupt ()
{
        //obtener el numero de interrupción mas prioritaria
        uint8_t pri = itc_regs->nivector;
        //Deshabilitar las interrupciones menos prioritarias
        itc_regs->nimask = pri;
        //llamar al manejador de la interrupcion mas prioritaria
        itc_handlers[pri]();
        //al retornar, rehabilitar todas las interrupciones
        itc_regs->nimask = 0x31;
}

/*****************************************************************************/

/**
 * Da servicio a la interrupción rápida pendiente de más prioridad
 */
void itc_service_fast_interrupt ()
{
        //Obtener el indice del manejador de la fiq y llamar a la rutina
        itc_handlers[itc_regs->fivector]();
}

/*****************************************************************************/
