#include "HotLoadBPLibraryUtils.h"
#include "IPlatformFilePak.h"
#include "Engine/StreamableManager.h"
#include "AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/StaticMesh.h"



UHotLoadBPLibraryUtils::UHotLoadBPLibraryUtils(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

bool UHotLoadBPLibraryUtils::DirectoryExists(const FString & InPath)
{
	return FPaths::DirectoryExists(InPath);
}

bool UHotLoadBPLibraryUtils::FileExists(const FString & InPath)
{
	return FPaths::FileExists(InPath);
}

void UHotLoadBPLibraryUtils::GetDirectoryOfFile(const FString & InPath, FString& DirectoryPath)
{
	DirectoryPath = FPaths::GetPath(InPath);
}

void UHotLoadBPLibraryUtils::ListFilesInFolder(const FString& InPath, const bool bCheckExtention, const TArray<FString>& Extensions, TArray<FString>& Files)
{
	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> TempFiles;
	FString DirectoryPath = InPath + "\\*.*";
	FileManager.FindFiles(TempFiles, *DirectoryPath, true, false);
	if (bCheckExtention && Extensions.Num()>0)
	{
		for (TIndexedContainerIterator<TArray<FString>, FString, int32> it = TempFiles.CreateIterator(); it; it++)
		{
			if (MatchesExtension(*it, Extensions))
				Files.Add(*it);
		}
	}
	else
		Files = TempFiles;
	Files.Sort();
}

void UHotLoadBPLibraryUtils::ListDirectoriesInFolder(const FString& InPath, TArray<FString>& Directories)
{
	IFileManager& FileManager = IFileManager::Get();
	FString DirectoryPath = InPath + "\\*.*";
	FileManager.FindFiles(Directories, *DirectoryPath, false, true);
	Directories.Sort();
}

bool UHotLoadBPLibraryUtils::MatchesExtension(const FString& FileName, const TArray<FString>& Extensions)
{
	//if no extension provided in filter return true
	if (Extensions.Num() == 0)
		return true;

	FString FileExtention = FPaths::GetExtension(FileName);
	return Extensions.Contains(FileExtention);
}

void UHotLoadBPLibraryUtils::NormalizeAndCleanFileName(const FString & InPath, FString& OutPath)
{
	OutPath = InPath;
	FPaths::RemoveDuplicateSlashes(OutPath);
	FPaths::NormalizeFilename(OutPath);
}

void UHotLoadBPLibraryUtils::NormalizeAndCleanDirectoryName(const FString & InPath, FString& OutPath)
{
	OutPath = InPath;
	FPaths::RemoveDuplicateSlashes(OutPath);
	FPaths::NormalizeDirectoryName(OutPath);
}


bool UHotLoadBPLibraryUtils::Test(const FString & InPath)
{
	return true;
}

bool UHotLoadBPLibraryUtils::RegisterMountPoint(const FString & GamePath, const FString & InPath)
{
	FPackageName::RegisterMountPoint(GamePath, InPath);
/*
#if WITH_EDITOR
#else
	FAssetRegistryModule& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& assetRegistry = assetRegistryModule.Get();
	assetRegistry.ScanPathsSynchronous({ GamePath }, true);
	UE_LOG(LogTemp, Warning, TEXT("#### Force scan paths"));
#endif
*/
	return true;
}


bool UHotLoadBPLibraryUtils::RegisterMountPointv2(const FString & GamePath, const FString & InPath)
{
	FString GamePath2 = "/Game/TestFranck/";
	FPackageName::RegisterMountPoint(GamePath2, InPath);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	auto& AssetRegistry = AssetRegistryModule.Get();
	TArray<FString> ToScan;
	ToScan.Add(GamePath2);
	AssetRegistry.ScanPathsSynchronous(ToScan, true);
	UE_LOG(LogTemp, Log, TEXT("scanning %s"), *GamePath2);
	TArray<FAssetData> Datas;
	AssetRegistry.GetAssetsByPath(FName(*GamePath2), Datas, true, false);
	UE_LOG(LogTemp, Log, TEXT("there is %i asset in %s"), Datas.Num(), *GamePath2);
	for (auto Data : Datas)
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *Data.PackageName.ToString());
	}
	return true;
}

