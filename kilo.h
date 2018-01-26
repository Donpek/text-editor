#ifndef KILO
#define KILO

#define EDITOR_MAX_CHOICES 3

// NOTE(gunce): input bits.
#define EDITOR_QUIT 1
#define EDITOR_UP 2
#define EDITOR_DOWN 4
#define EDITOR_LEFT 8
#define EDITOR_RIGHT 16

// NOTE(gunce): pixel info bits.
#define EDITOR_NEED_TO_DRAW 1
#define EDITOR_RED_FG 2
#define EDITOR_GREEN_FG 4
#define EDITOR_BLUE_FG 8
#define EDITOR_RED_BG 16
#define EDITOR_GREEN_BG 32
#define EDITOR_BLUE_BG 64

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
	u32 Character;
	u32 BitInfo;
} editor_pixel;

typedef struct
{
	editor_pixel *Text;
	u32 Length;
} editor_line;

// NOTE(gunce): services that the platform provides to the application.
internal void PlatformQuit(void);

// NOTE(gunce): services that the application provides to the platform.
internal void EditorUpdateScreen(editor_screen_buffer *Video, b32 Input);
#endif
