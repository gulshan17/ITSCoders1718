#include "mk_typedef.h"
#include "mk_NVMIO.h"
#include "fw_SelectStatusGetResponse.h"
#include "fw_CreateFile.h"
#include "fw_ReadUpdate.h"
#include "fw_Utility.h"
#include "macros.h"
#include "fw_DeleteFile.h"
#include "fw_ResizeFile.h"
#include "fw_FreeSize.h"

U8 value_array[256] = {0}, filling_length;
void fw_ResizeFile()
{
    U8 FCP_len = 0x00, FCP, i;
    U16 oldsize, freesize;
    U32 fileAddr = NULL, count, FileID;                   //stores file to be resized

	//If file system present or not
	if(getHashTable() == (U32)eepBuff)
	{
		mkgSetSW(TECHNICAL_PROBLEM);                    //Technical Error
		return;
	}

    //read new size
    if((P1 != 0x00) || (P2 != 0x00))	
	{
		mkgSetSW(0x6B00);
		return;
	}

	//newsize = ((mkgInputBuffer[0] << 8) | (mkgInputBuffer[1]));

    FCP=mkgInputBuffer[0];
	FCP_len=mkgInputBuffer[1];

	//Check for FCP template
	//First tag in Data part of TLV objects
	if(FCP != 0x62) //check for FCP tag
	{
		mkgSetSW(0x6F00);
		return; 
	}


    for(count=2; count < LcLe;)
    {
        switch(mkgInputBuffer[count])
        {
            case FILE_ID_83:
                    if(mkgInputBuffer[count+1] == 0x02)	//check length
	                {
	    	            FileID  = byteConverter(mkgInputBuffer[count+2],8,mkgInputBuffer[count+3]);	//"str" to be replaced with defined structure name
		            }
                    else
                    {
                        mkgSetSW(0x6700);
		                return;    
                    }
                    break;
        
            case FILE_SIZE_80:
                    if(mkgInputBuffer[count+1] == 0x02 )
                    {
                        newsize = byteConverter(mkgInputBuffer[count+2],8,mkgInputBuffer[count+3]); 
                    }
                    break;

            case 0xA5:
                    filling_length = mkgInputBuffer[count + 1 ] - 2;
                    if(mkgInputBuffer[count+2] == 0xC1)
                    {
                        if(filling_length == mkgInputBuffer[count + 3])
                        {
                            value_array[0] = 1;
                            for(i=1;i<=filling_length;i++)
                            {
                                value_array[i]=mkgInputBuffer[count+4+i];
                            }
                        }
                        else
                        {
                            mkgSetSW(0x6700);
                            return;
                        }
                    }
                    else if(mkgInputBuffer[count + 2] == 0xC2)
                    {
                        if(filling_length==mkgInputBuffer[count + 3])
                        {
                            value_array[0] = 2;
                            for(i=1;i<=filling_length;i++)
                            {
                                value_array[i]=mkgInputBuffer[count + 4 + i];
                            }
                        }
                        else
                        {
                            mkgSetSW(0x6700);
                            return;
                        }
                    }
                    else
                    {
                        mkgSetSW(0x6E00);
                        return;
                    }
        }
        count=(count+1)+mkgInputBuffer[count+1]+1;          
    }
    
    //if resize command is for currently selected EF
	if((currentEF != NULL) && (FileID == (returnFileId(currentEF))))
	{
		fileAddr=currentEF;
	}
    else if( (matchChild(currentDF, FileID)) || (matchSibling(tempAddr, FileID)) )
	{
		fileAddr=currentEF;
	}
	
    mkgReadNVM(fileAddr + OFFSET_FILESIZE, (U8 *)&oldsize, LEN_FILESIZE);           //retrieve old file size
    if(newsize > oldsize)
    {
        freesize = 0x20000 - (File_Size() + sizeof(objFile));                   //calculate free space available
        if(freesize < (newsize - oldsize))                                                  //free space available < additional space return
        {
            mkgSetSW(NOT_ENOUGH_MEMORY);
            return;
        }

        Increase_MoveFiles(fileAddr, oldsize);

        //update file size
    }

    else if(oldsize > newsize)
    {
        newsize = oldsize - newsize;
        Decrease_MoveFiles(fileAddr, oldsize);

        //update file size
    }
	

	mkgSetSW(NORMAL_ENDING);
	return;
}

