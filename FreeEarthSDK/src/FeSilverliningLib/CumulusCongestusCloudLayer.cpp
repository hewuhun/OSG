// Copyright (c) 2006-2009 Sundog Software. All rights reserved worldwide.

#include <FeSilverliningLib/CumulusCongestusCloudLayer.h>
#include <FeSilverliningLib/CumulusCongestusCloud.h>

using namespace SilverLining;

CumulusCongestusCloudLayer::CumulusCongestusCloudLayer() : CumulusCloudLayer()
{
    ReadConfiguration("cumulus-congestus");
}

CumulusCloud *CumulusCongestusCloudLayer::CreateCloud(CloudLayer *layer)
{
    return SL_NEW CumulusCongestusCloud(layer);
}
