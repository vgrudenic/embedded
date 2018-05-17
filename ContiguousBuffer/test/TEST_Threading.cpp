
#include "../../Catch/catch.hpp"

#include "ContiguousRingbuffer.hpp"
#include <cstddef>      // size_t
#include <cstdint>      // uint8_t, uint16_t
#include <thread>


static ContiguousRingbuffer<int> ringBuff;          // Our buffer used for testing

static const int   NR_ITEMS_THREAD_TEST  = 2000;
static int refArr [NR_ITEMS_THREAD_TEST] = {};
static int measArr[NR_ITEMS_THREAD_TEST] = {};


void Producer(size_t nr_items)
{
    // Use a 'const' to prevent changes to 'nr_of_items'
    const size_t nr_of_items = nr_items;
    REQUIRE(nr_of_items > 0);

    int* data_prod = nullptr;
    size_t size_prod;

    for (size_t i = 0; i < NR_ITEMS_THREAD_TEST; i += nr_of_items)
    {
        // Try to put an item in the queue, but queue may be full.
        // Assume Consumer will empty it so we can continue.

        volatile bool result = false;
        do
        {
            sched_yield();

            size_prod = nr_of_items;
            if (ringBuff.Poke(data_prod, size_prod))
            {
                size_t k = i;
                for (size_t j = 0; j < nr_of_items; j++)
                {
                    *data_prod++ = refArr[k++];
                }

                result = ringBuff.Write(nr_of_items);
            }
        } while (result == false);
    }
}

void Consumer(size_t nr_items)
{
    // Use a 'const' to prevent changes to 'nr_of_items'
    const size_t nr_of_items = nr_items;
    REQUIRE(nr_of_items > 0);

    int* data_cons = nullptr;
    size_t size_cons;

    for (size_t i = 0; i < NR_ITEMS_THREAD_TEST; i += nr_of_items)
    {
        // Try to get an item from the queue, but queue may be empty.
        // Assume the Producer will fill the queue.

        volatile bool result = false;
        do
        {
            sched_yield();

            size_cons = nr_of_items;
            if (ringBuff.Peek(data_cons, size_cons))
            {
                size_t k = i;
                for (size_t j = 0; j < nr_of_items; j++)
                {
                    measArr[k++] = *data_cons++;
                }
                result = ringBuff.Read(nr_of_items);
            }
        } while (result == false);
    }
}

void Threaded_Iteration(size_t buffer_size, uint16_t nr_of_runs, uint8_t prod_nr_items, uint8_t cons_nr_items)
{
    REQUIRE(nr_of_runs > 0);
    REQUIRE(NR_ITEMS_THREAD_TEST % prod_nr_items == 0);         // As NR_ITEMS_THREAD_TEST is used, be sure we can completely finish our run
    REQUIRE(NR_ITEMS_THREAD_TEST % cons_nr_items == 0);


    for (auto run = 0; run < nr_of_runs; run++)
    {
        REQUIRE(ringBuff.Resize(buffer_size) == true);          // Clears previous state

        // Clear the measurement array for each iteration
        for (auto i = 0; i < NR_ITEMS_THREAD_TEST; i++)
        {
            measArr[i] = 0;
        }

        // Create and immediately start threads
        std::thread prod(Producer, prod_nr_items);
        std::thread cons(Consumer, cons_nr_items);

        // Threads running...

        // Cleanup when done
        prod.join();
        cons.join();

        // Validate results
        for (auto i = 0; i < NR_ITEMS_THREAD_TEST; i++)
        {
            REQUIRE(refArr[i] == measArr[i]);
        }
    }
}


TEST_CASE( "ContiguousRingbuffer threading operations", "[ContiguousRingbuffer]" )
{
    // Fill the reference array with 'known' values
    for (auto i = 0; i < NR_ITEMS_THREAD_TEST; i++)
    {
        refArr[i] = i;
    }

    const size_t buffer_size = 15;
    const uint16_t nrOfRuns  = 200;

    SECTION( "threading with Producer(1), Consumer(1)" )
    {
        Threaded_Iteration(buffer_size, nrOfRuns, 1, 1);
    }
    SECTION( "threading with Producer(1), Consumer(2)" )
    {
        Threaded_Iteration(buffer_size, nrOfRuns, 1, 2);
    }
    SECTION( "threading with Producer(2), Consumer(1)" )
    {
        Threaded_Iteration(buffer_size, nrOfRuns, 2, 1);
    }
    SECTION( "threading with Producer(2), Consumer(2)" )
    {
        Threaded_Iteration(buffer_size, nrOfRuns, 2, 2);
    }
    SECTION( "threading with Producer(4), Consumer(1)" )
    {
        Threaded_Iteration(buffer_size, nrOfRuns, 4, 1);
    }
    SECTION( "threading with Producer(1), Consumer(4)" )
    {
        Threaded_Iteration(buffer_size, nrOfRuns, 1, 4);
    }
    SECTION( "threading with Producer(4), Consumer(4)" )
    {
        Threaded_Iteration(buffer_size, nrOfRuns, 4, 4);
    }
}
