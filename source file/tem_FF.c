#define OFFSET_EForDF 13	//OFFSET FOR Identification of EF or DF
margin = margin(curretEF);	//calculating updated additional size of EF

void TotalSizeIncrement(U32 file)	//totalsize incrementing in case of creeation of an ef
{
	U8 *ptr;
	U16 totalfilesize;
	U32 container;
	mkgReadNVM(file + OFFSET_PARENTADDR, (U8 *)&ptr, LEN_ADDRESS);
	container = (U32)ptr;
	if(container != NULL)
	{
		totalfilesize = returnTotalFileSize(container);
		totalfilesize = margin + totalfilesize;
		mkgWriteNVM(container +  OFFSET_TOTALFILESIZE, (U8 *)&totalfilesize, LEN_FILESIZE);
		Traverse_FF(container);
	}
}

void TotalSizeDecrement(U32 file)	//total size decreasing in case of deletion of an EF file
{
	U8 *ptr;
	U16 totalfilesize;
	U32 container;
	mkgReadNVM(file + OFFSET_PARENTADDR, (U8 *)&ptr, LEN_ADDRESS);
	container = (U32)ptr;
	if(container != NULL)
	{
		totalfilesize = returnTotalFileSize(container);
		totalfilesize = totalfilesize - margin;
		mkgWriteNVM(container +  OFFSET_TOTALFILESIZE, (U8 *)&totalfilesize, LEN_FILESIZE);
		Traverse_FF(container);
	}
}

U16 returnTotalFileSize(U32 fileAddr)	//retun total file size
{
	U8* data;
	
	mkgReadNVM(fileAddr + OFFSET_TOTALFILESIZE, &data, LEN_FILESIZE);
	return (U16)data;
}

U16 margin(U32 fileAddr)
{
	U8 fileType, *data;
	U16 localfilesize;

	if(returnFileType(fileAddr) != 0x02){	//0x02 is for transparent file otherwise cyclic or linear 
		localfilesize = ((returnRecordLen(fileAddr)) * (returnFileSize(fileAddr)));	
		return (U16)localfilesize;		
	}
	localfilesize = returnFileSize();
	return (U16)localfilesize;
}

U8 returnFileType(U32 fileAddr)
{
	U8* data;
	
	mkgReadNVM(fileAddr + OFFSET_EForDF, &data, LEN_FDB);
	return (U8)data;
}