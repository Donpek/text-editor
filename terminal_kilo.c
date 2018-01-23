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

// TODO(gunce): put the following in a seperate file (char.c?)
#define CHAR_MAX_BYTES_IN_A_CHARACTER 4

typedef struct
{
	u8 Bytes[CHAR_MAX_BYTES_IN_A_CHARACTER];
	u8 ByteCount;
} char_t;

global_variable struct {
	char_t Space;
	char_t Equals;
	char_t Tilde;
	char_t VerticalBar;
	char_t ZWithStroke;
	char_t KanjiOne;
	char_t KanjiTwo;
	char_t KanjiThree;
} GlobalCharacterTable = {0};

internal void
CharCopy(char_t Source, char_t *Destination)
{
	ASSERT(Source.ByteCount <= CHAR_MAX_BYTES_IN_A_CHARACTER);
	for(u32 ByteIndex = 0;
			ByteIndex < CHAR_MAX_BYTES_IN_A_CHARACTER;
			++ByteIndex)
	{
		if(ByteIndex < Source.ByteCount)
		{
			Destination->Bytes[ByteIndex] = Source.Bytes[ByteIndex];
		}else
		{
			Destination->Bytes[ByteIndex] = 0;
		}
	}
	Destination->ByteCount = Source.ByteCount;
}

internal void
CharInitGlobalCharacterTable(void)
{
	GlobalCharacterTable.Equals.ByteCount = 1;
	GlobalCharacterTable.Equals.Bytes[0] = 0x3d;
	GlobalCharacterTable.Space.ByteCount = 1;
	GlobalCharacterTable.Space.Bytes[0] = 0x20;
	GlobalCharacterTable.Tilde.ByteCount = 1;
	GlobalCharacterTable.Tilde.Bytes[0] = 0x7e;
	GlobalCharacterTable.VerticalBar.ByteCount = 1;
	GlobalCharacterTable.VerticalBar.Bytes[0] = 0x7c;
	GlobalCharacterTable.ZWithStroke.ByteCount = 2;
	GlobalCharacterTable.ZWithStroke.Bytes[0] = 0xc6;
	GlobalCharacterTable.ZWithStroke.Bytes[1] = 0xb5;
	GlobalCharacterTable.KanjiOne.ByteCount = 3;
	GlobalCharacterTable.KanjiOne.Bytes[0] = 0xe3;
	GlobalCharacterTable.KanjiOne.Bytes[1] = 0x86;
	GlobalCharacterTable.KanjiOne.Bytes[2] = 0x92;
	GlobalCharacterTable.KanjiTwo.ByteCount = 3;
	GlobalCharacterTable.KanjiTwo.Bytes[0] = 0xe3;
	GlobalCharacterTable.KanjiTwo.Bytes[1] = 0x86;
	GlobalCharacterTable.KanjiTwo.Bytes[2] = 0x93;
	GlobalCharacterTable.KanjiThree.ByteCount = 3;
	GlobalCharacterTable.KanjiThree.Bytes[0] = 0xe3;
	GlobalCharacterTable.KanjiThree.Bytes[1] = 0x86;
	GlobalCharacterTable.KanjiThree.Bytes[2] = 0x94;
}
// TODO(gunce): put the above in a seperate file (char.c?)

internal void
TerminalWriteBytes(const void *Bytes, u8 ByteCount)
{
	write(STDOUT_FILENO, Bytes, ByteCount);
}

internal void
TerminalWriteChar(char_t Char)
{
	// printf("%lu", Char.ByteCount);
	write(STDOUT_FILENO, Char.Bytes, Char.ByteCount);
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
	i32 nread;
	u32 c;
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
	// BUG(gunce): Can't quit the program if using Unicode for some reason.
	u32 c = TerminalReadKey();
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
TerminalUpdateScreen(editor_output_buffer *Buffer)
{
	// TODO(gunce): turn this function into one large write,
	// instead of many small ones.
	TerminalWriteBytes(SEQUENCE_CLEARSCREEN);
	char_t *CharPointer = (char_t *)Buffer->Memory;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer->Height;
	    ++RowIndex)
	{
		for(u32 ColumnIndex = 0;
		    ColumnIndex < Buffer->Width;
		    ++ColumnIndex)
		{
			TerminalWriteChar(*CharPointer);
			++CharPointer;
		}
		if(RowIndex != (Buffer->Height-1))
		{
			TerminalWriteBytes(SEQUENCE_NEWLINE);
		}
	}
	TerminalWriteBytes(SEQUENCE_RESETCURSOR);
}

internal void
TerminalEmptyBuffer(editor_output_buffer *Buffer)
{
	char_t *CharPointer = (char_t *)Buffer->Memory;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer->Height;
	    ++RowIndex)
	{
		for(u32 ColumnIndex = 0;
		    ColumnIndex < Buffer->Width;
		    ++ColumnIndex)
		{
			CharCopy(GlobalCharacterTable.Space, CharPointer);
			++CharPointer;
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
	setlocale(LC_ALL, "");
	CharInitGlobalCharacterTable();

	editor_input Input = {0};

	// TODO(gunce): implement terminal window resizing.
	editor_output_buffer MenuBuffer = {0};
	TerminalGetDimensions(&MenuBuffer.Width, &MenuBuffer.Height);
	ASSERT(MenuBuffer.Width && MenuBuffer.Height);
	MenuBuffer.Memory = malloc(MenuBuffer.Width * MenuBuffer.Height *
			       sizeof(char_t));
	ASSERT(MenuBuffer.Memory);
	TerminalEmptyBuffer(&MenuBuffer);
	EditorInitMenuBuffer(&MenuBuffer);

	TerminalEnableRawMode();
	while(1)
	{
		EditorUpdateBuffer(&MenuBuffer, &Input);
		TerminalUpdateScreen(&MenuBuffer);
		TerminalProcessKeypress(&Input);
	}
	return(0);
}
