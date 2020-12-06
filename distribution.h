#include <stdint.h>

/**********************************************************
 **********************************************************
 * This file contains the public interface for the system.*
 *             DO NOT MODIFY THIS FILE                    *
 **********************************************************
 **********************************************************
 */

#ifndef _DISTRIBUTION_H
#define _DISTRIBUTION_H

#define NUM_DATA_BLOCKS 10
#define BLOCK_MAX_WRITES 10

typedef enum {
    SUCCESS = 0,
    BLOCK_FULL,
    DATA_INVALID,
} status_e;

typedef enum {
    CMD_READ = 1,
    CMD_WRITE = 2
} command_e;

void initialize(void);
status_e process_command(uint64_t *command);
void reset(void);
uint32_t get_write_counter(int index);
void dump_blocks(void);

#endif //_DISTRIBUTION_H
