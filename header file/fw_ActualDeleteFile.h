#if ! defined _DELETEFILE_H
	#define _DELETEFILE_H
	
	U32 DeleteFile(U32 deleteFileAddr);										//Delete the file
	void Traverse_FF(U32 file);												//Traverse from DeleteFile to all it's children to set FF
	#endif