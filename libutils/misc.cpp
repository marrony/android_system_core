/*
 * Copyright (C) 2005 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "misc"

//
// Miscellaneous utility functions.
//
#include <utils/misc.h>
#include <utils/Log.h>

#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#if defined(HAVE_PTHREADS)
# include <pthread.h>
#endif

#include <utils/Vector.h>

using namespace android;

namespace android {

struct sysprop_change_callback_info {
    sysprop_change_callback callback;
    int priority;
};

#if defined(HAVE_PTHREADS)
static pthread_mutex_t gSyspropMutex = PTHREAD_MUTEX_INITIALIZER;
static Vector<sysprop_change_callback_info>* gSyspropList = NULL;
#endif

void add_sysprop_change_callback(sysprop_change_callback cb, int priority) {
#if defined(HAVE_PTHREADS)
    pthread_mutex_lock(&gSyspropMutex);
    if (gSyspropList == NULL) {
        gSyspropList = new Vector<sysprop_change_callback_info>();
    }
    sysprop_change_callback_info info;
    info.callback = cb;
    info.priority = priority;
    bool added = false;
    for (size_t i=0; i<gSyspropList->size(); i++) {
        if (priority >= gSyspropList->itemAt(i).priority) {
            gSyspropList->insertAt(info, i);
            added = true;
            break;
        }
    }
    if (!added) {
        gSyspropList->add(info);
    }
    pthread_mutex_unlock(&gSyspropMutex);
#endif
}

void report_sysprop_change() {
#if defined(HAVE_PTHREADS)
    pthread_mutex_lock(&gSyspropMutex);
    Vector<sysprop_change_callback_info> listeners;
    if (gSyspropList != NULL) {
        listeners = *gSyspropList;
    }
    pthread_mutex_unlock(&gSyspropMutex);

    //ALOGI("Reporting sysprop change to %d listeners", listeners.size());
    for (size_t i=0; i<listeners.size(); i++) {
        listeners[i].callback();
    }
#endif
}

}; // namespace android

#include <dlfcn.h>

#define DEFN(x, ret, args) \
typedef ret (*x##_Func) args; \
x##_Func x##_func = 0;

#define GETFN(handle, x) x##_func = (x##_Func)dlsym(handle, #x)

DEFN(_ZN7android12AssetManagerC1ENS0_9CacheModeE, void, (void*, int));
DEFN(_ZN7android12AssetManager16addDefaultAssetsEv, bool, (void*));
DEFN(_ZN7android12AssetManager4openEPKcNS_5Asset10AccessModeE, void*, (void*, const char*, int));
DEFN(_ZN7android12AssetManager12openNonAssetEPKcNS_5Asset10AccessModeE, void*, (void*, const char*, int));

bool initialized = false;

void init() {
    void* handle = dlopen("libandroidfw.so", RTLD_NOW);
  
    GETFN(handle, _ZN7android12AssetManagerC1ENS0_9CacheModeE);
    GETFN(handle, _ZN7android12AssetManager16addDefaultAssetsEv);
    GETFN(handle, _ZN7android12AssetManager4openEPKcNS_5Asset10AccessModeE);
    GETFN(handle, _ZN7android12AssetManager12openNonAssetEPKcNS_5Asset10AccessModeE);

    initialized = true;
}

//AssetManager::AssetManager(CacheMode cacheMode)
extern "C" void _ZN7android12AssetManagerC1ENS0_9CacheModeE(void* mgr, int cacheMode) {
    if(!initialized)
        init();

    _ZN7android12AssetManagerC1ENS0_9CacheModeE_func(mgr, cacheMode);
}

//bool AssetManager::addDefaultAssets()
extern "C" bool _ZN7android12AssetManager16addDefaultAssetsEv(void* mgr) {
    if(!initialized)
        init();

    return _ZN7android12AssetManager16addDefaultAssetsEv_func(mgr);
}

//Asset* AssetManager::open(const char* fileName, AccessMode mode)
extern "C" void* _ZN7android12AssetManager4openEPKcNS_5Asset10AccessModeE(void* mgr,
                       const char* fileName, int mode) {
    if(!initialized)
        init();

    return _ZN7android12AssetManager4openEPKcNS_5Asset10AccessModeE_func(mgr, fileName, mode);
}

//Asset* AssetManager::openNonAsset(const char* fileName, AccessMode mode)
extern "C" void* _ZN7android12AssetManager12openNonAssetEPKcNS_5Asset10AccessModeE(void* mgr,
                        const char* fileName, int mode) {
    if(!initialized)
        init();

    return _ZN7android12AssetManager12openNonAssetEPKcNS_5Asset10AccessModeE_func(mgr, fileName, mode);
}

