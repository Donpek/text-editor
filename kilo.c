#include "kilo.h"

internal void
EditorFillRow(editor_output_buffer *Buffer, u32 Row, const char *Character, u8 CharSizeInBytes)
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
EditorUpdateBuffer(editor_output_buffer *Buffer, editor_input *Input)
{
	if(!Input->Quit)
	{
		EditorFillRow(Buffer, 1, "~", 1);
		EditorFillRow(Buffer, 2, "~", 1);		
		EditorFillRow(Buffer, 3, "|", 1);
		EditorFillRow(Buffer, 4, "~", 1);
		EditorFillRow(Buffer, Buffer->Height-3, "~", 1);
		EditorFillRow(Buffer, Buffer->Height-2, "|", 1);			
		EditorFillRow(Buffer, Buffer->Height-1, "~", 1);
		// NOTE(gunce): Buffer-Height doesn't print for some reason.
		EditorFillRow(Buffer, Buffer->Height, "~", 1);
	}else
	{
		PlatformQuit();		
	}	
}

