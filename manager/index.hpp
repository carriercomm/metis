#pragma once
#ifndef __FL_MANAGER_INDEX_HPP
#define	__FL_MANAGER_INDEX_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Index control classes
///////////////////////////////////////////////////////////////////////////////

#include "config.h"
#ifdef HAVE_CXX11
	#include <unordered_map>
	using std::unordered_map;
#else
	#include <boost/unordered_map.hpp>
	using boost::unordered_map;
#endif

#include <memory>
	
#include "../types.hpp"
#include "mysql.hpp"
#include "mutex.hpp"
#include "cluster_manager.hpp"

	
namespace fl {
	namespace metis {
		using fl::db::Mysql;
		using fl::db::MysqlResult;
		using fl::threads::Mutex;
		using fl::threads::AutoMutex;
		
		class Range
		{
		public:
			Range(MysqlResult *res, ClusterManager &clusterManager);
			TRangeID rangeID() const
			{
				return _rangeID;
			}
			TItemKey rangeIndex() const
			{
				return _rangeIndex;
			}
			void update(Range *src);
			TStorageList storages()
			{
				return _storages;
			}
			StorageNode *getPutStorage(const TSize size, Config *config, class ClusterManager &clusterManager, 
				bool &wasAdded);
		private:
			TRangeID _rangeID;
			TItemKey _rangeIndex;
			TServerID _managerID;
			TStorageList _storages;
		};
		typedef std::shared_ptr<Range> TRangePtr;
		
		class RangeIndex
		{
		public:
			typedef uint8_t TStatus;
			typedef uint32_t TIndexID;
			RangeIndex(MysqlResult *res);
			const TIndexID id() const
			{
				return _id;
			}
			void add(TRangePtr &range)
			{
				AutoMutex autoSync(&_sync);
				addNL(range);
			}
			void addNL(TRangePtr &range);
			void update(RangeIndex *src);
			bool find(const TItemKey rangeIndex, TRangePtr &range);
			TItemKey calcRangeIndex(const TItemKey rangeIndex);
			bool loadRange(TRangePtr &range, const TItemKey rangeIndex, class ClusterManager &clusterManager, Mysql &sql);
		private:
			TIndexID _id;
			TStatus _status;
			typedef unordered_map<TItemKey, TRangePtr> TRangeMap;
			TRangeMap _ranges;
			Mutex _sync;
			TItemKey _rangeSize;
		};
		typedef std::shared_ptr<RangeIndex> TRangeIndexPtr;
		
		class IndexManager
		{
		public:
			IndexManager(class Config *config);
			bool loadAll(Mysql &sql, class ClusterManager &clusterManager);
			bool parseURL(const std::string &host, const std::string &fileName, ItemHeader &item, TCrc &crc);
			bool fillAndAdd(ItemHeader &item, TRangePtr &range, class ClusterManager &clusterManager, bool &wasAdded);
			bool addLevel(const TLevel level, const TSubLevel subLevel);
			bool loadLevel(const TLevel level, const TSubLevel subLevel, Mysql &sql);
			StorageNode *getPutStorage(const TRangeID rangeID, const TSize size, class ClusterManager &clusterManager, 
				bool &wasAdded);
		private:
			bool _loadIndex(Mysql &sql);
			bool _loadIndexRanges(Mysql &sql, class ClusterManager &clusterManager);
			void _add(const TLevel level, const TSubLevel sublevel, TRangeIndexPtr &rangeIndex);
			
			typedef unordered_map<TSubLevel, TRangeIndexPtr> TSubLevelMap;
			typedef unordered_map<TLevel, TSubLevelMap> TLevelMap;
			TLevelMap _index;
			
			typedef unordered_map<TRangeID, TRangePtr> TRangeMap;
			TRangeMap _ranges;
			
			typedef unordered_map<RangeIndex::TIndexID, TRangeIndexPtr> TRangeIndexMap;
			TRangeIndexMap _indexRanges;
			Mutex _sync;
			
			class Config *_config;
		};
	};
};

#endif	// __FL_MANAGER_INDEX_HPP
