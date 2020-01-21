# Este archivo es jni/Android.mk
# No se chequea TARGET_ARCH_ABI, si o si tiene que ser armeabi-v7a

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

##debug
#LOCAL_CFLAGS			:= -Wall -ggdb -std=c99 -pg -O0 -v # -O3 -mfpu=neon -ftree-vectorize
#final
LOCAL_CFLAGS			:= -Wall -std=c99 -O3 -mfpu=neon -ftree-vectorize
LOCAL_LDLIBS			:= -llog
LOCAL_ARM_MODE			:= arm
LOCAL_ARM_NEON			:= true
LOCAL_MODULE			:= interfazParaJava
LOCAL_STATIC_LIBRARIES	:= android-ndk-profiler
# Codigo fuente 
LOCAL_SRC_FILES			:= interfazParaJava.neon.c realdft.neon.c funciones.neon.c ecualizador.neon.c analizador.neon.c funcionesASM.neon.s

include $(BUILD_SHARED_LIBRARY)


# Profiler
$(call import-module,android-ndk-profiler)
