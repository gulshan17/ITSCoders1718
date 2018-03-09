#include "mk_NVMIO.h"

void fw_FreeSize()
{
	U32 size = ((0x 20000) - ((U32)getHashTable - (U32)eepBuff));
	
	mkgSetSW(size);
}