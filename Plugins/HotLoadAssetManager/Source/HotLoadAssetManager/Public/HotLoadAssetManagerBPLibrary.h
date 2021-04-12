// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Dom/JsonObject.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HotLoadAssetManagerBPLibrary.generated.h"

/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/
UCLASS()
class UHotLoadAssetManagerBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static bool DirectoryExists(const FString& InPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static bool FileExists(const FString& InPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static void GetDirectoryOfFile(const FString& InPath, FString& DirectoryPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static void ListFilesInFolder(const FString& InPath, const bool bCheckExtention, const TArray<FString>& Extensions, TArray<FString>& Files);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static void ListDirectoriesInFolder(const FString& InPath, TArray<FString>& Directories);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static void NormalizeAndCleanFileName(const FString& InPath, FString& OutPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static void NormalizeAndCleanDirectoryName(const FString& InPath, FString& OutPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static bool RegisterMountPoint(const FString& GamePath, const FString& InPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static bool RegisterMountPointv2(const FString& GamePath, const FString& InPath);

	UFUNCTION(BlueprintPure, Category = "File Helpers Wrapper")
	static bool MountExternalPak(const FString& PakPath, const FString& ContentFolderMountPoint);

	UFUNCTION(BlueprintPure, Category = "Utilities")
	static bool IsWithEditor();


	

private:
	static bool MatchesExtension(const FString& FileName, const TArray<FString>& Extensions);

};
