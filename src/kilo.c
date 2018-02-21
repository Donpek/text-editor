#include "str.c"
#include "drawing_functions.c"

internal void
EditorScroll(editor_screen_buffer *Buffer, editor_memory *Memory,
						 i32 Amount)
{
	editor_window *Windows = Memory->Windows;
	editor_window CurrentWindow = Windows[0];

	if(Amount < 0 && CurrentWindow.RenderOffset < Amount)
	{
		return;
	}
	u32 LowerLimit = CurrentWindow.Height - CurrentWindow.Y;
	if(CurrentWindow.RenderOffset < LowerLimit)
	{
		Windows[0].RenderOffset += Amount;
		EditorFillWholeScreen(Buffer, 'X', EDITOR_RED_FG);
		for(u32 WindowIndex = 0;
				WindowIndex < Memory->WindowCount;
				++WindowIndex)
		{
			EditorFillWindow(Buffer, Windows + WindowIndex, Memory);
		}
	}
}

internal editor_pixel *
EditorGetPixelAddress(editor_screen_buffer *Buffer, u32 X, u32 Y)
{
	editor_pixel *Result = Buffer->Memory;
	Result += X + (Y * Buffer->Width);
	return(Result);
}

internal void
EditorSetToOpenFile(editor_screen_buffer *Video, editor_memory *Memory)
{
	Memory->WriteBits = EDITOR_GREEN_FG;
	EditorFillWholeScreen(Video, ' ', 0);
	EditorFillRow(Video, Video->Height/2, 'T', EDITOR_RED_FG);
	Memory->Cursor = EditorGetPixelAddress(Video, 4, (Video->Height/2) - 1);
	Memory->CursorBounds[0] = Memory->Cursor;
	Memory->CursorBounds[1] = Memory->Cursor + Video->Width - 8;
	EditorInvertPixel(Memory->Cursor);
	EditorFillPrettyBorders(Video);
	Memory->CurrentMode = EDITOR_INPUT_FILENAME;
}

internal void
EditorSetToMessageBox(editor_screen_buffer *Buffer, editor_memory *Memory,
											u32 *Message, u8 ModeToSwitchToAfterConfirmation)
{
	u32 Width = Str32GetStringLength((u8 *)Message);
	u32 X = (Buffer->Width - Width) / 2;
	u32 Y = (Buffer->Height) / 2;
	EditorFillRectangle(Buffer, X, Y - 2, Width + 4, 5, '#',
		EDITOR_GREEN_FG, ' ',	0);
	EditorWriteLine(Buffer, X + 2, Y, Message, EDITOR_RED_FG, 0, 0);
	Memory->CurrentMode = EDITOR_MESSAGE_BOX;
	Memory->SavedMode = ModeToSwitchToAfterConfirmation;
}

internal void
EditorSetToEdit(editor_screen_buffer *Video, editor_memory *Memory)
{
	EditorFillWholeScreen(Video, ' ', 0);
	Memory->WriteBits = EDITOR_WHITE_FG;
	for(u32 WindowIndex = 0;
			WindowIndex < Memory->WindowCount;
			++WindowIndex)
	{
		EditorFillWindow(Video, &Memory->Windows[WindowIndex], Memory);
	}
	Memory->Cursor = (editor_pixel *)Video->Memory + Memory->CursorOffset;
	EditorInvertPixel(Memory->Cursor);
	Memory->CurrentMode = EDITOR_EDITING;
}

internal void
EditorSetToHomeMenu(editor_screen_buffer *Video, editor_memory *Memory)
{
	EditorFillWholeScreen(Video, ' ', 0);
	EditorFillPrettyBorders(Video);
	u32 Green = EDITOR_GREEN_FG;
	Memory->Choices[0] = EditorWriteLine(
		Video, Video->Width/2, Video->Height/2 - 2,
		(u32 *)"Open an exƵisting file.", EditorInvertColors(Green),
		EDITOR_ALIGN_CENTER, EDITOR_LABEL_OPEN_FILE
	);
  Memory->Choices[1] = EditorWriteLine(
		Video, Video->Width/2, Video->Height/2,
		(u32 *)"Create a new file.", Green, EDITOR_ALIGN_CENTER,
		EDITOR_LABEL_NEW_FILE
	);
	Memory->Choices[2] = EditorWriteLine(
		Video, Video->Width/2, Video->Height/2 + 2,
		(u32 *)"Show the Ƶettings.", Green, EDITOR_ALIGN_CENTER,
		EDITOR_LABEL_SETTINGS
	);
	Memory->ChoiceIndex = 0;
	Memory->ChoiceCount = 3;
	Memory->CurrentMode = EDITOR_HOME_MENU;
}

