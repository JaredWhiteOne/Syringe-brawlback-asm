#include <OS/OSCache.h>
#include <memory.h>
#include <modules.h>
#include <mu/mu_sel_char_player_area.h>
#include <nw4r/g3d/g3d_resfile.h>
#include <types.h>
#include <vector.h>

#include "sel_char_load_thread.h"
#include "sy_core.h"

using namespace nw4r::g3d;

namespace CSSHooks {

    selCharLoadThread* threads[4];
    void createThreads(muSelCharPlayerArea* area)
    {
        selCharLoadThread* thread = new (Heaps::Thread) selCharLoadThread(area);
        threads[area->areaIdx] = thread;
        thread->start();
    }

    // clang-format off

    // Hacky fix to force setCharPic to return
    // if data is still loading
    extern void setCharPic__end();
    extern void setCharPic__cont();
    asm void setCharPicFix()
    {
            nofralloc // don't need stack frame

            cmpwi r3, 0
            bne skip
            
            mr r3, r30
            b setCharPic__end // branch to end of original func
            
            skip:
                mr r26, r3
                b setCharPic__cont // continue where we left off
    }
    // clang-format on

    ResFile* getCharPicTexResFile(muSelCharPlayerArea* area, u32 charKind)
    {
        selCharLoadThread* thread = threads[area->areaIdx];

        // if data hasn't been loaded, use transparent texture
        // and request data be loaded by load thread
        if (thread->m_dataReady == false)
        {
            thread->requestLoad(charKind);
            area->muObject->setFrameTex(0);
            return NULL;
        }
        else
        {
            ResFile* resFile = &area->charPicRes;

            // flush cache
            DCFlushRange(area->charPicData, 0x40000);

            // set ResFile to point to filedata
            area->charPicRes = (nw4r::g3d::ResFile)area->charPicData;

            // init resFile and return
            nw4r::g3d::ResFile::Init(resFile);

            // to ensure we load more than just
            // the first hovered character
            thread->reset();

            return resFile;
        }
    }

    muSelCharPlayerArea* (*_destroyPlayerAreas)(void*, int);
    muSelCharPlayerArea* destroyPlayerAreas(muSelCharPlayerArea* object, int external)
    {
        // destroy our load thread
        delete threads[object->areaIdx];
        threads[object->areaIdx] = 0;

        // call base function first
        muSelCharPlayerArea* ret = _destroyPlayerAreas(object, external);

        return ret;
    }

    void InstallHooks()
    {
        // hook to load portraits from RSPs
        SyringeCore::syReplaceFuncRel(0x1107c,
                                      reinterpret_cast<void*>(getCharPicTexResFile),
                                      NULL,
                                      Modules::SORA_MENU_SEL_CHAR);

        // simple asm hook to force setCharPic to
        // return if data is still loading
        SyringeCore::sySimpleHook(0x806975a8, reinterpret_cast<void*>(setCharPicFix), Modules::SORA_MENU_SEL_CHAR);

        // hook to clean up our mess when unloading CSS
        SyringeCore::syReplaceFunc(0x806937bc,
                                   reinterpret_cast<void*>(destroyPlayerAreas),
                                   (void**)&_destroyPlayerAreas,
                                   Modules::SORA_MENU_SEL_CHAR);

        // hook to create threads when booting the CSS
        SyringeCore::syInlineHook(0x80685DE8, reinterpret_cast<void*>(createThreads), Modules::SORA_MENU_SEL_CHAR);
    }
} // namespace CSSHooks
