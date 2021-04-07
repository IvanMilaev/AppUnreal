#include "ClientWorker.h"


FClientWorker::FClientWorker(const FString& IN_IPAdress, const int IN_Port)
{
	server_adress = IN_IPAdress;
	port = IN_Port;
	bFinished = true;
	Thread = NULL;
	listenSocket = nullptr;
	step = 0;
}


void FClientWorker::Start()
{
	const int32 BufferSize = 16 * 1024 * 1024;

	StopTaskCounter.Reset();

	if (listenSocket == nullptr)
	{
		listenSocket = FTcpSocketBuilder("Tcp Receiver Actor")
			.AsBlocking()
			.AsReusable()
			.WithReceiveBufferSize(BufferSize);
	}

	Thread = FRunnableThread::Create(this, TEXT("FClientWorker"), 0, TPri_BelowNormal);
}

bool FClientWorker::LaunchNewJob(const FString& IN_JobDescription)
{
	job_description = IN_JobDescription;
	step = 1;
	Run();

}

uint32 FClientWorker::Run()
{
	StatusUpdateEvent.Broadcast(EWorkerStatus::ThreadStarted);

	bFinished = false;

	//Initial wait before starting 
	FPlatformProcess::Sleep(0.03);

	//While not told to stop this thread 
	//and not yet finished finding Prime Numbers 
	while (StopTaskCounter.GetValue() == 0 && !IsFinished())
	{
		switch (step)
		{
		case 0:
		{
			
			break;
		}
		case 1:
		{
			ConnectServer();
			FPlatformProcess::Sleep(0.1);
			break;
		}
		case 2:
		{
			SendJobToServer();
			FPlatformProcess::Sleep(0.05);
			break;
		}
		
		default:
		{
			FPlatformProcess::Sleep(0.1);
			break;
		}
		}
	}


	return 0;
}

bool FClientWorker::Init()
{
	return true;
}

bool FClientWorker::ConnectServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Connect server:"));
	if (listenSocket == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket not initialized"));
		return;
	}

	if (listenSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
	{
		UE_LOG(LogTemp, Warning, TEXT("Client already connected"));
		return true;
	}


	FIPv4Address ip(127, 0, 0, 1);
	FIPv4Address::Parse(server_adress, ip);
	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(port);

	

	if (listenSocket->Connect(*addr))
	{
		if (listenSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
		{
			UE_LOG(LogTemp, Warning, TEXT("Connected to server"));
			StatusUpdateEvent.Broadcast(EWorkerStatus::ClientConnected);
			step++;
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

void FClientWorker::SendJobToServer()
{

}






bool FClientWorker::ShutDown()
{
	return true;
}


bool FClientWorker::IsFinished() const
{
	return bFinished;
}

bool FClientWorker::ParseJobDescriptionToJSON()
{
	json_job_description = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(job_description);
	return FJsonSerializer::Deserialize(JsonReader, json_job_description);
	
}