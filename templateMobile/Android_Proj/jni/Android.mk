LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := AddInNative
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS += -fexceptions

LOCAL_C_INCLUDES += $(LOCAL_PATH) $(LOCAL_PATH)/../../../../include
LOCAL_SRC_FILES := \
    jnienv.cpp \
    ../../AddInNative.cpp \
    ../../ConversionWchar.cpp \
    ../../dllmain.cpp \
    ../../stdafx.cpp 

include $(BUILD_SHARED_LIBRARY)
