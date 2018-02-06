#define CTRL_PLUS(key) ((key) & 0x1f)
#define UNICODE_2 0x32
#define UNICODE_8 0x38
#define UNICODE_4 0x34
#define UNICODE_6 0x36
#define UNICODE_5 0x35
#define UNICODE_ENTER 0xD
#define UNICODE_BACKSPACE 0x7F
#define UNICODE_NEWLINE 0xA

internal u32
Str8IntToString(u32 Number, u8 *Destination, u32 MaxDigitCount)
{
	u32 DigitCount = 1;
	u32 NumberCopy = Number;
	while(NumberCopy)
	{
		NumberCopy /= 10;
		++DigitCount;
	}
	if(MaxDigitCount < DigitCount)
	{
		return(0);
	}

	u32 DigitIndex;
	for(DigitIndex = DigitCount - 1;
			DigitIndex > 0;
			--DigitIndex, Number /= 10)
	{
		switch(Number % 10)
		{
			case 0: Destination[DigitIndex] = '0'; break;
			case 1: Destination[DigitIndex] = '1'; break;
			case 2: Destination[DigitIndex] = '2'; break;
			case 3: Destination[DigitIndex] = '3'; break;
			case 4: Destination[DigitIndex] = '4'; break;
			case 5: Destination[DigitIndex] = '5'; break;
			case 6: Destination[DigitIndex] = '6'; break;
			case 7: Destination[DigitIndex] = '7'; break;
			case 8: Destination[DigitIndex] = '8'; break;
			case 9: Destination[DigitIndex] = '9'; break;
		}
	}
	// Parse the remaining first digit.
	switch(Number)
	{
		case 0: *Destination = '0'; break;
		case 1: *Destination = '1'; break;
		case 2: *Destination = '2'; break;
		case 3: *Destination = '3'; break;
		case 4: *Destination = '4'; break;
		case 5: *Destination = '5'; break;
		case 6: *Destination = '6'; break;
		case 7: *Destination = '7'; break;
		case 8: *Destination = '8'; break;
		case 9: *Destination = '9'; break;
	}
	return(DigitCount);
}

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
Str32GetStringLength(const u8 *Bytes)
{
	u32 Result = 0;
	while(*Bytes != 0)
	{
		u32 CharacterSize = Str32GetCharacterSize(Bytes);
		if(CharacterSize)
		{
			++Result;
			Bytes += CharacterSize;
		}else
		{
#ifdef DEBUG
			ASSERT(!"Str32Str32GetCharacterSize: not utf-8.");
#endif
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
#ifdef DEBUG
			ASSERT(!"Str32Str32GetCharacterSize: not utf-8.");
#endif
		}
	}
}

internal u32
Str32ConvertBytesIntoCharacter(u32 ByteCount, u8 *Bytes)
{
	u32 Result = 0;
	for(u32 ByteIndex = 0;
			ByteIndex < ByteCount;
			++ByteIndex)
	{
		Result += (*Bytes) << (ByteIndex * BYTES(1));
	}
	return(Result);
}

internal void
Str32ConvertCharacterIntoBytes(u32 Character, u8 *Output)
{
	u32 Reversed = BitManipReverseBytes(Character);
	u32 ByteCount = Str32GetCharacterSize((u8 *)&Reversed);
	for(u32 ByteIndex = 0;
			ByteIndex < ByteCount;
			++ByteIndex)
	{
		*Output = 0xFF & (Reversed >> (ByteIndex * BYTES(1)));
		++Output;
	}
}
