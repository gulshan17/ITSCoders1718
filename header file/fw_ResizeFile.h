#if ! defined _RESIZEFILE_H
	#define _RESIZEFILE_H
	
	void Increase_MoveFiles(U32 fileAddr, U16 length, U16 oldsize);          //Shift file when file size increases
	void Decrease_MoveFiles(U32 fileAdddr, U16 length);         //Shift file when file size decreases

#endif