void Increase_MoveFiles(U32 fileAddr, U16 oldsize)
{
    U32 tempAddr[2500], tempAddr1, tempAddr2, tempAddr3;
    U16 fileSize, i, j, changeLength = newsize - oldsize;
    U8 *ptr;

    i = 0;
    tempAddr1 = fileAddr;
    tempAddr2 = (U32)getHashTable();               
    setHashTable(tempAddr2 + changeLength);               //change the next allocation address

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

	mkgWriteNVM(fileAddr + OFFSET_FILESIZE, (U8*)&newsize, LEN_FILESIZE);       //change the filesize offset to newsize

    //this part shift the files and change the address as required
    --i;
    while(i > 0)                        
    {
        tempAddr1 = tempAddr[i--];

        //change the child Parent address to new location of file
        mkgReadNVM(tempAddr1 + OFFSET_childAddr, (U8 *)&ptr, LEN_ADDRESS);
        tempAddr2 = (U32)ptr;
        tempAddr3 = tempAddr1 + changeLength;
        while(tempAddr2 != NULL)
        {
            mkgWriteNVM(tempAddr2 + OFFSET_PARENTADDR, (U8 *)&tempAddr3, LEN_ADDRESS);

            mkgReadNVM(tempAddr2 + OFFSET_SIBLINGADDR, (U8 *)&ptr, LEN_ADDRESS);
            tempAddr2 = (U32)ptr;
        }

        //if file is first child then its Parent's child address need to be changed else file sibling address need to be changed
        mkgReadNVM(tempAddr1 + OFFSET_PARENTADDR, (U8 *)&ptr, LEN_ADDRESS);
        tempAddr2 = (U32)ptr;
        mkgReadNVM(tempAddr2 + OFFSET_childAddr, (U8 *)&ptr, LEN_ADDRESS);
        tempAddr3 = (U32)ptr;
        if(tempAddr3 == tempAddr1)                      //check whether file is first child or not
        {
            tempAddr3 += changeLength;
            mkgWriteNVM(tempAddr2 + OFFSET_childAddr, (U8 *)&tempAddr3, LEN_ADDRESS);
        }
        else
        {
            while(tempAddr3 != tempAddr1)
            {
                tempAddr2 = tempAddr3;
                mkgReadNVM(tempAddr2 + OFFSET_SIBLINGADDR, (U8 *)&ptr, LEN_ADDRESS);
                tempAddr3 = (U32)ptr;
            }

            tempAddr3 += changeLength;
            mkgWriteNVM(tempAddr2 + OFFSET_SIBLINGADDR, (U8 *)&tempAddr3, LEN_ADDRESS);     //change the sibling address to new location of file
        }

        mkgReadNVM(tempAddr1 + OFFSET_FILESIZE, (U8 *)&fileSize, LEN_FILESIZE);
        if(fileSize == NULL)
        {
            fileSize = 0x00;
        }
        tempAddr1 += sizeof(objFile) + fileSize - 1; 
        tempAddr2 = tempAddr1 + changeLength;
        mkgReverseWriteNVM(tempAddr2, (U8 *)tempAddr1, sizeof(objFile) + fileSize);
    }

    if(value_array[0] == 0)
    {
       tempAddr1 = fileAddr;
       tempAddr1 += (sizeof(objFile) + oldsize);
       for(i=0;i<changeLength;i++)
       {
           mkgWriteNVMOne(tempAddr1,0xFF);
           tempAddr1++;
       } 
    }
    else if(value_array[0] == 1)
    {
       tempAddr1 = fileAddr;
       tempAddr1 += sizeof(objFile) + oldsize;
       for(i=0,j=1;i<changeLength;i++,j++)
       {
           if(j<filling_length)
           {
                mkgWriteNVMOne(tempAddr1,value_array[j]);
                tempAddr1++;
           }
           else
           {
                mkgWriteNVMOne(tempAddr1,value_array[filling_length]);
                tempAddr1++; 
           }
       } 
    }
    if(value_array[0] == 2)
    {
       tempAddr1 = fileAddr;
       tempAddr1 += sizeof(objFile) + oldsize;

       for(i=0;i<changeLength;i++)
       {
           j=( i % filling_length ) + 1;
           mkgWriteNVMOne(tempAddr1,value_array[j]);
           tempAddr1++; 
       } 
    }
    return;
}

