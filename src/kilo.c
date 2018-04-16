// COMMIT AFTER FINISHING: Character insertion / generation of new arraylist nodes.
#include "str.c"

internal editor_line *
EditorGetLine(editor_memory *Memory, u32 LineNum)
{
	editor_file File = Memory->File;
	editor_line *Line = File.Lines;
	for(u32 LineIndex = 1;
			LineIndex < LineNum;
			++LineIndex)
	{
		Line = File.Lines + Line->NextIndex;
	}
	return(Line);
}

internal editor_char *
EditorGetCharacter(editor_memory *Memory, u32 LineNum, u32 CharNum)
{
	editor_file File = Memory->File;
	editor_char *Char = (editor_char *)EditorGetLine(Memory, LineNum)->Start;
	for(u32 CharIndex = 1;
			CharIndex < CharNum;
			++CharIndex)
	{
		Char = File.Characters + Char->NextIndex;
	}
	return(Char);
}

internal u32
EditorGetCharacterNodeIndex(editor_memory *Memory, u32 LineNum, u32 CharNum)
{
	editor_file File = Memory->File;
	editor_char *Char = (editor_char *)EditorGetLine(Memory, LineNum)->Start;
	u32 Result = File.Characters[Char->NextIndex].PrevIndex;
	for(u32 CharIndex = 1;
			CharIndex < CharNum;
			++CharIndex)
	{
		Result = Char->NextIndex;
		Char = File.Characters + Char->NextIndex;
	}
	return(Result);
}

internal u32
EditorGetLineNodeIndex(editor_memory *Memory, u32 LineNum)
{
	editor_file File = Memory->File;
	editor_line *Line = File.Lines;
	u32 Result = File.Lines[Line->NextIndex].PrevIndex;
	for(u32 LineIndex = 1;
			LineIndex < LineNum;
			++LineIndex)
	{
		Result = Line->NextIndex;
		Line = File.Lines + Line->NextIndex;
	}
	return(Result);
}

#include "drawing_functions.c"

internal void
EditorScroll(editor_screen_buffer *Buffer, editor_memory *Memory,
						 b32 ScrollUp)
{
	if(ScrollUp)
	{
		if(!Memory->RenderOffset)
		{
			return;
		}
		Memory->RenderOffset -= 1;
	}else
	{
		// TODO (Donatas) Check for end of file, too.
		if(Memory->File.CurrLineIndex + 1 == Memory->File.LineCount)
		{
			return;
		}
		Memory->RenderOffset += 1;
	}
	EditorFillWholeScreen(Buffer, ' ', Memory->WriteBits);
	EditorFillWithContent(Buffer, Memory);
}

internal editor_pixel *
EditorGetPixelAddress(editor_screen_buffer *Buffer,	u32 X, u32 Y)
{
	editor_pixel *Result = (editor_pixel *)Buffer->Memory;
	Result += X + (Y * Buffer->Width);
	return(Result);
}

internal void
EditorSetToOpenFile(editor_screen_buffer *Video, editor_memory *Memory)
{
	Memory->WriteBits = PIXEL_GREEN_FG;
	EditorFillWholeScreen(Video, ' ', 0);
	EditorFillRow(Video, Video->Height/2, 'T', PIXEL_RED_FG);
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
		PIXEL_GREEN_FG, ' ',	0);
	EditorWriteLine(Buffer, X + 2, Y, Message, PIXEL_RED_FG, 0, 0);
	Memory->CurrentMode = EDITOR_MESSAGE_BOX;
	Memory->SavedMode = ModeToSwitchToAfterConfirmation;
}

internal void
EditorSetToEdit(editor_screen_buffer *Video, editor_memory *Memory)
{
	Memory->WriteBits = PIXEL_WHITE_BG;
	EditorFillWholeScreen(Video, ' ', Memory->WriteBits);
	EditorFillWithContent(Video, Memory);
	Memory->Cursor = (editor_pixel *)Video->Memory;
	EditorInvertPixel(Memory->Cursor);
	Memory->CurrentMode = EDITOR_EDITING;
}

