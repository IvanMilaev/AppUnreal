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
	cmd_cli_stop_server = FString("STOP_SERVER");
	cmd_ack = FString("ACK");

	LaunchServer();
	CreateWorker();
	StartWorker();
	return true;
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

bool AUtilsNetwork::LaunchSocketConnectionWithServer()
{
	const int32 BufferSize = 16 * 1024 * 1024;
	listenSocket = FTcpSocketBuilder("Tcp Receiver Actor")
		.AsBlocking()
		.AsReusable()
		.WithReceiveBufferSize(BufferSize);


	FIPv4Address ip(127, 0, 0, 1);
	FIPv4Address::Parse(server_address, ip);
	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(port);



	if (listenSocket->Connect(*addr))
	{
		if (listenSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
		{
			UE_LOG(LogTemp, Warning, TEXT("Connected to server"));
			StatusUpdateEvent.Broadcast(EWorkerStatus::NetworkUtilsClientConnected);

			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot connect to server"));
			return false;
		}

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Connect Fail"));
		return false;
	}
	
}

void AUtilsNetwork::CreateWorker()
{
	worker = new FClientWorker(server_address, port);

}

bool AUtilsNetwork::LaunchNewJob(EJobType IN_JobType, const FString& IN_JobDescription)
{
	worker->LaunchNewJob(IN_JobType, IN_JobDescription);
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
	FString serialized = cmd_cli_stop_server;
	TCHAR* serializedChar = serialized.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent = 0;
	bool successful1 = listenSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	return WaitForMessage(cmd_ack);
}

bool AUtilsNetwork::WaitForMessage(FString msg)
{
	if (listenSocket == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket not initialized"));
		return false;
	}

	uint32 Size = 0;
	while ( !listenSocket->HasPendingData(Size))
	{
		FPlatformProcess::Sleep(0.01);
	}


	uint8* Buffer = new uint8[Size + 1];
	int32 Read = 0;

	if (listenSocket->Recv(Buffer, Size, Read))
	{
		Buffer[Read] = 0;
		const FString StringData = UTF8_TO_TCHAR((char*)Buffer);
		if (!StringData.Equals(msg))
		{
			UE_LOG(LogTemp, Warning, TEXT("Unexpected reply from server %s"), *StringData);
			return false;
		}
		UE_LOG(LogTemp, Warning, TEXT("Server acknowledge"));
		return true;
	}
	delete[] Buffer;
	return false;
}

void AUtilsNetwork::UpdateStatus(EWorkerStatus newStatus)
{
	UE_LOG(LogTemp, Warning, TEXT("Update status"));
}

EWorkerStatus AUtilsNetwork::getCurrentStatus()
{
	return EWorkerStatus::ClientConnected;
}
