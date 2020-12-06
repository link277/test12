#include <stdint.h>
#include "distribution.h"

/**********************************************************
 **********************************************************
 * This file contains functions to simulate storing data. *
 *             DO NOT MODIFY THIS FILE                    *
 **********************************************************
 **********************************************************
 */

#ifndef _DATABLOCK_H
#define _DATABLOCK_H

typedef struct {
    uint32_t write_counter;
    uint32_t data;
} data_block_t;

status_e write_data_block(int index, uint32_t data);
status_e read_data_block(int index, uint32_t *data);
void reset_data_blocks(void);
void dump_blocks(void);
uint32_t get_write_counter(int index);

#endif //_DATABLOCK_H
