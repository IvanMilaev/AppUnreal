#include "UtilsNetwork.h"
#include "HotLoadAssetManager.h"


AUtilsNetwork::AUtilsNetwork()
{
}


AUtilsNetwork::~AUtilsNetwork()
{
}

bool AUtilsNetwork::Init(const FString& IN_Working_directory, const FString& IN_Python_Script_Filename, const FString& IN_IPAddress, const int IN_Port)
{
	working_directory = IN_Working_directory;
	python_script_filename = IN_Python_Script_Filename;
	server_address = IN_IPAddress;
	port = IN_Port;

	LaunchServer();
	CreateWorker();
	StartWorker();
}

bool AUtilsNetwork::LaunchServer()
{

	FString serverPath = "python.exe";
	FString attributes = python_script_filename + " " + server_address + " " + FString::FromInt(port);

	UE_LOG(LogTemp, Warning, TEXT("Working Directory: %s"), *working_directory);
	UE_LOG(LogTemp, Warning, TEXT("Launch server: %s"), *serverPath);

	server_handle = FPlatformProcess::CreateProc(*serverPath, *attributes, true, false, false, nullptr, 0, *working_directory, nullptr);

	if (server_handle.IsValid() && FPlatformProcess::IsProcRunning(server_handle))
	{
		UE_LOG(LogTemp, Warning, TEXT("Local Server Process started"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Local Server Process failed"));

	return server_handle.IsValid() && FPlatformProcess::IsProcRunning(server_handle);
}

FClientWorker* AUtilsNetwork::CreateWorker()
{
	worker = new FClientWorker(server_address, port);

	return worker;
}

bool AUtilsNetwork::LaunchNewJob(const FString& IN_JobDescription)
{
	worker->LaunchNewJob(IN_JobDescription);
	return true;
}

bool AUtilsNetwork::StartWorker()
{
	worker->Start();
	return true;
}

bool AUtilsNetwork::StopWorker()
{
	worker->ShutDown();
	return true;
}

bool AUtilsNetwork::StopServer()
{
	return true;
}



EHotLoadAssetManagerStatus AUtilsNetwork::getCurrentStatus()
{
	return EHotLoadAssetManagerStatus::ClientConnected;
}
