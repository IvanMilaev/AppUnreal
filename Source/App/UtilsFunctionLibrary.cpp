// Fill out your copyright notice in the Description page of Project Settings.


#include "UtilsFunctionLibrary.h"
#include "Misc/FileHelper.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "Misc/Paths.h"
#include "Misc/Base64.h"
#include "Dom/JsonObject.h"
#include "Templates/SharedPointer.h"

TArray<uint8> UUtilsFunctionLibrary::ReadFile(const FString& filePath)
{


	//UE_LOG(LogTemp, Warning, TEXT("Project saved dir is %s"), *directory);

	TArray<uint8> RawFileData;
	FFileHelper::LoadFileToArray(RawFileData, *filePath);
	//UE_LOG(LogTemp, Warning, TEXT("Read File"));
	return RawFileData;
}

void UUtilsFunctionLibrary::SendPngByHttpPostBinaryData(const TArray<uint8>& rawFileData, const FString& url)
{
	//FHttpModule * http = &FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "image/png");
	HttpRequest->SetContent(rawFileData);
	HttpRequest->SetURL(url);
	HttpRequest->ProcessRequest();
}

void UUtilsFunctionLibrary::SendPngByHttpPostBase64(const FString& base64Data, const FString& url, const FString& filename)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("data"), base64Data);
	JsonObject->SetStringField(TEXT("name"), filename);

	FString OutputString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->SetURL(url);
	//HttpRequest->OnProcessRequestComplete().BindUObject(this, &USource3DFuncLib::OnTokenRequestCompleted);
	HttpRequest->ProcessRequest();
}

const FString UUtilsFunctionLibrary::GetProjectSavedDir()
{
	const FString directory = FPaths::ProjectSavedDir();
	return directory;
}

const FString UUtilsFunctionLibrary::BinaryToBase64(const TArray<uint8>& RawFileData)
{
	const FString base64 = FBase64::Encode(RawFileData);
	return base64;
}