// NOTE(gunce): external libraries.
/* STUDY(gunce): once you finish this project, find about
| whether it's plausible to get rid of all of these
| includes. If it is, then be good and implement all of
| the externs you're using yourself. This will probably
| make this file more platform-specific, which is
| good in the way that I'll get to port this editor
| over to other platforms (OSes and/or terminals). */
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>

// NOTE(gunce): internal stuff.
#include "../kilo.h"
#include "bitmanip.c"

#define FORMAT(format, ...) {char B[1000]; sprintf(B, format); PlatformQuit(B);}

// NOTE(gunce): pixel info bits.
#define PIXEL_NEED_TO_DRAW 1
#define PIXEL_RED_FG 2
#define PIXEL_GREEN_FG 4
#define PIXEL_BLUE_FG 8
#define PIXEL_WHITE_FG PIXEL_RED_FG | PIXEL_GREEN_FG | PIXEL_BLUE_FG
#define PIXEL_RED_BG 16
#define PIXEL_GREEN_BG 32
#define PIXEL_BLUE_BG 64
#define PIXEL_WHITE_BG PIXEL_RED_BG | PIXEL_GREEN_BG | PIXEL_BLUE_BG
#define PIXEL_COLOR_MASK 0x7E

#define SEQUENCE_NEWLINE "\r\n", 2
#define SEQUENCE_CLEARSCREEN "\x1b[2J", 4
#define SEQUENCE_RESETCURSOR "\x1b[H", 3
#define SEQUENCE_NUDGE_CURSOR_RIGHT "\x1b[C", 3

#define SEQUENCE_RED_FG "\x1b[31m", 5
#define SEQUENCE_GREEN_FG "\x1b[32m", 5
#define SEQUENCE_BLUE_FG "\x1b[34m", 5
#define SEQUENCE_BLACK_FG "\x1b[30m", 5
#define SEQUENCE_WHITE_FG "\x1b[37m", 5
#define SEQUENCE_CYAN_FG "\x1b[36m", 5
#define SEQUENCE_MAGENTA_FG "\x1b[35m", 5
#define SEQUENCE_YELLOW_FG "\x1b[33m", 5
#define SEQUENCE_RED_BG "\x1b[41m", 5
#define SEQUENCE_GREEN_BG "\x1b[42m", 5
#define SEQUENCE_BLUE_BG "\x1b[44m", 5
#define SEQUENCE_BLACK_BG "\x1b[40m", 5
#define SEQUENCE_WHITE_BG "\x1b[47m", 5
#define SEQUENCE_CYAN_BG "\x1b[46m", 5
#define SEQUENCE_MAGENTA_BG "\x1b[45m", 5
#define SEQUENCE_YELLOW_BG "\x1b[43m", 5
#define SEQUENCE_RESET_ATTRIBUTES "\x1b[0m", 4

#define ESCAPE_CHARACTER 0x1B

typedef struct
{
	u32 Character;
	u32 ByteCount;
} x_input;

typedef struct
{
	u32 Character;
	u32 BitInfo;
} x_pixel;

typedef struct
{
	u32 SizeInBytes;
	void *Memory;
	u32 Width;
	u32 Height;
} x_screen_buffer;

global_variable struct termios GlobalOriginalTerminal;
global_variable b32 GlobalNeedToReverseBytes;


internal u8
XWriteBytes(const void *Bytes, u8 ByteCount)
{
	i32 BytesWritten = write(STDOUT_FILENO, Bytes, ByteCount);
	if(BytesWritten == ByteCount){
		return(1);
	}else
	{
		return(0);
	}
}

internal void
XRefreshScreen(void)
{
	XWriteBytes(SEQUENCE_RESET_ATTRIBUTES);
	XWriteBytes(SEQUENCE_CLEARSCREEN);
	XWriteBytes(SEQUENCE_RESETCURSOR);
}
#define ASSERT(expression)\
if(!(expression))\
{\
	XRefreshScreen();\
	fprintf(stderr, "\n\n\rEXPRESSION:" #expression "| FILE:%s | LINE:%d\r\n",\
	__FILE__, __LINE__);\
	exit(1);}
#include "kilo.c"

