#include <SMS/macros.h>
#include <SMS/Map/Map.hxx>
#include <SMS/Player/Mario.hxx>


#include "module.hxx"
#include "p_settings.hxx"

#if BETTER_SMS_NO_DOWNWARP

// Fix intersecting slopes
static f32 getRoofNoWater(TMap *map, f32 x, f32 y, f32 z, const TBGCheckData **out) {
    f32 height = y;
    while (true) {
        height = map->checkRoof(x, height + 100.0f, z, out);
        if (!(*out)->isWaterSurface() || (gpMarioAddress->mState & TMario::STATE_WATERBORN) != 0) {
            break;
        }
    }
    return height;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802573C8, 0, 0, 0), getRoofNoWater);

#endif

// -- ROUNDED CORNERS -- //

constexpr float CornerThreshold = -0.9f;

// Credits to frameperfection
static size_t checkWallListExotic(const TBGCheckList *list, TBGWallCheckRecord *record) {
    TBGCheckData *surf;
    f32 offset;
    f32 radius = record->mRadius;
    f32 positions[3];
    Mtx33 VMatrix;
    s32 numCols = 0;
    s32 i;
    f32 DOOD[5];
    f32 invDenom;
    f32 v, w;
    f32 margin_radius = radius - 1.0f;
    positions[0]      = record->mPosition.x;
    positions[1]      = record->mPosition.y;
    positions[2]      = record->mPosition.z;

    // Stay in this loop until out of walls.
    while (list) {
        surf = list->mColTriangle;
        list = list->mNextTriangle;
        // Exclude a large number of walls immediately to optimize.
        if (positions[1] < surf->mMinHeight || positions[1] > surf->mMaxHeight) {
            continue;
        }

        offset = surf->mNormal.x * positions[0] + surf->mNormal.y * positions[1] +
                 surf->mNormal.z * positions[2] + surf->mProjectionFactor;

        if (offset < -radius || offset > radius) {
            continue;
        }

        VMatrix[0][0] = (surf->mVertices[1].x - surf->mVertices[0].x);
        VMatrix[1][0] = (surf->mVertices[2].x - surf->mVertices[0].x);
        VMatrix[2][0] = record->mPosition.x - surf->mVertices[0].x;

        VMatrix[0][1] = (surf->mVertices[1].y - surf->mVertices[0].y);
        VMatrix[1][1] = (surf->mVertices[2].y - surf->mVertices[0].y);
        VMatrix[2][1] = record->mPosition.y - surf->mVertices[0].y;

        VMatrix[0][2] = (surf->mVertices[1].z - surf->mVertices[0].z);
        VMatrix[1][2] = (surf->mVertices[2].z - surf->mVertices[0].z);
        VMatrix[2][2] = record->mPosition.z - surf->mVertices[0].z;

        DOOD[0]  = PSVECDotProduct(reinterpret_cast<Vec *>(VMatrix[0]),
                                   reinterpret_cast<Vec *>(VMatrix[0]));
        DOOD[1]  = PSVECDotProduct(reinterpret_cast<Vec *>(VMatrix[0]),
                                   reinterpret_cast<Vec *>(VMatrix[1]));
        DOOD[2]  = PSVECDotProduct(reinterpret_cast<Vec *>(VMatrix[1]),
                                   reinterpret_cast<Vec *>(VMatrix[1]));
        DOOD[3]  = PSVECDotProduct(reinterpret_cast<Vec *>(VMatrix[2]),
                                   reinterpret_cast<Vec *>(VMatrix[0]));
        DOOD[4]  = PSVECDotProduct(reinterpret_cast<Vec *>(VMatrix[2]),
                                   reinterpret_cast<Vec *>(VMatrix[1]));
        invDenom = 1.0f / (DOOD[0] * DOOD[2] - DOOD[1] * DOOD[1]);
        v        = (DOOD[2] * DOOD[3] - DOOD[1] * DOOD[4]) * invDenom;
        if (v < 0.0f || v > 1.0f)
            goto edge_1_2;

        w = (DOOD[0] * DOOD[4] - DOOD[1] * DOOD[3]) * invDenom;
        if (w < 0.0f || w > 1.0f || v + w > 1.0f)
            goto edge_1_2;

        positions[0] += surf->mNormal.x * (radius - offset);
        positions[2] += surf->mNormal.z * (radius - offset);
        goto hasCollision;

    edge_1_2:
        if (offset < 0)
            continue;
        // Edge 1-2
        if (VMatrix[0][1] != 0.0f) {
            v = (VMatrix[2][1] / VMatrix[0][1]);
            if (v < 0.0f || v > 1.0f)
                goto edge_1_3;
            DOOD[0]  = VMatrix[0][0] * v - VMatrix[2][0];
            DOOD[1]  = VMatrix[0][2] * v - VMatrix[2][2];
            invDenom = sqrtf(DOOD[0] * DOOD[0] + DOOD[1] * DOOD[1]);
            offset   = invDenom - margin_radius;
            if (offset > 0.0f)
                goto edge_1_3;
            invDenom = offset / invDenom;
            positions[0] += (DOOD[0] *= invDenom);
            positions[2] += (DOOD[1] *= invDenom);
            margin_radius += 0.01f;

            if (DOOD[0] * surf->mNormal.x + DOOD[1] * surf->mNormal.z < CornerThreshold * offset)
                continue;
            else
                goto hasCollision;
        }

    edge_1_3:
        // Edge 1-3
        if (VMatrix[1][1] != 0.0f) {
            v = (VMatrix[2][1] / VMatrix[1][1]);
            if (v < 0.0f || v > 1.0f)
                goto edge_2_3;
            DOOD[0]  = VMatrix[1][0] * v - VMatrix[2][0];
            DOOD[1]  = VMatrix[1][2] * v - VMatrix[2][2];
            invDenom = sqrtf(DOOD[0] * DOOD[0] + DOOD[1] * DOOD[1]);
            offset   = invDenom - margin_radius;
            if (offset > 0.0f)
                goto edge_2_3;
            invDenom = offset / invDenom;
            positions[0] += (DOOD[0] *= invDenom);
            positions[2] += (DOOD[1] *= invDenom);
            margin_radius += 0.01f;

            if (DOOD[0] * surf->mNormal.x + DOOD[1] * surf->mNormal.z < CornerThreshold * offset)
                continue;
            else
                goto hasCollision;
        }

    edge_2_3:
        // Edge 2-3
        VMatrix[1][0] = (surf->mVertices[2].x - surf->mVertices[1].x);
        VMatrix[2][0] = record->mPosition.x - surf->mVertices[1].x;
        VMatrix[1][1] = (surf->mVertices[2].y - surf->mVertices[1].y);
        VMatrix[2][1] = record->mPosition.y - surf->mVertices[1].y;
        VMatrix[1][2] = (surf->mVertices[2].z - surf->mVertices[1].z);
        VMatrix[2][2] = record->mPosition.z - surf->mVertices[1].z;

        if (VMatrix[1][1] != 0.0f) {
            v = (VMatrix[2][1] / VMatrix[1][1]);
            if (v < 0.0f || v > 1.0f)
                continue;
            DOOD[0]  = VMatrix[1][0] * v - VMatrix[2][0];
            DOOD[1]  = VMatrix[1][2] * v - VMatrix[2][2];
            invDenom = sqrtf(DOOD[0] * DOOD[0] + DOOD[1] * DOOD[1]);
            offset   = invDenom - margin_radius;
            if (offset > 0.0f)
                continue;
            invDenom = offset / invDenom;
            positions[0] += (DOOD[0] *= invDenom);
            positions[2] += (DOOD[1] *= invDenom);
            margin_radius += 0.01f;
            if (DOOD[0] * surf->mNormal.x + DOOD[1] * surf->mNormal.z < CornerThreshold * offset)
                continue;
            else
                goto hasCollision;
        } else
            continue;

    hasCollision:
        //! (Unreferenced Walls) Since this only returns the first four walls,
        //  this can lead to wall interaction being missed. Typically unreferenced walls
        //  come from only using one wall, however.
        if (record->mNumWalls < 4) {
            record->mWalls[record->mNumWalls++] = surf;
        }

        numCols++;
    }
    record->mPosition.x = positions[0];
    record->mPosition.z = positions[2];

    return numCols;
}

