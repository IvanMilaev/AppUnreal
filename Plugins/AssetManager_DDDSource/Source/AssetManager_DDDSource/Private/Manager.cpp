#include "Manager.h"
#include "AssetPrepWorker.h"
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

bool AManager::DoJob(FJobDescription IN_Job)
{
	return true;
}

bool AManager::Test()
{
	//FAssetPrepWorker* thread = FAssetPrepWorker::Test();
	return true;
}



