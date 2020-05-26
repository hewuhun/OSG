// Copyright (c) 2014 Sundog Software. All rights reserved worldwide.

#include <FeSilverliningLib/ToweringCumulusCloudLayer.h>
#include <FeSilverliningLib/ToweringCumulusCloud.h>

using namespace SilverLining;

ToweringCumulusCloudLayer::ToweringCumulusCloudLayer() : CumulusCloudLayer()
{
    ReadConfiguration("towering-cumulus");
}

CumulusCloud *ToweringCumulusCloudLayer::CreateCloud(CloudLayer *layer)
{
    return SL_NEW ToweringCumulusCloud(layer);
}
