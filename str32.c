internal b32
Str32IsControlCharacter(u32 Character)
{
	return (Character < 0x1F) || // C0 controls
				 (Character == 0x7F) || // C1 controls
				 (Character >= 0x80 && Character < 0x9F); // C2 controls
}

internal u32
Str32GetLength(const u32 *String)
{
	u32 Result = 0;
	u8 *BytePointer = (u8 *)String;
	while(*BytePointer != 0)
	{
		++BytePointer;
	}
	u8 *EndOfString = BytePointer;
	BytePointer = (u8 *)String;
	while(*BytePointer != 0)
	{
		if(*BytePointer < 0xC0) // single-byte character
		{
			++Result;
			++BytePointer;
		}else if( // double-byte character
			*BytePointer < 0xE0 && (BytePointer + 1) < EndOfString
			&& *(BytePointer + 1) > 0x7F)
		{
			++Result;
			BytePointer += 2;
		}else if( // triple-byte character
			*BytePointer < 0xF0 && (BytePointer + 2) < EndOfString
			&& *(BytePointer + 2) > 0x7F && *(BytePointer + 1) > 0x7F)
		{
			++Result;
			BytePointer += 3;
		}else if( // quadruple-byte character
			*BytePointer < 0xF8 && (BytePointer + 3) < EndOfString
			&& *(BytePointer + 3) > 0x7F &&
			*(BytePointer + 2) > 0x7F && *(BytePointer + 1) > 0x7F)
		{
			++Result;
			BytePointer += 4;
		}else
		{
			ASSERT(!"Str32: input string not in utf-8 format.");
		}
	}
	return(Result);
}

internal void
Str32GetCharacterLengths(const u32 *String, u8 Results[])
{
	u32 CharacterIndex = 0;
	u8 *BytePointer = (u8 *)String;
	while(*BytePointer != 0)
	{
		++BytePointer;
	}
	u8 *EndOfString = BytePointer;
	BytePointer = (u8 *)String;
	while(*BytePointer != 0)
	{
		if(*BytePointer < 0xC0) // single-byte character
		{
			++BytePointer;
			Results[CharacterIndex] = 1;
			++CharacterIndex;
		}else if( // double-byte character
			*BytePointer < 0xE0 && (BytePointer + 1) < EndOfString
			&& *(BytePointer + 1) > 0x7F)
		{
			BytePointer += 2;
			Results[CharacterIndex] = 2;
			++CharacterIndex;
		}else if( // triple-byte character
			*BytePointer < 0xF0 && (BytePointer + 2) < EndOfString
			&& *(BytePointer + 2) > 0x7F && *(BytePointer + 1) > 0x7F)
		{
			BytePointer += 3;
			Results[CharacterIndex] = 3;
			++CharacterIndex;
		}else if( // quadruple-byte character
			*BytePointer < 0xF8 && (BytePointer + 3) < EndOfString
			&& *(BytePointer + 3) > 0x7F &&
			*(BytePointer + 2) > 0x7F && *(BytePointer + 1) > 0x7F)
		{
			BytePointer += 4;
			Results[CharacterIndex] = 4;
			++CharacterIndex;
		}else
		{
			ASSERT(!"Str32: input string not in utf-8 format.");
		}
	}
}
