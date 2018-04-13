internal void
EditorWritePixel(editor_pixel *Destination, u32 Character,
								 u32 BitInfo, b32 CheckForReversal)
{
	if(CheckForReversal && GlobalNeedToReverseBytes)
	{
		Character = BitManipReverseBytes(Character);
	}
	if(!Character || !Str32IsControlCharacter(Character))
	{
		Destination->Character = Character;
	}
	Destination->BitInfo = BitInfo | PIXEL_NEED_TO_DRAW;
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
	u32 Result = PIXEL_COLOR_MASK & (~BitInfo);
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
	u32 Red = PIXEL_RED_FG;
	u32 Yellow = PIXEL_RED_FG | PIXEL_GREEN_FG;
	EditorFillColumn(Buffer, 0, '+', Yellow);
	EditorFillColumn(Buffer, 1, '=', Yellow);
	EditorFillColumn(Buffer, 2, '|', Yellow);
	EditorFillColumn(Buffer, Buffer->Width-3, '|', Yellow);
	EditorFillColumn(Buffer, Buffer->Width-2, '=', Yellow);
	EditorFillColumn(Buffer, Buffer->Width-1, '+', Yellow);
	EditorFillRow(Buffer, 0, 'Ƶ', Red);
	EditorFillRow(Buffer, Buffer->Height-1, 'Ƶ', Red);
}

internal void
EditorFillWithContent(editor_screen_buffer *Buffer, editor_memory *Memory)
{
	editor_pixel *Cursor = (editor_pixel *)Buffer->Memory;
	editor_line Line = *(Memory->File.Lines + Memory->RenderOffset);
	u32 LineIndex = Memory->RenderOffset;
	u32 *Character = (u32 *)(Line.Start);
	u32 RowsAvailable = Buffer->Height;
	while(RowsAvailable)
	{
		if(Buffer->Width >= Line.Length)
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
			if(LineIndex != Memory->File.LineCount)
			{
				Line = *(Memory->File.Lines + LineIndex);
			}else // screen can hold more lines than the file has to give.
			{
				return;
			}
		}else // line is longer than screen is wide.
		{
			for(u32 ColumnIndex = 0;
				ColumnIndex < Buffer->Width;
				++ColumnIndex)
			{
				EditorWritePixel(Cursor, *Character, Memory->WriteBits, 0);
				++Cursor;
				++Character;
			}
			Line.Length -= Buffer->Width;
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
