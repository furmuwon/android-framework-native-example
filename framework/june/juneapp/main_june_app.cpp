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
#include <utils/Log.h>


#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/threads.h>
#include <include/IJune.h>

using namespace android;

class DemoApp : public Thread {
public:
    DemoApp()
    {
        ALOGV("DemoApp()");
        mJune = 0;
        mAppDathNotification = 0;
    }

    virtual ~DemoApp()
    {
        mJune.clear();
        mAppDathNotification.clear();
        ALOGV("~DemoApp()");
    }

    virtual status_t readyToRun()
    {
        mJune = get_june_service();
        if (mJune == 0)
            return NO_MEMORY;

        return NO_ERROR;
    }

    virtual bool threadLoop()
    {
        while (!exitPending())
        {
            String8 desc;
            Mutex::Autolock lock(appLock);
            mJune->Init();
            ALOGI("June Client Init");
            desc = mJune->GetJuneServiceDesc();
            ALOGI("June Client GetJuneServiceDesc [%s]", desc.string());
            mJune->DeInit(0);
            ALOGI("June Client DeInit");
            usleep(1000000); // 1s
        }

        return false;
    }

    const sp<IJune> get_june_service()
    {
        ALOGV("get_june_service()");
        Mutex::Autolock lock(appLock);
        if (mJune == 0) {
            sp<IServiceManager> sm = defaultServiceManager();
            sp<IBinder> binder;
            do {
                binder = sm->getService(String16("service.june"));
                if (binder != 0)
                    break;
                ALOGW("June not published, waiting...");
                usleep(500000); // 0.5 s
            } while (true);
            if (mAppDathNotification == NULL) {
                mAppDathNotification = new AppDathNotification(this);
            } else {
            }
            binder->linkToDeath(mAppDathNotification, this);
            mJune = interface_cast<IJune>(binder);
        }
        ALOGE_IF(mJune==0, "no June!?");

        return mJune;
    }

    class AppDathNotification : public IBinder::DeathRecipient {
    public:
        AppDathNotification(DemoApp *app)
        {
            ALOGV("AppDathNotification!\n");
            mApp = app;
        }
        virtual ~AppDathNotification(){}
        // IBinder::DeathRecipient
        virtual void binderDied(const wp<IBinder>& who)
        {
            ALOGW("Service Die\n");
            mApp->requestExit();
        }
        DemoApp *mApp;
    };

    sp<IJune> mJune;
    sp<AppDathNotification> mAppDathNotification;
    mutable Mutex appLock;
};

int main(int argc, char** argv)
{
    ALOGV("Main DemoApp\n");
    sp<ProcessState> proc(ProcessState::self());
    android::ProcessState::self()->startThreadPool();

    sp<DemoApp> app = new DemoApp();
    if (app == 0) {
        ALOGE("DemoApp allocation fail\n");
        return -1;
    }
    ALOGV("Run DemoApp\n");

    app->run(); /* call the readyTorun in _threadLoop by calling run function */

    ALOGV("Join DemoApp\n");
    app->join();

    return 0;
}

