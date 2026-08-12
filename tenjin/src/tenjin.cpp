#define EXTENSION_NAME Tenjin
#define LIB_NAME "Tenjin"
#define MODULE_NAME "tenjin"

#define DLIB_LOG_DOMAIN LIB_NAME
#include <dmsdk/sdk.h>


#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID)
#include "tenjin.h"

static int init(lua_State* L) {
    const char* api_key = luaL_checkstring(L, 1);
    bool gdpr_consent = lua_toboolean(L, 2);
    const char* app_store = luaL_optstring(L, 3, "googleplay");

    Tenjin_Init(api_key, gdpr_consent, app_store);
    return 0;
}

static int connect(lua_State* L) {
    DM_LUA_STACK_CHECK(L, 0);
    Tenjin_Connect();
    return 0;
}

static int set_cache_event_setting(lua_State* L) {
    DM_LUA_STACK_CHECK(L, 0);
    bool is_enabled = lua_toboolean(L, 1);
    Tenjin_SetCacheEventSetting(is_enabled);
    return 0;
}

static int get_analytics_installation_id(lua_State* L) {
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushstring(L, Tenjin_GetAnalyticsInstallationId());
    return 1;
}

static int get_advertising_id(lua_State* L) {
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushstring(L, Tenjin_GetAdvertisingId());
    return 1;
}

static int set_customer_user_id(lua_State* L) {
    DM_LUA_STACK_CHECK(L, 0);
    const char* user_id = luaL_checkstring(L, 1);
    Tenjin_SetCustomerUserId(user_id);
    return 0;
}

static int custom_event(lua_State* L) {
    DM_LUA_STACK_CHECK(L, 0);
    const char* event_name = luaL_checkstring(L, 1);
    Tenjin_CustomEvent(event_name);
    return 0;
}

static int custom_event_with_value(lua_State* L) {
    DM_LUA_STACK_CHECK(L, 0);
    const char* event_name = luaL_checkstring(L, 1);
    const char* event_value = luaL_checkstring(L, 2);
    Tenjin_CustomEventWithValue(event_name, event_value);
    return 0;
}

static int purchase_event(lua_State* L) {
    DM_LUA_STACK_CHECK(L, 0);
    const char* product_id = luaL_checkstring(L, 1);
    const char* currency_code = luaL_checkstring(L, 2);
    const int quantity = luaL_checkinteger(L, 3);
    const lua_Number price = luaL_checknumber(L, 4);
    const char* transaction_id = luaL_optstring(L, 5, "");
    const char* receipt = luaL_optstring(L, 6, "");
    const char* signature = luaL_optstring(L, 7, "");

    Tenjin_PurchaseEvent(product_id, currency_code, quantity, price, transaction_id, receipt, signature);
    return 0;
}

static int purchase_event_non_validated(lua_State* L) {
    DM_LUA_STACK_CHECK(L, 0);
    const char* product_id = luaL_checkstring(L, 1);
    const char* currency_code = luaL_checkstring(L, 2);
    const int quantity = luaL_checkinteger(L, 3);
    const lua_Number price = luaL_checknumber(L, 4);

    Tenjin_PurchaseEventNonValidated(product_id, currency_code, quantity, price);
    return 0;
}

static const luaL_reg Module_methods[] =
{
    {"init", init},
    {"connect", connect},
    {"set_cache_event_setting", set_cache_event_setting},
    {"get_analytics_installation_id", get_analytics_installation_id},
    {"get_advertising_id", get_advertising_id},
    {"set_customer_user_id", set_customer_user_id},
    {"custom_event", custom_event},
    {"custom_event_with_value", custom_event_with_value},
    {"purchase_event", purchase_event},
    {"purchase_event_non_validated", purchase_event_non_validated},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeTenjin(dmExtension::AppParams* params)
{
    dmLogWarning("Registered %s Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeTenjin(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeTenjin(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeTenjin(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

#else // unsupported platforms

dmExtension::Result AppInitializeTenjin(dmExtension::AppParams* params)
{
    dmLogWarning("Registered %s Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeTenjin(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeTenjin(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeTenjin(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

#endif


DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializeTenjin, AppFinalizeTenjin, InitializeTenjin, 0, 0, FinalizeTenjin)
