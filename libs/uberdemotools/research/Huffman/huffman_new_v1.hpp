#pragma once


static const u8 SmallCodeBitCounts[256] =
{
	0, 8, 2, 0, 7, 0, 2, 0, 7, 7, 2, 7, 8, 0, 2, 8,
	7, 8, 2, 7, 0, 5, 2, 8, 8, 8, 2, 5, 6, 0, 2, 6,
	6, 8, 2, 0, 6, 7, 2, 7, 8, 7, 2, 0, 0, 6, 2, 7,
	7, 0, 2, 0, 7, 5, 2, 6, 0, 8, 2, 5, 8, 0, 2, 6,
	7, 8, 2, 8, 0, 0, 2, 8, 7, 7, 2, 8, 7, 0, 2, 0,
	7, 8, 2, 8, 8, 5, 2, 8, 0, 7, 2, 5, 6, 8, 2, 6,
	6, 0, 2, 0, 6, 7, 2, 8, 7, 7, 2, 0, 7, 6, 2, 8,
	0, 0, 2, 8, 8, 5, 2, 6, 8, 8, 2, 5, 7, 7, 2, 6,
	0, 0, 2, 8, 7, 8, 2, 8, 7, 7, 2, 7, 8, 0, 2, 0,
	7, 0, 2, 7, 0, 5, 2, 0, 8, 0, 2, 5, 6, 0, 2, 6,
	6, 8, 2, 8, 6, 7, 2, 7, 0, 7, 2, 0, 0, 6, 2, 7,
	7, 8, 2, 8, 7, 5, 2, 6, 8, 0, 2, 5, 0, 0, 2, 6,
	7, 0, 2, 8, 0, 0, 2, 0, 7, 7, 2, 0, 7, 0, 2, 8,
	7, 0, 2, 0, 8, 5, 2, 0, 8, 7, 2, 5, 6, 0, 2, 6,
	6, 8, 2, 0, 6, 7, 2, 8, 7, 7, 2, 0, 7, 6, 2, 8,
	0, 0, 2, 0, 0, 5, 2, 6, 0, 0, 2, 5, 7, 7, 2, 6
};

static const u8 SmallCodeSymbols[256] =
{
	0, 134, 0, 0, 67, 0, 0, 0, 126, 196, 0, 11, 110, 0, 0, 132,
	6, 5, 0, 12, 0, 8, 0, 118, 237, 119, 0, 1, 104, 0, 0, 128,
	195, 68, 0, 0, 255, 16, 0, 48, 105, 129, 0, 0, 0, 13, 0, 254,
	101, 0, 0, 0, 9, 8, 0, 32, 0, 14, 0, 1, 97, 0, 0, 7,
	117, 102, 0, 29, 0, 0, 0, 114, 2, 232, 0, 120, 130, 0, 0, 0,
	65, 69, 0, 52, 192, 8, 0, 138, 0, 116, 0, 1, 104, 194, 0, 128,
	195, 0, 0, 0, 255, 127, 0, 136, 125, 10, 0, 0, 3, 13, 0, 124,
	0, 0, 0, 123, 139, 8, 0, 32, 47, 115, 0, 1, 66, 131, 0, 7,
	0, 0, 0, 133, 67, 112, 0, 135, 126, 196, 0, 11, 113, 0, 0, 0,
	6, 0, 0, 12, 0, 8, 0, 0, 108, 0, 0, 1, 104, 0, 0, 128,
	195, 4, 0, 122, 255, 16, 0, 48, 0, 129, 0, 0, 0, 13, 0, 254,
	101, 53, 0, 111, 9, 8, 0, 32, 199, 0, 0, 1, 0, 0, 0, 7,
	117, 0, 0, 49, 0, 0, 0, 0, 2, 232, 0, 0, 130, 0, 0, 64,
	65, 0, 0, 0, 31, 8, 0, 0, 95, 116, 0, 1, 104, 0, 0, 128,
	195, 121, 0, 0, 255, 127, 0, 137, 125, 10, 0, 0, 3, 13, 0, 50,
	0, 0, 0, 0, 0, 8, 0, 32, 0, 0, 0, 1, 66, 131, 0, 7
};

