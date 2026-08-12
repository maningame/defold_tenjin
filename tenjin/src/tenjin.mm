#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_IOS)
#include "tenjin.h"
#include "TenjinSDK.h"

void Tenjin_Init(const char* api_key, bool gdpr_consent, const char* app_store) {
    // The App Store is the only distribution channel on iOS, app_store is Android only.
    [TenjinSDK init:[NSString stringWithUTF8String:api_key]];

    if (gdpr_consent) {
        [TenjinSDK optIn];
    } else {
        [TenjinSDK optOut];
    }

    [TenjinSDK connect];
}

void Tenjin_Connect() {
    [TenjinSDK connect];
}

void Tenjin_SetCacheEventSetting(bool is_enabled) {
    dmLogWarning("set_cache_event_setting is not supported on iOS");
}

const char* Tenjin_GetAnalyticsInstallationId() {
    dmLogWarning("get_analytics_installation_id is not supported on iOS");
    return "";
}

const char* Tenjin_GetAdvertisingId() {
    dmLogWarning("get_advertising_id is not supported on iOS");
    return "";
}

void Tenjin_SetCustomerUserId(const char* user_id) {
    [TenjinSDK setCustomerUserId:[NSString stringWithUTF8String:user_id]];
}

void Tenjin_CustomEvent(const char* event_name) {
    [TenjinSDK sendEventWithName:[NSString stringWithUTF8String:event_name]]; 
}

void Tenjin_CustomEventWithValue(const char* event_name, const char* event_value) {
    [TenjinSDK sendEventWithName:[NSString stringWithUTF8String:event_name] andEventValue:[NSString stringWithUTF8String:event_value]]; 
}

void Tenjin_PurchaseEvent(const char* product_id, const char* currency_code, const int quantity, const double price, const char* transaction_id, const char* receipt, const char* signature) {
    NSString* productName = [NSString stringWithUTF8String:product_id];
    NSString* currencyCode = [NSString stringWithUTF8String:currency_code];
    NSDecimalNumber* unitPrice = [NSDecimalNumber decimalNumberWithDecimal:[[NSNumber numberWithDouble:price] decimalValue]];
    NSString* transactionId = [NSString stringWithUTF8String:transaction_id];
    NSString* base64Receipt = [NSString stringWithUTF8String:receipt];

    // Pass the App Store transaction id + base64 receipt so Tenjin can verify the
    // purchase; verified purchases are the ones that show in the dashboard.
    // Fall back to the unverified call when no receipt is supplied.
    if (transactionId.length > 0 && base64Receipt.length > 0) {
        [TenjinSDK transactionWithProductName: productName
                    andCurrencyCode: currencyCode
                    andQuantity: (NSInteger) quantity
                    andUnitPrice: unitPrice
                    andTransactionId: transactionId
                    andBase64Receipt: base64Receipt];
    } else {
        [TenjinSDK transactionWithProductName: productName
                    andCurrencyCode: currencyCode
                    andQuantity: (NSInteger) quantity
                    andUnitPrice: unitPrice];
    }
}

void Tenjin_PurchaseEventNonValidated(const char* product_id, const char* currency_code, const int quantity, const double price) {
    // Purchases on iOS always go through the App Store, so the S2S
    // non-validated path is Android only.
    dmLogWarning("purchase_event_non_validated is not supported on iOS");
}

#endif
