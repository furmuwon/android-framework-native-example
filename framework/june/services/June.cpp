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
    : BnJune()
{
    ALOGV("%s", __func__);
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
    int id = 0;
    ALOGV("%s", __func__);
    return id;
}

String8 June::GetJuneServiceDesc(void)
{
    ALOGV("%s", __func__);

    return String8("This is June Service, June is middle name of my son");
}

status_t June::DeInit(int id)
{
    ALOGV("%s", __func__);
    return NO_ERROR;
}

// ----------------------------------------------------------------------------

status_t June::onTransact(
        uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    return BnJune::onTransact(code, data, reply, flags);
}

}; // namespace android