static const u16 BigCodeTableOffsets[256] =
{
	0, 0, 0, 8, 0, 16, 0, 24, 0, 0, 0, 0, 0, 32, 0, 0,
	0, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 48, 0, 0,
	0, 0, 0, 56, 0, 0, 0, 0, 0, 0, 0, 64, 72, 0, 0, 0,
	0, 80, 0, 88, 0, 0, 0, 0, 96, 0, 0, 0, 0, 104, 0, 0,
	0, 0, 0, 0, 112, 120, 0, 0, 0, 0, 0, 0, 0, 128, 0, 136,
	0, 0, 0, 0, 0, 0, 0, 0, 144, 0, 0, 0, 0, 0, 0, 0,
	0, 152, 0, 160, 0, 0, 0, 0, 0, 0, 0, 168, 0, 0, 0, 0,
	176, 184, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	192, 200, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 208, 0, 216,
	0, 224, 0, 0, 232, 0, 0, 240, 0, 248, 0, 0, 0, 256, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 264, 0, 0, 272, 280, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 288, 0, 0, 296, 304, 0, 0,
	0, 312, 0, 0, 320, 328, 0, 336, 0, 0, 0, 344, 0, 352, 0, 0,
	0, 360, 0, 368, 0, 0, 0, 376, 0, 0, 0, 0, 0, 384, 0, 0,
	0, 0, 0, 392, 0, 0, 0, 0, 0, 0, 0, 400, 0, 0, 0, 0,
	408, 416, 0, 424, 432, 0, 0, 0, 440, 448, 0, 0, 0, 0, 0, 0
};

static const u8 BigCodeBitCounts[456] =
{
	9, 0, 9, 10, 9, 11, 9, 10, 10, 9, 10, 9,
	10, 9, 10, 9, 10, 9, 10, 9, 10, 9, 10, 9,
	9, 10, 9, 10, 9, 10, 9, 10, 9, 10, 9, 10,
	9, 10, 9, 10, 10, 9, 10, 9, 10, 9, 10, 9,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 9, 10, 9, 10, 9, 10, 9, 10, 9, 10, 9,
	10, 9, 10, 9, 10, 10, 10, 10, 10, 10, 10, 10,
	9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 10, 9,
	10, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	10, 10, 10, 10, 10, 10, 10, 10, 9, 10, 9, 10,
	9, 10, 9, 10, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 10, 9, 10, 9, 10, 9, 10, 10, 9, 10, 9,
	10, 9, 10, 9, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9,
	9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 10,
	9, 10, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 10,
	9, 10, 9, 10, 9, 10, 9, 10, 9, 10, 9, 10,
	9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 10, 9,
	10, 9, 10, 9, 10, 9, 10, 9, 10, 9, 10, 9,
	9, 10, 9, 10, 9, 10, 9, 10, 10, 9, 10, 9,
	10, 9, 10, 9, 9, 10, 9, 10, 9, 10, 9, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 9, 10, 9, 10,
	9, 10, 9, 10, 9, 10, 9, 10, 9, 10, 9, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9,
	9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10,
	9, 10, 9, 10, 9, 10, 9, 10, 10, 9, 10, 9,
	10, 9, 10, 9, 10, 10, 10, 10, 10, 10, 10, 10,
	9, 10, 9, 10, 9, 10, 9, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 9, 10, 9, 10, 9, 10, 9, 10,
	10, 9, 10, 9, 10, 9, 10, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 10, 9, 10, 9, 10, 9, 10,
	9, 9, 9, 9, 9, 9, 9, 9, 10, 9, 10, 9,
	10, 9, 10, 9, 10, 10, 10, 10, 10, 10, 10, 10
};

