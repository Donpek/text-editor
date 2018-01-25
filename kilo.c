#include "kilo.h"

internal void
EditorWritePixel(editor_pixel *Destination, u32 Character, u32 BitInfo)
{
	if(GlobalNeedToReverseBytes)
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
	u32 Red = EDITOR_RED_FG;
	u32 Yellow = EDITOR_RED_FG | EDITOR_GREEN_FG;
	EditorFillWholeScreen(Video, ' ', 0);
	EditorFillColumn(Video, 0, '+', Yellow);
	EditorFillColumn(Video, 1, '=', Yellow);
	EditorFillColumn(Video, 2, '|', Yellow);
	EditorFillColumn(Video, Video->Width-3, '|', Yellow);
	EditorFillColumn(Video, Video->Width-2, '=', Yellow);
	EditorFillColumn(Video, Video->Width-1, '+', Yellow);
	EditorFillRow(Video, 0, 'Ƶ', Red);
	EditorFillRow(Video, Video->Height-1, 'Ƶ', Red);
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
