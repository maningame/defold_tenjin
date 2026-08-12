package com.anvil.tenjin;

import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.util.Log;

import com.google.android.gms.ads.identifier.AdvertisingIdClient;
import com.tenjin.android.TenjinSDK;

import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;
import java.util.Locale;

class Tenjin {

    private static final String TAG = "Tenjin";
    private static final String PURCHASE_URL = "https://track.tenjin.com/v0/purchase";

    private static TenjinSDK tenjinInstance;
    private static Context appContext;
    private static String apiKey;
    private static TenjinSDK.AppStoreType appStoreType = TenjinSDK.AppStoreType.googleplay;
    private static volatile String advertisingId = "";

    public static final void Init(Activity appActivity, String apiKey, boolean consent, String appStore) {
        Tenjin.apiKey = apiKey;
        Tenjin.appContext = appActivity.getApplicationContext();
        Tenjin.appStoreType = parseAppStore(appStore);

        tenjinInstance = TenjinSDK.getInstance(appActivity, apiKey);

        // The app store must be set between getInstance and connect. It duplicates the
        // TENJIN_APP_STORE manifest meta-data on purpose: if the manifest merge ever
        // breaks, the store silently falls back to unspecified and attribution rots.
        tenjinInstance.setAppStore(appStoreType);

        if (consent) {
            tenjinInstance.optIn();
        }
        else {
            tenjinInstance.optOut();
        }

        tenjinInstance.connect();

        // AdvertisingIdClient must not be called on the main thread
        new Thread(new Runnable() {
            public void run() {
                advertisingId = fetchAdvertisingId();
            }
        }).start();
    }

    // The SDK requires connect on every onResume, not only on the first app open,
    // otherwise sessions and re-engagement are lost.
    public static final void Connect() {
        if (tenjinInstance == null) {
            Log.w(TAG, "Connect called before Init, ignored");
            return;
        }

        tenjinInstance.setAppStore(appStoreType);
        tenjinInstance.connect();
    }

    public static final void SetCacheEventSetting(boolean isEnabled) {
        if (tenjinInstance == null) {
            Log.w(TAG, "SetCacheEventSetting called before Init, ignored");
            return;
        }

        tenjinInstance.setCacheEventSetting(isEnabled);
    }

    public static final String GetAnalyticsInstallationId() {
        if (tenjinInstance == null) {
            Log.w(TAG, "GetAnalyticsInstallationId called before Init, ignored");
            return "";
        }

        String installationId = tenjinInstance.getAnalyticsInstallationId();

        return installationId == null ? "" : installationId;
    }

    // The GAID is fetched in the background during Init, so this returns an empty
    // string for the first moments of the session. Never blocks: fetching it on the
    // caller's thread would freeze the main thread.
    public static final String GetAdvertisingId() {
        return advertisingId;
    }

    public static final void SetCustomerUserId(String userId) {
        tenjinInstance.setCustomerUserId(userId);
    }

    public static final void CustomEvent(String eventName) {
        tenjinInstance.eventWithName(eventName);
    }

    public static final void CustomEventWithValue(String eventName, String eventValue) {
        tenjinInstance.eventWithNameAndValue(eventName, eventValue);
    }

    public static final void PurchaseEvent(String productId, String currencyCode, int quantity, double unitPrice, String purchaseData, String dataSignature) {
        if (purchaseData != null && !purchaseData.isEmpty() && dataSignature != null && !dataSignature.isEmpty()) {
            tenjinInstance.transaction(productId, currencyCode, quantity, unitPrice, purchaseData, dataSignature);
        } else {
            tenjinInstance.transaction(productId, currencyCode, quantity, unitPrice);
        }
    }

