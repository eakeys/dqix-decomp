#pragma once

#include "TreasureMapMetadata.h"
#include "DetailedTreasureMapData.h"

bool ExportDetailedTreasureMapData(const TreasureMapMetadata* from, DetailedTreasureMapData* to, int p3, int p4);
bool ExportTreasureMapMetadata(const DetailedTreasureMapData* from, TreasureMapMetadata* to);