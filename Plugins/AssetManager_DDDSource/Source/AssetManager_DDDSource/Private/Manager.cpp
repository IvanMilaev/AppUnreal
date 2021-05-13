#include "Manager.h"

#include "AssetManager_DDDSource.h"


//AManager::AManager(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
//{
//}

AManager::AManager()
{}

AManager::~AManager()
{}

bool AManager::Init()
{
	return true;
}

bool AManager::DoJob(FString IN_Job)
{
	UE_LOG(LogTemp, Display, TEXT("Launch worker for job %s"), *IN_Job);
	FAssetPrepWorker* thread = FAssetPrepWorker::RunJob(IN_Job);
	thread->StatusUpdateEvent.Clear();
	thread->StatusUpdateEvent.AddUObject(this, &AManager::UpdateStatus);
	UE_LOG(LogTemp, Display, TEXT("Thread..."));
	return true;
}

bool AManager::Test()
{
	/*UE_LOG(LogTemp, Display, TEXT("Launch test worker"));
	FAssetPrepWorker* thread = FAssetPrepWorker::RunJob();
	thread->StatusUpdateEvent.Clear();
	thread->StatusUpdateEvent.AddUObject(this, &AManager::UpdateStatus);
	UE_LOG(LogTemp, Display, TEXT("Thread..."));*/

	return true;
}

EAssetPrepWorkerStatus AManager::getCurrentStatus()
{
	return CurrentStatus;
}

void AManager::UpdateStatus(EAssetPrepWorkerStatus newStatus)
{
	CurrentStatus = newStatus;
}



