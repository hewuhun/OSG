// Copyright (c) 2004-2014  Sundog Software, LLC. All rights reserved worldwide.

#include <FeSilverliningLib/CloudLayerFactory.h>
#include <FeSilverliningLib/CumulusMediocrisCloudLayer.h>
#include <FeSilverliningLib/CirrusCloudLayer.h>
#include <FeSilverliningLib/StratusCloudLayer.h>
#include <FeSilverliningLib/CumulusCongestusCloudLayer.h>
#include <FeSilverliningLib/CumulonimbusCloudLayer.h>
#include <FeSilverliningLib/CirroCumulusLayer.h>
#include <FeSilverliningLib/StratocumulusCloudLayer.h>
#include <FeSilverliningLib/ToweringCumulusCloudLayer.h>
#include <FeSilverliningLib/Configuration.h>

using namespace SilverLining;

CloudLayer *CloudLayerFactory::Create(CloudTypes layerType)
{
    CloudLayer *cl = 0;

    switch (layerType) {
    case CIRROCUMULUS:
        cl = SL_NEW CirroCumulusCloudLayer();
        break;

    case CUMULUS_MEDIOCRIS:
        cl = SL_NEW CumulusMediocrisCloudLayer();
        break;

    case CUMULUS_CONGESTUS:
        cl = SL_NEW CumulusCongestusCloudLayer();
        break;

    case CUMULUS_CONGESTUS_HI_RES: {
        cl = SL_NEW CumulusCongestusCloudLayer();
        cl->SetUsingCloudAtlas(true);
    }
    break;

    case SANDSTORM: {
        cl = SL_NEW CumulusCongestusCloudLayer();
        cl->SetIsSoft(true);
        cl->SetDensity(2.0);
        cl->SetThickness(0);
        cl->SetFadeTowardEdges(false);
        cl->SetBaseAltitude(0);

        double r = 0.91, g = 0.72, b = 0.74;
        Configuration::GetDoubleValue("sandstorm-dust-red", r);
        Configuration::GetDoubleValue("sandstorm-dust-blue", b);
        Configuration::GetDoubleValue("sandstorm-dust-green", g);

        Vector3 dust(r, g, b);

        cl->OverrideCloudColor(dust);
    }
    break;

    case CUMULONIMBUS_CAPPILATUS:
        cl = SL_NEW CumulonimbusCloudLayer();
        break;

    case CIRRUS_FIBRATUS:
        cl = SL_NEW CirrusCloudLayer();
        break;

    case STRATUS:
        cl = SL_NEW StratusCloudLayer();
        break;

    case STRATOCUMULUS:
        cl = SL_NEW StratocumulusCloudLayer();
        break;

    case TOWERING_CUMULUS:
        cl = SL_NEW ToweringCumulusCloudLayer();
        break;

    case NUM_CLOUD_TYPES:
        break;
    }

    if (cl) {
        cl->SetType(layerType);
    }

    return cl;
}

bool CloudLayerFactory::Serialize(CloudLayer *layer, std::ostream& s)
{
    int type = layer->GetType();
    s.write((char *)&type, sizeof(int));

    layer->Serialize(s);

    return true;
}

CloudLayer *CloudLayerFactory::Unserialize(const Atmosphere& atm, std::istream& s)
{
    int type;
    s.read((char *)&type, sizeof(int));
    CloudLayer *cl = Create((CloudTypes)type);
    if (cl) {
        cl->Unserialize(atm, s);
    }
    return cl;
}
