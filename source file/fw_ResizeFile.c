#include "mk_typedef.h"
#include "mk_NVMIO.h"
#include "fw_SelectStatusGetResponse.h"
#include "fw_CreateFile.h"
#include "fw_ReadUpdate.h"
#include "fw_Utility.h"
#include "macros.h"
#include "fw_DeleteFile.h"

void fw_ResizeFile()
{
    U32 fileAddr;                   //stores file to be resized
    U16 newsize, oldsize, freesize;

    //read new size

    mkgReadNVM(file + OFFSET_FILESIZE, (U8 *)&oldsize, LEN_FILESIZE);           //retrieve old file size
    if(newsize > oldsize)
    {
        freesize = 0x20000 - (File_Size() + sizeof(objFile));                   //calculate free space available
        newsize -= oldsize;                                                     //newsize = size to be increased
        if(freesize < newsize)                                                  //free space available < additional space return
        {
            mkgSetSW(NOT_ENORUGH_MEMORY);
            return;
        }

        Increase_MoveFiles(fileAddr, newsize);

        //update file size
    }

    else if(oldsize > newsize)
    {
        newsize = oldsize - newsize;
        Decrease_MoveFiles(fileAddr, newsize);

        //update file size
    }
}

void Increase_MoveFiles(U32 fileAddr, U16 length)
{
    U32 tempAddr[2500], tempAddr1, tempAddr2, tempAddr3;
    U16 fileSize, i;
    U8 *ptr;

    i = 0;
    tempAddr1 = fileAddr;
    tempAddr2 = (U32)getHashTable();               
    setHashTable(tempAddr2 + length);               //change the next allocation address
    while(tempAddr1 != tempAddr2)
    {
        tempAddr[i++] = tempAddr1;                  //saving the addresses of files need to be moved in array

        mkgReadNVM(tempAddr1 + OFFSET_FILESIZE, (U8 *)&fileSize, LEN_FILESIZE);         
        if(fileSize == NULL)
        {
            fileSize = 0x00;
        }
        tempAddr1 += sizeof(objFile) + fileSize;                //pointing to next file in the memory
    }

    //this part shift the files and change the address as required
    --i;
    while(i > 0)                        
    {
        tempAddr1 = tempAddr[i--];

        //change the child Parent address to new location of file
        mkgReadNVM(tempAddr1 + OFFSET_childAddr, (U8 *)&ptr, LEN_ADDRESS);      
        tempAddr2 = (U32)ptr;
        if(tempAddr2 != NULL)
        {
            tempAddr3 = tempAddr1 + length;
            mkgWriteNVM(tempAddr2 + OFFSET_PARENTADDR, (U8 *)&tempAddr3, LEN_ADDRESS);
        }

        //if file is first child then its Parent's child address need to be changed else file sibling address need to be changed
        mkgReadNVM(tempAddr1 + OFFSET_PARENTADDR, (U8 *)&ptr, LEN_ADDRESS);
        tempAddr2 = (U32)ptr;
        mkgReadNVM(tempAddr2 + OFFSET_childAddr, (U8 *)&ptr, LEN_ADDRESS);
        tempAddr3 = (U32)ptr;
        if(tempAddr3 == tempAddr1)                      //check whether file is first child or not
        {
            tempAddr3 += length;
            mkgWriteNVM(tempAddr2 + OFFSET_childAddr, (U8 *)&tempAddr3, LEN_ADDRESS);
        }
        else
        {
            while(tempAddr3 != tempAddr1)
            {
                tempAddr2 = tempAddr3;
                mkgReadNVM(tempAddr2 + OFFSET_SIBLING, (U8 *)&ptr, LEN_ADDRESS);
                tempAddr3 = (U32)ptr;
            }

            tempAddr3 += length;
            mkgWriteNVM(tempAddr2 + OFFSET_SIBLING, (U8 *)&tempAddr3, LEN_ADDRESS);     //change the sibling address to new location of file
        }

        mkgReadNVM(tempAddr1 + OFFSET_FILESIZE, (U8 *)&fileSize, LEN_FILESIZE);
        if(fileSize == NULL)
        {
            fileSize = 0x00;
        }
        tempAddr1 += sizeof(objFile) + fileSize - 1; 
        tempAddr2 = tempAddr1 + length;
        mkgReverseWriteNVM(tempAddr2, (U8 *)tempAddr1, sizeof(objFile) + fileSize);
    }
    return;
}

