package com.anvil.tenjin;

import android.app.Activity;
import android.content.Context;
import android.os.Build;
import android.util.Log;

import com.google.android.gms.ads.identifier.AdvertisingIdClient;
import com.tenjin.android.TenjinSDK;
import com.tenjin.android.config.TenjinConsts;

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
    private static volatile String advertisingId = "";

    public static final void Init(Activity appActivity, String apiKey, boolean consent) {
        Tenjin.apiKey = apiKey;
        Tenjin.appContext = appActivity.getApplicationContext();

        tenjinInstance = TenjinSDK.getInstance(appActivity, apiKey);

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
                try {
                    advertisingId = AdvertisingIdClient.getAdvertisingIdInfo(appContext).getId();
                } catch (Exception e) {
                    Log.w(TAG, "Failed to fetch advertising id: " + e);
                }
            }
        }).start();
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

                    StringBuilder url = new StringBuilder(PURCHASE_URL);
                    url.append("?api_key=").append(encode(apiKey));
                    url.append("&analytics_installation_id=").append(encode(tenjinInstance.getAnalyticsInstallationId()));
                    url.append("&advertising_id=").append(encode(advertisingId));
                    url.append("&bundle_id=").append(encode(appContext.getPackageName()));
                    url.append("&platform=android");
                    url.append("&os_version=").append(Build.VERSION.SDK_INT);
                    url.append("&app_version=").append(encode(appVersion));
                    url.append("&sdk_version=").append(encode(TenjinConsts.sdkVersion));
                    url.append("&product_id=").append(encode(productId));
                    url.append("&price=").append(unitPrice);
                    url.append("&quantity=").append(quantity);
                    url.append("&currency=").append(encode(currencyCode));
                    url.append("&os_version_release=").append(encode(Build.VERSION.RELEASE));
                    url.append("&locale=").append(encode(Locale.getDefault().toLanguageTag()));
                    url.append("&build_id=").append(encode(Build.ID));
                    url.append("&device_model=").append(encode(Build.MODEL));

                    HttpURLConnection connection = (HttpURLConnection) new URL(url.toString()).openConnection();
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

    private static String encode(String value) throws Exception {
        return URLEncoder.encode(value == null ? "" : value, "UTF-8");
    }

}
