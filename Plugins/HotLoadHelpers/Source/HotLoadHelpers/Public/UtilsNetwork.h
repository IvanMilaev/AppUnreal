#pragma once

#include "GameFramework/Actor.h"
#include "ClientPrepWorker.h"
#include "UtilsNetwork.generated.h"



//client launching 
UCLASS()
class HOTLOADHELPERS_API AUtilsNetwork : public AActor
{
	GENERATED_BODY()
	
	public:

		UFUNCTION(BlueprintPure, Category = "Client Data Prep")
		bool MyLaunchClientPrep(const FString & IN_JobDescription, const FString& IN_IPAdress, const int IN_Port, const bool IN_SpawnServer);

		UFUNCTION(BlueprintPure, Category = "Client Data Prep")
		bool StopThread();

		UFUNCTION(BlueprintPure, Category = "Client Data Prep")
		EDataPrepStatus getCurrentStatus();

		void UpdateStatus(EDataPrepStatus newStatus);

		AUtilsNetwork();
		~AUtilsNetwork();

	private:
		EDataPrepStatus CurrentStatus;
	
};
