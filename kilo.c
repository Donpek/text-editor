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
	if(!Str32IsControlCharacter(Character))
	{
		Destination->Character = Character;
	}
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
#ifdef DEBUG
	ASSERT(Row < Buffer->Height);
#endif
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
#ifdef DEBUG
	ASSERT(Column < Buffer->Width);
#endif
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
	u32 TextLength = Str32GetStringLength((u8 *)Text);
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
		default:
		{
#ifdef DEBUG
			ASSERT(!"EditorWriteLine: no such alignemnt.");
#endif
		}
	}
#ifdef DEBUG
	ASSERT(X < Buffer->Width && Y < Buffer->Height);
#endif
	editor_pixel *PixelPointer = (editor_pixel *)Buffer->Memory;
	PixelPointer += X + (Y * Buffer->Width);
	Result.Start = (void *)PixelPointer;
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
			{
#ifdef DEBUG
				ASSERT(!"EditorWriteLine: character not in 1-4 byte range.");
#endif
			}
		}
		EditorWritePixel(PixelPointer, CharacterToWrite, BitInfo, 0);
		WritePointer += CharsInBytes[CharacterIndex];
		++X;
		++PixelPointer;
		if(X == Buffer->Width)
		{
			X = 0;
			++Y;
#ifdef DEBUG
			if(Y == Buffer->Height)
			{
				ASSERT(!"EditorWriteLine: out of bounds write.");
			}
#endif
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
	EditorFillRow(Video, Video->Height/2, 'T', EDITOR_RED_FG);
	Memory->Cursor = EditorGetPixelAddress(Video, 4, (Video->Height/2) - 1);
	Memory->CursorBounds[0] = Memory->Cursor;
	Memory->CursorBounds[1] = Memory->Cursor + Video->Width - 8;
	EditorInvertPixel(Memory->Cursor);
	EditorFillPrettyBorders(Video);
	Memory->CurrentMode = EDITOR_INPUT_FILENAME;
}

internal void
EditorFillWindow(editor_screen_buffer *Buffer, editor_window *Window,
								 editor_memory *Memory)
{
#ifdef DEBUG
	ASSERT(Window->Width <= Buffer->Width);
	ASSERT(Window->Height <= Buffer->Height);
#endif
	editor_pixel *Cursor = (editor_pixel *)Buffer->Memory;
	Cursor += Window->X + (Window->Y * Buffer->Width);
	editor_line Line = *Window->Contents.Lines;
	u32 LineIndex = 0;
	u32 *Character = Window->Contents.Characters;
	u32 RowsAvailable = Window->Height;
	while(RowsAvailable)
	{
		if(Window->Width >= Line.Length)
		{
			for(u32 ColumnIndex = 0;
				ColumnIndex < Line.Length;
				++ColumnIndex)
			{
				EditorWritePixel(Cursor, *Character, Memory->WriteBits, 0);
				++Cursor;
				++Character;
			}
			Cursor += Buffer->Width - Line.Length;
			++LineIndex;
			if(LineIndex != Window->Contents.LineCount)
			{
				Line = *(Window->Contents.Lines + LineIndex);
			}else // window can hold more lines than the file has to give.
			{
				return;
			}
		}else // line is longer than window is wide.
		{
			for(u32 ColumnIndex = 0;
				ColumnIndex < Window->Width;
				++ColumnIndex)
			{
				EditorWritePixel(Cursor, *Character, Memory->WriteBits, 0);
				++Cursor;
				++Character;
			}
			Cursor += Buffer->Width - Window->Width;
			Line.Length -= Window->Width;
		}
		--RowsAvailable;
	}
}