internal void
EditorReadCharactersFromBytes(u32 ByteCount, u8 *Bytes, u32 *Output)
{
	for(u32 CharacterIndex = 0;
			CharacterIndex < ByteCount;
			++CharacterIndex)
	{
		u32 SizeInBytes = Str32GetCharacterSize(Bytes);
		*Output = Str32ConvertBytesIntoCharacter(SizeInBytes, Bytes);
		Bytes += SizeInBytes;
		++Output;
	}
}

internal void
EditorReadCharactersFromPixels(u32 PixelCount, editor_pixel *Pixels, u32 *Output)
{
	for(u32 CharacterIndex = 0;
			CharacterIndex < PixelCount;
			++CharacterIndex)
	{
		*Output = Pixels[CharacterIndex].Character;
		++Output;
	}
}

internal u32
EditorReadLines(u32 CharacterCount, u32 *Characters, editor_line *Output)
{
	u32 LineCount = 0;
	while(CharacterCount)
	{
		editor_line Line = {0};
		Line.Start = (void *)Characters;

		while(*(Characters) != '\n')
		{
			++Line.Length;
			--CharacterCount;
			++Characters;
		}
		++Line.Length; // don't forget to add \n
		--CharacterCount;
		++Characters;

		++LineCount; // next line
		*Output = Line;
		++Output;
	}
	return(LineCount);
}

