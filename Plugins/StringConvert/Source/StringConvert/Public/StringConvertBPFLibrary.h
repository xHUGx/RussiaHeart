// Copyright 2020-2022 Goodriver2. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StringConvertBPFLibrary.generated.h"

/**
 * 
 */
UCLASS()
class STRINGCONVERT_API UStringConvertBPFLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	//Convert FString to Url
	static bool IsAllowedChar(UTF8CHAR LookupChar);
	UFUNCTION(BlueprintCallable,BlueprintPure,Category="StringConvert")
	static FString UrlEncode(const FString &UnencodedString);
	//Convert FString from Url
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StringConvert")
	static FString UrlDecode(const FString &EncodedString);
	
	//Convert FString to Unicode
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StringConvert")
	static FString UnicodeEncode(const FString& UnencodedString,bool ConvertStandardChar);
	//Convert FString from Unicode
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StringConvert")
	static FString UnicodeDecode(const FString& EncodedString);
	
	//Convert FString to Bytes
	UFUNCTION(BlueprintPure,BlueprintPure,Category="StringConvert")
	static bool StringToBytes(const FString& string,TArray<uint8>& Bytes);
	//Convert FString from Bytes
	UFUNCTION(BlueprintPure,BlueprintPure,Category="StringConvert")
	static bool StringFromBytes(const TArray<uint8>& Bytes,FString& string);

	//Convert FString to Base64
	UFUNCTION(BlueprintPure,BlueprintPure,Category="StringConvert")
	static FString StringToBase64(const FString& Source);
	//Convert FString from Base64
	UFUNCTION(BlueprintPure,BlueprintPure,Category="StringConvert")
	static bool StringFromBase64(const FString& Base64Str, FString& Result);

	//Convert FString to Base64
	UFUNCTION(BlueprintPure,BlueprintPure,Category="StringConvert")
	static TArray<int32> StringToAscii(const FString& Source);
	UFUNCTION(BlueprintPure,BlueprintPure,Category="StringConvert")
	static  bool StringFromAscii(const TArray<int32> AsciiArray,FString& Source);
};
