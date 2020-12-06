#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "distribution.h"
#include "datablock.h"

#define RUN_TEST(RESULT, DESCRIPTION) \
do { \
    if (RESULT) { \
        printf("Test Complete: %s\n\n", DESCRIPTION); \
    } else { \
        printf("Test Failed: %s\n\n", DESCRIPTION); \
    } \
} while(0)
    
bool check_expected_data(uint32_t *expected)
{
    int i;
    uint64_t command;
    status_e status;
    int errors = 0;
    uint32_t readdata;

    //check against expected data
    for (i = 0; i < NUM_DATA_BLOCKS; i++)
    {
        if (expected[i] == 0xFFFFFFFF)
        {
            continue;
        }

        command = 1 + (i << 3);
        status = process_command(&command);
        if (status != SUCCESS)
        {
            printf("ERROR: Data check unable to read back data for index %d\n", i);
            errors++;
            continue;
        }
        readdata = ((uint32_t)((command >> 10) & 0xFFFFFFFF));
        if (expected[i] != readdata)
        {
            printf("ERROR: Incorrect data at index %d; Actual: 0x%x, Expected: 0x%x\n", i, readdata, expected[i]);
            errors++;
        }
    }
    
    if (errors > 0)
    {
        return false;
    }
    return true;
}

//Test basic write/read & data conversion
bool basic_write_read(void)
{
    status_e status;
    uint64_t command;
    uint64_t expected;

    //write 0xA1B2C3D4 to index 5
    command = 0x286cb0f502a;
    if ((status = process_command(&command)) != SUCCESS)
    {
        printf("ERROR: basic_write_read: write to index 5 failed with %d\n", (int)status);
        return false;
    }
    //read the data back from index 5
    command = 0x29;
    if ((status = process_command(&command)) != SUCCESS)
    {
        printf("ERROR: basic_write_read: reading back data from index 5 failed with %d\n", (int)status);
        return false;
    }
    expected = 0x286cb0f5029;
    if (command != expected)
    {
        printf("ERROR: basic_write_read: Retrieved data does not match expected data\n");
        printf("ERROR: basic_write_read: Actual: %lx, Expected: %lx\n", command, expected);
        return false;
    }
    
    //read an index that has not been written
    command = 0x1;
    if ((status = process_command(&command)) != DATA_INVALID)
    {
        printf("ERROR: basic_write_read: Able to read an index which has not been written\n");
        return false;
    }

    printf("Test Passed\n");
    return true;
}

//Repeatedly write to a single index until full
//Minimum writes achieved should be BLOCK_MAX_WRITES, maximum writes possible is (BLOCK_MAX_WRITES * NUM_DATA_BLOCKS)
bool repeated_write(void)
{
    int i;
    int counter;
    unsigned char flag;
    status_e status;
    uint64_t data = 0xA1B1C1D1;
    uint64_t expected;
    uint64_t command;

    counter = 0;
    flag = 1;
    for (i = 0; i < ((BLOCK_MAX_WRITES * NUM_DATA_BLOCKS) + 10) && flag; i++)
    {
        command = 0xA + (data << 10);
        status = process_command(&command);
        if (status == BLOCK_FULL)
        {
            //system says it's full, stop the writes
            flag = 0;
        }
        else if (status != SUCCESS)
        {
            //stop the writes
            printf("ERROR: repeated_write: Write of data failed with error %d\n", (int)status);
            flag = 0;
            return false;
        }
        else
        {
            data++;
            counter++;
        }
    }

    //check the stored data to make sure it's intact
    command = 0x9;
    expected = (0x9 + ((data - 1) << 10));
    if (process_command(&command) != SUCCESS)
    {
        printf("ERROR: repeated_write: Unable to read back data\n");
        return false;
    }
    else
    {
        if (command != expected)
        {
            printf("ERROR: repeated_write: Retrieved data does not match expected data\n");
            printf("ERROR: repeated_write: Actual: %lx, Expected: %lx\n", command, expected);
            return false;
        }
    }

    printf("INFO: Achieved %d writes of a possible %d\n", counter, BLOCK_MAX_WRITES * NUM_DATA_BLOCKS);

    //test to make sure the number of writes did not exceed the max before the system declared it was full
    if (counter > (BLOCK_MAX_WRITES * NUM_DATA_BLOCKS))
    {
        printf("ERROR: repeated_write: Actual write count exceeds theoretical write count\n");
        return false;
    }

    return true;
}

//write_then_fill: write all indices once, then write to the last index until full
bool write_then_fill(void)
{
    //setup a local cache to track what has been written
    uint32_t expected[NUM_DATA_BLOCKS];
    uint32_t data;
    uint64_t command;
    int i, counter = 0;
    status_e status;

    memset(expected, 0xFF, sizeof(expected));

    //write to all indices once
    data = 0x11111111;
    for (i = 0; i < NUM_DATA_BLOCKS; i++)
    {
        command = (i << 3) + (((uint64_t)data) << 10) + 0x2;
        status = process_command(&command);
        if (status != SUCCESS)
        {
            printf("ERROR: write_then_fill: Error writing to index %d. Failed with %d\n", i, (int)status);
            return false;
        }
        expected[i] = data;
        data++;
        counter++;
    }

    //repeatedly write to the first index until full
    data = 0x33333333;
    for (i = 0; i < ((BLOCK_MAX_WRITES * NUM_DATA_BLOCKS) + 10); i++)
    {
        command = (((uint64_t)data) << 10) + 0x2;
        if ((status = process_command(&command)) == SUCCESS)
        {
            expected[0] = data;
            data++;
            counter++;
        }
        else
        {
            break;
        }
    }

    if (status != BLOCK_FULL)
    {
        printf("ERROR: write_then_fill: Writing to index %d failed with %d\n", i, (int)status);
        return false;
    }

    if (!check_expected_data(expected))
    {
        printf("ERROR: write_then_fill: invalid data found\n");
        return false;
    }
    
    printf("INFO: Achieved %d writes\n", counter);

    return true;
}


int main(int argc, char *argv[])
{
    initialize();

    printf("Testing a single write/read and bit conversion\n");
    RUN_TEST(basic_write_read(), "basic_write_read");
    reset();    //reset the system so we can run another test

    printf("Testing repeated write to same index\n");
    RUN_TEST(repeated_write(), "repeated_write");
    reset();    //reset the system so we can run another test
    
    printf("Testing write before fill\n");
    RUN_TEST(write_then_fill(), "write_then_fill");
    reset();    //reset the system so we can run another test

    return 0;
}