static const u8 BigCodeSymbols[456] =
{
	208, 0, 208, 243, 208, 247, 208, 243, 203, 193, 186, 193,
	203, 193, 186, 193, 248, 93, 238, 93, 248, 93, 238, 93,
	56, 234, 56, 198, 56, 234, 56, 198, 54, 212, 54, 38,
	54, 212, 54, 38, 43, 160, 185, 160, 43, 160, 185, 160,
	207, 204, 246, 166, 207, 204, 246, 166, 228, 60, 236, 23,
	228, 60, 236, 23, 148, 156, 22, 152, 148, 156, 22, 152,
	163, 80, 201, 80, 163, 80, 201, 80, 157, 144, 233, 144,
	157, 144, 233, 144, 42, 162, 87, 79, 42, 162, 87, 79,
	176, 168, 176, 168, 176, 168, 176, 168, 151, 72, 178, 72,
	151, 72, 178, 72, 34, 62, 34, 62, 34, 62, 34, 62,
	202, 149, 155, 171, 202, 149, 155, 171, 143, 175, 143, 206,
	143, 175, 143, 206, 30, 103, 30, 103, 30, 103, 30, 103,
	17, 219, 17, 249, 17, 219, 17, 249, 169, 55, 235, 55,
	169, 55, 235, 55, 73, 145, 190, 27, 73, 145, 190, 27,
	150, 82, 172, 83, 150, 82, 172, 83, 71, 90, 71, 90,
	71, 90, 71, 90, 229, 85, 44, 75, 229, 85, 44, 75,
	70, 146, 70, 146, 70, 146, 70, 146, 142, 211, 142, 189,
	142, 211, 142, 189, 191, 25, 253, 218, 191, 25, 253, 218,
	109, 99, 109, 99, 109, 99, 109, 99, 225, 59, 225, 231,
	225, 59, 225, 231, 40, 217, 40, 215, 40, 217, 40, 215,
	100, 51, 100, 51, 100, 51, 100, 51, 226, 58, 183, 58,
	226, 58, 183, 58, 81, 76, 188, 76, 81, 76, 188, 76,
	91, 251, 91, 245, 91, 251, 91, 245, 26, 98, 77, 98,
	26, 98, 77, 98, 15, 41, 15, 181, 15, 41, 15, 181,
	213, 159, 252, 209, 213, 159, 252, 209, 61, 187, 61, 179,
	61, 187, 61, 179, 106, 216, 106, 220, 106, 216, 106, 220,
	221, 165, 239, 177, 221, 165, 239, 177, 107, 141, 107, 141,
	107, 141, 107, 141, 180, 205, 242, 36, 180, 205, 242, 36,
	92, 78, 92, 197, 92, 78, 92, 197, 174, 140, 89, 140,
	174, 140, 89, 140, 173, 214, 35, 39, 173, 214, 35, 39,
	224, 167, 224, 244, 224, 167, 224, 244, 33, 84, 230, 184,
	33, 84, 230, 184, 88, 240, 74, 147, 88, 240, 74, 147,
	210, 20, 164, 222, 210, 20, 164, 222, 154, 86, 170, 182,
	154, 86, 170, 182, 94, 28, 94, 158, 94, 28, 94, 158,
	241, 18, 45, 18, 241, 18, 45, 18, 57, 200, 57, 200,
	57, 200, 57, 200, 96, 63, 96, 21, 96, 63, 96, 21,
	24, 46, 24, 46, 24, 46, 24, 46, 223, 19, 227, 19,
	223, 19, 227, 19, 153, 161, 250, 37, 153, 161, 250, 37
};

static void myT_ReadSymbol(u32& symbol, u32& bitsRead, u32 look)
{	
	const u32 lookFirstByte = look & 0xFF;
	const u8 bitCount = SmallCodeBitCounts[lookFirstByte];
	if(bitCount == 0)
	{
		const u32 secondTableIndex = BigCodeTableOffsets[lookFirstByte];
		const u32 secondTableOffset = (look >> 8) & 7;
		const u32 idx = secondTableIndex + secondTableOffset;
		bitsRead = BigCodeBitCounts[idx];
		symbol = BigCodeSymbols[idx];
		return;
	}

	bitsRead = bitCount;
	symbol = SmallCodeSymbols[lookFirstByte];
}

// Get the right-aligned bits starting at bitIndex.
// We only really need the first 11 and don't care what comes after that.
UDT_FORCE_INLINE u32 myT_GetBits(u32 bitIndex, const u8* fin)
{
	return *(u32*)(fin + (bitIndex >> 3)) >> (bitIndex & 7);
}

// Get the bit at bitIndex in the LSB of the result and have all other bits be 0.
UDT_FORCE_INLINE s32 myT_GetBit(s32 bitIndex, const u8* fin)
{
	return (fin[(bitIndex >> 3)] >> (bitIndex & 7)) & 1;
}

UDT_FORCE_INLINE void myT_OffsetReceive(s32* ch, const u8* fin, s32* offset)
{
	const u32 input = myT_GetBits(*(u32*)offset, fin);

	u32 bitsRead = 0;
	myT_ReadSymbol(*(u32*)ch, bitsRead, input);

	*offset += (s32)bitsRead;
}