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

#ifndef ANDROID_JUNE_H
#define ANDROID_JUNE_H

#include <stdint.h>
#include <sys/types.h>
#include <limits.h>

#include <cutils/compiler.h>

#include <include/IJune.h>

#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <utils/TypeHelpers.h>
#include <utils/Vector.h>
#include <utils/String8.h>

#include <binder/BinderService.h>
#include <binder/MemoryDealer.h>

namespace android {
// ----------------------------------------------------------------------------
class June :
    public BinderService<June>,
    public BnJune
{
    friend class BinderService<June>;
public:
    static const char* getServiceName() ANDROID_API { return "service.june"; }
    //static const char* getServiceName() { return "service.june"; }
    virtual void onFirstRef();


    virtual     status_t    dump(int fd, const Vector<String16>& args);
    virtual     status_t    onTransact(
                                uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags);
    virtual int Init(void);
    virtual String8 GetJuneServiceDesc(void);
    virtual status_t DeInit(int id);

private:
    June() ANDROID_API;
    //June();
    virtual ~June();

// ----------------------------------------------------------------------------
};

}; // namespace android

#endif // ANDROID_JUNE_H
