#include "kilo.h"

global_variable b32 GlobalNeedToReverseBytes = 0;

internal void
EditorInitializeGlobals(void)
{
	GlobalNeedToReverseBytes = BitManipIsLittleEndian();
}

internal void
EditorCopyChar(u32 Source, u32 *Destination)
{
	if(GlobalNeedToReverseBytes)
	{
		Source = BitManipReverseBytes(Source);
	}
	*Destination = Source;
}

internal void
EditorFillWholeBuffer(editor_output_buffer *Buffer, u32 Brush)
{
	u32 *CharPointer = (u32 *)Buffer->Memory;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer->Height;
	    ++RowIndex)
	{
		for(u32 ColumnIndex = 0;
		    ColumnIndex < Buffer->Width;
		    ++ColumnIndex)
		{
			EditorCopyChar(Brush, CharPointer);
			++CharPointer;
		}
	}
}

internal void
EditorFillRow(editor_output_buffer *Buffer, u32 Row,
							u32 Brush)
{
	ASSERT(Row < Buffer->Height);
	u32 *CharPointer = (u32 *)Buffer->Memory;
	CharPointer += (Row * Buffer->Width);
	for(u32 ColumnIndex = 0;
	    ColumnIndex < Buffer->Width;
	    ++ColumnIndex)
	{
		EditorCopyChar(Brush, CharPointer);
		++CharPointer;
	}
}

internal void
EditorFillColumn(editor_output_buffer *Buffer, u32 Column,
								 u32 Brush)
{
	ASSERT(Column < Buffer->Width);
	u32 *CharPointer = (u32 *)Buffer->Memory;
	CharPointer += Column;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer->Height;
	    ++RowIndex)
	{
		EditorCopyChar(Brush, CharPointer);
		CharPointer += Buffer->Width;
	}
}

internal void
EditorInitMenuBuffer(editor_output_buffer *Menu)
{
	EditorFillColumn(Menu, 0, '+');
	EditorFillColumn(Menu, 1, '=');
	EditorFillColumn(Menu, 2, '|');
	EditorFillColumn(Menu, Menu->Width-3, '|');
	EditorFillColumn(Menu, Menu->Width-2, '=');
	EditorFillColumn(Menu, Menu->Width-1, '+');
	EditorFillRow(Menu, 0, 'Ƶ');
	EditorFillRow(Menu, Menu->Height-1, 'Ƶ');
}

internal void
EditorUpdateBuffer(editor_output_buffer *Menu, b32 Input)
{
	if(!(Input & EDITOR_QUIT))
	{

	}else
	{
		PlatformQuit();
	}
}
