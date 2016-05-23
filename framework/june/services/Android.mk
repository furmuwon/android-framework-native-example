LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    June.cpp

LOCAL_C_INCLUDES := frameworks/june

LOCAL_SHARED_LIBRARIES := \
	libpjune \
    libcutils \
    libutils \
    liblog \
    libbinder

#LOCAL_CFLAGS += -fvisibility=hidden

LOCAL_MODULE:= libnjune

include $(BUILD_SHARED_LIBRARY)