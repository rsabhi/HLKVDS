#ifndef _KV_DB_DATAHANDLE_H_
#define _KV_DB_DATAHANDLE_H_

#include <string>
#include <sys/types.h>

#include <list>

#include "Db_Structure.h"
#include "BlockDevice.h"
#include "KeyDigestHandle.h"
#include "IndexManager.h"
#include "SuperBlockManager.h"
#include "SegmentManager.h"

using namespace std;

namespace kvdb{

    class SegmentOnDisk;
    class DataHeader;
    class DataHeaderOffset;
    class HashEntry;
    class IndexManager;
    class SegmentManager;


    class KVSlice{
    public:
        KVSlice();
        ~KVSlice();
        KVSlice(const KVSlice& toBeCopied);
        KVSlice& operator=(const KVSlice& toBeCopied);

        KVSlice(const char* key, int key_len, const char* data, int data_len);

        const Kvdb_Digest& GetDigest() const { return *digest_; }
        const char* GetKey() const { return key_; }
        const char* GetData() const { return data_; }
        string GetKeyStr() const;
        string GetDataStr() const;
        uint32_t GetKeyLen() const { return keyLength_; }
        uint16_t GetDataLen() const { return dataLength_; }
        bool IsDigestComputed() const { return isComputed_; }
        bool IsAlignedData() const{ return GetDataLen() == ALIGNED_SIZE; }
        HashEntry& GetHashEntry() const { return *entry_; }
        uint32_t GetSegId() const { return segId_; }

        void SetKeyValue(const char* key, int key_len, const char* data, int data_len);
        bool ComputeDigest();
        void SetHashEntry(const HashEntry *hash_entry);
        void SetSegId(uint32_t seg_id);

    private:
        const char* key_;
        uint32_t keyLength_;
        const char* data_;
        uint16_t dataLength_;
        Kvdb_Digest *digest_;
        bool isComputed_;
        HashEntry *entry_;
        uint32_t segId_;

        void copy_helper(const KVSlice& toBeCopied);

    };

    class Request{
    public:
        Request();
        ~Request();
        Request(const Request& toBeCopied);
        Request& operator=(const Request& toBeCopied);
        Request(KVSlice& slice);

        KVSlice& GetSlice() const { return *slice_; }
        int IsDone() const { return isDone_; }
        void Done();

        void SetState(bool state);
        bool GetState() const { return writeStat_; }

        void Wait();
        void Signal();

    private:
        int isDone_;
        bool writeStat_;
        KVSlice *slice_;
        Mutex *mtx_;
        Cond *cond_;

    };

    class SegmentSlice{
    public:
        SegmentSlice();
        ~SegmentSlice();
        SegmentSlice(const SegmentSlice& toBeCopied);
        SegmentSlice& operator=(const SegmentSlice& toBeCopied);

        SegmentSlice(SegmentManager* sm, BlockDevice* bdev);

        bool Put(Request* req);
        bool WriteSegToDevice(uint32_t seg_id);
        void Complete();
        bool CompleteIfExpired();
        void NotifyFailed();

    private:
        bool isExpire();
        bool isCanFit(Request* req) const;
        void copyHelper(const SegmentSlice& toBeCopied);
        void fillSlice();
        void fillSegHead();
        void notifyAndClean(bool req_state);
        bool _writeToDeviceHugeHole();
        bool _writeToDevice();
        //bool _writeDataToDevice();
        //void copyToData();

        uint32_t segId_;
        SegmentManager* segMgr_;
        BlockDevice* bdev_;
        uint32_t segSize_;
        KVTime startTime_;

        uint32_t headPos_;
        uint32_t tailPos_;

        uint32_t keyNum_;
        uint32_t keyAlignedNum_;
        bool isCompleted_;
        bool hasReq_;

        Mutex *mtx_;

        std::list<Request *> reqList_;
        SegmentOnDisk *segOndisk_;

        //char* data_;
    };

} //end namespace kvdb

#endif //#ifndef _KV_DB_DATAHANDLE_H_
