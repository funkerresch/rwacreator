/*
 * stub_backend.cpp — satisfies the RwaBackend symbols referenced by
 * rwaimport.cpp in the headless rwatrace build. getInstance() returns nullptr,
 * which routes RwaImport through its headless fallbacks; the member stubs
 * below exist only for the linker and must never execute.
 */

#include "rwabackend.h"

#include <cstdlib>

RwaBackend *RwaBackend::getInstance()
{
    return nullptr;
}

QList<RwaScene *> &RwaBackend::getScenes()
{
    static QList<RwaScene *> empty;
    abort(); // unreachable: RwaImport null-guards every backend call
    return empty;
}

RwaScene *RwaBackend::getScene(QString sceneName)
{
    (void)sceneName;
    abort();
    return nullptr;
}

void RwaBackend::receiveLastTouchedScene(RwaScene *scene)
{
    (void)scene;
    abort();
}