internal void
EditorFillRectangle(editor_screen_buffer *Buffer, u32 X, u32 Y,
u32 Width, u32 Height, u32 BorderCharacter, u32 BorderBits,
u32 FillCharacter, u32 FillBits)
{
	editor_pixel *Start = (editor_pixel *)Buffer->Memory + X +
		Y * Buffer->Width;
	u32 BufferWidth = Buffer->Width;

	editor_pixel *Pixel = Start;
	Height -= 1;
	for(u32 HorizontalBorderIndex = 0;
			HorizontalBorderIndex < Width;
			++HorizontalBorderIndex)
	{
		EditorWritePixel(Pixel,
			BorderCharacter, BorderBits, 0);
		EditorWritePixel(Pixel + Height * BufferWidth,
			BorderCharacter, BorderBits, 0);
		++Pixel;
	}

	Pixel = Start + BufferWidth;
	Height -= 1;
	for(u32 VerticalBorderIndex = 0;
			VerticalBorderIndex < Height;
			++VerticalBorderIndex)
	{
		EditorWritePixel(Pixel,
			BorderCharacter, BorderBits, 0);
		EditorWritePixel(Pixel + Width - 1,
			BorderCharacter, BorderBits, 0);
		Pixel += BufferWidth;
	}

	Pixel = Start + BufferWidth + 1;
	Width -= 2;
	for(u32 RowIndex = 0;
			RowIndex < Height;
			++RowIndex)
	{
		for(u32 ColumnIndex = 0;
				ColumnIndex < Width;
				++ColumnIndex)
		{
			EditorWritePixel(Pixel, FillCharacter, FillBits, 0);
			++Pixel;
		}
		Pixel += BufferWidth - Width;
	}
}

internal void
EditorSetToMessageBox(editor_screen_buffer *Buffer, editor_memory *Memory,
											u32 *Message, u8 ModeToSwitchToAfterConfirmation)
{
	u32 Width = Str32GetStringLength((u8 *)Message);
	u32 X = (Buffer->Width - Width) / 2;
	u32 Y = (Buffer->Height) / 2;
	EditorFillRectangle(Buffer, X, Y - 2, Width + 4, 5, '#',
		EDITOR_GREEN_FG, ' ',	0);
	EditorWriteLine(Buffer, X + 2, Y, Message, EDITOR_RED_FG, 0, 0);
	Memory->CurrentMode = EDITOR_MESSAGE_BOX;
	Memory->SavedMode = ModeToSwitchToAfterConfirmation;
}

