#ifndef KILO
#define KILO

#define CTRL_PLUS(key) ((key) & 0x1f)
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
typedef int8_t i8;
typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;

// NOTE(gunce): input bits.
#define EDITOR_QUIT 1
#define EDITOR_UP 2
#define EDITOR_DOWN 4
#define EDITOR_LEFT 8
#define EDITOR_RIGHT 16

// NOTE(gunce): pixel info bits.
#define EDITOR_NEED_TO_REDRAW 1
#define EDITOR_RED 2
#define EDITOR_GREEN 4
#define EDITOR_BLUE 8

typedef struct
{
	void *Memory;
	u32 Width;
	u32 Height;
} editor_screen_buffer;

typedef struct
{
	u32 Character;
	u32 BitInfo;
} editor_pixel;

// NOTE(gunce): services that the platform provides to the application.
internal void PlatformQuit(void);

// NOTE(gunce): services that the application provides to the platform.
internal void EditorUpdateScreen(editor_screen_buffer *Video, b32 Input);
internal void EditorSetToMenu(editor_screen_buffer *Video);
#endif