internal void
XReadKey(x_input *Input)
{
	u32 Character = 0;
	i32 BytesRead = read(STDIN_FILENO, &Character, 4);
#ifdef DEBUG
	if(BytesRead > 0)
	{
		// NOTE(gunce): not sure about the (errno != EAGAIN) part.
		// Could potentially be buggy, but I need Cygwin to find out.
		ASSERT(BytesRead != -1 && errno != EAGAIN);
	}
#endif
	Input->ByteCount = BytesRead;
	Input->Character = Character;
}

internal void
XDisableRawMode(void)
{
#ifdef DEBUG
	ASSERT(tcsetattr(STDIN_FILENO, TCSAFLUSH,
		  &GlobalOriginalTerminal) != -1);
#else
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &GlobalOriginalTerminal);
#endif
}

internal void
XEnableRawMode(void)
{
	// TODO(gunce): make it so you can't scroll whilst the program is running.
	atexit(XDisableRawMode);
	struct termios RawTerminal = GlobalOriginalTerminal;
	RawTerminal.c_iflag &= ~(IXON | ICRNL | INPCK | BRKINT | ISTRIP);
	RawTerminal.c_oflag &= ~(OPOST);
	RawTerminal.c_cflag |= CS8;
	RawTerminal.c_lflag &= ~(ECHO | IEXTEN | ICANON | ISIG);
	RawTerminal.c_cc[VMIN] = 0;
	RawTerminal.c_cc[VTIME] = 1;
#ifdef DEBUG
	ASSERT(tcgetattr(STDIN_FILENO, &GlobalOriginalTerminal) != -1);
	ASSERT(tcsetattr(STDIN_FILENO, TCSAFLUSH, &RawTerminal) != -1);
#else
	tcgetattr(STDIN_FILENO, &GlobalOriginalTerminal);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &RawTerminal);
#endif

}

internal void
XSetColor(u32 BitInfo)
{
	u32 Foreground = BitInfo & (PIXEL_RED_FG | PIXEL_GREEN_FG | PIXEL_BLUE_FG);
	u32 Background = BitInfo & (PIXEL_RED_BG | PIXEL_GREEN_BG | PIXEL_BLUE_BG);
	switch(Foreground)
	{
		case 0:
		{
			XWriteBytes(SEQUENCE_BLACK_FG);
		}break;
		case PIXEL_RED_FG | PIXEL_GREEN_FG | PIXEL_BLUE_FG:
		{
			XWriteBytes(SEQUENCE_WHITE_FG);
		}break;
		case PIXEL_RED_FG:
		{
			XWriteBytes(SEQUENCE_RED_FG);
		}break;
		case PIXEL_GREEN_FG:
		{
			XWriteBytes(SEQUENCE_GREEN_FG);
		}break;
		case PIXEL_BLUE_FG:
		{
			XWriteBytes(SEQUENCE_BLUE_FG);
		}break;
		case PIXEL_RED_FG | PIXEL_GREEN_FG:
		{
			XWriteBytes(SEQUENCE_YELLOW_FG);
		}break;
		case PIXEL_RED_FG | PIXEL_BLUE_FG:
		{
			XWriteBytes(SEQUENCE_MAGENTA_FG);
		}break;
		case PIXEL_BLUE_FG | PIXEL_GREEN_FG:
		{
			XWriteBytes(SEQUENCE_CYAN_FG);
		}break;
		default:
		{
#ifdef DEBUG
			ASSERT(!"XSetColor: no such color.");
#endif
		}
	}
	switch(Background)
	{
		case 0:
		{
			XWriteBytes(SEQUENCE_BLACK_BG);
		}break;
		case PIXEL_RED_BG | PIXEL_GREEN_BG | PIXEL_BLUE_BG:
		{
			XWriteBytes(SEQUENCE_WHITE_BG);
		}break;
		case PIXEL_RED_BG:
		{
			XWriteBytes(SEQUENCE_RED_BG);
		}break;
		case PIXEL_GREEN_BG:
		{
			XWriteBytes(SEQUENCE_GREEN_BG);
		}break;
		case PIXEL_BLUE_BG:
		{
			XWriteBytes(SEQUENCE_BLUE_BG);
		}break;
		case PIXEL_RED_BG | PIXEL_GREEN_BG:
		{
			XWriteBytes(SEQUENCE_YELLOW_BG);
		}break;
		case PIXEL_RED_BG | PIXEL_BLUE_BG:
		{
			XWriteBytes(SEQUENCE_MAGENTA_BG);
		}break;
		case PIXEL_BLUE_BG | PIXEL_GREEN_BG:
		{
			XWriteBytes(SEQUENCE_CYAN_BG);
		}break;

		default:
		{
#ifdef DEBUG
			ASSERT(!"XSetColor: no such color.");
#endif
		}
	}
}