internal void
EditorSetToHomeMenu(editor_screen_buffer *Video, editor_memory *Memory)
{
	EditorFillWholeScreen(Video, ' ', 0);
	EditorFillPrettyBorders(Video);
	u32 Green = PIXEL_GREEN_FG;
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

internal u32
EditorReadCharactersFromBytes(u32 ByteCount, u8 *Bytes, editor_char *Output)
{
	u32 SizeInBytes = Str32GetCharacterSize(Bytes);
	Output->PrevIndex = 0; Output->NextIndex = 1;
	Output->Value = Str32ConvertBytesIntoCharacter(SizeInBytes, Bytes);
	Bytes += SizeInBytes;
	++Output;
	u32 CharacterIndex = 1;
	for(;CharacterIndex < ByteCount;
			++CharacterIndex)
	{
		SizeInBytes = Str32GetCharacterSize(Bytes);
		Output->PrevIndex = CharacterIndex - 1;
		Output->NextIndex = CharacterIndex + 1;
		Output->Value = Str32ConvertBytesIntoCharacter(SizeInBytes, Bytes);
		Bytes += SizeInBytes;
		++Output;
	}
	return(CharacterIndex);
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
EditorReadLines(u32 CharacterCount, editor_char *Characters, editor_line *Output)
{
	u32 LineCount = 0;
	u32 CharacterIndex = 0;
	editor_line Line = {0};

	Line.Start = (void *)(Characters + CharacterIndex);
	Line.PrevIndex = 0;
	Line.NextIndex = 1;

	while(Characters[CharacterIndex].Value != (u32)'\n')
	{
		++Line.Length;
		--CharacterCount;
		CharacterIndex = Characters[CharacterIndex].NextIndex;
	}
	++Line.Length; // don't forget to add \n
	--CharacterCount;
	CharacterIndex = Characters[CharacterIndex].NextIndex;

	++LineCount; // next line
	*Output = Line;
	++Output;

	while(CharacterCount)
	{
		Line.Start = (void *)(Characters + CharacterIndex);
		Line.PrevIndex = LineCount - 1;
		Line.NextIndex = LineCount + 1;
		Line.Length = 0; Line.Label = 0;

		while(Characters[CharacterIndex].Value != (u32)'\n')
		{
			++Line.Length;
			--CharacterCount;
			CharacterIndex = Characters[CharacterIndex].NextIndex;
		}
		++Line.Length; // don't forget to add \n
		--CharacterCount;
		CharacterIndex = Characters[CharacterIndex].NextIndex;

		++LineCount; // next line
		*Output = Line;
		++Output;
	}
	return(LineCount);
}

internal void
EditorMoveChoiceCursorUp(editor_memory *Memory)
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
}

internal void
EditorMoveChoiceCursorDown(editor_memory *Memory)
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
}

internal void
EditorRemovePixelFromScreen(editor_memory *Memory)
{
	EditorInvertPixel(Memory->Cursor);
	--Memory->Cursor;
	EditorWritePixel(Memory->Cursor, ' ', 0, 0);
	EditorInvertPixel(Memory->Cursor);
}

internal void
EditorTryOpeningAnExistingFile(editor_memory *Memory, editor_screen_buffer *Video)
{
	editor_line InputtedLine = {0};
	InputtedLine.Start = Memory->CursorBounds[0];
	InputtedLine.Length = Memory->Cursor - Memory->CursorBounds[0];

	u32 InputtedFilename[InputtedLine.Length];
	EditorReadCharactersFromPixels(InputtedLine.Length,
		InputtedLine.Start, InputtedFilename);

	u8 ConvertedFilename[InputtedLine.Length * sizeof(u32) + 1];
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
		Memory->File.Characters = (editor_char *)(ReadOutputLocation + MEGABYTES(1));
		Memory->File.CharacterCount =
			EditorReadCharactersFromBytes(BytesRead,
				ReadOutputLocation, Memory->File.Characters);

		Memory->File.Lines = (editor_line *)(ReadOutputLocation);
		Memory->File.LineCount = EditorReadLines(Memory->File.CharacterCount,
			Memory->File.Characters, Memory->File.Lines);

		EditorSetToEdit(Video, Memory);
	}else
	{
		EditorSetToMessageBox(Video, Memory, (u32 *)"File not found.",
			EDITOR_HOME_MENU);
	}
}

internal editor_pixel *
EditorTryToGetPixelFromFile(editor_memory *Memory, editor_screen_buffer *Video)
{
	editor_pixel *Result = 0;
	editor_file File = Memory->File;

	if(File.CurrLineIndex >= Memory->RenderOffset && File.CurrLineIndex <
		 Memory->RenderOffset + Video->Height)
	{
		Result = (editor_pixel *)Video->Memory;
		Result += (File.CurrLineIndex - Memory->RenderOffset) * Video->Width +
							File.CurrCharIndex;
	}
	return(Result);
}

