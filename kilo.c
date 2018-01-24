#include "kilo.h"

internal void
EditorWritePixel(editor_pixel *Destination, u32 Character, u32 BitInfo)
{
	if(GlobalNeedToReverseBytes)
	{
		Character = BitManipReverseBytes(Character);
	}
	Destination->Character = Character;
	Destination->BitInfo = BitInfo;
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
			EditorWritePixel(PixelPointer, Character, BitInfo);
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
		EditorWritePixel(PixelPointer, Character, BitInfo);
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
		EditorWritePixel(PixelPointer, Character, BitInfo);
		PixelPointer += Buffer->Width;
	}
}

internal void
EditorSetToMenu(editor_screen_buffer *Video)
{
	EditorFillWholeScreen(Video, ' ', 0);
	EditorFillColumn(Video, 0, '+', 0);
	EditorFillColumn(Video, 1, '=', 0);
	EditorFillColumn(Video, 2, '|', 0);
	EditorFillColumn(Video, Video->Width-3, '|', 0);
	EditorFillColumn(Video, Video->Width-2, '=', 0);
	EditorFillColumn(Video, Video->Width-1, '+', 0);
	EditorFillRow(Video, 0, 'Ƶ', 0);
	EditorFillRow(Video, Video->Height-1, 'Ƶ', 0);
}

internal void
EditorUpdateScreen(editor_screen_buffer *Video, b32 Input)
{
	if(!(Input & EDITOR_QUIT))
	{

	}else
	{
		PlatformQuit();
	}
}
