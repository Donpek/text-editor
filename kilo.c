#include "kilo.h"

internal void
EditorWritePixel(editor_pixel *Destination, u32 Character,
								 u32 BitInfo, b32 CheckForReversal)
{
	if(CheckForReversal && GlobalNeedToReverseBytes)
	{
		Character = BitManipReverseBytes(Character);
	}
	Destination->Character = Character;
	Destination->BitInfo = BitInfo | EDITOR_NEED_TO_DRAW;
}

internal void
EditorFillWholeScreen(editor_screen_buffer *Buffer, u32 Character,
											u32 BitInfo)
{
	editor_pixel *PixelPointer = (editor_pixel *)Buffer->Memory;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer->Height;
	    ++RowIndex)
	{
		for(u32 ColumnIndex = 0;
		    ColumnIndex < Buffer->Width;
		    ++ColumnIndex)
		{
			EditorWritePixel(PixelPointer, Character, BitInfo, 1);
			++PixelPointer;
		}
	}
}

internal void
EditorFillRow(editor_screen_buffer *Buffer, u32 Row,
							u32 Character, u32 BitInfo)
{
	ASSERT(Row < Buffer->Height);
	editor_pixel *PixelPointer = (editor_pixel *)Buffer->Memory;
	PixelPointer += (Row * Buffer->Width);
	for(u32 ColumnIndex = 0;
	    ColumnIndex < Buffer->Width;
	    ++ColumnIndex)
	{
		EditorWritePixel(PixelPointer, Character, BitInfo, 1);
		++PixelPointer;
	}
}

internal void
EditorFillColumn(editor_screen_buffer *Buffer, u32 Column,
								 u32 Character, u32 BitInfo)
{
	ASSERT(Column < Buffer->Width);
	editor_pixel *PixelPointer = (editor_pixel *)Buffer->Memory;
	PixelPointer += Column;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer->Height;
	    ++RowIndex)
	{
		EditorWritePixel(PixelPointer, Character, BitInfo, 1);
		PixelPointer += Buffer->Width;
	}
}

// TODO(gunce): move this out to str32.c
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

internal editor_line
EditorWriteLine(editor_screen_buffer *Buffer, u32 X,
								u32 Y, const u32 *Text, u32 BitInfo, u32 Alignment)
{
	editor_line Result = {0};
	u32 TextLength = Str32GetLength(Text);
	u8 CharsInBytes[TextLength];
	Str32GetCharacterLengths(Text, CharsInBytes);
	Result.Length = TextLength;
	switch(Alignment)
	{
		case EDITOR_ALIGN_LEFT: break;
		case EDITOR_ALIGN_RIGHT:
		{
			X -= TextLength;
		}break;
		case EDITOR_ALIGN_CENTER:
		{
			X -= TextLength/2;
		}break;
		default: ASSERT(!"EditorWriteLine: no such alignemnt.");
	}
	ASSERT(X < Buffer->Width && Y < Buffer->Height);
	editor_pixel *PixelPointer = (editor_pixel *)Buffer->Memory;
	PixelPointer += X + (Y * Buffer->Width);
	Result.Start = PixelPointer;
	u8 *WritePointer = (u8 *)Text;
	for(u32 CharacterIndex = 0;
			CharacterIndex < TextLength;
			++CharacterIndex)
	{
		u32 CharacterToWrite = 0;
		switch(CharsInBytes[CharacterIndex])
		{
			case 1:
			{
				CharacterToWrite = *((u32 *)WritePointer) & 0x000000FF;
			} break;
			case 2:
			{
				CharacterToWrite = *((u32 *)WritePointer) & 0x0000FFFF;
			} break;
			case 3:
			{
				CharacterToWrite = *((u32 *)WritePointer) & 0x00FFFFFF;
			} break;
			case 4:
			{
				CharacterToWrite = *((u32 *)WritePointer);
			} break;
			default:
			ASSERT(!"EditorWriteLine: character not in 1-4 byte range.");
		}
		EditorWritePixel(PixelPointer, CharacterToWrite, BitInfo, 0);
		WritePointer += CharsInBytes[CharacterIndex];
		++X;
		++PixelPointer;
		if(X == Buffer->Width)
		{
			X = 0;
			++Y;
			if(Y == Buffer->Height)
			{
				ASSERT(!"EditorWriteLine: out of bounds write.");
			}
		}
	}
	return(Result);
}

