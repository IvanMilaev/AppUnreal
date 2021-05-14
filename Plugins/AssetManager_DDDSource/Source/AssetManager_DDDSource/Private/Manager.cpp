#include "Manager.h"

#include "AssetManager_DDDSource.h"


//AManager::AManager(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
//{
//}

AManager::AManager()
{}

AManager::~AManager()
{}

bool AManager::Init()
{
	return true;
}

bool AManager::DoJob(FString IN_Job)
{
	UE_LOG(LogTemp, Display, TEXT("Launch worker for job %s"), *IN_Job);
	FAssetPrepWorker* thread = FAssetPrepWorker::RunJob(IN_Job);
	thread->StatusUpdateEvent.Clear();
	thread->StatusUpdateEvent.AddUObject(this, &AManager::UpdateStatus);
	UE_LOG(LogTemp, Display, TEXT("Thread..."));
	return true;
}

bool AManager::Test()
{
	/*UE_LOG(LogTemp, Display, TEXT("Launch test worker"));
	FAssetPrepWorker* thread = FAssetPrepWorker::RunJob();
	thread->StatusUpdateEvent.Clear();
	thread->StatusUpdateEvent.AddUObject(this, &AManager::UpdateStatus);
	UE_LOG(LogTemp, Display, TEXT("Thread..."));*/

	return true;
}

bool AManager::SendMessageToServer(FString message)
{
	FSocket* listenSocket;
	const int32 BufferSize = 16 * 1024 * 1024;
	listenSocket = FTcpSocketBuilder("Tcp Receiver Actor")
		.AsBlocking()
		.AsReusable()
		.WithReceiveBufferSize(BufferSize);

	FString server_address = TEXT("127.0.0.1");
	int port = 10000;
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
			
			FString cmd = TEXT("ECHO");

			FString payload = cmd+"|"+message;

			TCHAR* serializedChar = (payload).GetCharArray().GetData();

			int32 size = FCString::Strlen(serializedChar);
			int32 sent = 0;
			return listenSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot connect to server"));

		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Connect Fail"));
	}
	return true;
}

EAssetPrepWorkerStatus AManager::getCurrentStatus()
{
	return CurrentStatus;
}

void AManager::UpdateStatus(EAssetPrepWorkerStatus newStatus)
{
	CurrentStatus = newStatus;
}



