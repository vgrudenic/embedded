/**
 * \file heap_check.c
 *
 * \licence "THE BEER-WARE LICENSE" (Revision 42):
 *          <terry.louwers@fourtress.nl> wrote this file. As long as you retain
 *          this notice you can do whatever you want with this stuff. If we
 *          meet some day, and you think this stuff is worth it, you can buy me
 *          a beer in return.
 *                                                                Terry Louwers
 *
 * \brief   Heap check functions for Atmel Cortex-M4.
 *
 * \details This code is intended to be used to determine the heap usage at
 *          runtime. The code is implemented in 'C', to be usable in both 'C'
 *          and 'C++' projects.
 *
 *          As example:
 *          // Include the header as 'C' file
 *          extern "C"
 *          {
 *              #include "heap_check.h"
 *          }
 *
 *          // At a later point check where the block of memory can be allocated:
 *          static volatile uint32_t used_heap = 0;         <-- global to store the (growing) heap value
 *          void Application::GetUsedHeap()
 *          {
 *              uint32_t tmp = get_used_heap();
 *              if (tmp > used_heap)
 *              {
 *                  used_heap = tmp;
 *              }
 *          }
 *
 * \note    This code is not to be used 'as-is': be sure you know where the
 *          stack and heap are located in your project and modify the code to
 *          match these areas.
 *
 * \note    Inspiration from:
 *          https://github.com/angrave/SystemProgramming/wiki/Memory,-Part-1:-Heap-Memory-Introduction
 *          http://library.softwareverify.com/memory-fragmentation-your-worst-nightmare/
 *
 * \author  Terry Louwers (terry.louwers@fourtress.nl)
 * \version 1.0
 * \date    05-2018
 */

/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "heap_check.h"
#include <unistd.h>     // caddr_t


/************************************************************************/
/* Externals                                                            */
/************************************************************************/
/**
 * \brief   Notify we use _estack provided by linker.
 */
extern uint32_t _estack;            // Top of stack

/**
 * \brief   External declaration for _sbrk, in newlib_stubs.c.
 * \param   incr    Size to increment with.
 * \return  Pointer to current used heap memory.
 */
extern caddr_t _sbrk(int incr);


/************************************************************************/
/* Public functions                                                     */
/************************************************************************/
/**
 * \brief   Get the used heap memory size.
 * \return  Used heap size in bytes.
 */
uint32_t get_used_heap(void)
{
    char* heap_end   = (char*)_sbrk(0);
    char* heap_start = (char*)&_estack;

    return (heap_end - heap_start);
}