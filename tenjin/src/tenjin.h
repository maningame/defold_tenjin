#pragma once
#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID)

extern void Tenjin_Init(const char*api_key, bool gdpr_consent, const char*app_store);
extern void Tenjin_Connect();
extern void Tenjin_SetCacheEventSetting(bool is_enabled);
extern const char* Tenjin_GetAnalyticsInstallationId();
extern const char* Tenjin_GetAdvertisingId();
extern void Tenjin_SetCustomerUserId(const char*user_id);
extern void Tenjin_CustomEvent(const char*event_name);
extern void Tenjin_CustomEventWithValue(const char*event_name, const char*event_value);
extern void Tenjin_PurchaseEvent(const char*product_id, const char*currency_code, const int quantity, const double price, const char*transaction_id, const char*receipt, const char*signature);
extern void Tenjin_PurchaseEventNonValidated(const char*product_id, const char*currency_code, const int quantity, const double price);

#endif