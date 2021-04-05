// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "HotLoadBPLibraryUtils.generated.h"

//wrappers around directory and files utility functions
UCLASS()
class UHotLoadBPLibraryUtils : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static bool DirectoryExists(const FString & InPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static bool FileExists(const FString & InPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static void GetDirectoryOfFile(const FString & InPath, FString& DirectoryPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static void ListFilesInFolder(const FString& InPath, const bool bCheckExtention, const TArray<FString>& Extensions, TArray<FString>& Files);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static void ListDirectoriesInFolder(const FString& InPath, TArray<FString>& Directories);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static void NormalizeAndCleanFileName(const FString & InPath, FString& OutPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static void NormalizeAndCleanDirectoryName(const FString & InPath, FString& OutPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static bool RegisterMountPoint(const FString & GamePath, const FString & InPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static bool RegisterMountPointv2(const FString & GamePath, const FString & InPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
		static bool MountExternalPak(const FString & PakPath, const FString & ContentFolderMountPoint);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static bool Test(const FString & InPath);

	UFUNCTION(BlueprintPure, Category = "Utilities")
	static bool IsWithEditor();

	private:
		static bool MatchesExtension(const FString& FileName, const TArray<FString>& Extensions);
	
};
