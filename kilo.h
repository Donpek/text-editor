#ifndef KILO
#define KILO

#define CTRL_PLUS(key) ((key) & 0x1f)
#define ASSERT(expression)\
if(!(expression))\
{\
	printf("EXPRESSION:" #expression "| FILE:%s | LINE:%d\r\n", __FILE__, __LINE__); exit(1);\
}

#define internal static
#define global_variable static

#define MAX_PIXEL_SIZE_IN_BYTES 5

#include <stdint.h>
typedef int32_t b32;
typedef int8_t i8;
typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;

typedef struct
{
	char Bytes[MAX_PIXEL_SIZE_IN_BYTES];
	char ByteCount;
} pixel_t;

typedef struct
{
	b32 Quit;
	// TODO(gunce): add more fields
} editor_input;

typedef struct
{
	void *Memory;
	u32 Width;
	u32 Height;
} editor_output_buffer;

// TODO(gunce): services that the platform provides to the application.
internal void PlatformQuit(void);
#endif
