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

#define LOG_TAG "juneapp"
#define LOG_NDEBUG 0

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>

#include <include/IJune.h>

using namespace android;

sp<IJune> gJune;
const sp<IJune>& get_june_service()
{
    if (gJune == 0) {
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;
        do {
            binder = sm->getService(String16("service.june"));
            if (binder != 0)
                break;
            ALOGW("June not published, waiting...");
            usleep(500000); // 0.5 s
        } while (true);
        gJune = interface_cast<IJune>(binder);
    }
    ALOGE_IF(gJune==0, "no June!?");

    return gJune;
}


int main(int argc, char** argv)
{
    String8 desc;
    sp<IJune> june = get_june_service();
    if (june == 0)
        return -1;

    june->Init();
    ALOGI("June Client Init");
    desc = june->GetJuneServiceDesc();
    ALOGI("June Client GetJuneServiceDesc [%s]", desc.string());
    june->DeInit(0);
    ALOGI("June Client DeInit");
    return 0;
}