bool UHotLoadBPLibraryUtils::MountExternalPak(const FString & PakPath, const FString & ContentFolderMountPoint)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FPakPlatformFile* PakPlatformFile = nullptr;
	if (PlatformFile.GetName() == FPakPlatformFile::GetTypeName())
	{
		PakPlatformFile = static_cast<FPakPlatformFile*>(&PlatformFile);
	}
	else
	{
		PakPlatformFile = new FPakPlatformFile();
		PakPlatformFile->Initialize(&PlatformFile, TEXT(""));
		FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);
	}

	FString pakMountPoint = "../../../tempPakMountPoint/";

	if (PakPlatformFile->Mount(*PakPath, 0, *pakMountPoint))
	{
		//MountedHashFileNameMap.Add(Info.Hash, PakFilename);
		//UE_LOG(LogTemp, Log, TEXT("mounting %s at %s"), *PakPath, *MountPoint);
		FPackageName::RegisterMountPoint(*ContentFolderMountPoint, *pakMountPoint);
		UE_LOG(LogTemp, Log, TEXT("registering mount point %s at %s"), *pakMountPoint, *ContentFolderMountPoint);
		TArray<FString> FileList;
		PakPlatformFile->FindFilesRecursively(FileList, *pakMountPoint, TEXT("uasset"));
		FStreamableManager& AssetLoad = UAssetManager::GetStreamableManager();
		FString AssetName = FileList[3];
		FString AssetShortName = FPackageName::GetShortName(AssetName);
		FString LeftStr;
		FString RightStr;
		AssetShortName.Split(TEXT("."), &LeftStr, &RightStr);
		AssetName = TEXT("/Game/") + LeftStr + TEXT(".") + LeftStr;
		FStringAssetReference Reference = AssetName;

		UObject* LoadObject = AssetLoad.LoadSynchronous(Reference);
		if (LoadObject != nullptr)
		{
			UE_LOG(LogClass, Log, TEXT("Object load Success... $s"), *LoadObject->GetDetailedInfo());
		}
		else
		{
			UE_LOG(LogClass, Error, TEXT("Can't load asset..."));
		}
		for (auto File : FileList)
		{
			UE_LOG(LogTemp, Log, TEXT("find file  %s"), *File);
		}
		PakPlatformFile->FindFilesRecursively(FileList, *pakMountPoint, TEXT("umap"));
		for (auto File : FileList)
		{
			UE_LOG(LogTemp, Log, TEXT("find file  %s"), *File);
		}
		/*FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		auto& AssetRegistry = AssetRegistryModule.Get();
		TArray<FString> ToScan;
		ToScan.Add(TmpRoot);
		AssetRegistry.ScanPathsSynchronous(ToScan, true);
		UE_LOG(LogTemp, Log, TEXT("scanning %s"), *TmpRoot);
		TArray<FAssetData> Datas;
		AssetRegistry.GetAssetsByPath(FName(*TmpRoot), Datas, true, false);
		UE_LOG(LogTemp, Log, TEXT("there is %i asset in %s"), Datas.Num(), *TmpRoot);
		for (auto Data : Datas)
		{
			UE_LOG(LogTemp, Log, TEXT("%s"), *Data.PackageName.ToString());
		}*/
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Mount failed: %s"), *PakPath);
	}
	return true;
}



void UHotLoadBPLibraryUtils::AsyncSpawnActor(UObject* WorldContextObject, TAssetSubclassOf<AActor> AssetPtr, FTransform SpawnTransform)
{
	//FStreamableManager& Streamable = UGameGlobals::Get().StreamableManager;

	FSoftObjectPath pathobject;
	pathobject = FSoftObjectPath(TEXT("'/Game/Hotloadimport/SM_Totinos.SM_Totinos'"));
	FStreamableManager& AssetLoad = UAssetManager::GetStreamableManager();
	AssetLoad.RequestAsyncLoad(pathobject, FStreamableDelegate::CreateStatic(&UHotLoadBPLibraryUtils::OnAsyncSpawnActorComplete, WorldContextObject, SpawnTransform));


}



void UHotLoadBPLibraryUtils::OnAsyncSpawnActorComplete(UObject* WorldContextObject, FTransform SpawnTransform)
{
	UE_LOG(LogTemp, Error, TEXT("Loaded: "));
}


void UHotLoadBPLibraryUtils::AsyncAssetLoadTest()
{

	
	UObjectLibrary* ObjectLibrary;

	ObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
	ObjectLibrary->AddToRoot();
	FString path = TEXT("/Game/Hotloadimport");
	ObjectLibrary->LoadAssetDataFromPath(path);
	
	ObjectLibrary->LoadAssetsFromAssetData();
	TArray<FAssetData> Datas;
	ObjectLibrary->GetAssetDataList(Datas);
	
	UE_LOG(LogTemp, Log, TEXT("there is %i asset in %s"), Datas.Num(), *path);
	for (auto Data : Datas)
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *Data.PackageName.ToString());
	}
}


bool UHotLoadBPLibraryUtils::IsWithEditor()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}