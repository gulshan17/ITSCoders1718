
#if ! defined _UTILITY_H
  #define _UTILITY_H



#include "mk_typedef.h"



#define OFFSET_REHABILATE_INVALIDATE_ACCESSCONDITION  21
#define OFFSET_READ_UPDATE_ACCESSCONDITION 21             // It was actually 22 but was replaced with 21 for debugging.
#define OFFSET_DELETE_INCREASE_ACCESSCONDITION 21  // It was actually 23 but was replaced with 21 for debugging.

U8 efSelected;				//Check for EF selected or not
U8 adf_selected;
U32 tempAddr;
U32 tempAddr_SFI;
U8 gLe;

U8 returnReadUpdateAccessConditionByte( U32 );
U8 returnRehabilitateInvalidateAccessConditionByte(U32 fileAddr);
U8 returnSeakIncreaseAccessConditionByte (U32 fileAddr);

U32 returnRecordPointer( U32 );
U16 returnRecordLen(U32);

void mkgWriteOneByte(U32, U8*);
void mkgReadOneByte( U32, U8 );

U8 returnFileFDB( U32 );
U16 returnFileSize( U32 );

U16 returnFileId(U32);
U16 returnSFID( U32);
U8 isDF(U8 fdb);
U16 isFileID( U16 );
U8 returnSpecialFileInfo( U32 );
U8 returnLCSI( U32 );
void update_RecordPointerValue(U32 );

U8 matchParent(U32,U16);
U8 matchChild(U32,U16);
U8 matchSibling(U32,U16);
U8 matchFileId(U16,U32);
void SQN_Read_Record(U8);
U8 searchForSFI(U32,U16);
//************************************To be Checked**************************************
U8 returnIncreaseStatus(U32);
U8 returnIncreaseAllowedStatus(U32);
//************************************To be Checked**************************************

U16 VerifyKey(U8);

void set_FF(U32 dest, U32 length);                        //set FF starting from dest upto length

#endif