internal void
XUpdateScreen(x_screen_buffer Buffer)
{
	x_pixel *PixelPointer = (x_pixel *)Buffer.Memory;
	u8 Sequence[32];
	u8 SequenceLength;

	for(u32 RowIndex = 1;
	    RowIndex <= Buffer.Height;
	    ++RowIndex)
	{
		for(u32 ColumnIndex = 1;
		    ColumnIndex <= Buffer.Width;
		    ++ColumnIndex, ++PixelPointer)
		{
			if(PixelPointer->BitInfo & PIXEL_NEED_TO_DRAW)
			{
				Sequence[0] = 0x1B; Sequence[1] = '[';
				SequenceLength = 2;
				SequenceLength += Str8IntToString(RowIndex,
					Sequence + SequenceLength, 12);
				Sequence[SequenceLength++] = ';';
				SequenceLength += Str8IntToString(ColumnIndex,
					Sequence + SequenceLength, 16);
				Sequence[SequenceLength++] = 'H';
				// Move cursor to the correct position.
				XWriteBytes(Sequence, SequenceLength);
				XSetColor(PixelPointer->BitInfo);
				XWriteBytes(&PixelPointer->Character, sizeof(PixelPointer->Character));
				PixelPointer->BitInfo ^= PIXEL_NEED_TO_DRAW;
			}
		}
	}
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
PlatformQuit(char *QuitMessage)
{
	XWriteBytes(SEQUENCE_RESET_ATTRIBUTES);
	XRefreshScreen();
	printf("Quit message: %s\n\r", QuitMessage);
	exit(0);
}

internal i32
// NOTE(gunce): may be buggy, hasn't been tested.
PlatformReadWholeFile(const char *Path, void *Output)
{
	// STUDY(gunce): is casting to char * enough or do I need to process
	// Path for open() to work?
	i32 Result = 0;
	i32 Descriptor = open(Path, O_RDONLY);
	if(Descriptor > -1)
	{
		off_t Size = lseek(Descriptor, 0, SEEK_END);
		if(Size > -1)
		{
			lseek(Descriptor, 0, SEEK_SET);
			ssize_t BytesRead = read(Descriptor, Output, Size);
			if(BytesRead > -1)
			{
				Result = BytesRead;
			}else
			{
				// TODO(gunce): logging.
			}
		}else
		{
			// TODO(gunce): logging.
		}
	}else{
		// TODO(gunce): logging.
	}
	return(Result);
}
// NOTE(gunce): platform services above.

int main(void)
{
	setlocale(LC_ALL, "");
	GlobalNeedToReverseBytes = BitManipIsLittleEndian();

	// TODO(gunce): support terminal window resizing.
	// TODO(gunce): disable scrolling.
	x_screen_buffer XVideo = {0};
	XGetDimensions(&XVideo.Width, &XVideo.Height);
	if(XVideo.Width && XVideo.Height)
	{
		XVideo.Memory = malloc
		(
			XVideo.Width * XVideo.Height * sizeof(x_pixel)
		);
		if(XVideo.Memory)
		{
			editor_screen_buffer Video = {0};
			Video.Memory = XVideo.Memory;
			Video.Width = XVideo.Width;
			Video.Height = XVideo.Height;

			x_input XInput = {0};
			editor_input Input = {0};
			void *Memory = malloc(MEGABYTES(2) + KILOBYTES(1));
			if(Memory)
			{
				XEnableRawMode();
				EditorSetToHomeMenu(&Video, Memory);
				while(1)
				{
					EditorUpdateScreen(&Video, Input, Memory);
					XUpdateScreen(XVideo);
					XReadKey(&XInput);
					Input.ByteCount = XInput.ByteCount;
					Input.Character = XInput.Character;
				}
			}else
			{
				// TODO(gunce): logging.
			}
		}else
		{
			// TODO(gunce): logging.
		}
	}else
	{
		// TODO(gunce): logging.
	}

	return(0);
}
