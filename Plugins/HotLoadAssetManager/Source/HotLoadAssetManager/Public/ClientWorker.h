
#pragma once

#include "Networking.h"
#include "Dom/JsonObject.h"

UENUM(BlueprintType)
enum EJobType
{
    ASSET_HOT_LOAD = 0 UMETA(DisplayName = "Find asset(s) by ID, make pak file and load it to the game")
};

UENUM(BlueprintType)
enum EWorkerStatus
{
    ThreadStarted = 0 UMETA(DisplayName = "Worker Thread started"),
    ClientConnected = 1 UMETA(DisplayName = "Worker client connected to server"),
    JobStarted = 2 UMETA(DisplayName = "Job started")
   /* DataPrepStarted = 2 UMETA(DisplayName = "Preparing CAD Data"),
    ImportComplete = 3 UMETA(DisplayName = "Cooking Assets"),
    CookComplete = 4 UMETA(DisplayName = "Copying Assets"),
    CopyComplete = 5 UMETA(DisplayName = "Importing Assets")*/
};

class  FClientWorker : public FRunnable
{

    DECLARE_EVENT_OneParam(FMultiThreadTest, FCLientStatusUpdateEvent, EWorkerStatus);

    /** Singleton instance **/
    static FClientWorker* Runnable;

    /** Thread to run the worker FRunnable on */
    FRunnableThread* Thread;

    /** Stop this thread? Uses Thread Safe Counter */
    FThreadSafeCounter StopTaskCounter;

private:
    FString cmd_server_confirmation, cmd_cli_prepare_for_job, cmd_cli_start_job, cmd_cli_cancel_job, cmd_cli_waiting_for_statuses,
        cmd_cli_job_completed;
    FSocket* listenSocket;
    FString server_adress;
    FString job_description;
    TSharedPtr<FJsonObject> json_job_description;
    EJobType job_type;
    int32 port;

    bool bFinished;
    int step;

    //Constructor
    FClientWorker(const FString& IN_IPAdress, const int IN_Port);

    bool SendDataToServer(FString IN_Data);
    bool WaitForMessage(FString msg);
    void WaitStatus();
    bool isSocketWorks();
    bool ParseJobDescriptionToJSON();
    bool ConnectServer();
    void DisconnectFromServer();
    void SendJobToServer();
    void ProcessJob(int IN_Step);
    void AssetHotLoadProcess(int IN_Step);
    void FinishJob();

public:
    bool IsFinished() const;


    FCLientStatusUpdateEvent StatusUpdateEvent;
    // Begin FRunnable interface. 
    virtual bool Init();
    virtual uint32 Run();
    virtual void Stop();
    // End FRunnable interface
    void Start();

    bool ShutDown();
    bool LaunchNewJob(EJobType& IN_JobType, const FString& IN_JobDescription);
    

}