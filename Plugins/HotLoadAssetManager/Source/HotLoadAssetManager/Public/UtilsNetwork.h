#pragma once

#include "GameFramework/Actor.h"
#include "ClientWorker.h"
#include "UtilsNetwork.generated.h"




UCLASS()
class  AUtilsNetwork : public AActor
{

    GENERATED_BODY()

    DECLARE_EVENT_OneParam(FMultiThreadTest, FCLientStatusUpdateEvent, EWorkerStatus);
    public:

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool Init(const FString& IN_Working_directory, const FString& IN_Python_Script_Filename, const FString& IN_IPAddress, const int IN_Port);

       
        void CreateWorker();
        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool StartWorker();
        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool StopWorker();
        


        UFUNCTION(BlueprintPure, Category = "Job")
        bool LaunchNewJob(EJobType IN_JobType, const FString& IN_JobDescription);
        bool LaunchServer();
        bool StopServer();
        bool LaunchSocketConnectionWithServer();

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        EWorkerStatus getCurrentStatus();

        void UpdateStatus(EWorkerStatus newStatus);
        bool WaitForMessage(FString msg);

        AUtilsNetwork();
        ~AUtilsNetwork();

    private:
        FString working_directory, python_script_filename, server_address;
        FString cmd_cli_stop_server, cmd_ack;
        int32 port;
        EWorkerStatus CurrentStatus;
        FProcHandle server_handle;
        FClientWorker* worker;
        FSocket* listenSocket;
        FCLientStatusUpdateEvent StatusUpdateEvent;
        
};

