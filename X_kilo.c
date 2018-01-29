// NOTE(gunce): external libraries.
/* STUDY(gunce): once you finish this project, find about
| whether it's plausible to get rid of all of these
| includes. If it is, then be good and implement all of
| the externs you're using yourself. This will probably
| make this file more platform-specific, which is
| good in the way that I'll get to port this editor
| over to other platforms (OSes and/or terminals). */
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>

#define ASSERT(expression)\
if(!(expression))\
{\
	printf("EXPRESSION:" #expression "| FILE:%s | LINE:%d\r\n",\
	 			 __FILE__, __LINE__); exit(1);\
}

#define internal static
#define global_variable static

#include <stdint.h>
typedef int32_t b32;
typedef int8_t b8;
typedef int8_t i8;
typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;

// NOTE(gunce): internal stuff.
#include "kilo.h"
#include "bitmanip.c"

#define CTRL_PLUS(key) ((key) & 0x1f)
#define X_NUMPAD_2 0x32
#define X_NUMPAD_8 0x38
#define X_NUMPAD_4 0x34
#define X_NUMPAD_6 0x36
#define X_NUMPAD_5 0x35

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

typedef struct
{
	u32 Character;
	b8 AnyCharacter;
	b8 CtrlQ;
	b8 NumPad2;
	b8 NumPad4;
	b8 NumPad5;
	b8 NumPad6;
	b8 NumPad8;
} x_keyboard;

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
	if((BytesRead = read(STDIN_FILENO, &Character, 4)) > 0)
	{
		// NOTE(gunce): not sure about the (errno != EAGAIN) part.
		// Could potentially be buggy, but I need Cygwin to find out.
		ASSERT(BytesRead != -1 && errno != EAGAIN);
	}
	return(Character);
}

internal void
XProcessKeypress(x_keyboard *Input)
{
	// TODO(gunce): filter out Alt combinations, arrow keys, home, end, pgdn/up,
	// delete, insert, F1-F12.
	u32 Character = XReadKey();
	switch(Character)
	{
		case CTRL_PLUS('q'):
		{
			Input->CtrlQ = 1;
		} break;
	}
	if(Str32IsControlCharacter(Character))
	{
		return;
	}
	Input->Character = Character;
	switch(Character)
	{
		case X_NUMPAD_2:
		{
			Input->NumPad2 = 1;
		} break;
		case X_NUMPAD_8:
		{
			Input->NumPad8 = 1;
		} break;
		case X_NUMPAD_5:
		{
			Input->NumPad5 = 1;
		} break;
		case X_NUMPAD_4:
		{
			Input->NumPad4 = 1;
		} break;
		case X_NUMPAD_6:
		{
			Input->NumPad6 = 1;
		} break;
		default:
			Input->AnyCharacter = 1;
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
		case EDITOR_RED_FG | EDITOR_GREEN_FG | EDITOR_BLUE_FG:
		{
			XWriteBytes(SEQUENCE_WHITE_FG);
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
		case EDITOR_RED_FG | EDITOR_BLUE_FG:
		{
			XWriteBytes(SEQUENCE_MAGENTA_FG);
		}break;
		case EDITOR_BLUE_FG | EDITOR_GREEN_FG:
		{
			XWriteBytes(SEQUENCE_CYAN_FG);
		}break;
		default: ASSERT(!"XSetColor: no such color.");
	}
	switch(Background)
	{
		case 0:
		{
			XWriteBytes(SEQUENCE_BLACK_BG);
		}break;
		case EDITOR_RED_BG | EDITOR_GREEN_BG | EDITOR_BLUE_BG:
		{
			XWriteBytes(SEQUENCE_WHITE_BG);
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
		case EDITOR_RED_BG | EDITOR_BLUE_BG:
		{
			XWriteBytes(SEQUENCE_MAGENTA_BG);
		}break;
		case EDITOR_BLUE_BG | EDITOR_GREEN_BG:
		{
			XWriteBytes(SEQUENCE_CYAN_BG);
		}break;

		default: ASSERT(!"XSetColor: no such color.");
	}
}

internal void
XUpdateScreen(x_screen_buffer Buffer)
{
	x_pixel *PixelPointer = (x_pixel *)Buffer.Memory;
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
			}else
			{
				XWriteBytes(SEQUENCE_NUDGE_CURSOR_RIGHT);
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

			// NOTE(gunce): nothing to do with the XInput API.
			x_keyboard XInput = {0};
			editor_input Input = {0};
			Input.Quit = &XInput.CtrlQ;
			Input.Left = &XInput.NumPad4;
			Input.Right = &XInput.NumPad6;
			Input.Up = &XInput.NumPad8;
			Input.Down = &XInput.NumPad2;
			Input.Select = &XInput.NumPad5;
			Input.CurrentCharacter = &XInput.Character;
			Input.AnyCharacter = &XInput.AnyCharacter;

			void *Memory = malloc(MEGABYTES(1) + KILOBYTES(1));
			if(Memory)
			{
				XEnableRawMode();
				EditorSetToHomeMenu(&Video, Memory);
				while(1)
				{
					EditorUpdateScreen(&Video, Input, Memory);
					XUpdateScreen(XVideo);
					XProcessKeypress(&XInput);
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
