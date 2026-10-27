#include <stdio.h>
#include "decode.h"
#include "encode.h"
#include "types.h"
#include <string.h>
OperationType check_operation_type(char *argv[])
{
    if (strstr(argv[1], "-e") != NULL) 
    {
        return e_encode;
    }
    else if (strstr(argv[1], "-d") != NULL) 
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;;
    }
    

}

int main(int argc, char * argv[])
{
    if (argc < 4)
    {
        printf("Error : Insufficient arguments\n");
        return e_failure;
    }

    OperationType res = check_operation_type(argv);

    if(res == e_encode)
    {
        EncodeInfo encInfo;
        //validation
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            //do encoding
            if(do_encoding(&encInfo) == e_failure)
            {
                printf("Error : Encoding failed\n");
                return e_failure;
            }
            else
            {
                printf("Success : Encoding started\n");
                return e_success;
            }

        }
        else
        {
            printf("Error : Invalid arguments\n");
            return e_failure;
        }
    }
    else if(res == e_decode)
    {
        DecodeInfo decInfo;
        if(read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            if(do_decoding(&decInfo) == e_failure)
            {
                printf("Error : Decoding failed\n");
                return e_failure;
            }
            else
            {
                printf("Success : Decoding started\n");
                return e_success;
            }
        }
        else
        {
            printf("Error : Invalid arguments\n");
            return e_failure;
        }
    }
    else
    {
        printf("Error : Unsupported operation\n");
        return e_failure;
    }
    return 0;
}
