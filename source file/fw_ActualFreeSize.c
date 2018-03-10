#include "mk_typedef.h"
#include "fw_CreateFile.h"

int freeSize();

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
	size = (CountTraverse() * sizeof(objFile));
}


int CountTraverse()
{
	U32 ptr, TempAddr, MFAddr;
	U8 top, *value;
	U16 counter;						//used to count number of files present 
	
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
			++counter;
		}
	}while(TempAddr != NULL);
	
	return counter;
}
