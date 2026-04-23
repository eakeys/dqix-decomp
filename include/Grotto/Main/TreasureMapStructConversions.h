#pragma once

#include "TreasureMapMetadata.h"
#include "DetailedTreasureMapData.h"

bool ExportDetailedTreasureMapData(const TreasureMapMetadata* from, DetailedTreasureMapData* to, int p3, int p4);
void ExportTreasureMapMetadata(const DetailedTreasureMapData& from, TreasureMapMetadata& to);