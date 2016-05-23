LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main_june_app.cpp 

LOCAL_SHARED_LIBRARIES := \
	libbinder \
	libpjune \
	libutils \
	liblog

LOCAL_C_INCLUDES := \
    frameworks/june

LOCAL_MODULE:= june_app_demo

include $(BUILD_EXECUTABLE)
