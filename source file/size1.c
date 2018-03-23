#include<stdio.h>
typedef unsigned int U32;
typedef short int U16;
typedef char U8;

struct FILE				
{
	// Pointers for file accessing

	U32 parentAddr;			// 00	//Parent Address    \	---> For Select File
	U32 siblingAddr;		// 04	//Sibling Address	/
	U32 childAddr;			// 08	//Point to child address if DF (RFU by default 0xFFFFFFFF is case of EF)

	U16 FDB;				// 12	//FDB byte for MF/DF/EF
	U16 fileProp;			// 14
	U16 fileID;				// 16	File ID
	U16 fileSize;			// 18	Size of file

	U32 accessCondition;	// 20	File Access Condition
							//      00	00	11	00
							//		00	RI	UR	SI	

	U32 record_Pointer;     // 24  For EF-LF And EF-CY

	U16 RecordLen;			// 28

	U8 dataCodingByte;		// 30
	
	U8 LCSI;				// 31

	U8 specialFileInfo;		// 32

	U8 SFI;					// 33   SFI-Short File Identifer Tag88
	
	U8 totalFileSize;		// 34   TotalFileSize Tag81
	
	U8 dfName[16];			// 35
    U8 ddfNa;
    U8 ddfNaj;
}objFile;
int main()
{
    printf("Size=%d",sizeof(objFile));
    return 0;
}