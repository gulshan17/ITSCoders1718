

#include "mk_NVMIO.h"
#include "fw_Utility.h"
#include "fw_CreateFile.h"
#include "mk_typedef.h"

extern void mkgSetSW(U16);

//______________________NVM FUNCTIONS___________________________



void mkgWriteOneByte(U32 dest, U8* src)
{
	mkgWriteNVM(dest, &src, 1);
}

void mkgReadOneByte(U32 src, U8* dest )		/////-----Warning-----U8 *dest -> U8 dest/////
{
	mkgReadNVM(src, &dest, 1);
}


//______________________________________________________________

//Method returnReadUpdateAccessConditionByte - Returns the Read Update Access Condition Byte
U8 returnReadUpdateAccessConditionByte(U32 fileAddr)
{
	U8* data;
	
	mkgReadNVM(fileAddr + OFFSET_READ_UPDATE_ACCESSCONDITION, &data, 1);
	return (U8)data;
}


//Method returnRehabilateInvalidteAccessConditionByte - Returns the Rehabilate Invalidte Access Condition Byte 
U8 returnRehabilitateInvalidateAccessConditionByte(U32 fileAddr)
{
	U8* data;
	
	mkgReadNVM(fileAddr + OFFSET_REHABILATE_INVALIDATE_ACCESSCONDITION, &data, 1);
	return (U8)data;
}


//Method returnDeleteIncreaseAccessConditionByte - Returns the Delete Increase Access Condiotion Byte
U8 returnSeakIncreaseAccessConditionByte (U32 fileAddr)
{
	U8* data;
	
	mkgReadNVM(fileAddr + OFFSET_DELETE_INCREASE_ACCESSCONDITION, &data, 1);
	return (U8)data;
}


//Method returnRecordPointer - Returns the Record Pointer
U32 returnRecordPointer(U32 fileAddr)
{
	U32* data;
	
	mkgReadNVM(fileAddr + OFFSET_RECORDPOINTER, &data, LEN_ADDRESS);
	return data;
}


//Methd to return recordLen - Returns the Record Length
U16 returnRecordLen(U32 fileAddr)
{
	U16* data;
	
	mkgReadNVM(fileAddr + OFFSET_RECORDLEN, &data, LEN_RECORDLEN);
	return data;
}

//Method returnFileFDB - Returns the File FDB
U8 returnFileFDB(U32 fileAddr)
{
	U8* data;
	
	mkgReadNVM(fileAddr + OFFSET_FDB, &data, LEN_FDB);
	return (U8)data;
}

//Method returnFileSize - Returns the File size
U16 returnFileSize(U32 fileAddr)
{
	U8* data;
	
	mkgReadNVM(fileAddr + OFFSET_FILESIZE, &data, LEN_FILESIZE);
	return (U16)data;
}

//Method returnFileId - Returns the File ID 
U16 returnFileId(U32 fileAddr)
{
	U8* data;
	
	mkgReadNVM(fileAddr + OFFSET_FILEID, &data, LEN_FILEID);
	return (U16)data;
}

//Method returnSFID - Returns the SFI
U16 returnSFID( U32 fileAddr)
{
	U8* data;
	mkgReadNVM(fileAddr + OFFSET_SFI, &data, LEN_SFI);
	return (U8)data;
}

U8 returnLCSI( U32 fileAddr )
{
	U8* data;
	mkgReadNVM(fileAddr + OFFSET_LCSI, &data, LEN_LCSI);
	return (U8)data;
}


//Method matchParent - Match parent ID with selected ID 
U8 matchParent(U32 tempAddrLocal, U16 FileID)
{
	U8* data;
	U16 fileIDLocal;

	if( currentDF != ( U32 )( eepBuff + sizeof( objFile ) ) )
	{
		mkgReadNVM(tempAddrLocal + OFFSET_PARENTADDR, &data, LEN_ADDRESS);
		tempAddrLocal= (U32)data;
	}
	fileIDLocal = returnFileId(tempAddrLocal);	
	tempAddr = tempAddrLocal;
	if(FileID == fileIDLocal)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
	
}

