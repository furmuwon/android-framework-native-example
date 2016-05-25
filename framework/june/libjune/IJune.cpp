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

#define LOG_TAG "IJune"
#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>

#include <include/IJune.h>

namespace android {

enum {
    JUNE_INIT = IBinder::FIRST_CALL_TRANSACTION,
    JUNE_GET_DESC,
    JUNE_ALLOC_MEMORY,
    JUNE_DUMP_MEMORY,
    JUNE_DEINIT,
};

class BpJune : public BpInterface<IJune>
{
public:
    BpJune(const sp<IBinder>& impl)
        : BpInterface<IJune>(impl)
    {
    }

	virtual int Init(void)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IJune::getInterfaceDescriptor());
		remote()->transact(JUNE_INIT, data, &reply);
		return (int)reply.readInt32();
	}

	virtual String8 GetJuneServiceDesc(void)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IJune::getInterfaceDescriptor());
        remote()->transact(JUNE_GET_DESC, data, &reply);
        return reply.readString8();
	}

    virtual sp<IMemory> AllocMemory(int id, int size)
    {
        Parcel data, reply;
        sp<IMemory> iMem = 0;
        data.writeInterfaceToken(IJune::getInterfaceDescriptor());
        data.writeInt32((int32_t) id);
        data.writeInt32((int32_t) size);
        status_t status = remote()->transact(JUNE_ALLOC_MEMORY, data, &reply);
        if (status == NO_ERROR) {
            iMem = interface_cast<IMemory>(reply.readStrongBinder());
            return iMem;
        } else
            return 0;
    }

    virtual status_t DumpMemory(int id, int size)
    {
        Parcel data, reply;
        if (size <= 0)
            return BAD_VALUE;
        data.writeInterfaceToken(IJune::getInterfaceDescriptor());
        data.writeInt32((int32_t) id);
        data.writeInt32((int32_t) size);
        remote()->transact(JUNE_DUMP_MEMORY, data, &reply);
        return reply.readInt32();
    }

	virtual status_t DeInit(int id)
	{
		Parcel data, reply;
		data.writeInterfaceToken(IJune::getInterfaceDescriptor());
        data.writeInt32((int32_t) id);
        remote()->transact(JUNE_DEINIT, data, &reply);
        return reply.readInt32();
	}

};

#if 1
IMPLEMENT_META_INTERFACE(June, "android.June");
#else
const android::String16 IJune::descriptor("android.June");

const android::String16& IJune::getInterfaceDescriptor() const
{
    return IJune::descriptor;
}

android::sp<IJune> IJune::asInterface(const android::sp<android::IBinder>& obj)
{
    android::sp<IJune> intr;
    if (obj != NULL) {
        intr = static_cast<IJune*>(obj->queryLocalInterface(IJune::descriptor).get());
        if (intr == NULL) {
            intr = new BpJune(obj);
        }
    }
    return intr;
}

IJune::IJune()
{
}

IJune::~IJune()
{
}
#endif

// ----------------------------------------------------------------------
status_t BnJune::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
		case JUNE_INIT: {
			CHECK_INTERFACE(IJune, data, reply);
			reply->writeInt32(Init());
			return NO_ERROR;
		} break;

		case JUNE_GET_DESC: {
			CHECK_INTERFACE(IJune, data, reply);
			reply->writeString8(GetJuneServiceDesc());
			return NO_ERROR;
		} break;

        case JUNE_ALLOC_MEMORY: {
            sp<IMemory> iMem;
            int id, size;
            CHECK_INTERFACE(IJune, data, reply);
            id = data.readInt32();
            size = data.readInt32();
            iMem = AllocMemory(id, size);
            if (iMem == 0)
                return NO_MEMORY;
            else
                reply->writeStrongBinder(iMem->asBinder());
            return NO_ERROR;
        } break;

        case JUNE_DUMP_MEMORY: {
            int id, size;
            CHECK_INTERFACE(IJune, data, reply);
            id = data.readInt32();
            size = data.readInt32();
            reply->writeInt32(DumpMemory(id, size));
            return NO_ERROR;
        } break;

        case JUNE_DEINIT: {
			int id;
			CHECK_INTERFACE(IJune, data, reply);
			id = data.readInt32();
			reply->writeInt32(DeInit(id));
			return NO_ERROR;
		} break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------------

}; // namespace android
