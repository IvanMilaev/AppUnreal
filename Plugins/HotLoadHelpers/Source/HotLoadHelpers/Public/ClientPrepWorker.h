#pragma once

#include "Networking.h"

UENUM(BlueprintType)
enum EDataPrepStatus
{
	ThreadStarted = 0 UMETA(DisplayName = "Connecting to Data prep client"),
	ClientConnected = 1 UMETA(DisplayName = "Starting Data preparation"),
	DataPrepStarted = 2 UMETA(DisplayName = "Preparing CAD Data"),
	ImportComplete = 3 UMETA(DisplayName = "Cooking Assets"),
	CookComplete = 4 UMETA(DisplayName = "Copying Assets"),
	CopyComplete = 5 UMETA(DisplayName = "Importing Assets")
};

// Class containing the worker thread for managing the connection with the data prep server
class HOTLOADHELPERS_API FClientPrepWorker : public FRunnable
{

	DECLARE_EVENT_OneParam(FMultiThreadTest, FCLientStatusUpdateEvent, EDataPrepStatus);

	/** Singleton instance **/
	static FClientPrepWorker* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;


private:
	FString cmd_cli_config_dataprep, cmd_cli_start_dataprep, cmd_cli_waiting_for_statuses, cmd_cli_stop_server, cmd_ser_finish_dataprep, cmd_ser_finish_cook, cmd_ser_finish_copy, cmd_ser_waiting_commands, cmd_ack;
	FString project_name_str, output_folder_str, cad_files_path_str, content_folder_str, tesselation_parameters_str;

	FSocket* listenSocket;
	FString server_adress;
	FString job_description;
	int32 port;
	FProcHandle server_handle;
	bool bSpawnServer;
	int step;

	bool bFinished;

	void LaunchServer();
	void ConnectServer();
	void SendJobServer();
	void WaitStatus();
	void KillServer();
	bool WaitForMessage(FString msg);

	//~~~ Thread Core Functions ~~~
	//Constructor
	FClientPrepWorker(const FString & IN_JobDescription, const FString& IN_IPAdress, const int IN_Port, const bool IN_SpawnServer);

public:
	bool IsFinished() const;

	FCLientStatusUpdateEvent StatusUpdateEvent;

	//~~~ Thread Core Functions ~~~
	//Destructor 
	virtual ~FClientPrepWorker();

	// Begin FRunnable interface. 
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface
	void Start();

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();

	//~~~ Starting and Stopping Thread ~~~
	static FClientPrepWorker* LaunchPrep(const FString& IN_JobDescription, const FString& IN_IPAdress, const int IN_Port, const bool IN_SpawnServer);

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();

	static bool IsThreadFinished();
};