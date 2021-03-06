#include "mk_typedef.h"
#include "fw_CreateFile.h"

int File_Size();

void fw_FreeSize()
{
	int size;
	//check P1 and P2 if incorrect return 0x6B00
	if((P1 != 0x00) || (P2 != 0x00))
	{
		mkgSetSW(0x6B00);
		return;
	}
	
	// check the length of Data Field, don't know what it should be, will update on saturday after asking Sir
	if(LcLe != )
	{
		mkgSetSW(0x6700);
		return;
	}
	
	/*//If file system present or not, don't know what to do, update later
	if(getHashTable() == (U32)eepBuff)
	{
		mkgSetSW(TECHNICAL_PROBLEM);                    //Technical Error
		return;
	*/
	
	//size of object file = number of files * sizeof a single file
	size = 0x20000 - (File_Size() + sizeof(objFile));
}


int File_Size()
{
	U32 ptr, TempAddr, MFAddr;
	U8 *value;
	U16 counter;							//used to count number of files present
	U32 total_data_size = 0x00, data_size;						 
	
	counter = 1;
	top = 0;
	
	//storing address of Master FIle
	MFAddr = ptr = (U32)(eepBuff + sizeof(objFile));
	
	do
	{	
		//storing the address of child of the file pointed by ptr, initially MF
		mkgReadNVM(ptr + OFFSET_childAddr, (U8 *)&value, LEN_ADDRESS);
		TempAddr = (U32)value;
		
		//traversing till child becomes NULL(traversing Depth first)
		while(TempAddr != NULL)
		{
			ptr = TempAddr;
			mkgReadNVM(ptr + OFFSET_FILESIZE, (U8 *)&data_size, FILESIZE);
			total_data_size += data_size;
			
			mkgReadNVM(TempAddr + OFFSET_childAddr, (U8 *)&value, LEN_ADDRESS);
			TempAddr = (U8 *)value;
			
			++counter;
			
		}
		
		//storing the address of sibling
		mkgReadNVM(ptr + OFFSET_SIBLINGADDR, (U8 *)&value, LEN_ADDRESS);
		TempAddr = (U32)value;
		
		//if sibling is NULL, then point to parent sibling, parent is already counted
		while((TempAddr == NULL) && (ptr != MFAddr))
		{
			mkgReadNVM(ptr + OFFSET_PARENTADDR, (U8 *)&value, LEN_ADDRESS);
			ptr = (U32)value;
				
			mkgReadNVM(ptr + OFFSET_SILBLINGADDR, (U8 *)&value, LEN_ADDRESS);
			TempAddr = (U32)value;
		}
		
		//tempAddr == NULL means ptr points to MF(already counted)
		if(TempAddr != NULL)
		{
			ptr = TempAddr;
			mkgReadNVM(ptr + OFFSET_FILESIZE, (U8 *)&data_size, FILESIZE);
			total_data_size += data_size;

			++counter;
		}
	}while(TempAddr != NULL);
	
	return ((counter * sizeof(objFile)) + total_data_size);
}
