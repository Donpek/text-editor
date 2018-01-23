#include "kilo.h"

internal void
EditorFillRow(editor_output_buffer *Buffer, u32 Row,
							char_t Brush)
{
	ASSERT(Row < Buffer->Height);
	char_t *CharPointer = (char_t *)Buffer->Memory;
	CharPointer += (Row * Buffer->Width);
	for(u32 ColumnIndex = 0;
	    ColumnIndex < Buffer->Width;
	    ++ColumnIndex)
	{
		CharCopy(Brush, CharPointer);
		++CharPointer;
	}
}

internal void
EditorFillColumn(editor_output_buffer *Buffer, u32 Column,
								 char_t Brush)
{
	ASSERT(Column < Buffer->Width);
	char_t *CharPointer = (char_t *)Buffer->Memory;
	CharPointer += Column;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer->Height;
	    ++RowIndex)
	{
		CharCopy(Brush, CharPointer);
		CharPointer += Buffer->Width;
	}
}

internal void
EditorInitMenuBuffer(editor_output_buffer *Menu)
{
	EditorFillRow(Menu, 0,
								GlobalCharacterTable.ZWithStroke);
	EditorFillRow(Menu, Menu->Height-1,
								GlobalCharacterTable.ZWithStroke);
	EditorFillColumn(Menu, 0,	GlobalCharacterTable.VerticalBar);
	EditorFillColumn(Menu, 1, GlobalCharacterTable.Equals);
	EditorFillColumn(Menu, 2, GlobalCharacterTable.VerticalBar);
	EditorFillColumn(Menu, Menu->Width-3,
									 GlobalCharacterTable.VerticalBar);
	EditorFillColumn(Menu, Menu->Width-2,
									 GlobalCharacterTable.Equals);
	EditorFillColumn(Menu, Menu->Width-1,
									 GlobalCharacterTable.VerticalBar);
}

internal void
EditorUpdateBuffer(editor_output_buffer *Menu, editor_input *Input)
{
	if(!Input->Quit)
	{

	}else
	{
		PlatformQuit();
	}
}
