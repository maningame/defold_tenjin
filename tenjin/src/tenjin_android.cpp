#if defined(DM_PLATFORM_ANDROID)
#include <dmsdk/sdk.h>
#include <string.h>
#include "tenjin.h"

static char g_AnalyticsInstallationId[128] = { 0 };

static JNIEnv* Attach()
{
    JNIEnv* env;
    JavaVM* vm = dmGraphics::GetNativeAndroidJavaVM();
    vm->AttachCurrentThread(&env, NULL);
    return env;
}

static bool Detach(JNIEnv* env)
{
    bool exception = (bool) env->ExceptionCheck();
    env->ExceptionClear();
    JavaVM* vm = dmGraphics::GetNativeAndroidJavaVM();
    vm->DetachCurrentThread();
    return !exception;
}

namespace {
    struct AttachScope
    {
        JNIEnv* m_Env;
        AttachScope() : m_Env(Attach())
        {
        }
        ~AttachScope()
        {
            Detach(m_Env);
        }
    };
}

static jclass GetClass(JNIEnv* env, const char* classname)
{
    jclass activity_class = env->FindClass("android/app/NativeActivity");
    jmethodID get_class_loader = env->GetMethodID(activity_class,"getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject cls = env->CallObjectMethod(dmGraphics::GetNativeAndroidActivity(), get_class_loader);
    jclass class_loader = env->FindClass("java/lang/ClassLoader");
    jmethodID find_class = env->GetMethodID(class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    jstring str_class_name = env->NewStringUTF(classname);
    jclass outcls = (jclass)env->CallObjectMethod(cls, find_class, str_class_name);
    env->DeleteLocalRef(str_class_name);
    return outcls;
}

void Tenjin_Init(const char*api_key, bool gdpr_consent, const char*app_store)
{
    AttachScope attachscope;
    JNIEnv* env = attachscope.m_Env;

    jclass cls = GetClass(env, "com.anvil.tenjin.Tenjin");
    jmethodID method = env->GetStaticMethodID(cls, "Init", "(Landroid/app/Activity;Ljava/lang/String;ZLjava/lang/String;)V");

    jstring key = env->NewStringUTF(api_key);
    jstring appStore = env->NewStringUTF(app_store);

    env->CallStaticVoidMethod(cls, method, dmGraphics::GetNativeAndroidActivity(), key, gdpr_consent ? JNI_TRUE : JNI_FALSE, appStore);

    env->DeleteLocalRef(key);
    env->DeleteLocalRef(appStore);
}

void Tenjin_Connect()
{
    AttachScope attachscope;
    JNIEnv* env = attachscope.m_Env;

    jclass cls = GetClass(env, "com.anvil.tenjin.Tenjin");
    jmethodID method = env->GetStaticMethodID(cls, "Connect", "()V");

    env->CallStaticVoidMethod(cls, method);
}

void Tenjin_SetCacheEventSetting(bool is_enabled)
{
    AttachScope attachscope;
    JNIEnv* env = attachscope.m_Env;

    jclass cls = GetClass(env, "com.anvil.tenjin.Tenjin");
    jmethodID method = env->GetStaticMethodID(cls, "SetCacheEventSetting", "(Z)V");

    env->CallStaticVoidMethod(cls, method, is_enabled ? JNI_TRUE : JNI_FALSE);
}

const char* Tenjin_GetAnalyticsInstallationId()
{
    AttachScope attachscope;
    JNIEnv* env = attachscope.m_Env;

    g_AnalyticsInstallationId[0] = 0;

    jclass cls = GetClass(env, "com.anvil.tenjin.Tenjin");
    jmethodID method = env->GetStaticMethodID(cls, "GetAnalyticsInstallationId", "()Ljava/lang/String;");

    jstring value = (jstring) env->CallStaticObjectMethod(cls, method);

    if (value != NULL) {
        const char* chars = env->GetStringUTFChars(value, NULL);

        strncpy(g_AnalyticsInstallationId, chars, sizeof(g_AnalyticsInstallationId) - 1);
        g_AnalyticsInstallationId[sizeof(g_AnalyticsInstallationId) - 1] = 0;

        env->ReleaseStringUTFChars(value, chars);
        env->DeleteLocalRef(value);
    }

    return g_AnalyticsInstallationId;
}

void Tenjin_SetCustomerUserId(const char* user_id)
{
    AttachScope attachscope;
    JNIEnv* env = attachscope.m_Env;

    jclass cls = GetClass(env, "com.anvil.tenjin.Tenjin");
    jmethodID method = env->GetStaticMethodID(cls, "SetCustomerUserId", "(Ljava/lang/String;)V");

    jstring userId = env->NewStringUTF(user_id);

    env->CallStaticVoidMethod(cls, method, userId);

    env->DeleteLocalRef(userId);
}

void Tenjin_CustomEvent(const char*event_name)
{
    AttachScope attachscope;
    JNIEnv* env = attachscope.m_Env;

    jclass cls = GetClass(env, "com.anvil.tenjin.Tenjin");
    jmethodID method = env->GetStaticMethodID(cls, "CustomEvent", "(Ljava/lang/String;)V");

    jstring eventName = env->NewStringUTF(event_name);

    env->CallStaticVoidMethod(cls, method, eventName);

    env->DeleteLocalRef(eventName);
}

void Tenjin_CustomEventWithValue(const char*event_name, const char*event_value)
{
    AttachScope attachscope;
    JNIEnv* env = attachscope.m_Env;

    jclass cls = GetClass(env, "com.anvil.tenjin.Tenjin");
    jmethodID method = env->GetStaticMethodID(cls, "CustomEventWithValue", "(Ljava/lang/String;Ljava/lang/String;)V");

    jstring eventName = env->NewStringUTF(event_name);
    jstring eventValue = env->NewStringUTF(event_value);

    env->CallStaticVoidMethod(cls, method, eventName, eventValue);

    env->DeleteLocalRef(eventName);
    env->DeleteLocalRef(eventValue);
}

void Tenjin_PurchaseEvent(const char* product_id, const char* currency_code, const int quantity, const double price, const char* transaction_id, const char* receipt, const char* signature)
{
    AttachScope attachscope;
    JNIEnv* env = attachscope.m_Env;

    jclass cls = GetClass(env, "com.anvil.tenjin.Tenjin");
    jmethodID method = env->GetStaticMethodID(cls, "PurchaseEvent", "(Ljava/lang/String;Ljava/lang/String;IDLjava/lang/String;Ljava/lang/String;)V");

    jstring productId = env->NewStringUTF(product_id);
    jstring currencyCode = env->NewStringUTF(currency_code);
    jstring purchaseData = env->NewStringUTF(receipt);
    jstring dataSignature = env->NewStringUTF(signature);

    env->CallStaticVoidMethod(cls, method, productId, currencyCode, quantity, price, purchaseData, dataSignature);

    env->DeleteLocalRef(productId);
    env->DeleteLocalRef(currencyCode);
    env->DeleteLocalRef(purchaseData);
    env->DeleteLocalRef(dataSignature);
}

void Tenjin_PurchaseEventNonValidated(const char* product_id, const char* currency_code, const int quantity, const double price)
{
    AttachScope attachscope;
    JNIEnv* env = attachscope.m_Env;

    jclass cls = GetClass(env, "com.anvil.tenjin.Tenjin");
    jmethodID method = env->GetStaticMethodID(cls, "PurchaseEventNonValidated", "(Ljava/lang/String;Ljava/lang/String;ID)V");

    jstring productId = env->NewStringUTF(product_id);
    jstring currencyCode = env->NewStringUTF(currency_code);

    env->CallStaticVoidMethod(cls, method, productId, currencyCode, quantity, price);

    env->DeleteLocalRef(productId);
    env->DeleteLocalRef(currencyCode);
}

#endif
