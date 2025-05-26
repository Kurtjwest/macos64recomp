//patches in this file are required for the game to run. Patches exported from smashbrothers.us.toml
#include "patches.h"
#include "sys/dma.h"
#include "sys/main.h"
#include <sys/debug.h>
#include <sys/taskman.h>
#include <sys/audio.h>
#include <sys/scheduler.h>
#include <sys/controller.h>

// #include <linkersegs.h>
#include <macros.h>
#include <ssb_types.h>

#include <PR/R4300.h>
#include <PR/os.h>
#include <PR/rcp.h>
#include <PR/ultratypes.h>

#define STACK_PROBE_MAGIC 0xFEDCBA98
// size of stack in double words (u64, 8 bytes)
#define THREAD0_STACK_SIZE 57
#define THREAD1_STACK_SIZE 25
#define THREAD3_STACK_SIZE 121
#define THREAD4_STACK_SIZE 185
#define THREAD5_STACK_SIZE 1657
#define THREAD6_STACK_SIZE 249

// Thread Scheduler Priorities
#define THREAD3_PRI 120
#define THREAD4_PRI 110
#define THREAD5_PRI 50
#define THREAD6_PRI 115

extern u8 sUnref8003FAD0[56];
extern u64 gSYMainThread0Stack[THREAD0_STACK_SIZE];
extern OSThread sSYMainThread1;
extern u8 sUnref8003FE80[56];
extern u64 sSYMainThread1Stack[THREAD1_STACK_SIZE];
extern OSThread sSYMainThread3;
extern u8 sUnref80040130[56];
extern u64 sSYMainThread3Stack[THREAD3_STACK_SIZE];
extern OSThread sSYMainThread4;
extern u8 sUnref800406E0[56];
extern u64 sSYMainThread4Stack[THREAD4_STACK_SIZE];
extern OSThread gSYMainThread5;
extern u64 sSYMainUnknown0x80040E90[7];
extern u64 sSYMainThread5Stack[THREAD5_STACK_SIZE];
extern OSThread gSYMainThread6;
extern u8 sUnref8004440[56];
extern u64 sSYMainThread6Stack[THREAD6_STACK_SIZE];
extern u64 gSYMainRspBootCode[0x20]; // IP3 font?
extern sb8 gSYMainImemOK;
extern sb8 gSYMainDmemOK;
extern OSMesg sSYMainBlockMesg[1];
extern OSMesgQueue gSYMainThreadingMesgQueue;
extern OSMesg sSYMainPiCmdMesg[50];
extern OSMesgQueue sSYMainPiCmdQueue;
extern u8 sSYMainThreadArgBuf[0x80];
extern SYOverlay dSYMainSceneManagerOverlay;
void scManagerRunLoop(OSPri);
extern OSPiHandle* osCartRomInit_recomp(void);
#define osCartRomInit osCartRomInit_recomp

extern void osCreateThread_recomp(OSThread*, OSId, void (*)(void*), void*, void*, OSPri);
#define osCreateThread osCreateThread_recomp

extern void osStartThread_recomp(OSThread*);
#define osStartThread osStartThread_recomp

extern void yield_self(void);
extern void yield_self_1ms(void);

int dummyData = 1;
int dummyBss;
int logFirstDma = 0;


RECOMP_PATCH void syDmaLoadOverlay(SYOverlay* ovl)
{

    recomp_load_overlays((u32)ovl->rom_start, (void*)ovl->ram_load_start, (u32)ovl->rom_end - (u32)ovl->rom_start); //@recomp

    if ((ovl->ram_text_end - ovl->ram_text_start) != 0)
    {
        //osInvalICache((void*)ovl->ram_text_start, ovl->ram_text_end - ovl->ram_text_start); //@recomp
        //osInvalDCache((void*)ovl->ram_text_start, ovl->ram_text_end - ovl->ram_text_start); //@recomp
    }
    if ((ovl->ram_data_end - ovl->ram_data_start) != 0)
    {
        //osInvalDCache((void*)ovl->ram_data_start, ovl->ram_data_end - ovl->ram_data_start); //@recomp
    }
    if ((ovl->rom_end - ovl->rom_start) != 0)
    {
        syDmaCopy(gSYDmaRomPiHandle, ovl->rom_start, ovl->ram_load_start, ovl->rom_end - ovl->rom_start, OS_READ);
    }
    if ((ovl->ram_noload_end - ovl->ram_noload_start) != 0)
    {
        bzero((void*)ovl->ram_noload_start, ovl->ram_noload_end - ovl->ram_noload_start);
    }
}


