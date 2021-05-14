#pragma once
#include "Networking.h"

UENUM(BlueprintType)
enum EAssetPrepWorkerCommand
{
    ECHO  UMETA(DisplayName = "Echo message to server and from server"),
    CONFIRMED  UMETA(DisplayName = "Confirm message from server"),
    KILL_SERVER  UMETA(DisplayName = "Kill server process"),
    WAITING_FOR_STATUSES UMETA(DisplayName = "Waiting for statuses from server"),
    SEND_JOB_TO_SERVER UMETA(DisplayName = "Send job to server"),
    START_JOB UMETA(DisplayName = "Start Asset preparation job"),
    JOB_COMPLETED UMETA(DisplayName = "Job completed")
};

UENUM(BlueprintType)
enum EAssetPrepWorkerStatus
{
    ThreadStarted UMETA(DisplayName = "Worker Thread started"),
    NetworkUtilsClientConnected  UMETA(DisplayName = " Network Utils Client connected to server"),
    ConnectedToServer  UMETA(DisplayName = "Worker client connected to server"),
    JobStarted UMETA(DisplayName = "Job started"),
    JobCompleted UMETA(DisplayName = "Job completed")
};




class FAssetPrepWorker: public FRunnable
{
    DECLARE_EVENT_OneParam(FMultiThreadTest, FCLientStatusUpdateEvent, EAssetPrepWorkerStatus);

    /** Singleton instance **/
    static FAssetPrepWorker* Runnable;

    /** Thread to run the worker FRunnable on */
    FRunnableThread* Thread;

    /** Stop this thread? Uses Thread Safe Counter */
    FThreadSafeCounter StopTaskCounter;

    private:
        FString cmd_server_confirmation, cmd_cli_prepare_for_job, cmd_cli_start_job, cmd_cli_cancel_job, cmd_cli_waiting_for_statuses;
        FString  cmd_cli_job_completed;
        FSocket* listenSocket;
        FString server_address;
        FString python_script_filename;
        FString working_directory;
        FString job_description;
        int32 port;
        FProcHandle server_handle;
        bool bFinished;
        int step;

        void LaunchServer();
        void ConnectServer();
        void SendJobServer();
        void WaitStatus();
        void KillServer();
        bool WaitForMessage(EAssetPrepWorkerCommand cmd);


        bool SendCommandToServer(EAssetPrepWorkerCommand command, FString payload = "");

        //~~~ Thread Core Functions ~~~
        //Constructor
        FAssetPrepWorker(const FString& IN_IPAdress, const int IN_Port, const FString& IN_Job);


        bool IsSocketWorks();

public:
    bool IsFinished() const;

    FCLientStatusUpdateEvent StatusUpdateEvent;

    //~~~ Thread Core Functions ~~~
    //Destructor 
    virtual ~FAssetPrepWorker();

    // Begin FRunnable interface. 
    virtual bool Init();
    virtual uint32 Run();
    virtual void Stop();
    // End FRunnable interface
    void Start();

    static FAssetPrepWorker* RunJob(FString IN_Job);
   

    /** Makes sure this thread has stopped properly */
    void EnsureCompletion();


    /** Shuts down the thread. Static so it can easily be called from outside the thread context */
    static void Shutdown();

    static bool IsThreadFinished();

};