//Method matchChild - Match child ID with selected ID
U8 matchChild(U32 tempAddrLocal,U16 FileID)
 {
    U8* data;


	mkgReadNVM(tempAddrLocal + OFFSET_childAddr , &data, LEN_ADDRESS);

	tempAddrLocal= (U32)data;
	if( tempAddrLocal == NULL )
	{
		return 0;
	}
	tempAddr = tempAddrLocal;
	tempAddr_SFI = tempAddrLocal;
	if(matchFileId(FileID,tempAddr))
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

//Method matchSibling - Match Sibling ID with selected ID
U8 matchSibling(U32 tempAddrLocal,U16 FileID)
{
	U8* data;
	
	while (tempAddrLocal !=NULL)
	{	
		if(matchFileId(FileID,tempAddrLocal))
		{
			tempAddr = tempAddrLocal;
			return 1;
		}
		mkgReadNVM(tempAddrLocal+OFFSET_SIBLINGADDR,&data,LEN_ADDRESS);
		tempAddrLocal = (U32)data;
	}
	return 0;
}

//Method matchFileId - Match File ID with selected ID
U8 matchFileId(U16 FileID,U32 tempAddr)
{
	U16 fileIDLocal=0;
	U16 tempFileID;
	tempFileID=FileID;
	if(isFileID(tempFileID))	//check if required ID is FileID or SFID
	{
	fileIDLocal  = returnFileId(tempAddr); 
	}
	else
	{
		fileIDLocal  = returnSFID(tempAddr);
	}

	if(FileID == fileIDLocal)
	{
		return 1;
	}
	else 
		return 0;
}

//Method sendStatus - To send the status when select command is executed successfully
void sendStatus()
{
	U16 statusWord=0x0000;
	statusWord = (statusWord) | ((STATUS_3G)|(U8)gLe);
	mkgSetSW(statusWord);
}

void sendAuthenticationStatus(U16 error_status)
{
	error_status = ( error_status ) | ( U8 )gLe;

	mkgSetSW( error_status );
}

void sendVerificationStatus(U32 status)
{
	U8 no_of_attempts;

	if( status == 0x9000 )
		mkgSetSW( status );

	else
	{
		if( status == 0x9F00 )
			mkgSetSW( status );

		else if( status == 0x63C0 )
		{
			no_of_attempts = mkgReadNVMOne( keydata );
			
			status = status | ( no_of_attempts );

			mkgSetSW( status );
		}
	}
}



//Method isDF - To check for DF
U8 isDF(U8 fdb)
{
	if(fdb == DF_78)
	{
		return 1;
	}
	else 
		return 0;
}

U16 isFileID( U16 tempFileIDLocal )
{
	if(LEN_AID && (((tempFileIDLocal)&0xFFFF)==0x7FFF))
	{
		return 1;
	}
	else if( (((tempFileIDLocal) & 0xFF00) == 0x2E00) || (((tempFileIDLocal) & 0xFF00) == 0x3F00) || (((tempFileIDLocal) & 0xFF00) == 0x6E00) || (((tempFileIDLocal) & 0xFF00) == 0x7F00) || ((tempFileIDLocal) & 0xFF00) == 0x5F00  || ((tempFileIDLocal) & 0xFF00) == 0x4F00  || ((tempFileIDLocal) & 0xFF00) == 0x6F00 || ((tempFileIDLocal) & 0xFF00) == 0xC000 )			//(INS==0xB0 || INS==0xD6 || INS==0xB2 || INS==0xDC)
	{
		return 1;
	}
	else
		return 0;
}
	


void clearOutputBuffer()
{
	U16 i;
	for(i=0;i<260;i++)
	{
		mkgOutputBuffer[i]=0x00;
	}
}




 
void update_RecordPointerValue(U32 record_Pointer)
{
	U32* data4 = record_Pointer;
	mkgWriteNVM( (currentEF + OFFSET_RECORDPOINTER ), &data4, LEN_ADDRESS);
}

U8 returnSpecialFileInfo( U32 fileAddr)
{
	U8* data;
		mkgReadNVM(currentEF + OFFSET_SPECIALFILEINFO ,&data,LEN_STATUSBYTE);	
		return data;
}

//*****************************************To be checked*****************************************************
//To be added to Utility.c

U8 returnIncreaseStatus(U32 fileAddr)
{
	U8* data;
	
	mkgReadNVM(fileAddr + OFFSET_INCREASE_ACCESSCONDITION, &data, 1);	//(To ask) position of INCREASE_ACCESSCONDITION
	return (U8)data;
}

U8 returnIncreaseAllowedStatus(U32 fileAddr)
{
	U8* data;
	
	mkgReadNVM(fileAddr + OFFSET_ACCESSCONDITION, &data, 1);	//(To ask) position of INCREASE_ALLOWED
	return (U8)data;
}
//******************************************To be checked*************************************************


U8 searchForSFI(U32 tempAddr,U16 SFID)
{
	//U32 temp1 = tempAddr;
	if(matchChild(tempAddr,SFID))
	{
		return 1;		
	}
	else if(matchSibling(tempAddr_SFI,SFID))
	{
		return 1;
	}
	else
		return 0;
}

void set_FF(U32 dest, U32 length)
{
	int i;
	for(i = 0; i < length; ++i)
	{
		*((U8 *)dest) = 0xFF;
		++dest;
	}
}
