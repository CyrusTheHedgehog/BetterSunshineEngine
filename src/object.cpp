#include <Dolphin/string.h>
#include <JSystem/JGadget/List.hxx>
#include <SMS/Player/Mario.hxx>
#include <SMS/Strategic/HitActor.hxx>
#include <SMS/System/MarNameRefGen.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>


#include "libs/container.hxx"
#include "libs/global_unordered_map.hxx"
#include "libs/string.hxx"

#include "memory.hxx"
#include "module.hxx"
#include "object.hxx"

#if BETTER_SMS_EXTRA_OBJECTS || 1

using namespace BetterSMS;

// ---------------------------------------- //

static constexpr size_t sLoadAddrTableSize = 2;

static constexpr size_t sObjExpansionSize = 100;  // Allows up to 100 extra objects
static constexpr size_t sObjMaxCount      = ObjDataTableSize + sObjExpansionSize;
static size_t sOBJNewCount                = 0;

// Locates instructions to patch, pointing to our table
static u16 *sObjLoadAddrTable[sLoadAddrTableSize][2]{
    {(u16 *)SMS_PORT_REGION(0x801B1772, 0x801a962A, 0, 0),
     (u16 *)SMS_PORT_REGION(0x801B178A, 0x801a9642, 0, 0)},
    {(u16 *)SMS_PORT_REGION(0x801B1AF2, 0x801a99AA, 0, 0),
     (u16 *)SMS_PORT_REGION(0x801B1AFA, 0x801A99B2, 0, 0)}
};

static ObjData *sObjDataTableNew[ObjDataTableSize + sObjMaxCount];

static TGlobalUnorderedMap<TGlobalString, Objects::NameRefInitializer> sCustomMapObjList(64);
static TGlobalUnorderedMap<TGlobalString, Objects::NameRefInitializer> sCustomEnemyObjList(64);
static TGlobalUnorderedMap<TGlobalString, Objects::NameRefInitializer> sCustomMiscObjList(64);
static TGlobalUnorderedMap<u32, Objects::ObjectInteractor> sCustomObjInteractionList(64);
static TGlobalUnorderedMap<u32, Objects::ObjectInteractor> sCustomObjGrabList(64);

BETTER_SMS_FOR_EXPORT size_t BetterSMS::Objects::getRemainingCapacity() {
    return sObjExpansionSize - sOBJNewCount;
}

// Map objects (coins, blocks, etc)
BETTER_SMS_FOR_EXPORT bool BetterSMS::Objects::registerObjectAsMapObj(const char *name, ObjData *data,
                                                              Objects::NameRefInitializer initFn) {
    if (sCustomMapObjList.find(name) != sCustomMapObjList.end())
        return false;
    sCustomMapObjList[name] = initFn;
    sObjDataTableNew[ObjDataTableSize + sOBJNewCount] =
        sObjDataTableNew[ObjDataTableSize + sOBJNewCount -
                         1];  // Copy the default end to the next position
    sObjDataTableNew[ObjDataTableSize + sOBJNewCount - 1] = data;
    sOBJNewCount += 1;
    return true;
}

// Enemys (Strollin' Stus, Electrokoopas, etc)
BETTER_SMS_FOR_EXPORT bool BetterSMS::Objects::registerObjectAsEnemy(const char *name, ObjData *data,
                                                             Objects::NameRefInitializer initFn) {
    if (sCustomEnemyObjList.find(name) != sCustomEnemyObjList.end())
        return false;
    sCustomEnemyObjList[name] = initFn;
    sObjDataTableNew[ObjDataTableSize + sOBJNewCount] =
        sObjDataTableNew[ObjDataTableSize + sOBJNewCount -
                         1];  // Copy the default end to the next position
    sObjDataTableNew[ObjDataTableSize + sOBJNewCount - 1] = data;
    sOBJNewCount += 1;
    return true;
}

// Misc (Managers, tables, etc)
BETTER_SMS_FOR_EXPORT bool BetterSMS::Objects::registerObjectAsMisc(const char *name,
                                                            Objects::NameRefInitializer initFn) {
    if (sCustomMiscObjList.find(name) != sCustomMiscObjList.end())
        return false;
    sCustomMiscObjList[name] = initFn;
    return true;
}

BETTER_SMS_FOR_EXPORT bool
BetterSMS::Objects::registerObjectCollideInteractor(u32 objectID,
                                                    Objects::ObjectInteractor interactor) {
    if (sCustomObjInteractionList.find(objectID) != sCustomObjInteractionList.end())
        return false;
    sCustomObjInteractionList[objectID] = interactor;
    return true;
}