static size_t checkWallsExotic(TMapCollisionData* collision, TBGWallCheckRecord* record) {
    record->mNumWalls = 0;

    const f32 gridFraction = 1.0f / 1024.0f;

    const f32 boundsX = collision->mAreaSizeX;
    const f32 boundsZ = collision->mAreaSizeZ;

    // This fixes large objects consuming many cells when bug fixes are enabled
    const f32 cellRadius = BetterSMS::isCollisionRepaired() ? record->mRadius : 0.0f;

    const f32 minX       = record->mPosition.x - cellRadius;
    const f32 maxX       = record->mPosition.x + cellRadius;
    const f32 minZ       = record->mPosition.z - cellRadius;
    const f32 maxZ       = record->mPosition.z + cellRadius;

    if (maxX < -boundsX || minX >= boundsX)
        return 0;

    if (maxZ < -boundsZ || minZ >= boundsZ)
        return 0;

    const int cellMinX = Max(0, gridFraction * (minX + boundsX));
    const int cellMaxX = Min(collision->mBlockXCount - 1, gridFraction * (maxX + boundsX));

    const int cellMinZ = Max(0, gridFraction * (minZ + boundsZ));
    const int cellMaxZ = Min(collision->mBlockZCount - 1, gridFraction * (maxZ + boundsZ));

    size_t wallsFound = 0;

    // Fall back to simple corner behavior when not fixing bugs
    if (!BetterSMS::isCollisionRepaired()) {
        for (int cellX = cellMinX; cellX <= cellMaxX; ++cellX) {
            for (int cellZ = cellMinZ; cellZ <= cellMaxZ; ++cellZ) {
                wallsFound += collision->checkWallList(
                    collision->mMoveCollisionRoot[cellX + (cellZ * collision->mBlockXCount)]
                        .mCheckList[TBGCheckListRoot::WALL]
                        .mNextTriangle,
                    record);

                if (wallsFound >= record->mCollideMax)
                    return wallsFound;

                wallsFound += collision->checkWallList(
                    collision->mStaticCollisionRoot[cellX + (cellZ * collision->mBlockXCount)]
                        .mCheckList[TBGCheckListRoot::WALL]
                        .mNextTriangle,
                    record);

                if (wallsFound >= record->mCollideMax)
                    return wallsFound;
            }
        }
    } else {
        for (int cellX = cellMinX; cellX <= cellMaxX; ++cellX) {
            for (int cellZ = cellMinZ; cellZ <= cellMaxZ; ++cellZ) {
                wallsFound += checkWallListExotic(
                    collision->mMoveCollisionRoot[cellX + (cellZ * collision->mBlockXCount)]
                        .mCheckList[TBGCheckListRoot::WALL]
                        .mNextTriangle,
                    record);

                if (wallsFound >= record->mCollideMax)
                    return wallsFound;

                wallsFound += checkWallListExotic(
                    collision->mStaticCollisionRoot[cellX + (cellZ * collision->mBlockXCount)]
                        .mCheckList[TBGCheckListRoot::WALL]
                        .mNextTriangle,
                    record);

                if (wallsFound >= record->mCollideMax)
                    return wallsFound;
            }
        }
    }

    return wallsFound;
}
SMS_PATCH_B(SMS_PORT_REGION(0x8018C770, 0, 0, 0), checkWallsExotic);

