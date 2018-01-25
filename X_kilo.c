// NOTE(gunce): external libraries.
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>

// NOTE(gunce): internal stuff.
#include "kilo.h"
#include "bitmanip.c"

#define SEQUENCE_NEWLINE "\r\n", 2
#define SEQUENCE_CLEARSCREEN "\x1b[2J", 4
#define SEQUENCE_RESETCURSOR "\x1b[H", 3
#define SEQUENCE_RED_FG "\x1b[31m", 5
#define SEQUENCE_GREEN_FG "\x1b[32m", 5
#define SEQUENCE_BLUE_FG "\x1b[34m", 5
#define SEQUENCE_BLACK_FG "\x1b[30m", 5
#define SEQUENCE_YELLOW_FG "\x1b[33m", 5
#define SEQUENCE_RED_BG "\x1b[41m", 5
#define SEQUENCE_GREEN_BG "\x1b[42m", 5
#define SEQUENCE_BLUE_BG "\x1b[44m", 5
#define SEQUENCE_BLACK_BG "\x1b[40m", 5
#define SEQUENCE_YELLOW_BG "\x1b[43m", 5
#define SEQUENCE_RESET_ATTRIBUTES "\x1b[0m", 4

typedef struct
{
	u32 Character;
	u32 BitInfo;
} X_pixel;

typedef struct
{
	u32 SizeInBytes;
	void *Memory;
	u32 Width;
	u32 Height;
} X_screen_buffer;

global_variable struct termios GlobalOriginalTerminal;
global_variable b32 GlobalNeedToReverseBytes;

#include "kilo.c"

internal void
XWriteBytes(const void *Bytes, u8 ByteCount)
{
	write(STDOUT_FILENO, Bytes, ByteCount);
}

internal void
XRefreshScreen(void)
{
	XWriteBytes(SEQUENCE_CLEARSCREEN);
	XWriteBytes(SEQUENCE_RESETCURSOR);
}

internal u32
XReadKey(void)
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
XProcessKeypress(b32 *Input)
{
	// BUG(gunce): Can't quit the program if using Unicode for some reason.
	u32 Character = XReadKey();
	switch(Character)
	{
	case CTRL_PLUS('q'):{
		*Input |= EDITOR_QUIT;
	}break;
	}
}

internal void
XDisableRawMode(void)
{
	ASSERT(tcsetattr(STDIN_FILENO, TCSAFLUSH,
		  &GlobalOriginalTerminal) != -1);
}

internal void
XEnableRawMode(void)
{
	// TODO(gunce): make it so you can't scroll whilst the program is running.
	ASSERT(tcgetattr(STDIN_FILENO, &GlobalOriginalTerminal) != -1);
	atexit(XDisableRawMode);
	struct termios RawTerminal = GlobalOriginalTerminal;
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
XSetColor(u32 BitInfo)
{
	u32 Foreground = BitInfo & (EDITOR_RED_FG | EDITOR_GREEN_FG | EDITOR_BLUE_FG);
	u32 Background = BitInfo & (EDITOR_RED_BG | EDITOR_GREEN_BG | EDITOR_BLUE_BG);
	switch(Foreground)
	{
		case 0:
		{
			XWriteBytes(SEQUENCE_BLACK_FG);
		}break;
		case EDITOR_RED_FG:
		{
			XWriteBytes(SEQUENCE_RED_FG);
		}break;
		case EDITOR_GREEN_FG:
		{
			XWriteBytes(SEQUENCE_GREEN_FG);
		}break;
		case EDITOR_BLUE_FG:
		{
			XWriteBytes(SEQUENCE_BLUE_FG);
		}break;
		case EDITOR_RED_FG | EDITOR_GREEN_FG:
		{
			XWriteBytes(SEQUENCE_YELLOW_FG);
		}break;
		default: ASSERT(!"XSetColor: no such color.");
	}
	switch(Background)
	{
		case 0:
		{
			XWriteBytes(SEQUENCE_BLACK_BG);
		}break;
		case EDITOR_RED_BG:
		{
			XWriteBytes(SEQUENCE_RED_BG);
		}break;
		case EDITOR_GREEN_BG:
		{
			XWriteBytes(SEQUENCE_GREEN_BG);
		}break;
		case EDITOR_BLUE_BG:
		{
			XWriteBytes(SEQUENCE_BLUE_BG);
		}break;
		case EDITOR_RED_BG | EDITOR_GREEN_BG:
		{
			XWriteBytes(SEQUENCE_YELLOW_BG);
		}break;
		default: ASSERT(!"XSetColor: no such color.");
	}
}

internal void
XUpdateScreen(X_screen_buffer Buffer)
{
	X_pixel *PixelPointer = (X_pixel *)Buffer.Memory;
	for(u32 RowIndex = 0;
	    RowIndex < Buffer.Height;
	    ++RowIndex)
	{
		for(u32 ColumnIndex = 0;
		    ColumnIndex < Buffer.Width;
		    ++ColumnIndex)
		{
			if(PixelPointer->BitInfo & EDITOR_NEED_TO_DRAW)
			{
				XSetColor(PixelPointer->BitInfo);
				XWriteBytes(&PixelPointer->Character, sizeof(PixelPointer->Character));
				PixelPointer->BitInfo ^= EDITOR_NEED_TO_DRAW;
			}
			++PixelPointer;
		}
		if(RowIndex != (Buffer.Height-1))
		{
			XWriteBytes(SEQUENCE_NEWLINE);
		}
	}
	XWriteBytes(SEQUENCE_RESETCURSOR);
}

internal void
XGetDimensions(u32 *Width, u32 *Height)
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
	ASSERT(!"XGetDimensions: neither TIOCGSIZE nor TIOCGWINSZ found.");
#endif
}

// NOTE(gunce): platform services below.
internal void
PlatformQuit(void)
{
	XWriteBytes(SEQUENCE_RESET_ATTRIBUTES);
	XRefreshScreen();
	exit(0);
}
// NOTE(gunce): platform services above.

int main(void)
{
	setlocale(LC_ALL, "");
	GlobalNeedToReverseBytes = BitManipIsLittleEndian();

	// TODO(gunce): support terminal window resizing.
	// TODO(gunce): disable scrolling.
	X_screen_buffer XVideo = {0};
	XGetDimensions(&XVideo.Width, &XVideo.Height);
	ASSERT(XVideo.Width && XVideo.Height);
	XVideo.Memory = malloc
	(
		XVideo.Width * XVideo.Height * sizeof(X_pixel)
	);
	ASSERT(XVideo.Memory);
	// IMPORTANT(gunce): currently you don't need to do anything extra
	// to map the Linux buffer onto the platform-independant buffer. That
	// may or may not change some time in the future.
	editor_screen_buffer Video = {0};
	Video.Memory = XVideo.Memory;
	Video.Width = XVideo.Width;
	Video.Height = XVideo.Height;
	EditorSetToMenu(&Video);

	XEnableRawMode();
	b32 Input = 0;
	while(1)
	{
		EditorUpdateScreen(&Video, Input);
		XUpdateScreen(XVideo);
		XProcessKeypress(&Input);
	}
	return(0);
}
