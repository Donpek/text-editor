#include "kilo.h"
#include "str32.c"

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

internal editor_line
EditorWriteLine(editor_screen_buffer *Buffer, u32 X,
								u32 Y, const u32 *Text, u32 BitInfo, u32 Alignment,
								u32 Label)
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
	Result.Label = Label;
	return(Result);
}

internal u32
EditorInvertColors(u32 BitInfo)
{
	u32 Result = EDITOR_COLOR_MASK & (~BitInfo);
	return(Result);
}

internal void
EditorInvertPixel(editor_pixel *Pixel)
{
	u32 InvertedColor = EditorInvertColors(Pixel->BitInfo);
	EditorWritePixel(Pixel, Pixel->Character, InvertedColor, 0);
}

internal void
EditorInvertLineColors(editor_line Line)
{
	editor_pixel *PixelPointer = Line.Start;
	for(u32 PixelIndex = 0;
			PixelIndex < Line.Length;
			++PixelIndex)
	{
		u32 InvertedColor = EditorInvertColors(PixelPointer->BitInfo);
		EditorWritePixel(PixelPointer, PixelPointer->Character,
										 InvertedColor, 0);
		++PixelPointer;
	}
}

internal void
EditorFillPrettyBorders(editor_screen_buffer *Buffer)
{
	u32 Red = EDITOR_RED_FG;
	u32 Yellow = EDITOR_RED_FG | EDITOR_GREEN_FG;
	EditorFillColumn(Buffer, 0, '+', Yellow);
	EditorFillColumn(Buffer, 1, '=', Yellow);
	EditorFillColumn(Buffer, 2, '|', Yellow);
	EditorFillColumn(Buffer, Buffer->Width-3, '|', Yellow);
	EditorFillColumn(Buffer, Buffer->Width-2, '=', Yellow);
	EditorFillColumn(Buffer, Buffer->Width-1, '+', Yellow);
	EditorFillRow(Buffer, 0, 'Ƶ', Red);
	EditorFillRow(Buffer, Buffer->Height-1, 'Ƶ', Red);
}

internal editor_pixel *
EditorGetPixelAddress(editor_screen_buffer *Buffer, u32 X, u32 Y)
{
	editor_pixel *Result = Buffer->Memory;
	Result += X + (Y * Buffer->Width);
	return(Result);
}

internal void
EditorSetToOpenFile(editor_screen_buffer *Video, editor_memory *Memory)
{
	Memory->WriteBits = EDITOR_GREEN_FG;
	EditorFillWholeScreen(Video, ' ', 0);
	EditorFillRow(Video, Video->Height/2, '_', EDITOR_RED_BG);
	Memory->Cursor = EditorGetPixelAddress(Video, 4, (Video->Height/2) - 1);
	EditorInvertPixel(Memory->Cursor);
	EditorFillPrettyBorders(Video);
	Memory->CurrentMode = EDITOR_INPUT_FILENAME;
}

internal void
EditorSetToHomeMenu(editor_screen_buffer *Video, editor_memory *Memory)
{
	EditorFillWholeScreen(Video, ' ', 0);
	EditorFillPrettyBorders(Video);
	u32 Green = EDITOR_GREEN_FG;
	Memory->Choices[0] = EditorWriteLine(
		Video, Video->Width/2, Video->Height/2 - 2,
		(u32 *)"Open an exƵisting file.", EditorInvertColors(Green),
		EDITOR_ALIGN_CENTER, EDITOR_LABEL_OPEN_FILE
	);
  Memory->Choices[1] = EditorWriteLine(
		Video, Video->Width/2, Video->Height/2,
		(u32 *)"Create a new file.", Green, EDITOR_ALIGN_CENTER,
		EDITOR_LABEL_NEW_FILE
	);
	Memory->Choices[2] = EditorWriteLine(
		Video, Video->Width/2, Video->Height/2 + 2,
		(u32 *)"Show the Ƶettings.", Green, EDITOR_ALIGN_CENTER,
		EDITOR_LABEL_SETTINGS
	);
	Memory->ChoiceIndex = 0;
	Memory->ChoiceCount = 3;
	Memory->CurrentMode = EDITOR_HOME_MENU;
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
			case EDITOR_HOME_MENU:
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
				}else if(*Input.Select)
				{
					switch(Memory->Choices[Memory->ChoiceIndex].Label)
					{
						case EDITOR_LABEL_OPEN_FILE:
						{
							EditorSetToOpenFile(Video, Memory);
						} break;
						case EDITOR_LABEL_NEW_FILE:
						{

						} break;
						case EDITOR_LABEL_SETTINGS:
						{

						} break;
						default: ASSERT(!"EditorUpdateScreen: no such label.");
					}
				}
			} break;
			case EDITOR_INPUT_FILENAME:
			{
				// TODO(gunce): filter out control characters (Enter, Esc, Tab, etc).
				// BUG(gunce): multiple-byte characters aren't printing. :(
				if(*Input.AnyCharacter)
				{
					printf("[%08x %08x]", *Input.CurrentCharacter, 'ą');
					EditorWritePixel(Memory->Cursor, BitManipReverseBytes(*Input.CurrentCharacter),
													 Memory->WriteBits, 0);
					++Memory->Cursor;
					EditorInvertPixel(Memory->Cursor);
					*Input.AnyCharacter = 0;
				}//else if()
			} break;
			default: ASSERT(!"EditorUpdateScreen: no such mode exists.");
		}
	}
}
