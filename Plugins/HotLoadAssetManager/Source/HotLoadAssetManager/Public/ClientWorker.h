
#pragma once

#include "Networking.h"

class FClientWorker: public FRunnable
{
    // Begin FRunnable interface. 
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface
	void Start();
}