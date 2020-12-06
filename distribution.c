#include <stdbool.h>
#include <stdio.h>
#include "distribution.h"
#include "datablock.h"

/**********************************************************
 **********************************************************
       Implement your distribution system in this file.
      You may add additional private functions as needed.
 **********************************************************
 **********************************************************
 */

/**
 * Debug/testing function to "reset" the distribution system back to default
 * --- Implement This Function As Needed ---
 */
void reset(void)
{
    //reset the data blocks
    //leave this here. Implement additional reset code below.
    reset_data_blocks();
    
    //do any other reset needed here
}

/**
 * Process the requested command
 * --- Implement This Function ---
 * @param command Pointer to the bit-packed command data containing the command type (bit 0-2), index (bit 3-9), and data (bit 10-41)
 * @return Status of the comamnd request.
 * return SUCCESS on successful write to or read from a data_block_t
 * return BLOCK_FULL for a write command if all blocks writes have been "used up"
 * return DATA_INVALID for reads to an index that have not been written, or any other read/write failures
 * On read SUCCESS, data should be packed into the correct location in the command pointer (bits 10-41).
 */
status_e process_command(uint64_t *command)
{
    int comId = (*command & 0x7);
    int index = ((*command >> 3) & 0x7F);
    uint32_t data = ((*command >> 10) & 0xFFFFFFFF);
    
    status_e status = SUCCESS;
    
    if (comId == CMD_READ) {
        
        uint32_t data;
        
        read_data_block(index, &data);
        
        if(data == 0)
        {
            status = DATA_INVALID;
        }
        
        *command = ((((uint64_t)data & 0xFFFFFFFF) << 10) | ((index & 0x7F) << 3) | (comId & 0x7));
        
    } else if (comId == CMD_WRITE) {
        status = write_data_block(index, data);
    }
    
    return status;
}

/**
 * Function to do any initialization required
 * --- Implement This Function --- 
 */
void initialize(void)
{
    //leave this here. Implement additional initialization code below.
    reset();
    
    //do any other initialization needed here
}
