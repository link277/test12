#include <stdbool.h>
#include <stdio.h>
#include "datablock.h"

/**********************************************************
 **********************************************************
 * This file contains functions to simulate storing data. *
 *             DO NOT MODIFY THIS FILE                    *
 **********************************************************
 **********************************************************
 */

//the location where data must be stored. Accessible through the write_data_block and read_data_block functions
static volatile data_block_t datablocks[NUM_DATA_BLOCKS];

/**
 * Internal function to store data at the requested data block
 * --- Do Not Modify This Function ---
 * @param index The physical index where the data should be stored
 * @param data The data that should be stored in the data block
 * @return Status of the store request. SUCCESS on successful store to a data_block_t, other on failure.
 */
status_e write_data_block(int index, uint32_t data)
{
    if (datablocks[index].write_counter >= BLOCK_MAX_WRITES)
    {
        return BLOCK_FULL;
    }
    datablocks[index].data = data;
    datablocks[index].write_counter++;
    return SUCCESS;
}

/**
 * Internal function to read data from the requested data block
 * --- Do Not Modify This Function ---
 * @param index The physical index where the data should be stored
 * @param data Pointer where the data block should be stored
 * @return Status of the store request. SUCCESS on successful store to a data_block_t, other on failure.
 */
status_e read_data_block(int index, uint32_t *data)
{
    *data = datablocks[index].data;
    return SUCCESS;
}

/**
 * Debug/testing function to "reset" the simulated data blocks back to a clean state.
 * This is used during testing to run multiple simulations back-to-back
 * --- Do Not Modify This Function ---
 * @param None
 * @return None
 */
void reset_data_blocks(void)
{
    int i;
    for (i = 0; i < NUM_DATA_BLOCKS; i++)
    {
        datablocks[i].data = 0;
        datablocks[i].write_counter = 0;
    }
}

/**
 * Debug/testing function to retrieve the number of writes for a physical index
 * --- Do Not Modify This Function ---
 * @param index The physical index to retrieve the write count for
 * @return The write count for the requested index
 */
uint32_t get_write_counter(int index)
{
    return datablocks[index].write_counter;
}

/**
 * Debug function to print out the state of the data blocks
 * --- Do Not Modify This Function ---
 * @param None
 * @return None
 */
void dump_blocks(void)
{
    int i;
    printf("blocks dump\n");
    for (i = 0; i < NUM_DATA_BLOCKS; i++)
    {
        printf("%d: count = %d, data = %x\n", i, datablocks[i].write_counter, datablocks[i].data);
    }
}
