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

#define LOG_TAG "June"
#define LOG_NDEBUG 0

#include <dirent.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include <utils/Trace.h>
#include <binder/Parcel.h>
#include <utils/String16.h>
#include <utils/threads.h>
#include <utils/Atomic.h>

#include <cutils/bitops.h>
#include <cutils/properties.h>

#include "June.h"

#ifdef VERY_VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

namespace android {

// ----------------------------------------------------------------------------
June::June()
    : BnJune(),
    mMemoryDealer(0),
    unique_id(0)
{
    ALOGV("%s", __func__);
    mMemoryDealer = new MemoryDealer(1024*1024, "June_Memory_Test");
    if (mMemoryDealer == 0)
        ALOGE("Memory Dealer is NULL");
}

June::~June()
{
    ALOGV("%s", __func__);
}

void June::onFirstRef()
{

    ALOGV("%s", __func__);
}

status_t June::dump(int fd, const Vector<String16>& args)
{
    return NO_ERROR;
}

int June::Init(void)
{
    Mutex::Autolock jlock(mLock);
    unique_id++;
    ALOGV("%s", __func__);
    return unique_id;
}

String8 June::GetJuneServiceDesc(void)
{
    ALOGV("%s", __func__);
    return String8("This is June Service, June is middle name of my son");
}

sp<IMemory> June::AllocMemory(int id, int size)
{
    sp<IMemory> iMem;
    ssize_t index;

    Mutex::Autolock jlock(mLock);
    ALOGV("%s", __func__);

    if (mMemoryDealer == 0)
        return 0;

    index = mAllocedMemorys.indexOfKey(id);
    if (index >= 0) {
        iMem = mAllocedMemorys.valueFor(id);
        ALOGW("Already memory alloced id:%d, size:%d, memory point: 0x%x off:0x%x",
            id, iMem->size(), (unsigned int)iMem->pointer(), iMem->offset());
        return 0;
    } else {
        iMem = mMemoryDealer->allocate(size);
        if (iMem == 0) {
            ALOGE("failed to alloc id:%d, size:%d", id, size);
            return 0;
        }
    }

    ALOGV("Alloc Memory id:%d, size:%d, memory point: 0x%x off:0x%x",
        id, size, (unsigned int)iMem->pointer(), iMem->offset());

    mMemoryDealer->dump("In AllocMemory");

    LeaveTraceMemory(iMem);
    mAllocedMemorys.add(id, iMem);

    return iMem;
}

void June::LeaveTraceMemory(sp<IMemory> &iMem)
{
    uint32_t* p;
    p = static_cast<uint32_t*>(iMem->pointer());
    p[0] = 0x11112222;
    p[1] = 0x33334444;
    p[2] = 0x55556666;
    p[3] = 0x77778888;
    return;
}

status_t June::DumpMemory(int id, int size)
{
    int i;
    int next_i = 0;
    uint8_t *bytedata;
    sp<IMemory> iMem;
    Mutex::Autolock jlock(mLock);
    ALOGV("%s", __func__);
    iMem = mAllocedMemorys.valueFor(id);
    if (iMem == 0) {
        ALOGE("failed to search memory, id:%d", id);
        return NO_MEMORY;
    }

    ALOGV("DumpMemory id:%d, show size:%d, mem size %d, memory point: 0x%x off:0x%x",
        id, size, iMem->size(), (unsigned int)iMem->pointer(), iMem->offset());

    bytedata = static_cast<uint8_t*>(iMem->pointer());

    if ((size_t)size > iMem->size())
        size = (int)iMem->size();

    if (size/16) {
        for (i = 0; i < (size - 16); i += 16, next_i += 16) {
            ALOGV("0x%0x: %02x%02x%02x%02x %02x%02x%02x%02x "
                "%02x%02x%02x%02x %02x%02x%02x%02x",
                (unsigned int)bytedata+i,
                bytedata[i+0], bytedata[i+1], bytedata[i+2], bytedata[i+3],
                bytedata[i+4], bytedata[i+5], bytedata[i+6], bytedata[i+7],
                bytedata[i+8], bytedata[i+9], bytedata[i+10], bytedata[i+11],
                bytedata[i+12], bytedata[i+13], bytedata[i+14], bytedata[i+15]);
        }
    }

    if (size % 16) {
        int space = 0;
        String8 buf;
        buf.appendFormat("0x%0x: ", (unsigned int)bytedata+next_i);
        for (i = next_i; i < size; i++) {
            buf.appendFormat("%02x", bytedata[i]);
            space++;
            if ((space % 4) == 0)
                buf.appendFormat(" ");
        }
        ALOGV("%s", buf.string());
    }

    mMemoryDealer->dump("In DumpMemory");

    return NO_ERROR;
}


status_t June::DeInit(int id)
{
    sp<IMemory> iMem;
    Mutex::Autolock jlock(mLock);
    ALOGV("%s", __func__);
    iMem = mAllocedMemorys.valueFor(id);
    if (iMem != 0) {
        mAllocedMemorys.removeItem(id);

        // My mistake.
        // I do not need to be called directly deallocate function
        // Refer to the Strong Pointer
        //mMemoryDealer->deallocate(iMem->offset());
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------

status_t June::onTransact(
        uint32_t code, const Parcel& bytedata, Parcel* reply, uint32_t flags)
{
    return BnJune::onTransact(code, bytedata, reply, flags);
}

}; // namespace android
