// Copyright 2020-2022 Goodriver2. All Rights Reserved.


#include "StringConvertBPFLibrary.h"

#include "Kismet/KismetStringLibrary.h"
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"

bool UStringConvertBPFLibrary::IsAllowedChar(UTF8CHAR LookupChar)
{
	static char AllowedChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+=_.~:/?#[]@!$&*,;'()";
	static bool bTableFilled = false;
	static bool AllowedTable[256] = { false };

	if (!bTableFilled)
	{
		for (int32 Idx = 0; Idx < UE_ARRAY_COUNT(AllowedChars) - 1; ++Idx)    // -1 to avoid trailing 0
			{
			uint8 AllowedCharIdx = static_cast<uint8>(AllowedChars[Idx]);
			check(AllowedCharIdx < UE_ARRAY_COUNT(AllowedTable));
			AllowedTable[AllowedCharIdx] = true;
			}

		bTableFilled = true;
	}

	return AllowedTable[LookupChar];
}

FString UStringConvertBPFLibrary::UrlEncode(const FString& UnencodedString)
{
	FTCHARToUTF8 Converter(*UnencodedString);    //url encoding must be encoded over each utf-8 byte
	const UTF8CHAR* UTF8Data = (UTF8CHAR*)Converter.Get();    //converter uses ANSI instead of UTF8CHAR - not sure why - but other code seems to just do this cast. In this case it really doesn't matter
	FString EncodedString = TEXT("");

	TCHAR Buffer[2] = { 0, 0 };

	for (int32 ByteIdx = 0, Length = Converter.Length(); ByteIdx < Length; ++ByteIdx)
	{
		UTF8CHAR ByteToEncode = UTF8Data[ByteIdx];
		
		if (IsAllowedChar(ByteToEncode))
		{
			Buffer[0] = ByteToEncode;
			FString TmpString = Buffer;
			EncodedString += TmpString;
		}
		else if (ByteToEncode != '\0')
		{
			EncodedString += TEXT("%");
			EncodedString += FString::Printf(TEXT("%.2X"), ByteToEncode);
		}
	}
	return EncodedString;
}

FString UStringConvertBPFLibrary::UrlDecode(const FString& EncodedString)
{
	FTCHARToUTF8 Converter(*EncodedString, EncodedString.Len());
	const UTF8CHAR* UTF8Data = (UTF8CHAR*)Converter.Get();

	TArray<ANSICHAR> Data;
	Data.Reserve(EncodedString.Len());

	for (int32 CharIdx = 0; CharIdx < Converter.Length();)
	{
		if (UTF8Data[CharIdx] == '%')
		{
			int32 Value = 0;
			if (UTF8Data[CharIdx + 1] == 'u')
			{
				if (CharIdx + 6 <= Converter.Length())
				{
					// Treat all %uXXXX as code point
					Value = FParse::HexDigit(UTF8Data[CharIdx + 2]) << 12;
					Value += FParse::HexDigit(UTF8Data[CharIdx + 3]) << 8;
					Value += FParse::HexDigit(UTF8Data[CharIdx + 4]) << 4;
					Value += FParse::HexDigit(UTF8Data[CharIdx + 5]);
					CharIdx += 6;

					ANSICHAR Buffer[8] = { 0 };
					ANSICHAR* BufferPtr = Buffer;
					const int32 Len = UE_ARRAY_COUNT(Buffer);
					const int32 WrittenChars = FTCHARToUTF8_Convert::Utf8FromCodepoint(Value, BufferPtr, Len);

					Data.Append(Buffer, WrittenChars);
				}
				else
				{
					// Not enough in the buffer for valid decoding, skip it
					CharIdx++;
					continue;
				}
			}
			else if (CharIdx + 3 <= Converter.Length())
			{
				// Treat all %XX as straight byte
				Value = FParse::HexDigit(UTF8Data[CharIdx + 1]) << 4;
				Value += FParse::HexDigit(UTF8Data[CharIdx + 2]);
				CharIdx += 3;
				Data.Add((ANSICHAR)(Value));
			}
			else
			{
				// Not enough in the buffer for valid decoding, skip it
				CharIdx++;
				continue;
			}
		}
		else
		{
			// Non escaped characters
			Data.Add(UTF8Data[CharIdx]);
			CharIdx++;
		}
	}

	Data.Add('\0');
	return FString(UTF8_TO_TCHAR(Data.GetData()));
}


