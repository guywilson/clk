#include <node_api.h>

#ifndef _INCL_NODE_EXPORT
#define _INCL_NODE_EXPORT

napi_value cloak_api_version_wrapper(napi_env env, napi_callback_info info);
napi_value cloak_api_filelength_wrapper(napi_env env, napi_callback_info info);
napi_value cloak_api_hide_wrapper(napi_env env, napi_callback_info info);
napi_value cloak_api_reveal_wrapper(napi_env env, napi_callback_info info);
        
#endif