internal void
EditorMoveCursor(editor_memory *Memory, editor_screen_buffer *Video,
								 i32 MoveDirection)
{
	editor_file File = Memory->File;
	EditorInvertPixel(Memory->Cursor);
	b32 NeedToScroll = 0; u32 ScrollDirection = 0;
	editor_line *CurrLine = EditorGetLine(Memory, File.CurrLineIndex + 1);
	editor_line *PrevLine = File.Lines + CurrLine->PrevIndex;
	editor_line *NextLine = File.Lines + CurrLine->NextIndex;
	switch(MoveDirection)
	{
		case EDITOR_MOVE_UP: {
			if(File.CurrLineIndex > 0)
			{
				--File.CurrLineIndex;
				if(File.CurrLineIndex - Memory->RenderOffset + 1 == 0)
				{
					NeedToScroll = 1; ScrollDirection = EDITOR_SCROLL_UP;
				}
				if(PrevLine->Length <	File.CurrCharIndex + 1)
				{
					File.CurrCharIndex = PrevLine->Length - 1;
				}
			}
		} break;
		case EDITOR_MOVE_DOWN: {
			if(File.CurrLineIndex < File.LineCount - 1)
			{
				++File.CurrLineIndex;
				if(File.CurrLineIndex >= File.LineCount ||
					 File.CurrLineIndex >= Video->Height + Memory->RenderOffset)
				{
					NeedToScroll = 1; ScrollDirection = EDITOR_SCROLL_DOWN;
				}
				if(NextLine->Length < File.CurrCharIndex + 1)
				{
					File.CurrCharIndex = NextLine->Length - 1;
				}
			}
		} break;
		case EDITOR_MOVE_LEFT: {
			if(File.CurrCharIndex > 0)
			{
				--File.CurrCharIndex;
			}else if(File.CurrLineIndex + Memory->RenderOffset > 0)
			{
				--File.CurrLineIndex;
				File.CurrCharIndex = PrevLine->Length - 1;
				if(File.CurrLineIndex - Memory->RenderOffset + 1 == 0)
				{
					NeedToScroll = 1; ScrollDirection = EDITOR_SCROLL_UP;
				}
			}
		} break;
		case EDITOR_MOVE_RIGHT: {
			if(File.CurrCharIndex < CurrLine->Length - 1)
			{
				++File.CurrCharIndex;
			}else	if(File.CurrLineIndex < File.LineCount - 1 &&
 					 		 File.CurrLineIndex < Video->Height + Memory->RenderOffset)
			{
				++File.CurrLineIndex;
				File.CurrCharIndex = 0;
				if(File.CurrLineIndex == Video->Height + Memory->RenderOffset)
				{
					NeedToScroll = 1; ScrollDirection = EDITOR_SCROLL_DOWN;
				}
			}
		} break;
	}
	Memory->File = File;
	if(NeedToScroll)
	{
		EditorScroll(Video, Memory, ScrollDirection);
	}
	Memory->Cursor = EditorTryToGetPixelFromFile(Memory, Video);
	if(!Memory->Cursor)
	{
		// FIXME (Donatas) Going below the last line triggers this.
		FORMAT("cursor out of bounds. Ln%d Ch%d",
					 (int)File.CurrLineIndex, (int)File.CurrCharIndex)
	}
	EditorInvertPixel(Memory->Cursor);
}

