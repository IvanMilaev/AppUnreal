#pragma once

#include "GameFramework/Actor.h"
#include "AssetPrepWorker.h"
#include "Manager.generated.h"

UENUM(BlueprintType)
enum EJobType
{
	ASSET_PREPARE = 0 UMETA(DisplayName = "Make pak file from folder where assets is"),
	ASSET_HOT_LOAD = 1 UMETA(DisplayName = "Load asset(pak) into app")
};

USTRUCT(BlueprintType)
struct FJobDescription
{
	GENERATED_BODY()
	EJobType job_type;

	/** Json string **/
	FString job_description;
};

UCLASS()
class AManager : public AActor {

	GENERATED_BODY()

		

	public:

		UFUNCTION(BlueprintPure, Category = "Asset Manager 3DSource")
		bool Init();

		UFUNCTION(BlueprintPure, Category = "Asset Manager 3DSource")
		bool DoJob(FString IN_Job);

		

		UFUNCTION(BlueprintCallable, Category = "Asset Manager 3DSource")
		bool SendMessageToServer(FString message);

		UFUNCTION(BlueprintCallable, Category = "Asset Manager 3DSource")
		bool Test();
 
		UFUNCTION(BlueprintPure, Category = "Asset Manager 3DSource")
		EAssetPrepWorkerStatus getCurrentStatus();

		void UpdateStatus(EAssetPrepWorkerStatus newStatus);


		AManager();
		~AManager();

	private:
		EAssetPrepWorkerStatus CurrentStatus;
};