internal u32
EditorInvertPixelColors(u32 BitInfo)
{
	u32 Result = EDITOR_COLOR_MASK & (~BitInfo);
	return(Result);
}

internal void
EditorInvertLineColors(editor_line Line)
{
	editor_pixel *PixelPointer = Line.Start;
	for(u32 PixelIndex = 0;
			PixelIndex < Line.Length;
			++PixelIndex)
	{
		u32 InvertedColor = EditorInvertPixelColors(PixelPointer->BitInfo);
		EditorWritePixel(PixelPointer, PixelPointer->Character,
										 InvertedColor, 0);
		++PixelPointer;
	}
}

internal void
EditorSetToMenu(editor_screen_buffer *Video, editor_memory *Memory)
{
	u32 Red = EDITOR_RED_FG;
	u32 Yellow = EDITOR_RED_FG | EDITOR_GREEN_FG;
	u32 Green = EDITOR_GREEN_FG;
	EditorFillWholeScreen(Video, ' ', 0);
	EditorFillColumn(Video, 0, '+', Yellow);
	EditorFillColumn(Video, 1, '=', Yellow);
	EditorFillColumn(Video, 2, '|', Yellow);
	EditorFillColumn(Video, Video->Width-3, '|', Yellow);
	EditorFillColumn(Video, Video->Width-2, '=', Yellow);
	EditorFillColumn(Video, Video->Width-1, '+', Yellow);
	EditorFillRow(Video, 0, 'Ƶ', Red);
	EditorFillRow(Video, Video->Height-1, 'Ƶ', Red);
	Memory->Choices[0] = EditorWriteLine(
		Video, Video->Width/2, Video->Height/2 - 2,
		(u32 *)"Open an exƵisting file.", EditorInvertPixelColors(Green),
		EDITOR_ALIGN_CENTER
	);
  Memory->Choices[1] = EditorWriteLine(
		Video, Video->Width/2, Video->Height/2,
		(u32 *)"Create a new file.", Green, EDITOR_ALIGN_CENTER
	);
	Memory->Choices[2] = EditorWriteLine(
		Video, Video->Width/2, Video->Height/2 + 2,
		(u32 *)"Show the Ƶettings.", Green, EDITOR_ALIGN_CENTER
	);
	Memory->ChoiceIndex = 0;
	Memory->ChoiceCount = 3;
	Memory->CurrentMode = EDITOR_CHOOSING;
}

internal void
EditorUpdateScreen(editor_screen_buffer *Video, editor_input Input,
									 editor_memory *Memory)
{
	if(*Input.Quit)
	{
		PlatformQuit();
	}else
	{
		switch(Memory->CurrentMode)
		{
			case EDITOR_CHOOSING:
			{
				if(*Input.Down)
				{
					EditorInvertLineColors(
						Memory->Choices[Memory->ChoiceIndex]
					);
					if(Memory->ChoiceIndex < Memory->ChoiceCount - 1)
					{
						++Memory->ChoiceIndex;
					}else
					{
						Memory->ChoiceIndex = 0;
					}
					EditorInvertLineColors(
						Memory->Choices[Memory->ChoiceIndex]
					);
					*Input.Down = 0;
				}else if(*Input.Up)
				{
					EditorInvertLineColors(
						Memory->Choices[Memory->ChoiceIndex]
					);
					if(Memory->ChoiceIndex > 0)
					{
						--Memory->ChoiceIndex;
					}else
					{
						Memory->ChoiceIndex = Memory->ChoiceCount - 1;
					}
					EditorInvertLineColors(
						Memory->Choices[Memory->ChoiceIndex]
					);
					*Input.Up = 0;
				}
			} break;
			case EDITOR_EDITING:
			{

			} break;
			default: ASSERT(!"EditorUpdateScreen: no such mode exists.");
		}
	}
}
