#include "kilo.h"

internal void
EditorFillRow(editor_output_buffer *Buffer, u32 Row,
							const char *Character, u8 CharSizeInBytes)
{
	ASSERT(CharSizeInBytes <= MAX_PIXEL_SIZE_IN_BYTES);
	ASSERT(Row <= Buffer->Height);
	pixel_t *Pixel = (pixel_t *)Buffer->Memory;
	Pixel += (Row * Buffer->Width);
	for(u32 ColumnIndex = 0;
	    ColumnIndex < Buffer->Width;
	    ++ColumnIndex)
	{
		for(u32 ByteIndex = 0;
		    ByteIndex < CharSizeInBytes;
		    ++ByteIndex)
		{
			Pixel->Bytes[ByteIndex] = Character[ByteIndex];
		}
		Pixel->ByteCount = CharSizeInBytes;
		++Pixel;
	}
}

internal void
EditorFillColumn(editor_output_buffer *Buffer, u32 Column,
								 const char *Character, u8 CharSizeInBytes)
{
	ASSERT(CharSizeInBytes <= MAX_PIXEL_SIZE_IN_BYTES);
	ASSERT(Column <= Buffer->Width);
	pixel_t *Pixel = (pixel_t *)Buffer->Memory;
	Pixel += Column;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer->Height;
	    ++RowIndex)
	{
		for(u32 ByteIndex = 0;
		    ByteIndex < CharSizeInBytes;
		    ++ByteIndex)
		{
			Pixel->Bytes[ByteIndex] = Character[ByteIndex];
		}
		Pixel->ByteCount = CharSizeInBytes;
		Pixel += Buffer->Width;
	}
}

internal void
EditorInitMenuBuffer(editor_output_buffer *Menu)
{
	EditorFillRow(Menu, 0, "~", 1);
	EditorFillRow(Menu, 1, "~", 1);
	EditorFillRow(Menu, 2, "|", 1);
	EditorFillRow(Menu, 3, "~", 1);
	EditorFillRow(Menu, Menu->Height-4, "~", 1);
	EditorFillRow(Menu, Menu->Height-3, "|", 1);
	EditorFillRow(Menu, Menu->Height-2, "~", 1);
	EditorFillRow(Menu, Menu->Height-1, "~", 1);
	EditorFillColumn(Menu, 0, "|", 1);
	EditorFillColumn(Menu, Menu->Width-1, "|", 1);
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
