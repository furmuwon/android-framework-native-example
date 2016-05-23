LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    IJune.cpp 

LOCAL_SHARED_LIBRARIES := \
	liblog libcutils libutils libbinder

LOCAL_MODULE:= libpjune

LOCAL_C_INCLUDES := frameworks/june

include $(BUILD_SHARED_LIBRARY)
