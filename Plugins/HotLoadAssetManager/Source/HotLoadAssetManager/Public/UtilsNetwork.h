#pragma once

#include "GameFramework/Actor.h"
#include "UtilsNetwork.generated.h"
#include "ClientWorker.h"



UCLASS()
class HOTLOADASSETMANAGER_API AUtilsNetwork : public AActor
{
    GENERATED_BODY()

    public:

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool LaunchClient(const FString& IN_JobDescription, const FString& IN_IPAddress, const int IN_Port);

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool StopClient();

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool LaunchServer(const FString& IN_IPAddress, const int IN_Port);

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        bool StopServer();

        UFUNCTION(BlueprintPure, Category = "TCP Socket")
        EHotLoadAssetManagerStatus getCurrentStatus();

        void UpdateStatus(EHotLoadAssetManagerStatus newStatus);

        AUtilsNetwork();
        ~AUtilsNetwork();

    private:
        EHotLoadAssetManagerStatus CurrentStatus;



};

