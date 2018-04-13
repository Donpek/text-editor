#ifndef KILO
#define KILO

#define internal static
#define global_variable static

#include <stdint.h>
typedef int32_t b32;
typedef int8_t b8;
typedef int8_t i8;
typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;

#define EDITOR_MAX_CHOICES 3

// NOTE(gunce): modes of interaction
#define EDITOR_HOME_MENU 0
#define EDITOR_EDITING 1
#define EDITOR_INPUT_FILENAME 2
#define EDITOR_MESSAGE_BOX 3

// NOTE(gunce): line alignment types
#define EDITOR_ALIGN_LEFT 0
#define EDITOR_ALIGN_CENTER 1
#define EDITOR_ALIGN_RIGHT 2

// NOTE(gunce): labels
#define EDITOR_LABEL_OPEN_FILE 1
#define EDITOR_LABEL_NEW_FILE 2
#define EDITOR_LABEL_SETTINGS 3

#define EDITOR_BYTE 0
#define EDITOR_PIXEL 1

#define EDITOR_MOVE_UP 0
#define EDITOR_MOVE_DOWN 1
#define EDITOR_MOVE_LEFT 2
#define EDITOR_MOVE_RIGHT 3

#define EDITOR_SCROLL_UP 1
#define EDITOR_SCROLL_DOWN 0

typedef struct
{
	void *Memory;
	u32 Width, Height;
	u32 CursorX, CursorY;
} editor_screen_buffer;

typedef struct
{
	u32 Character;
	u32 BitInfo;
} editor_pixel;

typedef struct
{
	u32 PrevIndex, NextIndex;
	u32 Value;
} editor_char;

typedef struct
{
	void *Start;
	u32 Length;
	u32 Label;
} editor_line;

typedef struct
{
	u32 Character;
	u32 ByteCount;
} editor_input;

typedef struct
{
	editor_line *Lines;
	u32 LineCount;
	u32 *Characters;
	u32 CharacterCount;
	u8 *Bytes;
	u32 ByteCount;
	b32 IsModified;
} editor_file;

typedef struct
{
	editor_line Choices[EDITOR_MAX_CHOICES];
	editor_pixel *Cursor;
	editor_pixel *CursorBounds[2];
	editor_file File;
	u32 RenderOffset;
	u32 WriteBits;
	u32 ChoiceCount;
	u8 ChoiceIndex;
	u8 CurrentMode;
	u8 SavedMode;
} editor_memory;

// NOTE(gunce): services that the platform provides to the application.
internal void PlatformQuit(char *QuitMessage);
internal i32 PlatformReadWholeFile(const char *Path, void *Output);
// NOTE(gunce): services that the application provides to the platform.
// EditorUpdateScreen
#endif
