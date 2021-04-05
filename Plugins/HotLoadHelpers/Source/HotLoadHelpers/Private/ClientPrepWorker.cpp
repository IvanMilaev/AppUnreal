#include "ClientPrepWorker.h"


//messages exchanged between client and server
/*
>>> CONFIG DATAPREP
<<< ACK
>>> CONTENT
<<< ACK
>>> START DATAPREP
<<< ACK
>>> WAIT FOR STATUSES
<<< FINISH DATAPREP
>>> ACK
<<< FINISH COOK
>>> ACK
<<< FINISH COPY
>>> ACK
<<< WAITING COMMANDS
>>> STOP SERVER
*/

FClientPrepWorker* FClientPrepWorker::Runnable = NULL;

FClientPrepWorker::FClientPrepWorker(const FString & IN_JobDescription, const FString& IN_IPAdress, const int IN_Port, const bool IN_SpawnServer)
{
	server_adress = IN_IPAdress;
	job_description = IN_JobDescription;
	port = IN_Port;
	bSpawnServer = IN_SpawnServer;

	cmd_cli_config_dataprep = FString("CONFIG DATAPREP");
	cmd_cli_start_dataprep = FString("START DATAPREP");
	cmd_cli_waiting_for_statuses = FString("WAIT FOR STATUSES'");
	cmd_cli_stop_server = FString("STOP SERVER");
	cmd_ser_finish_dataprep = FString("FINISH DATAPREP");
	cmd_ser_finish_cook = FString("FINISH COOK");
	cmd_ser_finish_copy = FString("FINISH COPY");
	cmd_ser_waiting_commands = FString("WAITING COMMANDS");
	cmd_ack = FString("ACK");

	project_name_str = FString("Project Name");
	output_folder_str = FString("Output Folder");
	cad_files_path_str = FString("CAD Files Path");
	content_folder_str = FString("Content Folder");
	tesselation_parameters_str = FString("Tesselation Parameters");
	
	bFinished = true;
	Thread = NULL;
	listenSocket = nullptr;
	step = 0;
}

FClientPrepWorker::~FClientPrepWorker()
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

void FClientPrepWorker::Start()
{
	if (!bFinished && Thread != NULL)
		EnsureCompletion();
	StopTaskCounter.Reset();

	//create or re-create socket
	const int32 BufferSize = 16 * 1024 * 1024;
	if( listenSocket == nullptr)
	{
		listenSocket = FTcpSocketBuilder("Tcp Receiver Actor")
			//.AsNonBlocking()
			.AsBlocking()
			.AsReusable()
			.WithReceiveBufferSize(BufferSize);
	}

	Thread = FRunnableThread::Create(this, TEXT("FClientPrepWorker"), 0, TPri_BelowNormal);
}

//Init 
bool FClientPrepWorker::Init() 
{
	//Init the Data 
	return true;
}

//Run 
uint32 FClientPrepWorker::Run()
{
	//status 0 : staring thread
	StatusUpdateEvent.Broadcast(EDataPrepStatus::ThreadStarted);
	if (bSpawnServer)
		step = 0;
	else
		step = 1;
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
	//Run FPrimeNumberWorker::Shutdown() from the timer in Game Thread that is watching
	//to see when FPrimeNumberWorker::IsThreadFinished()
	if (listenSocket != nullptr)
	{
		listenSocket->Close();
		delete listenSocket;
		listenSocket = nullptr;
	}
	UE_LOG(LogTemp, Warning, TEXT("Data Prep thread stop"));
	return 0;
}

//stop 
void FClientPrepWorker::Stop()
{
	StopTaskCounter.Increment();
}

FClientPrepWorker* FClientPrepWorker::LaunchPrep(const FString & IN_JobDescription, const FString& IN_IPAdress, const int IN_Port, const bool IN_SpawnServer)
{
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
		Runnable = new FClientPrepWorker(IN_JobDescription, IN_IPAdress, IN_Port, IN_SpawnServer);
	Runnable->Start();
	return Runnable;
}

void FClientPrepWorker::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
	if (bSpawnServer && server_handle.IsValid() && FPlatformProcess::IsProcRunning(server_handle))
	{
		UE_LOG(LogTemp, Warning, TEXT("Close Proc"));
		FPlatformProcess::TerminateProc(server_handle);
	}
}

void FClientPrepWorker::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool FClientPrepWorker::IsThreadFinished()
{
	if (Runnable)
		return Runnable->IsFinished();
	return true;
}

bool FClientPrepWorker::IsFinished() const
{
	return bFinished;
}

