
#pragma once

#include "Networking.h"
#include "Dom/JsonObject.h"

UENUM(BlueprintType)
enum EWorkerStatus
{
    ThreadStarted = 0 UMETA(DisplayName = "Worker Thread started"),
    ClientConnected = 1 UMETA(DisplayName = "Worker client connected to server"),
    DataPrepStarted = 2 UMETA(DisplayName = "Preparing CAD Data"),
    ImportComplete = 3 UMETA(DisplayName = "Cooking Assets"),
    CookComplete = 4 UMETA(DisplayName = "Copying Assets"),
    CopyComplete = 5 UMETA(DisplayName = "Importing Assets")
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
    FSocket* listenSocket;
    FString server_adress;
    FString job_description;
    TSharedPtr<FJsonObject> json_job_description;
    int32 port;

    bool bFinished;
    int step;

    //Constructor
    FClientWorker(const FString& IN_IPAdress, const int IN_Port);

    bool ParseJobDescriptionToJSON();
    bool ConnectServer();
    void SendJobToServer();

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
    bool LaunchNewJob(const FString& IN_JobDescription);

}