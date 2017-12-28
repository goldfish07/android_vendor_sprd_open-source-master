LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := capture_oprofile.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_SRC_FILES := capture_oprofile.sh
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := capture_ftrace.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_SRC_FILES := capture_ftrace.sh
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := capture_blktrace.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_SRC_FILES := capture_blktrace.sh
include $(BUILD_PREBUILT)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := profiledaemon.c
LOCAL_MODULE := libprofiledaemon
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := blktrace.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

CUSTOM_MODULES += capture_oprofile.sh
CUSTOM_MODULES += capture_ftrace.sh
CUSTOM_MODULES += capture_blktrace.sh
CUSTOM_MODULES += blktrace.conf
