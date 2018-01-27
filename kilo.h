#ifndef KILO
#define KILO

#define EDITOR_MAX_CHOICES 3

// NOTE(gunce): modes of interaction
#define EDITOR_CHOOSING 0
#define EDITOR_EDITING 1

// NOTE(gunce): pixel info bits.
#define EDITOR_NEED_TO_DRAW 1
#define EDITOR_RED_FG 2
#define EDITOR_GREEN_FG 4
#define EDITOR_BLUE_FG 8
#define EDITOR_RED_BG 16
#define EDITOR_GREEN_BG 32
#define EDITOR_BLUE_BG 64

#define EDITOR_COLOR_MASK 0x7E

// NOTE(gunce): line info
#define EDITOR_ALIGN_CENTER 0
#define EDITOR_ALIGN_LEFT 1
#define EDITOR_ALIGN_RIGHT 2

typedef struct
{
	void *Memory;
	u32 Width;
	u32 Height;
} editor_screen_buffer;

typedef struct
{
	b8 *Quit;
	b8 *Select;
	b8 *Left;
	b8 *Right;
	b8 *Up;
	b8 *Down;
} editor_input;

typedef struct
{
	u32 Character;
	u32 BitInfo;
} editor_pixel;

typedef struct
{
	editor_pixel *Start;
	u32 Length;
} editor_line;

typedef struct
{
	editor_line Choices[EDITOR_MAX_CHOICES];
	u8 ChoiceIndex;
	u8 ChoiceCount;
	u8 CurrentMode;
} editor_memory;

// NOTE(gunce): services that the platform provides to the application.
internal void PlatformQuit();

// NOTE(gunce): services that the application provides to the platform.
internal void
EditorUpdateScreen(editor_screen_buffer *Video, editor_input Input,
									 editor_memory *Memory);
#endif