void Decrease_MoveFiles(U32 fileAdddr, U16 length)
{
    U32 tempAddr1, tempAddr2, tempAddr3, tempAddr4,tempAddr5;
    U16 fileSize;

    tempAddr1 = (U32)getHashTable();
    setHashTable(tempAddr1 - length);                   //change the next allocation address

    mkgReadNVM(fileAddr + OFFSET_FILESIZE, (U8 *)&fileSize, LEN_FILESIZE);
    if(fileSize == NULL)
    {
        fileSize = 0x00;
    }
    tempAddr2 = fileAddr + sizeof(objFile) + fileSize;                          //contains the address of first file which is to be moved

    mkgReadNVM(tempAddr2 + OFFSET_FILESIZE, (U8 *)&fileSize, LEN_FILESIZE);
    if(fileSize == NULL)
    {
        fileSize = 0x00;
    }
    tempAddr3 = tempAddr2 + sizeof(objFile) + fileSize;                          //contains the address of the second file which is to be moved     

    //this part shift the files and change the address as required
    while(tempAddr2 != tempAddr1)                                               
    {
        tempAddr4 = tempAddr2;

        //change the child Parent address to new location of file
        mkgReadNVM(tempAddr4 + OFFSET_childAddr, (U8 *)&ptr, LEN_ADDRESS);
        tempAddr4 = (U32)ptr;
        if(tempAddr4 != NULL)
        {
            tempAddr5 = tempAddr2 - length;
            mkgWriteNVM(tempAddr4 + OFFSET_PARENTADDR, (U8 *)&tempAddr5, LEN_ADDRESS);
        }

        //if file is first child then its Parent's child address need to be changed else file sibling address need to be changed
        mkgReadNVM(tempAddr2 + OFFSET_PARENTADDR, (U8 *)&ptr, LEN_ADDRESS);
        tempAddr4 = (U32)ptr;
        mkgReadNVM(tempAddr4 + OFFSET_childAddr, (U8 *)&ptr, LEN_ADDRESS);
        tempAddr5 = (U32)ptr;
        if(tempAddr5 == tempAddr2)                              //check whether file is first child or not
        {
            tempAddr5 -= length;
            mkgWriteNVM(tempAddr4 + OFFSET_childAddr, (U8 *)&tempAddr5, LEN_ADDRESS);
        }
        else
        {
            while(tempAddr5 != tempAddr2)
            {
                tempAddr4 = tempAddr5;
                mkgReadNVM(tempAddr4 + OFFSET_SIBLING, (U8 *)&ptr, LEN_ADDRESS);
                tempAddr5 = (U32)ptr;
            }

            tempAddr5 -= length;
            mkgWriteNVM(tempAddr4 + OFFSET_SIBLING, (U8 *)&tempAddr5, LEN_ADDRESS);         //change the sibling address to new location of file
        }
        mkgWriteNVM(tempAddr2 - length, (U8 *)tempAddr2, sizeof(objFile) + fileSize);       //shift the file to new location required
        
        //update tempAddr2, tempAddr3 to next files to be shift
        tempAddr2 = tempAddr3;
        mkgReadNVM(tempAddr2 + OFFSET_FILESIZE, (U8 *)&fileSize, LEN_FILESIZE);
        if(fileSize == NULL)
        {
            fileSize = 0x00;
        }
        tempAddr3 = tempAddr2 + sizeof(objFile) + fileSize;

    }
    return;
}