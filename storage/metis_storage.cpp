///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Metis Storage is a CDN and high available http server, which is especially useful for storing 
// and delivering of a huge number of small and medium size objects (images, js files, etc) 
///////////////////////////////////////////////////////////////////////////////


#include <memory>
#include <signal.h>
#include "socket.hpp"
#include "config.hpp"
#include "metis_log.hpp"
#include "time.hpp"
#include "accept_thread.hpp"
#include "storage_event.hpp"
#include "storage.hpp"
#include "sync_thread.hpp"

using fl::network::Socket;
using fl::chrono::Time;
using namespace fl::metis;
using namespace fl::events;

void sigInt(int sig)
{
	log::Fatal::L("Interruption signal (%d) has been received - flushing data\n", sig);
	static fl::threads::Mutex sigSync;
	if (!sigSync.tryLock())
		return;
	
	exit(0);
}

void setSignals()
{
	signal(SIGINT, sigInt);
	signal(SIGTERM, sigInt);
}

int main(int argc, char *argv[])
{
	std::unique_ptr<Config> config;
	std::unique_ptr<Storage> storage;
	std::unique_ptr<EPollWorkerGroup> workerGroup;
	std::unique_ptr<SyncThread> syncThread;
	try
	{
		config.reset(new Config(argc, argv));
		if (!log::MetisLogSystem::init(config->logLevel(), config->logPath(), config->isLogStdout()))
			return -1;
		
		log::Warning::L("Starting Metis Storage server %u on %s\n", config->serverID(), config->dataPath().c_str());
		if (!config->initNetwork())
			return -1;
		config->setProcessUserAndGroup();
		
		StorageEventFactory *factory = new StorageEventFactory(config.get());
		StorageThreadSpecificDataFactory *dataFactory = new StorageThreadSpecificDataFactory(config.get());
		workerGroup.reset(new EPollWorkerGroup(dataFactory, config->workers(), config->workerQueueLength(), 
			EPOLL_WORKER_STACK_SIZE));
		AcceptThread cmdThread(workerGroup.get(), &config->listenSocket(), factory);

		storage.reset(new Storage(config->dataPath().c_str(), config->minDiskFree(), config->maxSliceSize()));
		syncThread.reset(new SyncThread(storage.get(), config.get()));
		
		StorageEvent::setInited(storage.get(), config.get(), syncThread.get());
		setSignals();
		workerGroup->waitThreads();
	}
	catch (...)	
	{
		return -1;
	}
	return 0;
};
