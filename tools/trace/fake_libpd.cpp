/*
 * fake_libpd.cpp — implements the subset of the libpd API used by
 * rwaruntime.cpp (see fake_libpd.h). Signatures come from the real headers so
 * a drift in the vendored libpd shows up as a compile error here.
 */

#include "fake_libpd.h"

#include "z_libpd.h"
#include "util/z_queued.h"
#include "util/z_print_util.h"

#include <deque>
#include <map>
#include <set>
#include <string>

extern "C" {
#include "rwapdextra~.h" // freeverb_tilde_setup / oggread_tilde_setup decls
}

namespace {

fake_libpd_recorder_t recorder = nullptr;

// Patch handles: an opaque pointer carrying its deterministic dollarzero.
// Real pd starts $0 above 1000; we mimic that so traces look familiar.
int nextDollarZero = 1001;
std::map<void *, int> patchHandles;

std::set<std::string> boundReceivers;
std::deque<std::string> pendingBangs;

t_libpd_printhook queuedPrintHook = nullptr;
t_libpd_printhook concatenatedPrintHook = nullptr;
t_libpd_floathook queuedFloatHook = nullptr;
t_libpd_banghook queuedBangHook = nullptr;

} // namespace

void fake_libpd_set_recorder(fake_libpd_recorder_t r) { recorder = r; }

void fake_libpd_inject_bang(const std::string &receiver)
{
    pendingBangs.push_back(receiver);
}

bool fake_libpd_is_bound(const std::string &receiver)
{
    return boundReceivers.count(receiver) > 0;
}

extern "C" {

int libpd_init(void) { return 0; }

// No file system in the trace harness: nothing resolves paths, so recording the
// search path would add nothing to a trace.
void libpd_add_to_search_path(const char *path) { (void)path; }

int libpd_queued_init(void) { return 0; }
void libpd_queued_release(void) {}

void libpd_set_printhook(const t_libpd_printhook hook) { (void)hook; }
void libpd_set_floathook(const t_libpd_floathook hook) { (void)hook; }
void libpd_set_banghook(const t_libpd_banghook hook) { (void)hook; }

void libpd_set_concatenated_printhook(const t_libpd_printhook hook) { concatenatedPrintHook = hook; }
void libpd_print_concatenator(const char *s) { if(concatenatedPrintHook) concatenatedPrintHook(s); }

void libpd_set_queued_printhook(const t_libpd_printhook hook) { queuedPrintHook = hook; }
void libpd_set_queued_floathook(const t_libpd_floathook hook) { queuedFloatHook = hook; }
void libpd_set_queued_banghook(const t_libpd_banghook hook) { queuedBangHook = hook; }

void libpd_queued_receive_pd_messages(void)
{
    while(!pendingBangs.empty())
    {
        std::string receiver = pendingBangs.front();
        pendingBangs.pop_front();
        if(queuedBangHook)
            queuedBangHook(receiver.c_str());
    }
}

void *libpd_openfile(const char *name, const char *dir)
{
    (void)name; (void)dir;
    void *handle = new int(nextDollarZero++);
    patchHandles[handle] = *static_cast<int *>(handle);
    return handle;
}

void libpd_closefile(void *p)
{
    patchHandles.erase(p);
    delete static_cast<int *>(p);
}

int libpd_getdollarzero(void *p)
{
    auto it = patchHandles.find(p);
    return it != patchHandles.end() ? it->second : 0;
}

void *libpd_bind(const char *recv)
{
    auto result = boundReceivers.insert(std::string(recv));
    return const_cast<std::string *>(&*result.first);
}

void libpd_unbind(void *p)
{
    if(p)
        boundReceivers.erase(*static_cast<std::string *>(p));
}

int libpd_float(const char *recv, float x)
{
    if(recorder)
        recorder('f', recv, x, nullptr);
    return 0;
}

int libpd_bang(const char *recv)
{
    if(recorder)
        recorder('b', recv, 0, nullptr);
    return 0;
}

int libpd_symbol(const char *recv, const char *symbol)
{
    if(recorder)
        recorder('s', recv, 0, symbol);
    return 0;
}

// Externals registered by the RwaRuntime constructor — no-ops without DSP.
void rwa_binauralsimple_tilde_setup(void) {}
void vas_reverb_tilde_setup(void) {}
void freeverb_tilde_setup(void) {}
void oggread_tilde_setup(void) {}

} // extern "C"