BETTER_SMS_FOR_EXPORT bool
BetterSMS::Objects::registerObjectGrabInteractor(u32 objectID,
                                                 Objects::ObjectInteractor interactor) {
    if (sCustomObjGrabList.find(objectID) != sCustomObjGrabList.end())
        return false;
    sCustomObjGrabList[objectID] = interactor;
    return true;
}

BETTER_SMS_FOR_EXPORT bool BetterSMS::Objects::deregisterObject(const char *name) {
    if (sCustomMapObjList.erase(name)) {
        sOBJNewCount -= 1;
        return true;
    }
    if (sCustomEnemyObjList.erase(name)) {
        sOBJNewCount -= 1;
        return true;
    }
    if (sCustomMiscObjList.erase(name)) {
        sOBJNewCount -= 1;
        return true;
    }
    return false;
}

// ---------------------------------------- //

// extern -> SME.cpp
void makeExtendedObjDataTable() {
    memcpy(sObjDataTableNew, sObjDataTable,
           sizeof(u32) * ObjDataTableSize);  // last entry is default null
    {
        u32 addr = reinterpret_cast<u32>(&sObjDataTableNew);
        u16 lo   = addr;
        u16 hi   = (addr >> 16) + (lo >> 15);
        for (u32 i = 0; i < sLoadAddrTableSize; ++i) {  // Edit instructions to point to our table
            PowerPC::writeU16(sObjLoadAddrTable[i][0], hi);
            PowerPC::writeU16(sObjLoadAddrTable[i][1], lo);
        }
    }
}

static JDrama::TNameRef *makeExtendedMapObjFromRef(TMarNameRefGen *nameGen, const char *name) {
    JDrama::TNameRef *obj = nameGen->getNameRef_MapObj(name);
    if (obj)
        return obj;

    for (auto &item : sCustomMapObjList) {
        auto &dictItem = item;
        if (strcmp(dictItem.first.data(), name) == 0) {
            return dictItem.second();
        }
    }

    return nullptr;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8029E120, 0x80295FFC, 0, 0), makeExtendedMapObjFromRef);

static JDrama::TNameRef *makeExtendedBossEnemyFromRef(TMarNameRefGen *nameGen, const char *name) {
    JDrama::TNameRef *obj = nameGen->getNameRef_BossEnemy(name);
    if (obj)
        return obj;

    for (auto &item : sCustomMiscObjList) {
        auto &dictItem = item;
        if (strcmp(dictItem.first.data(), name) == 0) {
            return dictItem.second();
        }
    }

    return nullptr;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8029D2F4, 0x802951D0, 0, 0), makeExtendedBossEnemyFromRef);

static JDrama::TNameRef *makeExtendedGenericFromRef(TMarNameRefGen *nameGen, const char *name) {
    JDrama::TNameRef *obj = reinterpret_cast<JDrama::TNameRef *>(
        getNameRef__Q26JDrama11TNameRefGenCFPCc(nameGen, name));

    if (obj)
        return obj;

    for (auto &item : sCustomEnemyObjList) {
        auto &dictItem = item;
        if (strcmp(dictItem.first.data(), name) == 0) {
            return dictItem.second();
        }
    }

    return nullptr;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8029EDD8, 0, 0, 0), makeExtendedGenericFromRef);

static THitActor **objectInteractionHandler() {
    TMario *player;
    int objIndex;
    SMS_FROM_GPR(31, player);
    SMS_FROM_GPR(29, objIndex);

    THitActor *obj = player->mCollidingObjs[objIndex >> 2];

    for (auto &item : sCustomObjInteractionList) {
        auto &dictItem = item;
        if (dictItem.first == obj->mObjectID) {
            dictItem.second(obj, player);
            break;
        }
    }
    return player->mCollidingObjs;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80281510, 0x8027929C, 0, 0), objectInteractionHandler);

static THitActor *objGrabHandler() {
    TMario *player;
    SMS_FROM_GPR(31, player);

    THitActor *obj = player->mHeldObject;
    if (!obj)
        return obj;

    for (auto &item : sCustomObjGrabList) {
        auto &dictItem = item;
        if (dictItem.first == obj->mObjectID) {
            dictItem.second(obj, player);
            break;
        }
    }
    return obj;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80262400, 0x8025A18C, 0, 0), objGrabHandler);
SMS_WRITE_32(SMS_PORT_REGION(0x80262404, 0x8025A190, 0, 0), 0x2C030000);

void objects_staticResetter() {}
#endif