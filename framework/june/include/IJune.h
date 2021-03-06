/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_IJUNE_H
#define ANDROID_IJUNE_H

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/IMemory.h>
#include <utils/String8.h>

namespace android {

#define CLIENT_TO_SERVER 0
#define SERVER_TO_CLIENT 1

#define JUNE_CBLK_FUTEX_WAKE 1

#define JUNE_CTL_MAGIC 0x4A554E45

#define JUNE_CTL_DATA_MAX 128

#define JUNE_CTL_FLAG_DATA_PUSH        0x1
#define JUNE_CTL_FLAG_EXIT             0x2
#define JUNE_CTL_FLAG_UNKNOWN_ERR      0x4

struct June_control_block_t
{
    uint32_t magic;
    uint32_t mDirection; // Client -> Server, Server -> Client
    volatile int32_t mFutex;
    volatile int32_t mFlags;
    volatile int32_t stack_lock;
    volatile int32_t stack_cnt;
    uint32_t stack[JUNE_CTL_DATA_MAX];
};

// ----------------------------------------------------------------------------
class IJune : public IInterface
{
public:
#if 1
    DECLARE_META_INTERFACE(June);
#else
    static const android::String16 descriptor;
    static android::sp<IJune> asInterface(const android::sp<android::IBinder>& obj);
    virtual const android::String16& getInterfaceDescriptor() const;
    IJune();
    virtual ~IJune();
#endif

	virtual int Init(void) = 0;
	virtual String8 GetJuneServiceDesc(void) = 0;
    virtual sp<IMemory> NewControlBlock(int id , int dir) = 0;
    virtual sp<IMemory> AllocMemory(int id , int size) = 0;
    virtual status_t DumpMemory(int id, int size) = 0;
    virtual status_t DeInit(int id) = 0;
};

class BnJune : public BnInterface<IJune>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};
// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_IJUNE_H
