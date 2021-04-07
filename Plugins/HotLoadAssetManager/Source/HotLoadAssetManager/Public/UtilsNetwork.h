#pragma once

#include "GameFramework/Actor.h"
#include "ClientWorker.h"
#include "UtilsNetwork.generated.h"




UCLASS()
class  AUtilsNetwork : public AActor
{
    GENERATED_BODY()

    public:

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool Init(const FString& IN_Working_directory, const FString& IN_Python_Script_Filename, const FString& IN_IPAddress, const int IN_Port);

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        FClientWorker* CreateWorker();

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool StartWorker();

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool StopWorker();

        UFUNCTION(BlueprintPure, Category = "Job")
        bool LaunchNewJob(const FString& IN_JobDescription);


        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool LaunchServer();

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool StopServer();

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        EWorkerStatus getCurrentStatus();

        void UpdateStatus(EWorkerStatus newStatus);

        AUtilsNetwork();
        ~AUtilsNetwork();

    private:
        FString working_directory, python_script_filename, server_address;
        int32 port;
        EWorkerStatus CurrentStatus;
        FProcHandle server_handle;
        FClientWorker* worker;
        
};

