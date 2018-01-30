#ifndef KILO
#define KILO

#define EDITOR_MAX_CHOICES 3

// NOTE(gunce): modes of interaction
#define EDITOR_HOME_MENU 0
#define EDITOR_EDITING 1
#define EDITOR_INPUT_FILENAME 2

// NOTE(gunce): pixel info bits.
#define EDITOR_NEED_TO_DRAW 1
#define EDITOR_RED_FG 2
#define EDITOR_GREEN_FG 4
#define EDITOR_BLUE_FG 8
#define EDITOR_WHITE_FG EDITOR_RED_FG | EDITOR_GREEN_FG | EDITOR_BLUE_FG
#define EDITOR_RED_BG 16
#define EDITOR_GREEN_BG 32
#define EDITOR_BLUE_BG 64
#define EDITOR_WHITE_BG EDITOR_RED_BG | EDITOR_GREEN_BG | EDITOR_BLUE_BG

#define EDITOR_COLOR_MASK 0x7E

// NOTE(gunce): line alignment types
#define EDITOR_ALIGN_CENTER 0
#define EDITOR_ALIGN_LEFT 1
#define EDITOR_ALIGN_RIGHT 2

// NOTE(gunce): labels
#define EDITOR_LABEL_OPEN_FILE 0
#define EDITOR_LABEL_NEW_FILE 1
#define EDITOR_LABEL_SETTINGS 2

#define CTRL_PLUS(key) ((key) & 0x1f)
#define UNICODE_2 0x32
#define UNICODE_8 0x38
#define UNICODE_4 0x34
#define UNICODE_6 0x36
#define UNICODE_5 0x35
#define UNICODE_ENTER 0xD
#define UNICODE_BACKSPACE 0x7F

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

typedef struct
{
	editor_pixel *Start;
	u32 Length;
	u32 Label;
} editor_line;

typedef struct
{
	editor_line Choices[EDITOR_MAX_CHOICES];
	editor_pixel *Cursor;
	editor_pixel *CursorBounds[2];
	u32 WriteBits;
	u8 ChoiceIndex;
	u8 ChoiceCount;
	u8 CurrentMode;
} editor_memory;

// NOTE(gunce): services that the platform provides to the application.
internal void PlatformQuit();

// NOTE(gunce): services that the application provides to the platform.
internal void
EditorUpdateScreen(editor_screen_buffer *Video, u32 Input,
									 editor_memory *Memory);
#endif