internal void
EditorSetToEdit(editor_screen_buffer *Video, editor_memory *Memory)
{
	EditorFillWholeScreen(Video, ' ', 0);
	Memory->WriteBits = EDITOR_WHITE_FG;
	// TODO(gunce): set cursor bounds (line number column, window boundaries).
	for(u32 WindowIndex = 0;
			WindowIndex < Memory->WindowCount;
			++WindowIndex)
	{
		EditorFillWindow(Video, &Memory->Windows[WindowIndex], Memory);
	}
	Memory->Cursor = (editor_pixel *)Video->Memory + Memory->CursorOffset;
	EditorInvertPixel(Memory->Cursor);
	Memory->CurrentMode = EDITOR_EDITING;
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
EditorReadCharactersFromBytes(u32 ByteCount, u8 *Bytes, u32 *Output)
{
	for(u32 CharacterIndex = 0;
			CharacterIndex < ByteCount;
			++CharacterIndex)
	{
		u32 SizeInBytes = Str32GetCharacterSize(Bytes);
		*Output = Str32ConvertBytesIntoCharacter(SizeInBytes, Bytes);
		Bytes += SizeInBytes;
		++Output;
	}
}

internal void
EditorReadCharactersFromPixels(u32 PixelCount, editor_pixel *Pixels, u32 *Output)
{
	for(u32 CharacterIndex = 0;
			CharacterIndex < PixelCount;
			++CharacterIndex)
	{
		*Output = Pixels[CharacterIndex].Character;
		++Output;
	}
}



internal u32
EditorReadLines(u32 CharacterCount, u32 *Characters, editor_line *Output)
{
	u32 LineCount = 0;
	while(CharacterCount)
	{
		editor_line Line = {0};
		Line.Start = (void *)Characters;

		while(*(Characters) != '\n')
		{
			++Line.Length;
			--CharacterCount;
			++Characters;
		}
		++Line.Length; // don't forget to add \n
		--CharacterCount;
		++Characters;

		++LineCount; // next line
		*Output = Line;
		++Output;
	}
	return(LineCount);
}

internal void
EditorUpdateScreen(editor_screen_buffer *Video, u32 Input,
									 editor_memory *Memory)
{
	if(Input == CTRL_PLUS('q'))
	{
		PlatformQuit();
	}else
	{
		switch(Memory->CurrentMode)
		{
			case EDITOR_HOME_MENU:
			{
				if(Input == UNICODE_2)
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
				}else if(Input == UNICODE_8)
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
				}else if(Input == UNICODE_5)
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
						default:
						{
#ifdef DEBUG
							ASSERT(!"EditorUpdateScreen: no such label.");
#endif
						}
					}
				}
			} break;
			case EDITOR_INPUT_FILENAME:
			{
				if(Str32IsControlCharacter(Input))
				{
					if(Input == UNICODE_BACKSPACE &&
						 Memory->Cursor > Memory->CursorBounds[0])
					{
						EditorInvertPixel(Memory->Cursor);
						--Memory->Cursor;
						EditorWritePixel(Memory->Cursor, ' ', 0, 0);
						EditorInvertPixel(Memory->Cursor);
					}else if(Input == UNICODE_ENTER)
					{
						editor_line InputtedLine = {0};
						InputtedLine.Start = Memory->CursorBounds[0];
						InputtedLine.Length = Memory->Cursor - Memory->CursorBounds[0];

						u32 InputtedFilename[InputtedLine.Length];
						EditorReadCharactersFromPixels(InputtedLine.Length,
							InputtedLine.Start, InputtedFilename);

						u8 ConvertedFilename[InputtedLine.Length * 4 + 1];
						u8 *ConvertedPointer = (u8 *)ConvertedFilename;
						for(u32 CharacterIndex = 0;
							CharacterIndex < InputtedLine.Length;
							++CharacterIndex)
						{
							u8 ByteCount = Str32GetCharacterSize((u8 *)(InputtedFilename +
								CharacterIndex));
							Str32ConvertCharacterIntoBytes(InputtedFilename[CharacterIndex],
								ConvertedPointer);
							ConvertedPointer += ByteCount;
						}
						*ConvertedPointer = 0;
						ConvertedPointer = (u8 *)ConvertedFilename;

						u8 *ReadOutputLocation = (u8 *)Memory + KILOBYTES(1);
						i32 BytesRead = PlatformReadWholeFile((char *)ConvertedPointer,
																									ReadOutputLocation);
						if(BytesRead)
						{
							editor_file OpenedFile = {0};
							OpenedFile.Bytes = ReadOutputLocation;
							// NOTE(gunce): beware of overflow.
							OpenedFile.ByteCount = (u32)BytesRead;
							OpenedFile.Characters = (u32 *)(ReadOutputLocation + BytesRead);
							OpenedFile.CharacterCount = Str32GetStringLength(OpenedFile.Bytes);
							EditorReadCharactersFromBytes(OpenedFile.CharacterCount,
							 	(void *)OpenedFile.Bytes, OpenedFile.Characters);
							OpenedFile.Lines = (editor_line *)(OpenedFile.Characters +
								OpenedFile.CharacterCount);
							OpenedFile.LineCount = EditorReadLines(OpenedFile.CharacterCount,
								OpenedFile.Characters, OpenedFile.Lines);
							Memory->CursorOffset = 0;
							// TODO(gunce): set window size to be the size of the current tab.
							Memory->Windows[Memory->WindowCount].Width = Video->Width-2;
							Memory->Windows[Memory->WindowCount].Height = Video->Height-2;
							Memory->Windows[Memory->WindowCount].X = 1;
							Memory->Windows[Memory->WindowCount].Y = 1;
							Memory->Windows[Memory->WindowCount].Contents = OpenedFile;
							++Memory->WindowCount;
							EditorSetToEdit(Video, Memory);
						}else
						{
							EditorSetToMessageBox(Video, Memory, (u32 *)"File not found.",
								EDITOR_HOME_MENU);
						}
					}
				}else if(Memory->Cursor < Memory->CursorBounds[1])
				{
					EditorWritePixel(Memory->Cursor, Input, Memory->WriteBits, 0);
					++Memory->Cursor;
					EditorInvertPixel(Memory->Cursor);
				}
			} break;
			case EDITOR_EDITING: break;
			case EDITOR_MESSAGE_BOX:
			{
				if(Input)
				{
					switch(Memory->SavedMode)
					{
						case EDITOR_INPUT_FILENAME:
						{
							EditorSetToOpenFile(Video, Memory);
						} break;
						case EDITOR_HOME_MENU:
						{
							EditorSetToHomeMenu(Video, Memory);
						} break;
#ifdef DEBUG
						default: ASSERT(!"EDITOR_MESSAGE_BOX: implement me!");
#endif
					}
				}
			} break;
			default:
			{
#ifdef DEBUG
				ASSERT(!"EditorUpdateScreen: no such mode exists.");
#endif
			}
		}
	}
}
