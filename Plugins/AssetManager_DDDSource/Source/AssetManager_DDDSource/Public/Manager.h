#pragma once

#include "GameFramework/Actor.h"
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
		bool DoJob(FJobDescription IN_Job);

		UFUNCTION(BlueprintPure, Category = "Asset Manager 3DSource")
		bool Test();


		AManager();
		~AManager();
};