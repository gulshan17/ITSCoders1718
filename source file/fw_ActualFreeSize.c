#include "mk_typedef.h"
#include "mk_NVMIO.h"
#include "fw_SelectStatusGetResponse.h"
#include "fw_CreateFile.h"
#include "fw_ReadUpdate.h"
#include "fw_Utility.h"
#include "macros.h"
#include "fw_DeleteFile.h"

	U16 FileID = 0x00;                                  					//Initially FileID = 0x00
	U8 *ptr = 0x00;
	U8 fdb;

void E4_fwDeleteFile()
{
	//check P1 and P2 if incorrect return 0x6B00
	if((P1 != 0x00) || (P2 != 0x00))
	{
		mkgSetSW(0x6B00);
		return;
	}
	
	// check the length of Data Field
	if(LcLe != 2)
	{
		mkSetSW(0x6700);
		return;
	}
	
	//If file system present or not
	if(getHashTable() == (U32)eepbuff)
	{
		mkgSetSW(TECHNICAL_PROBLEM);                    //Technical Error
		return;
	}
	
	//FileID requested for Deletion
	FileID = ((mkgInputBuffer[0] << 8) | (mkgInputBuffer[1]));
	
	//if command is for MF
	if(FileID == FILE_ID_MF)
	{
		memset(eepBuff, 0xFF, 0x20000);
		currentDF = NULL;
		currentEF = NULL;
		mkgSetSW(NORMAL_ENDING);
		return;
	}
	
	//if Delete command is for currently selected DF
	else if(FileID == (returnFileId(currentDF)))
	{
		currentDF = DeleteFile(currentDF);
		selectDF();
		currentEF = NULL;
		
		mkgSetSW(NORMAL_ENDING);
		return;
	}
	
	//if Delete command is for currently selected EF
	else if((currentEF != NULL) && (FileID == (returnFileId(currentEF))))
	{
		DeleteFile(currentEF);
		currentEF = NULL;
		
		mkgSetSW(NORMAL_ENDING);
		return;
	}
	
	//Searching for the Delete file in the children of CurrentDF
	else if( (matchChild(currentDF, FileID)) || (matchSibling(tempAddr, FileID)) )
	{
		DeleteFile(tempAddr);
		currentEF = NULL;
		
		mkgSetSW(NORMAL_ENDING);
		return;
	}
	
	//Search file in parent
	else if(matchParent(tempAddr = currentDF, FileID))
	{
		currentDF = DeleteFile(tempAddr);
		selectDF();
		currentEF = NULL;
		
		mkgSetSW(NORMAL_ENDING);
		return;
	}
	
	//search DF file in the sibling of currentDF
	else if(matchChild(tempAddr, FileID) || (matchSibling(tempAddr, FileID)))
	{
		fdb = returnFileFDB(tempAddr);
		if(isDF(fdb))
		{
			currentDF = DeleteFile(tempAddr);
			selectDF();
			currentEF = NULL;
			
			mkgSetSW(NORMAL_ENDING);
			return;
		}
		
		else
		{
			mkgSetSW(FILE_NOT_FOUND);
			return;
		}
	}
}

U32 DeleteFile(U32 deleteFileAddr)
{
	U8 isFirstChild = 0x00;
	U32 tempParentAddr = NULL, tempAddr1 = NULL, tempAddr2 = NULL, tempAddr3 = NULL;

	//Storing the Address of Parent of the DeleteFile
	mkgReadNVM(deleteFileAddr + OFFSET_PARENTADDR, (U8 *)&ptr, LEN_ADDRESS);
	tempParentAddr = (U32)ptr;
	
	//checking whether the deleteFile is first child or not
	mkgReadNVM(tempParentAddr + OFFSET_childAddr, (U8 *)&ptr, LEN_ADDRESS);
	tempAddr1 = (U32)ptr;
	if(tempAddr1 == deleteFileAddr)
	{
		isFirstChild = 0x01;
	}
	//if deletefile is first child then
	if(isFirstChild)
	{
		//Address of Delete File sibling
		mkgReadNVM(deleteFileAddr + OFFSET_SIBLINGADDR, (U8 *)&ptr, LEN_ADDRESS);
		tempAddr1 = (U32)ptr;
		
		//setting the DeleteFile Parent's child as its sibling 
		mkgWriteNVM(tempParentAddr + OFFSET_childAddr, (U8 *)&tempAddr1, LEN_ADDRESS);
	
		//cleaning memory of deleting file 
		Put_FF(deleteFieAddr);
		ptr = (U8 *)(deleteFileAddr);
		memset(ptr, 0xff, 0x34);
		
		return tempParentAddr;
	}
	
	//if deleteFile is not first child
	else
	{
		//storing the address of first sibling of deleteFile
		mkgReadNVM(tempParentAddr + OFFSET_childAddr, (U8 *)&ptr, LEN_ADDRESS);
		tempAddr1 = (U32)ptr;
		//traversing to previous sibling of deleteFile
		while(tempAddr1 != deleteFileAddr)
		{
			tempAddr2 = tempAddr1;
			mkgReadNVM(tempAddr1 + OFFSET_SIBLINGADDR, (U8 *)&ptr, LEN_ADDRESS);
			tempAddr1 = (U32)ptr;
		}
		//storing address of next sibling of deleteFile
		mkgReadNVM(tempAddr1 + OFFSET_SIBLINGADDR, (U8 *)&ptr, LEN_ADDRESS);
		tempAddr3 = (U32)ptr;
		//setting deletefile previous sibling as deleteFile next sibling
		mkgWriteNVM(tempAddr2 + OFFSET_SIBLINGADDR, (U8 *)&tempAddr3, LEN_ADDRESS);
		
		//cleaning memory of deleting file 
		Put_FF(deleteFieAddr);
		ptr = (U8 *)(deleteFileAddr);
		memset(ptr, 0xff, 0x34);
		
		return tempParentAddr;
	}
}

void Put_FF(U32 file){
U8 *ptr;
U32 container;
mkgReadNVM(file + OFFSET_childAddr, (U8 *)&ptr, LEN_ADDRESS);
container = (U32)ptr;
	if(container != NULL){
		Put_FF(container);
		memset(ptr, 0xFF, 0x34);
	}
	mkgWriteNVM(file + OFFSET_SIBLINGADDR, (U8 *)&ptr, LEN_ADDRESS);
	container = (U32)ptr;
	if(container != NULL){
		Put_FF(container);
		memset(ptr, 0xFF, 0x34);
	}
}