RECOMP_PATCH void syMainThread5(void* arg)
{
    //osCreateViManager(OS_PRIORITY_VIMGR); //@recomp
    gSYDmaRomPiHandle = osCartRomInit();
    syDmaSramPiInit();
    //osCreatePiManager(OS_PRIORITY_PIMGR, &sSYMainPiCmdQueue, sSYMainPiCmdMesg, ARRAY_COUNT(sSYMainPiCmdMesg)); //@recomp
    syDmaCreateMesgQueue();

    syDmaReadRom(PHYSICAL_TO_ROM(0xB70), gSYMainRspBootCode, sizeof(gSYMainRspBootCode));
    //remove imem/dmem checks
    //syMainSetImemStatus(); //@recomp
    //syMainSetDmemStatus(); //@recomp
    osCreateMesgQueue(&gSYMainThreadingMesgQueue, sSYMainBlockMesg, ARRAY_COUNT(sSYMainBlockMesg));

    osCreateThread(&sSYMainThread3, 3, sySchedulerThreadMain, NULL, sSYMainThread3Stack + ARRAY_COUNT(sSYMainThread3Stack), THREAD3_PRI);
    sSYMainThread3Stack[0] = STACK_PROBE_MAGIC; osStartThread(&sSYMainThread3);
    osRecvMesg(&gSYMainThreadingMesgQueue, NULL, OS_MESG_BLOCK);

    osCreateThread(&sSYMainThread4, 4, syAudioThreadMain, NULL, sSYMainThread4Stack + ARRAY_COUNT(sSYMainThread4Stack), THREAD4_PRI);
    sSYMainThread4Stack[0] = STACK_PROBE_MAGIC; osStartThread(&sSYMainThread4);
    osRecvMesg(&gSYMainThreadingMesgQueue, NULL, OS_MESG_BLOCK);

    osCreateThread(&gSYMainThread6, 6, syControllerThreadMain, NULL, sSYMainThread6Stack + ARRAY_COUNT(sSYMainThread6Stack), THREAD6_PRI);
    sSYMainThread6Stack[0] = STACK_PROBE_MAGIC; osStartThread(&gSYMainThread6);
    osRecvMesg(&gSYMainThreadingMesgQueue, NULL, OS_MESG_BLOCK);

    func_80006B80();
    syDmaLoadOverlay(&dSYMainSceneManagerOverlay);
    scManagerRunLoop(0);
}

extern sb32 dSYAudioIsSettingsUpdated;

RECOMP_PATCH sb32 syAudioGetSettingsUpdated(void)
{
    yield_self(); //@recomp
    return dSYAudioIsSettingsUpdated;
}

extern s32 dSYAudioIsRestarting;

RECOMP_PATCH sb32 syAudioGetRestarting(void)
{
    yield_self_1ms(); //@recomp
    return dSYAudioIsRestarting;
}

RECOMP_PATCH s32 syAudioCheckBGMPlaying(s32 sngplayer)
{
    yield_self(); //@recomp
    if (gSYAudioCSPlayers[sngplayer]->state == AL_STOPPED)
    {
        return FALSE;
    }
    else return TRUE;
}

extern u32 sSYSchedulerTicCount;

RECOMP_PATCH u32 sySchedulerGetTicCount(void)
{
    yield_self(); //@recomp
    return sSYSchedulerTicCount;
}