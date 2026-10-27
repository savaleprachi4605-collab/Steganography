#include <stdio.h>
#include "common.h"
#include "decode.h"
#include "types.h"
#include <string.h>

OperationType decode_check_operation_type(char *argv[])
{
    if (strstr(argv[1], "-d") != NULL)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(strstr(argv[2], ".bmp") != NULL)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    if(argv[3] != NULL)
    {
        decInfo->secret_fname = argv[3];
    }
    else
    {
        decInfo->secret_fname = "decoded.txt";
    }

    return e_success;
}

Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if(decInfo->fptr_stego_image == NULL)
    {
        printf("Error : Unable to open stego image file\n");
        return e_failure;
    }
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb");
    if(decInfo->fptr_secret == NULL)
    {
        printf("Error : Unable to open secret file\n");
        return e_failure;
    }
    return e_success;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);
    char magic[10];

    decode_string_from_image(magic, strlen(MAGIC_STRING), decInfo->fptr_stego_image);

    if (strcmp(magic, MAGIC_STRING) == 0)
    {
        return e_success;
    }

    return e_failure;
}
Status decode_string_from_image(char *str, int len, FILE *fptr_stego)
{
    for(int i = 0; i < len; i++)
    {
        unsigned char buffer[8];
        if (fread(buffer, 8, 1, fptr_stego) != 1)
        {
            return e_failure;
        }
        str[i] = decode_byte_from_lsb((char *)buffer);
    }
    str[len] = '\0';
    return e_success;
}

char decode_byte_from_lsb(char *image_buffer)
{
    unsigned char value = 0;

    for (int i = 0; i < 8; i++)
    {
        value |= (unsigned char)((image_buffer[i] & 1) << i);
    }

    return (char)value;
}

Status decode_int_from_lsb(int *data, FILE *fptr_stego)
{
    char buffer[32];

    if (fread(buffer, 32, 1, fptr_stego) != 1)
    {
        return e_failure;
    }

    *data = 0;

    for (int i = 0; i < 32; i++)
    {
        *data |= (buffer[i] & 1) << i;
    }

    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    if (decode_int_from_lsb(&decInfo->extn_size, decInfo->fptr_stego_image) == e_failure)
    {
        printf("Error : failed to decode extension size\n");
        return e_failure;
    }
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    if (decode_string_from_image(decInfo->extn_secret_file, decInfo->extn_size, decInfo->fptr_stego_image) == e_failure)
    {
        printf("Error : failed to decode extension name\n");
        return e_failure;
    }
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    int size = 0;
    if (decode_int_from_lsb(&size, decInfo->fptr_stego_image) == e_failure)
    {
        printf("Error : failed to decode secret size\n");
        return e_failure;
    }
    decInfo->size_secret_file = size;
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;

    for(long i = 0; i < decInfo->size_secret_file; i++)
    {
        if (fread(image_buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            printf("Error : failed to read secret data byte %ld\n", i);
            return e_failure;
        }

        ch = decode_byte_from_lsb(image_buffer);

        if (fputc(ch, decInfo->fptr_secret) == EOF)
        {
            printf("Error : failed to write secret data\n");
            return e_failure;
        }
    }

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if (open_decode_files(decInfo) == e_failure)
    {
        return e_failure;
    }

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    if (decode_magic_string(decInfo) == e_failure)
    {
        return e_failure;
    }
    else
    {
        printf("Magic string matched. Proceeding with decoding...\n");
    }

    if (decode_secret_file_extn_size(decInfo) == e_failure)
    {
        return e_failure;
    }
    else
    {
        printf("Proceeding with decoding\n");
    }

    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        return e_failure;
    }
    else
    {
        printf("Proceeding with decoding\n");
    }

    if (decode_secret_file_size(decInfo) == e_failure)
    {
        return e_failure;
    }
    else
    {
        printf("Proceeding with decoding\n");
    }

    if (decode_secret_file_data(decInfo) == e_failure)
    {
        return e_failure;
    }
    else
    {
        printf("Decoding completed successfully\n");
    }

    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_secret);

    return e_success;
}