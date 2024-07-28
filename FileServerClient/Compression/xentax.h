#pragma once
#include <cstdint>

struct HuffmanData
{
	unsigned int HuffmanTable[0x200];
	unsigned int HelperArray[0x48];
	unsigned int *TempArray;
	unsigned int Var1,Var2;
};

class Xentax
{
public:
	unsigned char* DecompressFile(unsigned int* Input, int InputSize, int& outsize);
	bool     SetupNodesandTree(HuffmanData& HData);

private:	
	unsigned int  ESIplus8;
	unsigned int  ESIplusC;
	unsigned int  ESIplus10;
	unsigned int* ptrInputData;
	unsigned int* InputDataEnd;
};