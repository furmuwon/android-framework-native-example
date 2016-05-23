LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_juneserver.cpp 

LOCAL_SHARED_LIBRARIES := \
	libnjune \
	libutils \
	liblog \
	libbinder

LOCAL_C_INCLUDES := \
    frameworks/june	

LOCAL_MODULE:= juneserver

include $(BUILD_EXECUTABLE)
