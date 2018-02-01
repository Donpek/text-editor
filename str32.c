internal b32
Str32IsControlCharacter(u32 Character)
{
	return (Character < 0x1F) || // C0 controls
				 (Character == 0x7F) || // C1 controls
				 ((Character & 0xFF) == 0x1B) || // Escape character
				 (Character >= 0x80 && Character <= 0x9F); // C2 controls
}

internal u32
Str32GetCharacterSize(const u8 *Bytes)
{
	u32 Result = 0;
	if(*Bytes < 0xC0) // single-byte character
	{
		Result = 1;
	}else if( // double-byte character
		*Bytes < 0xE0 && *(Bytes + 1) > 0x7F)
	{
		Result = 2;
	}else if( // triple-byte character
		*Bytes < 0xF0 && *(Bytes + 2) > 0x7F && *(Bytes + 1) > 0x7F)
	{
		Result = 3;
	}else if( // quadruple-byte character
		*Bytes < 0xF8 && *(Bytes + 3) > 0x7F &&
		*(Bytes + 2) > 0x7F && *(Bytes + 1) > 0x7F)
	{
		Result = 4;
	}
	// NOTE(gunce): if Result == 0, then the character is not in utf-8 format.
	return(Result);
}

internal u32
Str32GetLength(const u32 *String)
{
	u32 Result = 0;
	u8 *BytePointer = (u8 *)String;
	while(*BytePointer != 0)
	{
		u32 CharacterSize = Str32GetCharacterSize(BytePointer);
		if(CharacterSize)
		{
			++Result;
			BytePointer += CharacterSize;
		}else
		{
			ASSERT(!"Str32Str32GetCharacterSize: not utf-8.");
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
		u32 CharacterSize = Str32GetCharacterSize(BytePointer);
		if(CharacterSize)
		{
			Results[CharacterIndex] = CharacterSize;
			++CharacterIndex;
			BytePointer += CharacterSize;
		}else
		{
			ASSERT(!"Str32Str32GetCharacterSize: not utf-8.");
		}
	}
}
