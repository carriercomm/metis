#pragma once
#ifndef __FL_METIS_MANAGER_WEBDAV_INTERFACE_HPP
#define	__FL_METIS_MANAGER_WEBDAV_INTERFACE_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Metis manager a WebDav interface class
///////////////////////////////////////////////////////////////////////////////

#include "cmd_event.hpp"
#include "webdav_interface.hpp"
#include "types.hpp"

namespace fl {
	namespace metis {
		using fl::http::WebDavInterface;
		
		class ManagerWebDavInterface : public WebDavInterface
		{
		public:
			ManagerWebDavInterface();
			virtual ~ManagerWebDavInterface();
			virtual bool parseURI(const char *cmdStart, const EHttpVersion::EHttpVersion version,
				const std::string &host, const std::string &fileName, const std::string &query);
			static void setInited(class Manager *manager);
		protected:
			static bool _isReady;
			static class Manager *_manager;
			virtual bool _put(const char *dataStart);
			virtual bool _putFile();
			virtual bool _mkCOL();
			ItemHeader _item;
		};
		
		class ManagerWebDavEventFactory : public WorkEventFactory 
		{
		public:
			ManagerWebDavEventFactory(class Config *config);
			virtual WorkEvent *create(const TEventDescriptor descr, const TIPv4 ip, const time_t timeOutTime, 
				Socket *acceptSocket);
			virtual ~ManagerWebDavEventFactory() {};
		private:
			class Config *_config;
		};
	};
};

#endif	// __FL_METIS_MANAGER_WEBDAV_INTERFACE_HPP
