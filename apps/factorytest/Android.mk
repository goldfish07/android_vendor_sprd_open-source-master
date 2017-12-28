ifneq ($(TARGET_SIMULATOR),true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
BOARD_PRODUCT_NAME:=$(TARGET_BOARD)
$(warning BOARD_PRODUCT_NAME = $(BOARD_PRODUCT_NAME))
commands_local_path := $(LOCAL_PATH)
ifeq ($(strip $(BOARD_USE_EMMC)),true)
LOCAL_CFLAGS += -DCONFIG_EMMC
endif
ifeq ($(strip $(TARGET_USERIMAGES_USE_UBIFS)),true)
LOCAL_CFLAGS += -DCONFIG_NAND
endif
ifeq ($(PRODUCT_WIFI_DEVICE),bcm)
LOCAL_CFLAGS += -DBCM
endif

#add for support 9830 4mod in diffrent modem 
ifeq ($(PRODUCT_MODEM_COUNT),lf)
LOCAL_CFLAGS += -DLF
endif

#add for support 9830 4mod in diffrent lsensor 
$(warning $(PRODUCT_LSENSOR_2182))
ifeq ($(PRODUCT_LSENSOR_2182),epl)
LOCAL_CFLAGS += -DEPL
$(warning "****************************")
endif

#add for support 9830 4mod in diffrent Tp
ifeq ($(PRODUCT_TP_MSG2138),msg)
LOCAL_CFLAGS += -DMSG
endif


LOCAL_C_INCLUDES    +=  $(LOCAL_PATH) \
			$(LOCAL_PATH)/minui \
			$(LOCAL_PATH)/common	\
		   	$(LOCAL_PATH)/testitem  \
			$(LOCAL_PATH)/res

LOCAL_C_INCLUDES    +=  vendor/sprd/open-source/apps/engmode
LOCAL_C_INCLUDES    +=  external/sqlite/dist/
LOCAL_C_INCLUDES    +=  vendor/sprd/open-source/libs/libatchannel/
LOCAL_C_INCLUDES    +=  vendor/sprd/open-source/libs/audio/nv_exchange/
LOCAL_C_INCLUDES    +=  vendor/sprd/open-source/libs/audio/
LOCAL_C_INCLUDES    +=  vendor/sprd/partner/trout/libfm/
LOCAL_C_INCLUDES    +=  external/tinyalsa/include
LOCAL_C_INCLUDES    +=  hardware/libhardware/include
LOCAL_C_INCLUDES    +=  $(TARGET_OUT_INTERMEDIATES)/KERNEL/source/include/uapi/mtd/


LOCAL_STATIC_LIBRARIES += libtinyxml2
LOCAL_STATIC_LIBRARIES += libftminui libpixelflinger_static libcutils
LOCAL_STATIC_LIBRARIES += libstdc++ libz liblog
LOCAL_SHARED_LIBRARIES := libsqlite  libbinder libutils libtinyalsa libhardware libdl\
						  libhardware_legacy \
						  libbt-vendor

#libasound libeng_audio_mode
LOCAL_SRC_FILES := factorytest.c  \
		eng_tok.c \
		ui.c \
		ui_touch.c \
		camera.cpp


LOCAL_SRC_FILES += $(call all-c-files-under, testitem)
LOCAL_MODULE := factorytest
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
#include $(call all-makefiles-under,$(LOCAL_PATH))
include $(commands_local_path)/minui/Android.mk
endif

