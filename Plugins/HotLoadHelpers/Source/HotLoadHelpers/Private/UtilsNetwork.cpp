#include "UtilsNetwork.h"

AUtilsNetwork::AUtilsNetwork()
{
}

AUtilsNetwork::~AUtilsNetwork()
{
	StopThread();
}

bool AUtilsNetwork::MyLaunchClientPrep(const FString & IN_JobDescription, const FString& IN_IPAdress, const int IN_Port, const bool IN_SpawnServer)
{
	FClientPrepWorker* thread = FClientPrepWorker::LaunchPrep(IN_JobDescription, IN_IPAdress, IN_Port, IN_SpawnServer);
	thread->StatusUpdateEvent.Clear();
	thread->StatusUpdateEvent.AddUObject(this, &AUtilsNetwork::UpdateStatus);
	return true;
}

bool AUtilsNetwork::StopThread()
{
	FClientPrepWorker::Shutdown();
	return true;
}

EDataPrepStatus AUtilsNetwork::getCurrentStatus()
{
	return CurrentStatus;
}

void AUtilsNetwork::UpdateStatus(EDataPrepStatus newStatus)
{
	CurrentStatus = newStatus;
}