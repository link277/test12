#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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

#define MAX_CLIENT 3
#define BUFF_SIZE 255
#define WRITE_PIPE 1
#define READ_PIPE 0
int listen_socketfd, client_pipe[MAX_CLIENT][2],server_pipe[MAX_CLIENT][2], pid[MAX_CLIENT];
char buf[BUFF_SIZE];
uint64_t Command[MAX_CLIENT * NUM_DATA_BLOCKS];

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

void finishChilren(void)
{
    int index;
    
    for(index = 0; index < MAX_CLIENT; index++)
    {
        write(client_pipe[index][WRITE_PIPE],"bye",3);
        close(client_pipe[index][READ_PIPE]);
        close(client_pipe[index][WRITE_PIPE]);
        close(server_pipe[index][READ_PIPE]);
        close(server_pipe[index][WRITE_PIPE]);
    }
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
    createProcess();
}

/**
 * Route  the requested command
 * @param command Pointer to the bit-packed command data containing the command type (bit 0-2), index (bit 3-9), and data (bit 10-41)
 * @return Status of the comamnd request.
 * return SUCCESS on successful write to or read from a data_block_t
 * return BLOCK_FULL for a write command if all blocks writes have been "used up"
 * return DATA_INVALID for reads to an index that have not been written, or any other read/write failures
 * On read SUCCESS, data should be packed into the correct location in the command pointer (bits 10-41).
 */
status_e commandRoute(uint64_t* command)
{
    int clientIndex;
    status_e status;
    
    clientIndex = ((*command >> 3) & 0x7F);
    //printf("ClientIndex before divide 10 = %d \n", clientIndex);
    clientIndex /= NUM_DATA_BLOCKS;
    //printf("ClientIndex after divide 10 = %d \n", clientIndex);
    
    if(clientIndex == 0)
    {
        status = process_command(command);
        if (status != SUCCESS)
        {
            printf("ERROR: Error writing to command %llx. Failed with %d\n", *command, (int)status);
            return false;
        }
    }
    else
    {
        if(clientIndex >= MAX_CLIENT)
            return false;

        memset(buf,0,BUFF_SIZE);
        sprintf(buf, "0x%llx",*command);
        printf("Send :data to clientIndex %d data : %s\n", clientIndex, buf);
        write(client_pipe[clientIndex-1][WRITE_PIPE],buf,BUFF_SIZE);
        while(1)
        {
            memset(buf,0,BUFF_SIZE);
            long recevLength = read(server_pipe[clientIndex-1][READ_PIPE], buf, sizeof(buf));
            if(recevLength > 0)
            {
                *command = (uint64_t)strtoll(buf,NULL,0);
                printf("Confirm Client %d recieved Data %s convert Data : 0x%llx\n",clientIndex, buf, *command);
                break;
            }
            sleep(1);
        }
    }
    
    return true;
}

/**
 * Create commands to test
 */
void createCommand()
{
    int data;
    int index;
    int cmdId;
    
    for (index = 0; index < (MAX_CLIENT * NUM_DATA_BLOCKS); index++ )
    {
        data = index+1;
        cmdId = CMD_WRITE;
        Command[index] = (((uint64_t)data & 0xFFFFFFFF) << 10 | (index & 0x7F) << 3 | (cmdId & 0x7));
        commandRoute(&Command[index]);
    }
}

bool confirmCommand()
{
    int index;
    int cmdId;
    uint64_t command;
    uint32_t origData;
    uint32_t readData;
    
    for(index = 0; index < (MAX_CLIENT * NUM_DATA_BLOCKS); index++)
    {
        cmdId = CMD_READ;
        command = ((index & 0x7F) << 3 | (cmdId & 0x7));
        commandRoute(&command);
        
        origData = ((Command[index] >> 10) & 0xFFFFFFFF);
        readData = ((command >> 10) & 0xFFFFFFFF);
        
        if(origData != readData)
        {
            printf("Data command[%d] : 0x%llx is not matched with command 0x%llx \n", index, Command[index], command);
            return false;
        }
        
    }
    
    return true;
}

/**
 * Create child processes to write more data
 */
void createProcess()
{
    int index = 0;
    int pipe_status = 0;
    
    printf("createProcess\n");
    for(index = 0; index < MAX_CLIENT; index++)
    {
        if((pipe_status = pipe(client_pipe[index])) == -1)
        {
            printf("Pipe error index %d\n", index);
            exit(1);
        }
        if((pipe_status = pipe(server_pipe[index])) == -1)
        {
            printf("Pipe error index %d\n", index);
            exit(1);
        }
        
        pid[index] = fork();
        if ( pid[index] == 0)
        {
            printf("fork child %d\n",index);
            childProcess(index);
        }
        else if ( pid[index] == -1)
        {
            printf("failed to fork\n");
            exit(0);
        }
        
    }
    
}

/**
 * Handle the child processes to receive and send command with a Server,
 * and process commands.
 */
int childProcess(int index)
{
    long nbyte = 0;
    uint64_t command;
    status_e status;
    
    while(1)
    {
        sleep(1);
        memset(buf,0,BUFF_SIZE);
        nbyte = read(client_pipe[index][READ_PIPE], buf, sizeof(buf));
        if(nbyte > 0)
        {
            if(strcmp(buf,"bye") == 0)
            {
                printf("Child process %d done \n",index);
                exit(2);
            }
            command = (uint64_t)strtol(buf, NULL,0);
            printf("Receive : index %d buff %s, command %llx\n", index, buf,command);
            status = process_command(&command);
            if (status != SUCCESS)
            {
                printf("ERROR: Error writing to command %llx. Failed with %d\n", command, (int)status);
                return false;
            }
            memset(buf,0,BUFF_SIZE);
            sprintf(buf, "0x%llx",command);
            write(server_pipe[index][WRITE_PIPE],buf,BUFF_SIZE);
        }
        else if(nbyte < 0)
        {
            printf("client index %d close pipe \n", index);
            exit(2);
        }
    }
    
    return true;
}
