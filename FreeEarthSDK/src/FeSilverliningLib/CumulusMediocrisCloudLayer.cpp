// Copyright (c) 2006-2009 Sundog Software. All rights reserved worldwide.

#include <FeSilverliningLib/CumulusMediocrisCloudLayer.h>
#include <FeSilverliningLib/CumulusMediocrisCloud.h>

using namespace SilverLining;

CumulusMediocrisCloudLayer::CumulusMediocrisCloudLayer() : CumulusCloudLayer()
{
    ReadConfiguration("cumulus-mediocris");
}

CumulusCloud *CumulusMediocrisCloudLayer::CreateCloud(CloudLayer *layer)
{
    return SL_NEW CumulusMediocrisCloud(layer);
}
