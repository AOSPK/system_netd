/*
 * Copyright (C) 2018 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <string>

#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "Netd"
#include <log/log.h>

#include <netd_resolv/resolv_stub.h>

struct ResolvStub RESOLV_STUB;

inline constexpr char APEX_LIB64_DIR[] = "/apex/com.android.resolv/lib64";
inline constexpr char APEX_LIB_DIR[] = "/apex/com.android.resolv/lib";
inline constexpr char LIBNAME[] = "libnetd_resolv.so";

template <typename FunctionType>
static void resolvStubInitFunction(void* handle, const char* symbol, FunctionType** stubPtr) {
    void* f = dlsym(handle, symbol);
    if (f == nullptr) {
        ALOGE("Can't find symbol %s in %s", symbol, LIBNAME);
        abort();
    }
    *stubPtr = reinterpret_cast<FunctionType*>(f);
}

int resolv_stub_init() {
    void* netdResolvHandle;

    for (const auto& dir : {APEX_LIB64_DIR, APEX_LIB_DIR}) {
        std::string path = std::string(dir) + "/" + LIBNAME;
        netdResolvHandle = dlopen(path.c_str(), RTLD_NOW);
        if (netdResolvHandle != nullptr) {
            break;
        }
        ALOGW("dlopen(%s) failed: %s", path.c_str(), strerror(errno));
    }

    if (netdResolvHandle == nullptr) {
        ALOGE("Fatal: couldn't open libnetd_resolv");
        abort();
    }

#define STR(x) #x
#define RESOLV_STUB_LOAD_SYMBOL(x) resolvStubInitFunction(netdResolvHandle, STR(x), &RESOLV_STUB.x)
    RESOLV_STUB_LOAD_SYMBOL(android_getaddrinfofornetcontext);
    RESOLV_STUB_LOAD_SYMBOL(android_gethostbyaddrfornetcontext);
    RESOLV_STUB_LOAD_SYMBOL(android_gethostbynamefornetcontext);
    RESOLV_STUB_LOAD_SYMBOL(android_net_res_stats_aggregate);
    RESOLV_STUB_LOAD_SYMBOL(android_net_res_stats_get_info_for_net);
    RESOLV_STUB_LOAD_SYMBOL(android_net_res_stats_get_usable_servers);
    RESOLV_STUB_LOAD_SYMBOL(resolv_delete_cache_for_net);
    RESOLV_STUB_LOAD_SYMBOL(resolv_delete_private_dns_for_net);
    RESOLV_STUB_LOAD_SYMBOL(resolv_get_private_dns_status_for_net);
    RESOLV_STUB_LOAD_SYMBOL(resolv_has_nameservers);
    RESOLV_STUB_LOAD_SYMBOL(resolv_register_private_dns_callback);
    RESOLV_STUB_LOAD_SYMBOL(resolv_res_nsend);
    RESOLV_STUB_LOAD_SYMBOL(resolv_set_nameservers_for_net);
    RESOLV_STUB_LOAD_SYMBOL(resolv_set_private_dns_for_net);
#undef RESOLV_STUB_LOAD_SYMBOL
#undef STR

    return 0;
}