internal b32
EditorInsertCharacter(editor_memory *Memory, editor_screen_buffer *Video,
											editor_input Input)
{
	if(Input.Character == '\r')
	{
		Input.Character = '\n';
	}
	b32 NeedToClearScreen = 0;
	editor_file *File = &Memory->File;
	editor_char NewCharacter = {0};
	NewCharacter.Value = Input.Character;
	u32 NodeIndex = EditorGetCharacterNodeIndex(Memory, File->CurrLineIndex + 1,
																							File->CurrCharIndex + 1);
	u32 PrevIndexOfCurrNode = File->Characters[NodeIndex].PrevIndex;

	u32 LineIndex = EditorGetLineNodeIndex(Memory, File->CurrLineIndex + 1);
	if(!File->CurrCharIndex)
	{
		File->Lines[LineIndex].Start = (void *)(File->Characters + File->CharacterCount);
	}

	if(!File->CurrLineIndex && !File->CurrCharIndex)
	{
		NewCharacter.PrevIndex = File->CharacterCount;
		NewCharacter.NextIndex = File->FirstCharIndex;

		File->Characters[File->FirstCharIndex].PrevIndex = File->CharacterCount;
		File->FirstCharIndex = File->CharacterCount;
	}else
	{
		NewCharacter.PrevIndex = PrevIndexOfCurrNode;
		NewCharacter.NextIndex = NodeIndex;

		File->Characters[PrevIndexOfCurrNode].NextIndex = File->CharacterCount;
		File->Characters[NodeIndex].PrevIndex = File->CharacterCount;
	}

	// TODO (Donatas) Reuse zombie characters.
	++File->Lines[LineIndex].Length;
	if((File->Lines[LineIndex].Length - 1) % Video->Width == 0)
	{
		NeedToClearScreen = 1;
	}
	++File->CurrCharIndex;
	File->Characters[File->CharacterCount] = NewCharacter;

	// XXX
	if(Input.Character == '\n')
	{
		NeedToClearScreen = 1;
		// FIXME (Donatas) Pressing enter at the last newline in the file bugs it out.
		// TODO (Donatas) Start tracking the virtual line count (overflowing lines + normal lines)
		//  and make it so they count as actual lines when moving the cursor up/down.
		editor_line NewLine = {0};
		NewLine.Length =
			File->Lines[LineIndex].Length - File->CurrCharIndex;
		NewLine.Start = (void *)(File->Characters + NodeIndex);
		NewLine.PrevIndex = LineIndex;
		u32 NextIndexOfCurrLine = File->Lines[LineIndex].NextIndex;
		NewLine.NextIndex = NextIndexOfCurrLine;

		File->Lines[LineIndex].NextIndex = File->LineCount;
		File->Lines[NextIndexOfCurrLine].PrevIndex = File->LineCount;

		File->Lines[LineIndex].Length = File->CurrCharIndex;

		// TODO (Donatas) Reuse zombie lines.
		++File->CurrLineIndex;
		File->Lines[File->LineCount] = NewLine;
		++File->LineCount;
		File->CurrCharIndex = 0;
	}
	++File->CharacterCount;
	File->IsModified = 1;
	return(NeedToClearScreen);
}

internal void
EditorUpdateScreen(editor_screen_buffer *Video, editor_input Input,
									 editor_memory *Memory)
{
	if(Input.Character == CTRL_PLUS('q'))
	{
		PlatformQuit("Control Q");
	}else
	{
		switch(Memory->CurrentMode)
		{
			case EDITOR_HOME_MENU:
			{
				if((Input.Character & 0xFF) == ESCAPE_CHARACTER)
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
								EditorMoveChoiceCursorUp(Memory);
							} break;
							case 'B':
							{
								EditorMoveChoiceCursorDown(Memory);
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
						EditorRemovePixelFromScreen(Memory);
					}else if(Input.Character == UNICODE_ENTER)
					{
						EditorTryOpeningAnExistingFile(Memory, Video);
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
				if(Input.Character != '\r' && Input.Character != '\n' &&
					 Str32IsControlCharacter(Input.Character))
				{
					// STUDY(gunce): whether this logic is platform-dependent.
					if((Input.Character & 0xFF) == ESCAPE_CHARACTER)
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
									EditorMoveCursor(Memory, Video, EDITOR_MOVE_UP);
								} break;
								case 'B':
								{
									EditorMoveCursor(Memory, Video, EDITOR_MOVE_DOWN);
								} break;
								case 'C':
								{
									EditorMoveCursor(Memory, Video, EDITOR_MOVE_RIGHT);
								} break;
								case 'D':
								{
									EditorMoveCursor(Memory, Video, EDITOR_MOVE_LEFT);
								} break;
							}
						}
					}
					// TODO(gunce): key combination features.
					// TODO (Donatas) Deleting characters.
				}else
				{
					// TODO(gunce): text input logic.
					b32 NeedToClearScreen = EditorInsertCharacter(Memory, Video, Input);
					if(NeedToClearScreen)
					{
						EditorFillWholeScreen(Video, ' ', Memory->WriteBits);
					}
					EditorFillWithContent(Video, Memory);
					Memory->Cursor = EditorTryToGetPixelFromFile(Memory, Video);
					EditorInvertPixel(Memory->Cursor);
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
