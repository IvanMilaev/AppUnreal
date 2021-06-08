// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtilsFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class APP_API UUtilsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		UFUNCTION(BlueprintCallable)
		static TArray<uint8> ReadFile(const FString& filePath);

		UFUNCTION(BlueprintCallable)
		static void SendPngByHttpPostBinaryData(const TArray<uint8>& rawFileData, const FString& url);

		UFUNCTION(BlueprintCallable)
		static void SendPngByHttpPostBase64(const FString& base64Data, const FString& url, const FString& filename);

		UFUNCTION(BlueprintCallable)
		static const FString GetProjectSavedDir();

		UFUNCTION(BlueprintCallable)
		static const FString BinaryToBase64(const TArray<uint8>& RawFileData);

		UFUNCTION(BlueprintCallable)
		static const FString HttpRequestGetJsonAnswer(const FString& url);

	
};
