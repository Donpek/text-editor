// NOTE(gunce): external libraries.
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define SEQUENCE_CLEARSCREEN "\x1b[2J", 4
#define SEQUENCE_RESETCURSOR "\x1b[H", 3

#include "kilo.h"

global_variable struct termios OriginalTerminal;

internal void
TerminalWriteBytes(const char *Bytes, int ByteCount)
{	 
	write(STDOUT_FILENO, Bytes, ByteCount);
}

internal void
TerminalRefreshScreen(void)
{ 
	TerminalWriteBytes(SEQUENCE_CLEARSCREEN);
	TerminalWriteBytes(SEQUENCE_RESETCURSOR);
}

internal u8
TerminalReadKey(void)
{
	i32 nread;
	char c;
	while((nread = read(STDIN_FILENO, &c, 1)) != 1)
	{
		// NOTE(gunce): not sure about the (errno != EAGAIN) part.
		// Could potentially be buggy, but I need Cygwin to find out.
		ASSERT(nread != -1 && errno != EAGAIN);
	}
	return(c);
}

internal void
TerminalProcessKeypress(editor_input *Input)
{ 
	u8 c = TerminalReadKey();
	switch(c)
	{
	case CTRL_PLUS('q'):{
		Input->Quit = 1;
	}break;
	}
}

internal void
TerminalDisableRawMode(void)
{
	ASSERT(tcsetattr(STDIN_FILENO, TCSAFLUSH,
		  &OriginalTerminal) != -1);
}

internal void
TerminalEnableRawMode(void)
{
	ASSERT(tcgetattr(STDIN_FILENO, &OriginalTerminal) != -1);
	atexit(TerminalDisableRawMode);
	struct termios RawTerminal = OriginalTerminal;
	RawTerminal.c_iflag &= ~(IXON | ICRNL | INPCK |
				BRKINT | ISTRIP);
	RawTerminal.c_oflag &= ~(OPOST);
	RawTerminal.c_cflag |= CS8;
	RawTerminal.c_lflag &= ~(ECHO | IEXTEN | ICANON |
				 ISIG);
	RawTerminal.c_cc[VMIN] = 0;
	RawTerminal.c_cc[VTIME] = 1;

	ASSERT(tcsetattr(STDIN_FILENO, TCSAFLUSH, &RawTerminal) != -1);
}

internal void
TerminalUpdateScreen(editor_output_buffer *Buffer)
{
	// TODO(gunce): turn this function into one large WriteBytes, instead of
	// many small ones.
	TerminalWriteBytes(SEQUENCE_CLEARSCREEN);
	pixel_t *Pixel = (pixel_t *)Buffer->Memory;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer->Height;
	    ++RowIndex)
	{
		for(u32 ColumnIndex = 0;
		    ColumnIndex < Buffer->Width;
		    ++ColumnIndex)
		{
			TerminalWriteBytes(Pixel->Bytes, Pixel->ByteCount);
			++Pixel;
		}
		TerminalWriteBytes("\r\n", 2);
	}
	TerminalWriteBytes(SEQUENCE_RESETCURSOR);	
}

internal void
TerminalZeroBuffer(editor_output_buffer *Buffer)
{
	pixel_t *Pixel = (pixel_t *)Buffer->Memory;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer->Height;
	    ++RowIndex)
	{
		for(u32 ColumnIndex = 0;
		    ColumnIndex < Buffer->Width;
		    ++ColumnIndex)
		{
			Pixel->ByteCount = 0;
			++Pixel;
		}
	}	
}

internal void
TerminalGetDimensions(u32 *Width, u32 *Height)
{
#ifdef TIOCGSIZE
	struct ttysize TerminalSize;
	ioctl(STDIN_FILENO, TIOCGSIZE, &TerminalSize);
	*Width = TerminalSize.ts_cols; 
	*Height = TerminalSize.ts_lines;
#elif defined(TIOCGWINSZ)
	struct winsize TerminalSize;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &TerminalSize);
	*Width = TerminalSize.ws_col;
	*Height = TerminalSize.ws_row;
#else
	ASSERT(!"TerminalGetDimensions: neither TIOCGSIZE nor TIOCGWINSZ found.");
#endif
}

// NOTE(gunce): platform-dependent definitions below.
internal void
PlatformQuit(void)
{	
	TerminalRefreshScreen();
	exit(0);
}

#include "kilo.c"
// NOTE(gunce): platform-dependent definitions above.

int main(void)
{
	editor_input Input = {0};

	// TODO(gunce): implement terminal window resizing.
	editor_output_buffer Buffer = {0};
	TerminalGetDimensions(&Buffer.Width, &Buffer.Height);
	ASSERT(Buffer.Width && Buffer.Height);
	Buffer.Memory = malloc(Buffer.Width * Buffer.Height * 
			       sizeof(pixel_t));	
	TerminalZeroBuffer(&Buffer);

	TerminalEnableRawMode();
	while(1)
	{
		EditorUpdateBuffer(&Buffer, &Input);
		TerminalUpdateScreen(&Buffer);
		TerminalProcessKeypress(&Input);
	}
	return(0);
}