void FClientPrepWorker::LaunchServer()
{
	FString serverPath = "python.exe";
	FString attributes = "server.py " + FString::FromInt(port);
	//FString working_directory = FPaths::ProjectDir() + "Dataprep";
	//FString working_directory = FPaths::GameDir() + "Dataprep";
	FString working_directory = FPaths::ProjectUserDir() + "Dataprep";
	UE_LOG(LogTemp, Warning, TEXT("Working Directory: %s"), *working_directory);
	UE_LOG(LogTemp, Warning, TEXT("Launch server: %s"), *serverPath);
	//cannot run bat files
	server_handle = FPlatformProcess::CreateProc(*serverPath, *attributes, true, false, false, nullptr, 0, *working_directory, nullptr);
	if (server_handle.IsValid() && FPlatformProcess::IsProcRunning(server_handle))
	{
		UE_LOG(LogTemp, Warning, TEXT("Local Server Process started"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Local Server Process failed"));

	step++;
}

void FClientPrepWorker::ConnectServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Connect server:"));
	if (listenSocket == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket not initialized"));
		return;
	}
	
	
	FIPv4Address ip(127, 0, 0, 1);
	FIPv4Address::Parse(server_adress, ip);

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(port);
	if(listenSocket->Connect(*addr))
	{
		if (listenSocket->GetConnectionState() == ESocketConnectionState::SCS_Connected)
		{
			UE_LOG(LogTemp, Warning, TEXT("Connected to server"));
			StatusUpdateEvent.Broadcast(EDataPrepStatus::ClientConnected);
			step++;
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("Cannot connect to server"));
	}
	UE_LOG(LogTemp, Warning, TEXT("Connect Fail"));

}

void FClientPrepWorker::SendJobServer()
{
	FString serialized = cmd_cli_config_dataprep + job_description;
	TCHAR *serializedChar = serialized.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent = 0;
	bool successful1 = listenSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	UE_LOG(LogTemp, Warning, TEXT("Wait for ACK1"));
	if (WaitForMessage(cmd_ack))
	{
		serialized = cmd_cli_start_dataprep;
		serializedChar = serialized.GetCharArray().GetData();
		size = FCString::Strlen(serializedChar);
		sent = 0;
		bool successful2 = listenSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
		UE_LOG(LogTemp, Warning, TEXT("Wait for ACK2"));
		if (WaitForMessage(cmd_ack))
		{
			StatusUpdateEvent.Broadcast(EDataPrepStatus::DataPrepStarted);
			step++;
		}
	}
}

void FClientPrepWorker::WaitStatus()
{
	FString serialized = cmd_cli_waiting_for_statuses;
	TCHAR *serializedChar = serialized.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent = 0;
	bool successfull = listenSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	UE_LOG(LogTemp, Warning, TEXT("Wait for Finish Dataprep"));
	WaitForMessage(cmd_ser_finish_dataprep);
	StatusUpdateEvent.Broadcast(EDataPrepStatus::ImportComplete);
	serializedChar = cmd_ack.GetCharArray().GetData();
	size = FCString::Strlen(serializedChar);
	sent = 0;
	successfull = listenSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	UE_LOG(LogTemp, Warning, TEXT("Wait for Finish Cook"));
	WaitForMessage(cmd_ser_finish_cook);
	StatusUpdateEvent.Broadcast(EDataPrepStatus::CookComplete);
	sent = 0;
	successfull = listenSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	UE_LOG(LogTemp, Warning, TEXT("Wait for Finish Copy"));
	WaitForMessage(cmd_ser_finish_copy);
	StatusUpdateEvent.Broadcast(EDataPrepStatus::CopyComplete);
	sent = 0;
	successfull = listenSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	UE_LOG(LogTemp, Warning, TEXT("Wait for server accepting new commands"));
	WaitForMessage(cmd_ser_waiting_commands);
	step++;
}

void FClientPrepWorker::KillServer()
{
	FString serialized = cmd_cli_stop_server;
	TCHAR *serializedChar = serialized.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent = 0;
	bool successful1 = listenSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	WaitForMessage(cmd_ack);
}

bool FClientPrepWorker::WaitForMessage(FString msg)
{
	if (listenSocket == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket not initialized"));
		return false;
	}

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
			UE_LOG(LogTemp, Warning, TEXT("Unexpected reply from server %s"),*StringData);
			return false;
		}
		UE_LOG(LogTemp, Warning, TEXT("Server acknowledge"));
		return true;
	}
	delete[] Buffer;
	return false;
}