static f32 patchedCheckGroundList(f32 x, f32 y, f32 z, u8 flags, const TBGCheckList *list,
                                  const TBGCheckData **out) {

    TBGCheckData *checkData;
    TBGCheckList *bgCheckNode;

    *out       = &TMapCollisionData::mIllegalCheckData;
    f32 exactY = -32767.0f;

    while (true) {
        do {
            if (!list)
                return exactY;

            checkData   = list->mColTriangle;
            bgCheckNode = list->mNextTriangle;
            list        = bgCheckNode;
        } while (y < checkData->mMinHeight);

        if ((flags & 4) != 0) {  // Pass through Check
            const u16 type = checkData->mType;
            if (type == 0x401 || type == 0x801 || type == 0x10A || type == 0x8400) {
                continue;
            }
        }

        if ((flags & 1) != 0) {  // Water Check
            const u16 type = checkData->mType;
            if ((type >= 0x100 && type < 0x106) || type == 0x4104) {
                continue;
            }
        }

        const f32 ax = checkData->mVertices[0].x;
        const f32 az = checkData->mVertices[0].z;
        const f32 bz = checkData->mVertices[1].z;
        const f32 bx = checkData->mVertices[1].x;

        const bool abxCheck = -1.0f <= (az - z) * (bx - ax) - (ax - x) * (bz - az);

        if (!abxCheck)
            continue;

        const f32 cz = checkData->mVertices[2].z;
        const f32 cx = checkData->mVertices[2].x;

        const bool bcxCheck = -1.0f <= (bz - z) * (cx - bx) - (bx - x) * (cz - bz);
        const bool caxCheck = -1.0f <= (cz - z) * (ax - cx) - (cx - x) * (az - cz);

        if (!(bcxCheck && caxCheck))
            continue;

        const f32 sampleExactY =
            -(checkData->mProjectionFactor + x * checkData->mNormal.x + z * checkData->mNormal.z) /
            checkData->mNormal.y;
        if (y - (sampleExactY - 78.0f) < 0.0f)
            continue;

        if (sampleExactY > exactY) {
            *out   = checkData;
            exactY = sampleExactY;
        }

        if (!BetterSMS::isCollisionRepaired())
            return exactY;  // Return on first sample (default behavior)
    }
}
SMS_PATCH_B(SMS_PORT_REGION(0x8018C334, 0, 0, 0), patchedCheckGroundList);

//static float removeQuarterFrames() { return 120.0f; }
//SMS_PATCH_BL(SMS_PORT_REGION(0x80299850, 0, 0, 0), removeQuarterFrames);


// Force exotic wall selection (Fixes intersecting walls)
SMS_WRITE_32(SMS_PORT_REGION(0x802556A0, 0, 0, 0), 0x4800000C);