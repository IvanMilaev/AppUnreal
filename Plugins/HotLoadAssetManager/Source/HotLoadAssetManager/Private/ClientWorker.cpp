#include "ClientWorker.h"


FClientWorker::FClientWorker(const FString& IN_IPAdress, const int IN_Port)
{
	server_adress = IN_IPAdress;
	port = IN_Port;
	bFinished = true;
	Thread = NULL;
	listenSocket = nullptr;
	step = 0;

	cmd_server_confirmation = FString("ACK");
	cmd_cli_prepare_for_job = FString("PREPARE_TO_JOB");
	cmd_cli_start_job = FString("START_JOB");
	cmd_cli_cancel_job = FString("CANCEL_JOB");
	cmd_cli_waiting_for_statuses = FString("WAITING_FOR_JOB_PROCESSING_STATUS");
	cmd_cli_job_completed = FString("JOB_COMPLETED");
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

bool FClientWorker::LaunchNewJob(EJobType& IN_JobType, const FString& IN_JobDescription)
{
	job_type = IN_JobType;
	job_description = IN_JobDescription;
	ParseJobDescriptionToJSON();
	step = 0;
	Run();
	

}

void FClientWorker::ProcessJob(int IN_Step)
{
	switch (job_type)
	{
		case EJobType::ASSET_HOT_LOAD:
		{
			AssetHotLoadProcess(IN_Step);
		}
	}
}

void FClientWorker::AssetHotLoadProcess(int IN_Step)
{
	switch (IN_Step)
	{
		case 0:
		{
			ConnectServer();
			FPlatformProcess::Sleep(0.1);
			break;
		}
		case 1:
		{
			SendJobToServer();
			FPlatformProcess::Sleep(0.05);
			break;
		}
		case 2:
		{
			WaitStatus();
			FPlatformProcess::Sleep(0.05);
			break;
		}
		case 3:
		{
			FinishJob();
			FPlatformProcess::Sleep(0.05);
			break;
		}
		case 4:
		{
			DisconnectFromServer();
			FPlatformProcess::Sleep(0.1);
			bFinished = true;
			break;
		}
	}
}

void 

void FClientWorker::DisconnectFromServer()
{
	listenSocket = nullptr;
	delete listenSocket;
	step++;
}

void FClientWorker::WaitStatus()
{
	UE_LOG(LogTemp, Warning, TEXT("Wait for status job"));
	SendDataToServer(cmd_cli_waiting_for_statuses);

	UE_LOG(LogTemp, Warning, TEXT("Wait for status COMPLETE JOB"));
	WaitForMessage(cmd_cli_job_completed);
	step++;
}

bool FClientWorker::ConnectServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Connect server:"));
	if (!isSocketWorks()) return;

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
	SendDataToServer(cmd_cli_prepare_for_job + job_description);
	UE_LOG(LogTemp, Warning, TEXT("Send job to server %s : %d, job: %f"), *server_adress, port, *job_description);
	UE_LOG(LogTemp, Warning, TEXT("Waiting for server prepareness confirmation..."));
	if (WaitForMessage(cmd_server_confirmation))
	{
		SendDataToServer(cmd_cli_start_job);
		UE_LOG(LogTemp, Warning, TEXT("Waiting for server starts job confirmation..."));
		if (WaitForMessage(cmd_server_confirmation))
		{
			StatusUpdateEvent.Broadcast(EWorkerStatus::JobStarted);
			step++;
		}
	}
}

bool FClientWorker::SendDataToServer(FString IN_Data)
{
	TCHAR* serializedChar = IN_Data.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent = 0;
	return listenSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
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
		ProcessJob(step);
	}


	return 0;
}

bool FClientWorker::Init()
{
	return true;
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

bool FClientWorker::WaitForMessage(FString msg)
{
	if (!isSocketWorks()) return false;

	uint32 Size = 0;
	while (StopTaskCounter.GetValue() == 0 && !listenSocket->HasPendingData(Size))
	{
		FPlatformProcess::Sleep(0.01);
	}

	//process stopping
	if (StopTaskCounter.GetValue() != 0)
		return false;

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

bool FClientWorker::isSocketWorks()
{
	if (listenSocket == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket not initialized"));
		return false;
	}

	if (listenSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
	{
		UE_LOG(LogTemp, Warning, TEXT("Client already connected"));
		return true;
	}
	else {
		return false;
	}
}