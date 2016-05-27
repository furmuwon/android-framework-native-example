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
extern "C" {
#include "../private/bionic_futex.h"
}

using namespace android;

class DemoApp : public Thread {
public:
    DemoApp(int tc)
    {
        test_case = tc;
        ALOGV("DemoApp(Test Case %d)", tc);
        mJune = 0;
        mAppDathNotification = 0;
        id = 0;
        mJcblk = NULL;
    }

    DemoApp()
    {
        DemoApp(0);
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
        pid_t tid = getTid();

        int ret = 0;
        Mutex::Autolock lock(appLock);
        switch(test_case) {
        case 0:
            ret = MemoryAllocTest();
            break;
        case 1:
            ret = MemoryShareControlBlockTest();
            break;
        default:
            break;
        };

        if (ret < 0)
            ALOGV("Test Case[%d] Fail!", test_case);

        ALOGV("Test Case[%d] Ok!", test_case);
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

    int MemoryAllocTest(void) {
        uint32_t* p;
        int memory_alloc_test_size = 4096;
        pid_t tid = getTid();

        while(!exitPending()) {
            id = mJune->Init();
            ALOGI("June Client[%d] Init tid[%d, 0x%x]", id, tid, tid);

            mImem = mJune->AllocMemory(id, memory_alloc_test_size);
            if (mImem == 0) {
                ALOGE("June Client[%d] failed to get memory", id);
                mJune->DeInit(id);
                return -1;
            }
            p = static_cast<uint32_t*>(mImem->pointer());
            ALOGI("June Client[%d] Getted Memory Info", id);
            ALOGI("June Client[%d] pointer [0x%x]", id, (unsigned int)mImem->pointer());
            ALOGI("June Client[%d] size    [%d]", id, mImem->size());
            ALOGI("June Client[%d] offset  [%d]", id, mImem->offset());
            ALOGI("June Client[%d] trace[0] [0x%x]", id, p[0]);
            ALOGI("June Client[%d] trace[1] [0x%x]", id, p[1]);
            ALOGI("June Client[%d] trace[2] [0x%x]", id, p[2]);
            ALOGI("June Client[%d] trace[3] [0x%x]", id, p[3]);
            p[4] = 0xaaaabbbb;
            p[5] = 0xccccdddd;
            p[6] = id;
            p[7] = (unsigned int)tid;
            mJune->DumpMemory(id, 40);

            mJune->DeInit(id);
            ALOGI("June Client[%d] DeInit", id);
            usleep(5000000); // 5s
        }

        return 0;
    }

    status_t MemoryShareControlBlockTest(void) {
        status_t status = NO_ERROR;
        struct timespec timeout;
        pid_t tid = getTid();
        id = mJune->Init();
        ALOGI("June Client[%d] Init tid[%d, 0x%x]", id, tid, tid);

        mImem = mJune->NewControlBlock(id, SERVER_TO_CLIENT);
        if (mImem == 0) {
            ALOGE("June Client[%d] failed to control", id);
            mJune->DeInit(id);
            return -1;
        }
        mJcblk = static_cast<June_control_block_t *>(mImem->pointer());
        if (mJcblk->magic != JUNE_CTL_MAGIC) {
            ALOGE("June Client[%d] failed to control magic key mismatch", id);
            mJune->DeInit(id);
            return -1;
        }

        if (mJcblk->mDirection == SERVER_TO_CLIENT) {
            while(!exitPending()) {
                int32_t flags = android_atomic_and(~JUNE_CTL_FLAG_DATA_PUSH, &mJcblk->mFlags);
                if (flags & JUNE_CTL_FLAG_EXIT) {
                    ALOGV("Receive Event EXIT");
                    status = NO_ERROR;
                    break;
                }

                if (flags & JUNE_CTL_FLAG_UNKNOWN_ERR) {
                    ALOGV("Receive Event UNKNOWN_ERROR");
                    status = UNKNOWN_ERROR;
                    break;
                }

                timeout.tv_sec = 2;
                timeout.tv_nsec = 0;

                if (android_atomic_release_load(&mJcblk->stack_lock) == 1) {
                    ALOGI("June Client[%d] structure locking", id);
                    usleep(10000); // 10 msec
                    continue;
                }

                android_atomic_inc(&mJcblk->stack_lock);

                if (android_atomic_release_load(&mJcblk->stack_cnt)) {
                    while(android_atomic_release_load(&mJcblk->stack_cnt) > 0) {
                        ALOGI("June Clinet[%d] pop data[%d]=[%d]", id, mJcblk->stack_cnt-1,
                                                mJcblk->stack[mJcblk->stack_cnt-1]);
                        android_atomic_dec(&mJcblk->stack_cnt);
                    }
                }

                android_atomic_dec(&mJcblk->stack_lock);

                // I was processing all data, so I write the zero to mFutex
                int32_t old = android_atomic_and(~JUNE_CBLK_FUTEX_WAKE, &mJcblk->mFutex);

                // If old value was 0x0, server is not set 0x1(server is not send data)
                // So will wait for server event
                if (!(old & JUNE_CBLK_FUTEX_WAKE)) {
                    int rc;
                    ALOGI("June Client[%d] Wait for futex", id);
                    //The value of the first parameter and the third parameter should be the same.
                    //If not, an error will be returned.
                    int ret = __futex_syscall4(&mJcblk->mFutex, FUTEX_WAIT, old & ~JUNE_CBLK_FUTEX_WAKE, &timeout);
                    ALOGI("June Client[%d] wake up by futex ret %d, wait time %ld.%ld", id, ret, timeout.tv_sec, timeout.tv_nsec);
                    switch (ret) {
                    case 0:             // normal wakeup by server, or by binderDied()
                    case -EWOULDBLOCK:  // benign race condition with server
                    case -EINTR:        // wait was interrupted by signal or other spurious wakeup
                        break;
                    case -ETIMEDOUT:    // time-out expired
                        ALOGE("%s timeout error %d", __func__, ret);
                        break;
                    default:
                        ALOGE("%s unexpected error %d", __func__, ret);
                        status = -ret;
                        break;
                    }
                }
                // Conversely, If the old value was 0x0, server is set 0x1 ((server is not send data)
                // So continue check data..
            }
        } else {
            ALOGW("To Be Announced");
        }

        mJune->DeInit(id);
        ALOGI("June Client[%d] DeInit", id);
        return status;
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

    June_control_block_t *mJcblk;
    sp<IJune> mJune;
    sp<AppDathNotification> mAppDathNotification;
    sp<IMemory> mImem;
    mutable Mutex appLock;
    int id;
    int test_case;
};

int main(int argc, char** argv)
{
    int test_case = 0;

    ALOGV("Main DemoApp\n");
    android::ProcessState::self()->startThreadPool();

    if (argc >= 2) {
        test_case = atoi(argv[1]);
        if (test_case < 0 || test_case > 2)
            test_case = 0;
    }

    sp<DemoApp> app = new DemoApp(test_case);
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

