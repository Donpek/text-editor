// NOTE(gunce): external libraries.
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>

#define SEQUENCE_NEWLINE "\r\n", 2
#define SEQUENCE_CLEARSCREEN "\x1b[2J", 4
#define SEQUENCE_RESETCURSOR "\x1b[H", 3

#include "kilo.h"

global_variable struct termios OriginalTerminal;

#define BYTES(amount) (8 * (amount))

internal b32
BitManipIsLittleEndian(void)
{
	u32 Bytes = 0x0102;
	return(!!(Bytes & 0x02));
}

internal u32
BitManipCountNonZeroBytes(u32 Bytes)
{
	u32 Result = 0;
	for(u32 ByteIndex = 0;
			ByteIndex < sizeof(Bytes);
			++ByteIndex)
	{
		if(Bytes & 0xff)
		{
			++Result;
			Bytes >>= BYTES(1);
		}
	}
	return(Result);
}

internal u32
BitManipReverseBytes(u32 Bytes)
{
	u32 ByteCount = BitManipCountNonZeroBytes(Bytes);
	u32 Result = 0;
	for(u32 ByteIndex = 0;
			ByteIndex < ByteCount;
			++ByteIndex)
	{
		u8 Byte = (Bytes >> (BYTES(1) * ByteIndex)) & 0xff;
		Result |= Byte << (BYTES(ByteCount - 1) - (BYTES(1) * ByteIndex));
	}
	return(Result);
}

internal void
TerminalWriteBytes(const void *Bytes, u8 ByteCount)
{
	write(STDOUT_FILENO, Bytes, ByteCount);
}

internal void
TerminalWriteChar(u32 Char)
{
	write(STDOUT_FILENO, &Char, sizeof(Char));
}

internal void
TerminalRefreshScreen(void)
{
	TerminalWriteBytes(SEQUENCE_CLEARSCREEN);
	TerminalWriteBytes(SEQUENCE_RESETCURSOR);
}

internal u32
TerminalReadKey(void)
{
	i32 BytesRead;
	u32 Character = 0;
	while((BytesRead = read(STDIN_FILENO, &Character, 1)) < 1)
	{
		// NOTE(gunce): not sure about the (errno != EAGAIN) part.
		// Could potentially be buggy, but I need Cygwin to find out.
		ASSERT(BytesRead != -1 && errno != EAGAIN);
	}
	return(Character);
}

internal void
TerminalProcessKeypress(b32 *Input)
{
	// BUG(gunce): Can't quit the program if using Unicode for some reason.
	u32 Character = TerminalReadKey();
	switch(Character)
	{
	case CTRL_PLUS('q'):{
		*Input |= EDITOR_QUIT;
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
	// TODO(gunce): make it so you can't scroll whilst the program is running.
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
TerminalUpdateScreen(editor_output_buffer Buffer)
{
	// TODO(gunce): turn this function into one large write,
	// instead of many small ones.
	u32 *CharPointer = (u32 *)Buffer.Memory;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer.Height;
	    ++RowIndex)
	{
		for(u32 ColumnIndex = 0;
		    ColumnIndex < Buffer.Width;
		    ++ColumnIndex)
		{
			TerminalWriteChar(*CharPointer);
			++CharPointer;
		}
		if(RowIndex != (Buffer.Height-1))
		{
			TerminalWriteBytes(SEQUENCE_NEWLINE);
		}
	}
	TerminalWriteBytes(SEQUENCE_RESETCURSOR);
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
	setlocale(LC_ALL, "");
	EditorInitializeGlobals();

	b32 Input = 0;
	
	// TODO(gunce): implement terminal window resizing.
	// TODO(gunce): disable scrolling (it's gonna be platform specific :( ).
	editor_output_buffer MenuBuffer = {0};
	TerminalGetDimensions(&MenuBuffer.Width, &MenuBuffer.Height);
	ASSERT(MenuBuffer.Width && MenuBuffer.Height);
	MenuBuffer.Memory = malloc(MenuBuffer.Width * MenuBuffer.Height *
			       sizeof(u32));
	ASSERT(MenuBuffer.Memory);
	EditorFillWholeBuffer(&MenuBuffer, ' ');
	EditorInitMenuBuffer(&MenuBuffer);

	TerminalEnableRawMode();
	while(1)
	{
		EditorUpdateBuffer(&MenuBuffer, Input);
		TerminalUpdateScreen(MenuBuffer);
		TerminalProcessKeypress(&Input);
	}
	return(0);
}
