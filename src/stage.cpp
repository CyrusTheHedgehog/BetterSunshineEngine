#include <Dolphin/DVD.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRNameRef.hxx>

#include <SMS/System/Application.hxx>
#include <SMS/System/GameSequence.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/macros.h>
#include <SMS/Manager/ModelWaterManager.hxx>
#include <SMS/MapObj/MapObjInit.hxx>

#include "libs/container.hxx"
#include "libs/global_unordered_map.hxx"
#include "libs/profiler.hxx"
#include "libs/string.hxx"

#include "loading.hxx"
#include "logging.hxx"
#include "stage.hxx"
#include "module.hxx"

using namespace BetterSMS;

static TGlobalUnorderedMap<TGlobalString, Stage::InitCallback> sStageInitCBs(64);
static TGlobalUnorderedMap<TGlobalString, Stage::UpdateCallback> sStageUpdateCBs(64);
static TGlobalUnorderedMap<TGlobalString, Stage::Draw2DCallback> sStageDrawCBs(64);
static TGlobalUnorderedMap<TGlobalString, Stage::ExitCallback> sStageExitCBs(64);

Stage::TStageParams *Stage::TStageParams::sStageConfig = nullptr;

BETTER_SMS_FOR_EXPORT Stage::TStageParams *BetterSMS::Stage::getStageConfiguration() {
    return TStageParams::sStageConfig;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Stage::registerInitCallback(const char *name, InitCallback cb) {
    if (sStageInitCBs.find(name) != sStageInitCBs.end())
        return false;
    sStageInitCBs[name] = cb;
    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Stage::registerUpdateCallback(const char *name, UpdateCallback cb) {
    if (sStageUpdateCBs.find(name) != sStageUpdateCBs.end())
        return false;
    sStageUpdateCBs[name] = cb;
    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Stage::registerDraw2DCallback(const char *name, Draw2DCallback cb) {
    if (sStageDrawCBs.find(name) != sStageDrawCBs.end())
        return false;
    sStageDrawCBs[name] = cb;
    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Stage::registerExitCallback(const char *name, ExitCallback cb) {
    if (sStageExitCBs.find(name) != sStageExitCBs.end())
        return false;
    sStageExitCBs[name] = cb;
    return true;
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Stage::deregisterInitCallback(const char *name) {
    sStageDrawCBs.erase(name);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Stage::deregisterUpdateCallback(const char *name) {
    sStageDrawCBs.erase(name);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Stage::deregisterDraw2DCallback(const char *name) {
    sStageDrawCBs.erase(name);
}

BETTER_SMS_FOR_EXPORT void BetterSMS::Stage::deregisterExitCallback(const char *name) {
    sStageExitCBs.erase(name);
}

BETTER_SMS_FOR_EXPORT const char *BetterSMS::Stage::getStageName(u8 area, u8 episode) {
    const auto *areaAry = gpApplication.mStageArchiveAry;
    if (!areaAry || area >= areaAry->mChildren.size())
        return nullptr;

    auto *episodeAry =
        reinterpret_cast<TNameRefAryT<TScenarioArchiveName> *>(areaAry->mChildren[area]);

    if (!episodeAry || episode >= episodeAry->mChildren.size())
        return nullptr;

    TScenarioArchiveName &stageName = episodeAry->mChildren[episode];
    return stageName.mArchiveName;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Stage::isDivingStage(u8 area, u8 episode) {
    Stage::TStageParams params;
    params.load(Stage::getStageName(area, episode));
    return params.mIsDivingStage.get();
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Stage::isExStage(u8 area, u8 episode) {
    Stage::TStageParams params;
    params.load(Stage::getStageName(area, episode));
    return params.mIsExStage.get();
}

void BetterSMS::Stage::TStageParams::reset() {
    mIsExStage.set(false);
    mIsDivingStage.set(false);
    mIsOptionStage.set(false);
    mIsMultiplayerStage.set(false);
    mIsEggFree.set(true);
    // mLightType.set(TLightContext::ActiveType::DISABLED);
    // mLightPosX.set(0.0f);
    // mLightPosY.set(3600.0f);
    // mLightPosZ.set(-7458.0f);
    // mLightSize.set(8000.0f);
    // mLightStep.set(100.0f);
    // mLightColor.set(JUtility::TColor(0, 20, 40, 0));
    // mLightLayerCount.set(5);
    // mLightDarkLevel.set(120);
    // mPlayerSelectWhiteList.set(0xFFFFFFFF);
    mPlayerHasFludd.set(true);
    mPlayerHasHelmet.set(false);
    mPlayerHasGlasses.set(false);
    mPlayerHasShirt.set(false);
    mPlayerCanRideYoshi.set(true);
}

static int findNumber(const char *string, size_t max) {
    for (int i = 0; i < max; ++i) {
        const char chr = string[i];
        if (chr == '\0')
            return -1;
        if (chr >= 0x30 && chr <= 0x39)
            return i;
    }
    return -1;
}

static int findExtension(const char *string, size_t max) {
    for (int i = 0; i < max; ++i) {
        const char chr = string[i];
        if (chr == '\0')
            return -1;
        if (chr == '.')
            return i;
    }
    return -1;
}

char *BetterSMS::Stage::TStageParams::stageNameToParamPath(char *dst, const char *stage,
                                                           bool generalize) {
    strncpy(dst, "/data/scene/params/", 20);

    const int numIDPos = findNumber(stage, 60);
    if (generalize && numIDPos != -1) {
        strncpy(dst + 19, stage, numIDPos);
        dst[19 + numIDPos] = '\0';
        strcat(dst, "+.prm");
    } else {
        const int extensionPos = findExtension(stage, 60);
        if (extensionPos == -1)
            strcat(dst, stage);
        else
            strncpy(dst + 19, stage, extensionPos);
        dst[19 + extensionPos] = '\0';
        strcat(dst, ".prm");
    }

    return dst;
}

void BetterSMS::Stage::TStageParams::load(const char *stageName) {
    DVDFileInfo fileInfo;
    s32 entrynum;

    char path[64];
    stageNameToParamPath(path, stageName, false);

    entrynum = DVDConvertPathToEntrynum(path);
    if (entrynum >= 0) {
        DVDFastOpen(entrynum, &fileInfo);
        void *buffer = JKRHeap::alloc(fileInfo.mLen, 32, nullptr);

        DVDReadPrio(&fileInfo, buffer, fileInfo.mLen, 0, 2);
        DVDClose(&fileInfo);
        {
            JSUMemoryInputStream stream(buffer, fileInfo.mLen);
            TParams::load(stream);
            JKRHeap::free(buffer, nullptr);
        }
        mIsCustomConfigLoaded = true;
        return;
    }

    stageNameToParamPath(path, stageName, true);

    entrynum = DVDConvertPathToEntrynum(path);
    if (entrynum >= 0) {
        DVDFastOpen(entrynum, &fileInfo);
        void *buffer = JKRHeap::alloc(fileInfo.mLen, 32, nullptr);

        DVDReadPrio(&fileInfo, buffer, fileInfo.mLen, 0, 2);
        DVDClose(&fileInfo);
        {
            JSUMemoryInputStream stream(buffer, fileInfo.mLen);
            TParams::load(stream);
            JKRHeap::free(buffer, nullptr);
        }
        mIsCustomConfigLoaded = true;
        return;
    }

    mIsCustomConfigLoaded = false;
}

extern void updateDebugCallbacks();

void initStageLoading(TMarDirector *director) {
    Loading::setLoading(true);
    director->loadResource();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80296DE0, 0x80291750, 0, 0), initStageLoading);

static bool sIsStageInitialized = false;

// Extern to stage init
BETTER_SMS_FOR_CALLBACK void loadStageConfig(TMarDirector *) {
    Stage::TStageParams::sStageConfig = new (JKRHeap::sSystemHeap, 4) Stage::TStageParams;

    Stage::TStageParams *config = Stage::getStageConfiguration();
    config->reset();
    config->load(Stage::getStageName(gpApplication.mCurrentScene.mAreaID,
                                     gpApplication.mCurrentScene.mEpisodeID));
}

// Extern to stage init
BETTER_SMS_FOR_CALLBACK void resetStageConfig(TApplication *) {
    delete Stage::TStageParams::sStageConfig;

    waterColor[0].set(0x3C, 0x46, 0x78, 0x14);  // Water rgba
    waterColor[1].set(0xFE, 0xA8, 0x02, 0x6E);  // Yoshi Juice rgba
    waterColor[2].set(0x9B, 0x01, 0xFD, 0x6E);
    waterColor[3].set(0xFD, 0x62, 0xA7, 0x6E);
    bodyColor[0].set(0x40, 0xA1, 0x24, 0xFF);  // Yoshi rgba
    bodyColor[1].set(0xFF, 0x8C, 0x1C, 0xFF);
    bodyColor[2].set(0xAA, 0x4C, 0xFF, 0xFF);
    bodyColor[3].set(0xFF, 0xA0, 0xBE, 0xFF);
    gAudioVolume = 0.75f;
    gAudioPitch  = 1.0f;
    gAudioSpeed  = 1.0f;
}

void initStageCallbacks(TMarDirector *director) {
    loadStageConfig(director);

    for (auto &item : sStageInitCBs) {
        item.second(director);
    }

    director->setupObjects();
    Loading::setLoading(false);

    sIsStageInitialized = true;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802998B8, 0x80291750, 0, 0), initStageCallbacks);

void updateStageCallbacks(TApplication *app) {
    u32 func;
    SMS_FROM_GPR(12, func);

    if (!sIsStageInitialized)
        return;

    if (gpMarDirector && app->mContext == TApplication::CONTEXT_DIRECT_STAGE) {
        for (auto &item : sStageUpdateCBs) {
            item.second(gpMarDirector);
        }
    }
}

void drawStageCallbacks(J2DOrthoGraph *ortho) {
    ortho->setup2D();

    for (auto &item : sStageDrawCBs) {
        item.second(gpMarDirector, ortho);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80143F14, 0x80138B50, 0, 0), drawStageCallbacks);

extern void resetPlayerDatas(TApplication *);

void exitStageCallbacks(TApplication *app) {
    if (app->mContext != TApplication::CONTEXT_DIRECT_STAGE)
        return;

    for (auto &item : sStageExitCBs) {
        item.second(app);
    }

    resetPlayerDatas(app);
    resetStageConfig(app);
    sIsStageInitialized = false;
}

#pragma region MapIdentifiers

static bool isExMap() {
    auto config = Stage::getStageConfiguration();
    if (config && config->isCustomConfig())
        return config->mIsExStage.get();
    else
        return (gpApplication.mCurrentScene.mAreaID >= TGameSequence::AREA_DOLPICEX0 &&
                gpApplication.mCurrentScene.mAreaID <= TGameSequence::AREA_COROEX6);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8B58, 0x802A0C00, 0, 0), isExMap);

static bool isMultiplayerMap() {
    auto config = Stage::getStageConfiguration();
    if (config && config->isCustomConfig())
        return config->mIsMultiplayerStage.get();
    else
        return (gpMarDirector->mAreaID == TGameSequence::AREA_TEST10 &&
                gpMarDirector->mEpisodeID == 0);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8B30, 0x802A0BD8, 0, 0), isMultiplayerMap);

static bool isDivingMap() {
    auto config = Stage::getStageConfiguration();
    if (config && config->isCustomConfig())
        return config->mIsDivingStage.get();
    else
        return (gpMarDirector->mAreaID == TGameSequence::AREA_MAREBOSS ||
                gpMarDirector->mAreaID == TGameSequence::AREA_MAREEX0 ||
                gpMarDirector->mAreaID == TGameSequence::AREA_MAREUNDERSEA);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8AFC, 0x802A0BA4, 0, 0), isDivingMap);

static bool isOptionMap() {
    auto config = Stage::getStageConfiguration();
    if (config && config->isCustomConfig())
        return config->mIsOptionStage.get();
    else
        return (gpMarDirector->mAreaID == 15);
}
SMS_PATCH_B(SMS_PORT_REGION(0x802A8AE0, 0x802A0B88, 0, 0), isOptionMap);

#pragma endregion