FString UStringConvertBPFLibrary::UnicodeEncode(const FString& UnencodedString , bool ConvertStandardChar)
{
	FTCHARToWChar Converter(*UnencodedString);    
	const WIDECHAR* UTF8Data = (WIDECHAR*)Converter.Get();    
	FString EncodedString = TEXT("");

	for (int32 ByteIdx = 0, Length = Converter.Length(); ByteIdx < Length; ++ByteIdx)
	{
		WIDECHAR ByteToEncode = UTF8Data[ByteIdx];
		FString TheString = FString::Printf(TEXT("%d"), ByteToEncode);
		int32 MyShinyNewInt = FCString::Atoi(*TheString);
		if ((MyShinyNewInt < 128)&&!ConvertStandardChar)
		{
			EncodedString += ByteToEncode;
		}
		else if (ByteToEncode != '\0')
		{
			EncodedString += TEXT("\\u");
			EncodedString += FString::Printf(TEXT("%.4x"), ByteToEncode);
		}
	}
	return EncodedString;
}

FString UStringConvertBPFLibrary::UnicodeDecode(const FString& EncodedString)
{
	FTCHARToWChar Converter(*EncodedString, EncodedString.Len());
	const WIDECHAR* UTF8Data = (WIDECHAR*)Converter.Get();

	TArray<ANSICHAR> Data;
	Data.Reserve(EncodedString.Len());

	for (int32 CharIdx = 0; CharIdx < Converter.Length();)
	{
		if (UTF8Data[CharIdx] == '\\' && UTF8Data[CharIdx + 1] == 'u')
		{
			int32 Value = 0;
			if (CharIdx + 6 <= Converter.Length())
			{
				// Treat all \uXXXX as code point
				Value = FParse::HexDigit(UTF8Data[CharIdx + 2]) << 12;
				Value += FParse::HexDigit(UTF8Data[CharIdx + 3]) << 8;
				Value += FParse::HexDigit(UTF8Data[CharIdx + 4]) << 4;
				Value += FParse::HexDigit(UTF8Data[CharIdx + 5]);
				CharIdx += 6;

				ANSICHAR Buffer[8] = { 0 };
				ANSICHAR* BufferPtr = Buffer;
				const int32 Len = UE_ARRAY_COUNT(Buffer);
				const int32 WrittenChars = FTCHARToUTF8_Convert::Utf8FromCodepoint(Value, BufferPtr, Len);

				Data.Append(Buffer, WrittenChars);
			}
			else
			{
				// Not enough in the buffer for valid decoding, skip it
				CharIdx++;
				continue;
			}
			
		}
		else
		{
			// Non escaped characters
			Data.Add(UTF8Data[CharIdx]);
			CharIdx++;
		}
	}

	Data.Add('\0');
	return FString(UTF8_TO_TCHAR(Data.GetData()));
}

bool UStringConvertBPFLibrary::StringToBytes(const FString& string, TArray<uint8>& Bytes)
{
	Bytes.Empty();
	if (string.IsEmpty())
	{
		return false;
	}
	FTCHARToUTF8 Converter(*string);   
	const UTF8CHAR* UTF8Data = (UTF8CHAR*)Converter.Get();   
	for (int32 ByteIdx = 0, Length = Converter.Length(); ByteIdx < Length; ++ByteIdx)
	{
		UTF8CHAR ByteToEncode = UTF8Data[ByteIdx];
	if (ByteToEncode != '\0')
		{
			Bytes.Add(ByteToEncode);
		}
	}
	return true;
}

bool UStringConvertBPFLibrary::StringFromBytes(const TArray<uint8>& Bytes, FString& string)
{
	string.Empty();
	if (!Bytes.Num())
	{
		return false;
	}
	FFileHelper::BufferToString(string,Bytes.GetData(),Bytes.Num());
	return true;
}

FString UStringConvertBPFLibrary::StringToBase64(const FString& Source)
{
	TArray<uint8> Bytes;
	StringToBytes(*Source,Bytes);
	return FBase64::Encode(Bytes);
}

bool UStringConvertBPFLibrary::StringFromBase64(const FString& Base64Str, FString& Result)
{
	TArray<uint8> bytes;
	if (!FBase64::Decode(Base64Str, bytes))
	{
		return false;
	}
	return StringFromBytes(bytes,Result);
}

TArray<int32> UStringConvertBPFLibrary::StringToAscii(const FString& Source)
{
	TArray<int32> AsciiArray;
	for (int i=0;i<Source.Len();i++)
	{
		AsciiArray.Add((int32)Source[i]);
	}
	return AsciiArray;
}

 bool UStringConvertBPFLibrary::StringFromAscii(const TArray<int32> AsciiArray,FString& Source)
{
	Source.Empty();
	for (auto Ascii : AsciiArray)
	{
		Source.AppendChar(Ascii);
	}
	return true;
}