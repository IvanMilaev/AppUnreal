#pragma once
#include "AssetPrepWorker.h"

FAssetPrepWorker* FAssetPrepWorker::Runnable = NULL;
FAssetPrepWorker::FAssetPrepWorker(const FString& IN_IPAdress, const int IN_Port)
{
	UE_LOG(LogTemp, Display, TEXT("Creating new worker"));
	server_address = IN_IPAdress;
	port = IN_Port;
	working_directory = FPaths::ProjectUserDir() + "Script/AssetsManagement";
	python_script_filename = "server.py";
	bFinished = false;
	Thread = NULL;
	listenSocket = nullptr;
	step = 0;
}

FAssetPrepWorker:: ~FAssetPrepWorker()
{
	delete Thread;
	Thread = NULL;

	if (listenSocket != nullptr)
	{
		UE_LOG(LogTemp, Display, TEXT("Deleting socket"));
		listenSocket->Close();
		//ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
		delete listenSocket;
		listenSocket = nullptr;
	}
}

bool FAssetPrepWorker::Init()
{
	return true;
}

void FAssetPrepWorker::Start()
{
	UE_LOG(LogTemp, Display, TEXT("Starting worker"));
	if (!bFinished && Thread != NULL)
		EnsureCompletion();
	StopTaskCounter.Reset();

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



uint32 FAssetPrepWorker::Run()
{
	step = 0;
	StatusUpdateEvent.Broadcast(EAssetPrepWorkerStatus::ThreadStarted);
	//While not told to stop this thread 
	//and not yet finished finding Prime Numbers 
	while (StopTaskCounter.GetValue() == 0 && !IsFinished())
	{
		switch (step)
		{

		
		case 0:
		{
			LaunchServer();
			FPlatformProcess::Sleep(0.5);
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
			SendJobServer();
			FPlatformProcess::Sleep(0.05);
			break;
		}
		case 3:
		{
			WaitStatus();
			FPlatformProcess::Sleep(0.05);
			break;
		}
		case 4:
		{
			KillServer();
			FPlatformProcess::Sleep(0.1);
			bFinished = true;
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

void FAssetPrepWorker::Stop()
{
	StopTaskCounter.Increment();
}

void FAssetPrepWorker::LaunchServer()
{
	FString serverPath = "python.exe";
	FString attributes = python_script_filename + " " + server_address + " " + FString::FromInt(port);

	UE_LOG(LogTemp, Warning, TEXT("Working Directory: %s"), *working_directory);
	UE_LOG(LogTemp, Warning, TEXT("Launch server: %s"), *serverPath);
	UE_LOG(LogTemp, Warning, TEXT("cmd: %s"), *attributes);

	server_handle = FPlatformProcess::CreateProc(*serverPath, *attributes, true, false, false, nullptr, 0, *working_directory, nullptr);

	if (server_handle.IsValid() && FPlatformProcess::IsProcRunning(server_handle))
	{
		UE_LOG(LogTemp, Warning, TEXT("Local Server Process started"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Local Server Process failed"));

	step++;
}

void FAssetPrepWorker::ConnectServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Connect server:"));
	if (listenSocket == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket not initialized"));
		return;
	}

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
			StatusUpdateEvent.Broadcast(EAssetPrepWorkerStatus::ConnectedToServer);
			step++;
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
	
}

void FAssetPrepWorker::SendJobServer()
{
	SendCommandToServer(EAssetPrepWorkerCommand::SEND_JOB_TO_SERVER);
	UE_LOG(LogTemp, Warning, TEXT("Wait for confirm"));
	if (WaitForMessage(EAssetPrepWorkerCommand::CONFIRMED))
	{
		StatusUpdateEvent.Broadcast(EAssetPrepWorkerStatus::JobStarted);
		step++;
	}
}

void FAssetPrepWorker::KillServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Killing server"));
	SendCommandToServer(EAssetPrepWorkerCommand::KILL_SERVER);
	WaitForMessage(EAssetPrepWorkerCommand::KILL_SERVER);
}
void FAssetPrepWorker::WaitStatus()
{
	SendCommandToServer(EAssetPrepWorkerCommand::WAITING_FOR_STATUSES);
	UE_LOG(LogTemp, Warning, TEXT("Wait for Finish Assetprep"));
	WaitForMessage(EAssetPrepWorkerCommand::JOB_COMPLETED);
	StatusUpdateEvent.Broadcast(EAssetPrepWorkerStatus::JobCompleted);
	step++;
}


FAssetPrepWorker* FAssetPrepWorker::RunJob()
{
	UE_LOG(LogTemp, Warning, TEXT("Lunch test in worker"));
	FString address = FString("127.0.0.1");
	int prt = 10000;
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
		Runnable = new FAssetPrepWorker( address, prt);
	Runnable->Start();
	return Runnable;
}


void FAssetPrepWorker::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
	if (server_handle.IsValid() && FPlatformProcess::IsProcRunning(server_handle))
	{
		UE_LOG(LogTemp, Warning, TEXT("Close Proc"));
		FPlatformProcess::TerminateProc(server_handle);
	}
}

bool FAssetPrepWorker::SendCommandToServer(EAssetPrepWorkerCommand command)
{
	FString cmd = UEnum::GetValueAsString<EAssetPrepWorkerCommand>(command);
	
	TCHAR* serializedChar = cmd.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent = 0;
	return listenSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
}

bool FAssetPrepWorker::WaitForMessage(EAssetPrepWorkerCommand cmd)
{
	if (!IsSocketWorks()) return false;

	FString msg = UEnum::GetValueAsString<EAssetPrepWorkerCommand>(cmd);

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

bool FAssetPrepWorker::IsSocketWorks()
{
	if (listenSocket == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket not initialized"));
		return false;
	}

	if (listenSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Client already connected"));
		return true;
	}
	else {
		return false;
	}
}

bool FAssetPrepWorker::IsThreadFinished()
{
	if (Runnable)
		return Runnable->IsFinished();
	return true;
}

bool FAssetPrepWorker::IsFinished() const
{
	return bFinished;
}



