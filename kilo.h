#ifndef KILO
#define KILO

#define EDITOR_MAX_CHOICES 3
#define EDITOR_MAX_WINDOWS 6

// NOTE(gunce): modes of interaction
#define EDITOR_HOME_MENU 0
#define EDITOR_EDITING 1
#define EDITOR_INPUT_FILENAME 2
#define EDITOR_MESSAGE_BOX 3

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
#define EDITOR_ALIGN_LEFT 0
#define EDITOR_ALIGN_CENTER 1
#define EDITOR_ALIGN_RIGHT 2

// NOTE(gunce): labels
#define EDITOR_LABEL_OPEN_FILE 1
#define EDITOR_LABEL_NEW_FILE 2
#define EDITOR_LABEL_SETTINGS 3

#define EDITOR_BYTE 0
#define EDITOR_PIXEL 1

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
	void *Start;
	u32 Length;
	u32 Label;
} editor_line;

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
	u32 X; u32 Y;
	u32 Width;
	u32 Height;
	editor_file Contents;
} editor_window;

typedef struct
{
	editor_window Windows[EDITOR_MAX_WINDOWS];
	editor_line Choices[EDITOR_MAX_CHOICES];
	editor_pixel *Cursor;
	editor_pixel *CursorBounds[2];
	u32 CursorOffset;
	u32 WriteBits;
	u32 WindowCount;
	u32 ChoiceCount;
	u8 ChoiceIndex;
	u8 CurrentMode;
	u8 SavedMode;
} editor_memory;

// NOTE(gunce): services that the platform provides to the application.
internal void PlatformQuit(void);
internal i32 PlatformReadWholeFile(const char *Path, void *Output);
// NOTE(gunce): services that the application provides to the platform.
// EditorUpdateScreen
#endif