    // Reports a purchase made outside of Google Play (alternative store or custom
    // billing) straight to the Tenjin REST API. No receipt validation happens: the
    // reported price is trusted as-is. analytics_installation_id ties the purchase
    // to the user's attribution, so this must be called after Init.
    public static final void PurchaseEventNonValidated(final String productId, final String currencyCode, final int quantity, final double unitPrice) {
        if (tenjinInstance == null) {
            Log.w(TAG, "PurchaseEventNonValidated called before Init, ignored");
            return;
        }
        new Thread(new Runnable() {
            public void run() {
                try {
                    String appVersion = appContext.getPackageManager().getPackageInfo(appContext.getPackageName(), 0).versionName;
                    String installationId = GetAnalyticsInstallationId();
                    String adId = ensureAdvertisingId();

                    if (installationId.isEmpty()) {
                        Log.w(TAG, "S2S purchase has no analytics_installation_id, it cannot be attributed to an install");
                    }

                    if (adId.isEmpty()) {
                        Log.w(TAG, "S2S purchase has no advertising_id, attribution falls back to ip matching");
                    }

                    StringBuilder params = new StringBuilder();
                    params.append("analytics_installation_id=").append(encode(installationId));
                    params.append("&advertising_id=").append(encode(adId));
                    params.append("&customer_user_id=").append(encode(tenjinInstance.getCustomerUserId()));
                    params.append("&bundle_id=").append(encode(appContext.getPackageName()));
                    params.append("&platform=android");
                    params.append("&os_version=").append(Build.VERSION.SDK_INT);
                    params.append("&app_version=").append(encode(appVersion));
                    // Tenjin support requires the literal "server" for S2S purchases,
                    // not the native SDK version
                    params.append("&sdk_version=server");
                    params.append("&product_id=").append(encode(productId));
                    params.append("&price=").append(unitPrice);
                    params.append("&quantity=").append(quantity);
                    params.append("&currency=").append(encode(currencyCode));
                    params.append("&os_version_release=").append(encode(Build.VERSION.RELEASE));
                    params.append("&locale=").append(encode(Locale.getDefault().toLanguageTag()));
                    params.append("&build_id=").append(encode(Build.ID));
                    params.append("&device_model=").append(encode(Build.MODEL));

                    // api_key is kept out of the log on purpose
                    Log.i(TAG, "S2S purchase: " + PURCHASE_URL + "?" + params);

                    String url = PURCHASE_URL + "?api_key=" + encode(apiKey) + "&" + params;

                    HttpURLConnection connection = (HttpURLConnection) new URL(url).openConnection();
                    try {
                        connection.setRequestMethod("POST");
                        connection.setFixedLengthStreamingMode(0);
                        int code = connection.getResponseCode();
                        if (code >= 200 && code < 300) {
                            Log.i(TAG, "S2S purchase sent, response " + code);
                        } else {
                            Log.w(TAG, "S2S purchase failed, response " + code);
                        }
                    } finally {
                        connection.disconnect();
                    }
                } catch (Exception e) {
                    Log.w(TAG, "S2S purchase failed: " + e);
                }
            }
        }).start();
    }

    private static TenjinSDK.AppStoreType parseAppStore(String appStore) {
        if ("other".equals(appStore)) {
            return TenjinSDK.AppStoreType.other;
        }

        if ("amazon".equals(appStore)) {
            return TenjinSDK.AppStoreType.amazon;
        }

        if (!"googleplay".equals(appStore)) {
            Log.w(TAG, "Unknown app store '" + appStore + "', falling back to googleplay");
        }

        return TenjinSDK.AppStoreType.googleplay;
    }

    // Must not be called on the main thread
    private static String ensureAdvertisingId() {
        if (advertisingId.isEmpty()) {
            advertisingId = fetchAdvertisingId();
        }

        return advertisingId;
    }

    private static String fetchAdvertisingId() {
        try {
            String id = AdvertisingIdClient.getAdvertisingIdInfo(appContext).getId();

            return id == null ? "" : id;
        } catch (Exception e) {
            Log.w(TAG, "Failed to fetch advertising id: " + e);

            return "";
        }
    }

    private static String encode(String value) throws Exception {
        return URLEncoder.encode(value == null ? "" : value, "UTF-8");
    }

}
