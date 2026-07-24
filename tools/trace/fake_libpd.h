/*
 * fake_libpd.h — link-time test double for libpd used by the rwatrace harness.
 *
 * The harness links this instead of the real libpd/portaudio. Every message
 * the engine sends (libpd_float/bang/symbol) is forwarded to a recorder
 * callback; pd→engine traffic (the "<dollarzero>-playfinished" bangs) can be
 * injected and is delivered through the queued bang hook exactly where the
 * real libpd delivers it: inside libpd_queued_receive_pd_messages(), i.e.
 * during RwaRuntime::emptyPdMessageQueue() at the top of the tick.
 */

#ifndef FAKE_LIBPD_H
#define FAKE_LIBPD_H

#include <string>

// kind: 'f' float (val valid), 'b' bang, 's' symbol (sym valid)
typedef void (*fake_libpd_recorder_t)(char kind, const char *recv, float val, const char *sym);

void fake_libpd_set_recorder(fake_libpd_recorder_t recorder);

// Queue a bang for delivery on the next libpd_queued_receive_pd_messages().
void fake_libpd_inject_bang(const std::string &receiver);

// True if the engine has bound a receiver with this name (via libpd_bind).
bool fake_libpd_is_bound(const std::string &receiver);

#endif