internal void
EditorUpdateScreen(editor_screen_buffer *Video, editor_input Input,
									 editor_memory *Memory)
{
	if(Input.Character == CTRL_PLUS('q'))
	{
		PlatformQuit();
	}else
	{
		switch(Memory->CurrentMode)
		{
			case EDITOR_HOME_MENU:
			{
				if((Input.Character & 0xFF) == 0x1B)
				{
					Input.Character >>= BYTES(1);
					if((Input.Character & 0xFF) == '[')
					{
						Input.Character >>= BYTES(1);
						Input.Character &= 0xFF;
						switch(Input.Character)
						{
							case 'A':
							{
								EditorInvertLineColors(
									Memory->Choices[Memory->ChoiceIndex]
								);
								if(Memory->ChoiceIndex > 0)
								{
									--Memory->ChoiceIndex;
								}else
								{
									Memory->ChoiceIndex = Memory->ChoiceCount - 1;
								}
								EditorInvertLineColors(
									Memory->Choices[Memory->ChoiceIndex]
								);
							} break;
							case 'B':
							{
								EditorInvertLineColors(
									Memory->Choices[Memory->ChoiceIndex]
								);
								if(Memory->ChoiceIndex < Memory->ChoiceCount - 1)
								{
									++Memory->ChoiceIndex;
								}else
								{
									Memory->ChoiceIndex = 0;
								}
								EditorInvertLineColors(
									Memory->Choices[Memory->ChoiceIndex]
								);
							} break;
						}
					}
				}else if(Input.Character == UNICODE_ENTER)
				{
					switch(Memory->Choices[Memory->ChoiceIndex].Label)
					{
						case EDITOR_LABEL_OPEN_FILE:
						{
							EditorSetToOpenFile(Video, Memory);
						} break;
						case EDITOR_LABEL_NEW_FILE:
						{
							// TODO(gunce): blank file opening logic.
						} break;
						case EDITOR_LABEL_SETTINGS:
						{
							// TODO(gunce): settings screen.
						} break;
						default:
						{
#ifdef DEBUG
							ASSERT(!"EditorUpdateScreen: no such label.");
#endif
						}
					}
				}
			} break;
			case EDITOR_INPUT_FILENAME:
			{
				// TODO(gunce): add arrow keys and shifting the whole string after
				// deleting a character
				if(Str32IsControlCharacter(Input.Character))
				{
					if(Input.Character == UNICODE_BACKSPACE &&
						 Memory->Cursor > Memory->CursorBounds[0])
					{
						EditorInvertPixel(Memory->Cursor);
						--Memory->Cursor;
						EditorWritePixel(Memory->Cursor, ' ', 0, 0);
						EditorInvertPixel(Memory->Cursor);
					}else if(Input.Character == UNICODE_ENTER)
					{
						editor_line InputtedLine = {0};
						InputtedLine.Start = Memory->CursorBounds[0];
						InputtedLine.Length = Memory->Cursor - Memory->CursorBounds[0];

						u32 InputtedFilename[InputtedLine.Length];
						EditorReadCharactersFromPixels(InputtedLine.Length,
							InputtedLine.Start, InputtedFilename);

						u8 ConvertedFilename[InputtedLine.Length * 4 + 1];
						u8 *ConvertedPointer = (u8 *)ConvertedFilename;
						for(u32 CharacterIndex = 0;
							CharacterIndex < InputtedLine.Length;
							++CharacterIndex)
						{
							u8 ByteCount = Str32GetCharacterSize((u8 *)(InputtedFilename +
								CharacterIndex));
							Str32ConvertCharacterIntoBytes(InputtedFilename[CharacterIndex],
								ConvertedPointer);
							ConvertedPointer += ByteCount;
						}
						*ConvertedPointer = 0;
						ConvertedPointer = (u8 *)ConvertedFilename;

						u8 *ReadOutputLocation = (u8 *)Memory + KILOBYTES(1);
						i32 BytesRead = PlatformReadWholeFile((char *)ConvertedPointer,
																									ReadOutputLocation);
						if(BytesRead)
						{
							editor_file OpenedFile = {0};
							OpenedFile.Bytes = ReadOutputLocation;
							// NOTE(gunce): beware of overflow.
							OpenedFile.ByteCount = (u32)BytesRead;
							OpenedFile.Characters = (u32 *)(ReadOutputLocation + BytesRead);
							OpenedFile.CharacterCount = Str32GetStringLength(OpenedFile.Bytes);
							EditorReadCharactersFromBytes(OpenedFile.CharacterCount,
							 	(void *)OpenedFile.Bytes, OpenedFile.Characters);
							OpenedFile.Lines = (editor_line *)(OpenedFile.Characters +
								OpenedFile.CharacterCount);
							OpenedFile.LineCount = EditorReadLines(OpenedFile.CharacterCount,
								OpenedFile.Characters, OpenedFile.Lines);
							Memory->CursorOffset = 0;
							// TODO(gunce): set window size to be the size of the current tab.
							Memory->Windows[Memory->WindowCount].Width = Video->Width-2;
							Memory->Windows[Memory->WindowCount].Height = Video->Height-2;
							Memory->Windows[Memory->WindowCount].X = 1;
							Memory->Windows[Memory->WindowCount].Y = 1;
							Memory->Windows[Memory->WindowCount].Contents = OpenedFile;
							++Memory->WindowCount;
							EditorSetToEdit(Video, Memory);
						}else
						{
							EditorSetToMessageBox(Video, Memory, (u32 *)"File not found.",
								EDITOR_HOME_MENU);
						}
					}
				}else if(Memory->Cursor < Memory->CursorBounds[1])
				{
					EditorWritePixel(Memory->Cursor, Input.Character, Memory->WriteBits, 0);
					++Memory->Cursor;
					EditorInvertPixel(Memory->Cursor);
				}
			} break;
			case EDITOR_EDITING:
			{
				if(Str32IsControlCharacter(Input.Character))
				{
					// STUDY(gunce): whether this logic is platform-dependent.
					if((Input.Character & 0xFF) == 0x1B)
					{
						Input.Character >>= BYTES(1);
						if((Input.Character & 0xFF) == '[')
						{
							Input.Character >>= BYTES(1);
							Input.Character &= 0xFF;
							switch(Input.Character)
							{
								case 'A':
								{
									EditorScroll(Video, Memory, -1);
								} break;
								case 'B':
								{
									EditorScroll(Video, Memory, 1);
								} break;
								case 'C':
								{
								} break;
								case 'D':
								{
								} break;
							}
						}
					}
					// TODO(gunce): key combination features.
				}else
				{
					// TODO(gunce): text input logic.
				}
			} break;
			case EDITOR_MESSAGE_BOX:
			{
				if(Input.Character)
				{
					switch(Memory->SavedMode)
					{
						case EDITOR_INPUT_FILENAME:
						{
							EditorSetToOpenFile(Video, Memory);
						} break;
						case EDITOR_HOME_MENU:
						{
							EditorSetToHomeMenu(Video, Memory);
						} break;
						case EDITOR_EDITING:
						{
							EditorSetToEdit(Video, Memory);
						} break;
#ifdef DEBUG
						default: ASSERT(!"EDITOR_MESSAGE_BOX: implement me!");
#endif
					}
				}
			} break;
			default:
			{
#ifdef DEBUG
				ASSERT(!"EditorUpdateScreen: no such mode exists.");
#endif
			}
		}
	}
}
