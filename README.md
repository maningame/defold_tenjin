# Tenjin SDK

This is a native extension for [Defold engine](http://www.defold.com) which allows to use [Tenjin SDK](https://docs.tenjin.com/en/).

ATTENTION! Not every API methods are fully supported, only initialization and sending custom events; see LUA Api section below for the list of supported methods.

ATTENTION-2! The native SDK expects `connect` on every Android "OnResume", while `tenjin.init()` is called once from your lua code. Call `tenjin.connect()` on every window focus gained event to keep sessions correct.

Used Tenjin iOS SDK v1.9.1: https://github.com/tenjin/tenjin-ios-sdk and Android SDK v1.9.3: https://github.com/tenjin/tenjin-android-sdk

## Installation

You can use the Tenjin extension in your own project by adding this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/).
Open your game.project file and in the dependencies field under project add:

>https://github.com/MaratGilyazov/def_tenjin/archive/master.zip
## Example
```lua
if tenjin then

  tenjin.init("YOUR_API_KEY", true, sys.get_config_string("tenjin.app_store", "googleplay"))
  tenjin.set_cache_event_setting(true)
  tenjin.custom_event("custom_event")
  tenjin.custom_event_with_value("custom_event", "10")
  tenjin.purchase_event("com.company.inapp", "USD", 1, 0.99)

  window.set_listener(function(self, event)
    if event == window.WINDOW_EVENT_FOCUS_GAINED then
      tenjin.connect()
    end
  end)

end
```

## LUA Api
#### tenjin.init(string api_key, bool gdpr_consent [, string app_store])
Initializes Tenjin, call this before any other calls to Tenjin. This call is enough to track your installs. 
api_key - is your API_KEY from your [Tenjin Organization tab](https://tenjin.io/dashboard/organizations)
gdpr_consent - boolean flag to forward your user's consent regarding sensitive data; send true for "OptIn", if permission are granted
app_store - `googleplay` (default), `amazon` or `other`; describes where the app was **installed from**, not how the user pays. Android only, it duplicates the `TENJIN_APP_STORE` manifest meta-data so a broken manifest merge cannot silently downgrade the store to unspecified. Pass the `app_store` extension setting to keep both in sync.
#### tenjin.connect()
Sends a session and re-runs attribution. The native SDK expects this on every Android "OnResume": call it on `window.WINDOW_EVENT_FOCUS_GAINED`. Without it only the very first session of a process is tracked. On Android the app store is re-applied before connecting, as the SDK documentation prescribes.
#### tenjin.set_cache_event_setting(bool is_enabled)
**Android only.** Lets the SDK queue events that were fired without network and send them later. Call once after `tenjin.init`.
#### tenjin.get_analytics_installation_id()
**Android only.** Returns the SDK installation UUID that ties events to the install, or an empty string if the SDK is not initialized yet. Useful to log next to your own purchase events: without it Tenjin cannot attribute a server-to-server event.
#### tenjin.get_advertising_id()
**Android only.** Returns the advertising ID (GAID), or an empty string while it is not ready — it is fetched in the background during `tenjin.init` and never on the calling thread. Needed when you report purchases to the Tenjin REST API yourself.
#### tenjin.custom_event(string event_name)
Send custom event with event_name
#### tenjin.custom_event_with_value(string event_name, string event_value)
Send custom event with event_name and event_value. 
IMPORTANT! event_value should be send as a string BUT it should contain the integer value!
#### tenjin.purchase_event(string product_id, string currency_code, int quantity, double price [, string transaction_id, string receipt, string signature])
Send purchase event. product_id -> the name or ID of your product; currency_code -> the currency of your unit price; quantity -> the number of products that are counted for this purchase event; price -> the price of each product.

The last three arguments are **optional but required for the purchase to show up in the Tenjin dashboard** — Tenjin only reports purchases it can verify server-side, and verification needs the store receipt. Map the fields from the response Google/Apple IAP returns:

- **Android**: `receipt` -> `original_json` (the purchaseData), `signature` -> `signature`. `transaction_id` is ignored.
- **iOS**: `transaction_id` -> `trans_ident`, `receipt` -> the base64 `receipt`. `signature` is ignored.

If you omit them, the SDK falls back to the old unverified call and purchases will not appear in the dashboard.

Example with a Google IAP response:
```lua
-- response is the table returned by the iap extension on Android
tenjin.purchase_event(response.ident, "USD", 1, 0.99, "", response.original_json, response.signature)
```
#### tenjin.purchase_event_non_validated(string product_id, string currency_code, int quantity, double price)
**Android only.** Send a purchase made outside of Google Play (alternative app stores, custom billing) to Tenjin. Since there is no Google receipt to verify, the purchase is reported server-to-server via `POST https://track.tenjin.com/v0/purchase` and the price is trusted as-is.

The request includes `analytics_installation_id` and `customer_user_id` taken from the SDK, which tie the purchase to the user's attribution and to your own user id in the raw data export — so call this only after `tenjin.init`, and set the customer user id before the purchase. The advertising ID (GAID) is fetched in the background during `tenjin.init`, and fetched on the spot if it is not ready yet. The request is fired once from a background thread; the sent parameters (without `api_key`), the missing identifiers and the response code are logged under the `Tenjin` tag, there are no retries.

For builds distributed outside of Google Play also set the `app_store` extension setting (`amazon` or `other`) in game.project.

```lua
tenjin.purchase_event_non_validated("com.company.inapp", "USD", 1, 0.99)
```

## SDK support level
Some APIs are not supported in this version of extention, see the full list of native SDK methods in the [Tenjin iOS SDK repo](https://github.com/tenjin/tenjin-ios-sdk) and [Tenjin Andoir SDK repo](ttps://github.com/tenjin/tenjin-android-sdk) 

## Testing
See an official documentation describing how to test your integration: https://docs.tenjin.com/en/send-events/#check

Feel free to push a Pull Request with other features implementation.