void Decrease_MoveFiles(U32 fileAddr, U16 oldsize)
{
    U8 *ptr;
	U16 fileSize, changeLength = oldsize - newsize;
	U32 tempAddr1, tempAddr2, tempAddr3, tempAddr4,tempAddr5;

    tempAddr1 = (U32)getHashTable();
    setHashTable(tempAddr1 - changeLength);                   //change the next allocation address
	mkgWriteNVM(fileAddr + OFFSET_FILESIZE, (U8*)&newsize, LEN_FILESIZE);       //change the filesize offset to newsize

    mkgReadNVM(fileAddr + OFFSET_FILESIZE, (U8 *)&fileSize, LEN_FILESIZE);
    tempAddr2 = fileAddr + sizeof(objFile) + fileSize;                          //contains the address of first file which is to be moved

    //this part shift the files and change the address as required
    while(tempAddr2 != tempAddr1)                                               
    {
        mkgReadNVM(tempAddr2 + OFFSET_FILESIZE, (U8 *)&fileSize, LEN_FILESIZE);
        if(fileSize == NULL)
        {
            fileSize = 0x00;
        }
        tempAddr3 = tempAddr2 + sizeof(objFile) + fileSize;               //contains the address of the second file which is to be moved     

        //change the child Parent address to new location of file
        mkgReadNVM(tempAddr2 + OFFSET_childAddr, (U8 *)&ptr, LEN_ADDRESS);
        tempAddr4 = (U32)ptr;
        tempAddr5 = tempAddr2 - changeLength;
        while(tempAddr4 != NULL)
        {
            mkgWriteNVM(tempAddr4 + OFFSET_PARENTADDR, (U8 *)&tempAddr5, LEN_ADDRESS);

            mkgReadNVM(tempAddr4 + OFFSET_SIBLINGADDR, (U8 *)&ptr, LEN_ADDRESS);
            tempAddr4 = (U32)ptr;
        }

        //if file is first child then its Parent's child address need to be changed else file sibling address need to be changed
        mkgReadNVM(tempAddr2 + OFFSET_PARENTADDR, (U8 *)&ptr, LEN_ADDRESS);
        tempAddr4 = (U32)ptr;
        mkgReadNVM(tempAddr4 + OFFSET_childAddr, (U8 *)&ptr, LEN_ADDRESS);
        tempAddr5 = (U32)ptr;
        if(tempAddr5 == tempAddr2)                              //check whether file is first child or not
        {
            tempAddr5 -= changeLength;
            mkgWriteNVM(tempAddr4 + OFFSET_childAddr, (U8 *)&tempAddr5, LEN_ADDRESS);
        }
        else
        {
            while(tempAddr5 != tempAddr2)
            {
                tempAddr4 = tempAddr5;
                mkgReadNVM(tempAddr4 + OFFSET_SIBLINGADDR, (U8 *)&ptr, LEN_ADDRESS);
                tempAddr5 = (U32)ptr;
            }

            tempAddr5 -= changeLength;
            mkgWriteNVM(tempAddr4 + OFFSET_SIBLINGADDR, (U8 *)&tempAddr5, LEN_ADDRESS);         //change the sibling address to new location of file
        }
        mkgWriteNVM(tempAddr2 - changeLength, (U8 *)tempAddr2, sizeof(objFile) + fileSize);       //shift the file to new location required
        
        //update tempAddr2, tempAddr3 to next files to be shift
        tempAddr2 = tempAddr3;
    }
    return;
}