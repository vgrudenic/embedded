/**
 * \file stack_painting.c
 *
 * \licence "THE BEER-WARE LICENSE" (Revision 42):
 *          <terry.louwers@fourtress.nl> wrote this file. As long as you retain
 *          this notice you can do whatever you want with this stuff. If we
 *          meet some day, and you think this stuff is worth it, you can buy me
 *          a beer in return.
 *                                                                Terry Louwers
 *
 * \brief   Stack painting functions for Atmel Cortex-M4.
 *
 * \details This code is intended to be used to determine the stack usage at
 *          runtime. The code is implemented in 'C', to be usable in both 'C'
 *          and 'C++' projects.
 *
 *          As example:
 *          // Include the header as 'C' file
 *          extern "C"
 *          {
 *              #include "stack_painting.h"
 *          }
 *
 *          // Right after the start of main(), 'paint' the stack:
 *          void main(void)
 *          {
 *              paint_stack();          <-- here
 *
 *              // Clocks, pins, remainder ...
 *          }
 *
 *          // At certain intervals log the used stack memory (not too often, every 10 seconds or so):
 *          static volatile uint32_t used_stack = 0;			<-- global to store the (growing) stack value
 *          void Application::GetUsedStack()
 *          {
 *              uint32_t tmp = get_used_stack();
 *              if (tmp > used_stack )
 *              {
 *                  used_stack = tmp;
 *              }
 *          }
 *
 *          // To get an idea of the total stack available:
 *          void Application::GetTotalStack()
 *          {
 *              uint32_t total_stack = get_total_stack();
 *          }
 *
 * \note    This code is not to be used 'as-is': be sure you know where the
 *          stack and heap are located in your project and modify the code to
 *          match these areas.
 *
 * \note    Inspiration from:
 *          https://ucexperiment.wordpress.com/2015/01/02/arduino-stack-painting/
 *          https://embeddedgurus.com/stack-overflow/2009/03/computing-your-stack-size/
 *
 * \author  Terry Louwers (terry.louwers@fourtress.nl)
 * \version 1.0
 * \date    05-2018
 */

 /************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "stack_painting.h"
#include <interrupt.h>


/************************************************************************/
/* Externals                                                            */
/************************************************************************/
/**
 * \brief   Notify we use _sstack provided by linker.
 */
extern uint32_t _sstack;            // Bottom of stack

/**
 * \brief   Notify we use _estack provided by linker.
 */
extern uint32_t _estack;            // Top of stack


/************************************************************************/
/* Constants                                                            */
/************************************************************************/
/**
 * \brief   The value used as 'paint'.
 */
const uint32_t PAINT_VALUE = 0xC5C5C5C5;


/************************************************************************/
/* Static variables                                                     */
/************************************************************************/
static uint32_t total_stack_size = 0;
static uint32_t used_stack_size  = 0;


/************************************************************************/
/* Public functions                                                     */
/************************************************************************/
/**
 * \brief   Fills all of the stack with a defined value (PAINT_VALUE).
 * \note    Should be done as one of the first things in main().
 */
void paint_stack(void)
{
    // Find the top and bottom of the stack in the linker file: 'flash.ld'
    uint32_t* bottom_of_stack = &_sstack;

    // Get the space occupied by the application: stack_pointer since start of SRAM
    uint32_t application = __get_MSP();

    // Find out what needs to be 'painted'
    uint32_t area_to_paint = (application - (uint32_t)bottom_of_stack) / 4;

    // Paint that area
    for (uint32_t i = 0; i < area_to_paint; i++)
    {
        bottom_of_stack[i] = PAINT_VALUE;
    }
}

/**
 * \brief   Get the total amount of stack available.
 * \return  Total stack size in bytes.
 */
uint32_t get_total_stack(void)
{
    // Could be we never called 'get_used_stack' before.
    if (total_stack_size == 0)
    {
        get_used_stack();
    }

    return total_stack_size;
}

/**
 * \brief   Get the (once) used stack size.
 * \return  Used stack size in bytes.
 */
uint32_t get_used_stack(void)
{
    // Prevent interrupts during this section
    irqflags_t irq_state = cpu_irq_save();

    // Find the top and bottom of the stack in the linker file: 'flash.ld'
    uint32_t* bottom_of_stack = &_sstack;
    uint32_t* top_of_stack    = &_estack;

    uint32_t area_to_search = (top_of_stack - bottom_of_stack);

    uint32_t i;
    for (i = 1; i < area_to_search; ++i)
    {
        if (bottom_of_stack[i] != PAINT_VALUE)
        {
            break;
        }
    }

    // Restore interrupts
    cpu_irq_restore(irq_state);

    total_stack_size = (area_to_search * 4);            // * 4: byte to uint32_t
    used_stack_size  = (total_stack_size) - (i * 4);

    return used_stack_size;
}
