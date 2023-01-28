#include <jni.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <android/log.h>

//
// Created by vlad on 28.01.2023.
//

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_INFO, "sfml-activity", __VA_ARGS__));

void* loadLibrary(const char* libraryName, JNIEnv* lJNIEnv, jobject& objectActivityInfo)
{
    // Find out the absolute path of the library
    jclass   ClassActivityInfo     = lJNIEnv->FindClass("android/content/pm/ActivityInfo");
    jfieldID FieldApplicationInfo  = lJNIEnv->GetFieldID(ClassActivityInfo,
                                                         "applicationInfo",
                                                         "Landroid/content/pm/ApplicationInfo;");
    jobject  ObjectApplicationInfo = lJNIEnv->GetObjectField(objectActivityInfo, FieldApplicationInfo);

    jclass   ClassApplicationInfo  = lJNIEnv->FindClass("android/content/pm/ApplicationInfo");
    jfieldID FieldNativeLibraryDir = lJNIEnv->GetFieldID(ClassApplicationInfo, "nativeLibraryDir", "Ljava/lang/String;");

    jobject ObjectDirPath = lJNIEnv->GetObjectField(ObjectApplicationInfo, FieldNativeLibraryDir);

    jclass    ClassSystem                = lJNIEnv->FindClass("java/lang/System");
    jmethodID StaticMethodMapLibraryName = lJNIEnv->GetStaticMethodID(ClassSystem,
                                                                      "mapLibraryName",
                                                                      "(Ljava/lang/String;)Ljava/lang/String;");

    jstring LibNameObject = lJNIEnv->NewStringUTF(libraryName);
    jobject ObjectName    = lJNIEnv->CallStaticObjectMethod(ClassSystem, StaticMethodMapLibraryName, LibNameObject);

    jclass    ClassFile       = lJNIEnv->FindClass("java/io/File");
    jmethodID FileConstructor = lJNIEnv->GetMethodID(ClassFile, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
    jobject   ObjectFile      = lJNIEnv->NewObject(ClassFile, FileConstructor, ObjectDirPath, ObjectName);

    // Get the library absolute path and convert it
    jmethodID   MethodGetPath   = lJNIEnv->GetMethodID(ClassFile, "getPath", "()Ljava/lang/String;");
    jstring     javaLibraryPath = static_cast<jstring>(lJNIEnv->CallObjectMethod(ObjectFile, MethodGetPath));
    const char* libraryPath     = lJNIEnv->GetStringUTFChars(javaLibraryPath, nullptr);

    // Manually load the library
    void* handle = dlopen(libraryPath, RTLD_NOW | RTLD_GLOBAL);
    if (!handle)
    {
        LOGE("dlopen(\"%s\"): %s", libraryPath, dlerror());
        exit(1);
    }

    // Release the Java string
    lJNIEnv->ReleaseStringUTFChars(javaLibraryPath, libraryPath);

    return handle;
}

extern "C"
JNIEXPORT void JNICALL
Java_org_sfmldev_android_MainActivity_loadSfml(JNIEnv *env, jobject thiz) {

    loadLibrary("openal", env, thiz);
}
