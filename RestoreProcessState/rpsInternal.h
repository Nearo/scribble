﻿#ifndef rpsInternal_h
#define rpsInternal_h

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <windows.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <dbghelp.h>
#include <psapi.h>
#include <tlhelp32.h>
#ifdef max
#   undef max
#   undef min
#endif // max

#include "rps.h"
#include "rpsFoundation.h"
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "ws2_32.lib")

// todo
#define rpsPrintError(...) printf(__VA_ARGS__)
#define rpsPrintInfo(...) printf(__VA_ARGS__)

#define rpsHookAPI __declspec(noinline)

struct rpsHookInfo
{
    const char *dllname;
    const char *funcname;
    uint32_t funcordinal;
    void *hookfunc;
    void **origfunc;

    rpsHookInfo(const char *dll=nullptr, const char *fname=nullptr, uint32_t ford=0, void *hook=nullptr, void **orig=nullptr)
        : dllname(dll)
        , funcname(fname)
        , funcordinal(ford)
        , hookfunc(hook)
        , origfunc(orig)
    {
    }
};


class rpsIModule
{
public:
    virtual ~rpsIModule() {}
    virtual void            release() { delete this; }
    virtual const char*     getModuleName() const=0;
    virtual rpsHookInfo*    getHooks() const=0;
    virtual void            initialize()=0;
    virtual void            serialize(rpsArchive &ar)=0;
    virtual void            handleMessage(rpsMessage &mes) {}

    void* operator new(size_t s) { return rpsMalloc(s); }
    void  operator delete(void *p) { return rpsFree(p); }
};
typedef rpsIModule* (*rpsModuleCreator)();

class rpsCommunicator;

inline bool rpsIsRpsHandle(HANDLE h) { return (DWORD)h>>24=='R'; }

struct rpsHandleInfo
{
    HANDLE rps_handle;
    HANDLE win_handle;
    rpsIModule *owner;
};

rpsAPI void* rpsGetHeapBlock();
rpsAPI HANDLE rpsCreateHandle(rpsIModule *owner, HANDLE win_handle);
rpsAPI bool rpsReleaseHandle(HANDLE rps_handle);
rpsAPI HANDLE rpsToWinHandle(HANDLE rps_handle);
rpsAPI HANDLE rpsToRpsHandle(HANDLE win_handle);
rpsAPI rpsHandleInfo* rpsGetHandleInfo(HANDLE rps_handle);



class rpsMainModule
{
public:
    struct SerializeRequest
    {
        char path[256];
        rpsArchive::Mode mode;

        SerializeRequest(const char *p, rpsArchive::Mode m)
        {
            strncpy(path, p, _countof(path));
            mode = m;
        }
    };
    struct ModuleHookConfig
    {
    };
    struct ThreadHookConfig
    {
    };
    typedef std::vector<rpsIModule*, rps_allocator<rpsIModule*> > Modules;
    typedef std::map<rps_string, rpsIModule*, std::less<rps_string>, rps_allocator<std::pair<rps_string, rpsIModule*> > > ModuleMap;
    typedef std::vector<rpsHookInfo*, rps_allocator<rpsHookInfo*> > Hooks;
    typedef std::map<rps_string, Hooks, std::less<rps_string>, rps_allocator<std::pair<rps_string, Hooks> > > FuncHookTable;
    typedef std::map<rps_string, FuncHookTable, std::less<rps_string>, rps_allocator<std::pair<rps_string, FuncHookTable> > > DLLHookTable;
    typedef std::vector<SerializeRequest, rps_allocator<SerializeRequest> > Requests;
    typedef std::map<rps_string, ModuleHookConfig, std::less<rps_string>, rps_allocator<std::pair<rps_string, ModuleHookConfig> > > ModuleHookConfigs;
    typedef std::map<DWORD, ThreadHookConfig, std::less<DWORD>, rps_allocator<std::pair<DWORD, ThreadHookConfig> > > ThreadHookConfigs;

    static void initialize();
    static rpsMainModule* getInstance();

    void sendMessage(rpsMessage &m);
    void pushRequest(SerializeRequest &req);
    void waitForCompleteRequests();
	void setHooks(HMODULE mod);

    void* operator new(size_t s) { return rpsMalloc(s); }
    void  operator delete(void *p) { return rpsFree(p); }

private:
    friend DWORD __stdcall rpsMainThread(LPVOID lpThreadParameter);

    rpsMainModule();
    ~rpsMainModule();
    void serializeImpl(rpsArchive &ar);
    void serializeImpl(const char *path, rpsArchive::Mode mode);
    void mainloop();

    // F: [](rpsIModule*) -> void
    template<class F>
    void eachModules(const F &f)
    {
        rpsEach(m_modules, f);
    }

    Modules m_modules;
    ModuleMap m_module_map;
    DLLHookTable m_hooks;
    Requests m_requests;
    rpsMutex m_mtx_requests;
    rpsCommunicator *m_communicator;
    DWORD m_tid;
};

#include "rpsInlines.h"
#include "rpsFuncTypes.h"

#endif // rpsInternal_h
