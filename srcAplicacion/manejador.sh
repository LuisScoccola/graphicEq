#!/bin/bash

SDK=$HOME/android/adt-bundle-linux-x86_64-20131030/sdk
NDK=$HOME/android/android-ndk-r9c

function limpiar {
    echo "------------------------- Limpiado Todo"
    rm -r bin/* libs/* obj/* gen/*
}

function makear {
    echo "------------------------- Limpiado jni"
    $NDK/ndk-build clean

    echo "------------------------- Udateamos proyecto"
    $SDK/tools/android update project --name Ecualizador  --subprojects --target 1 --path ./

    echo "------------------------- Makeando jni"
    $NDK/ndk-build all

    echo "------------------------- Creando dump de assembly"
    $NDK/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin/arm-linux-androideabi-objdump -d libs/armeabi-v7a/libinterfazParaJava.so > ../disassembly/todoElCodigoASM.txt

    echo "------------------------- Limpiando proyecto"
    ant clean

    echo "------------------------- Buldeando proyecto"
    ant debug
}

function correr {
    echo "------------------------- Instalando"
    $SDK/platform-tools/adb uninstall com.ecualizador
    $SDK/platform-tools/adb install bin/Ecualizador-debug.apk

    echo "------------------------- Corriendo"
    $SDK/platform-tools/adb shell am start -n com.ecualizador/.Inicio
}

function traerProfile {
    echo "------------------------- Trayendo profile"
    $SDK/platform-tools/adb pull /sdcard/gmon.out
}

function mostrarProfile {
    echo "------------------------- Aqui el profile"
    $NDK/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gprof obj/local/armeabi-v7a/libinterfazParaJava.so
}

function logcat {
    echo "------------------------- Logcat aqui va!"
    $SDK/platform-tools/adb logcat *:V
}

function emulador {
    echo "-------------------------- Emulador"
    $SDK/tools/emulator -avd org2
}

function consola {
    echo "------------------------- Nos conectamos al aparato"
    $SDK/platform-tools/adb shell
}




cd Ecualizador

if [[ $1 == limpiar ]] ; then
    limpiar
fi

if [[ $1 == make ]] ; then
    makear
fi

if [[ $1 == correr ]] ; then
    correr
fi

if [[ $1 == ambas ]] ; then
    makear
    correr
fi

if [[ $1 == traer ]] ; then
    traerProfile
fi

if [[ $1 == mostrar ]] ; then
    mostrarProfile
fi

if [[ $1 == logcat ]] ; then
    logcat
fi

if [[ $1 == emulador ]] ; then
    emulador
fi

if [[ $1 == consola ]] ; then
    consola
fi
