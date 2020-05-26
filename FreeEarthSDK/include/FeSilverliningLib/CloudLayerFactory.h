// Copyright (c) 2004-2012 Sundog Software, LLC. All rights reserved worldwide.

/** \file CloudLayerFactory.h
   \brief Instantiates specific types of CloudLayer objects.
 */

#ifdef SWIG
#define SILVERLINING_API
%module SilverLiningCloudLayerFactory
%include <FeSilverliningLib/CloudTypes.h>
%include <FeSilverliningLib/CloudLayer.h>
%{
#include <FeSilverliningLib/CloudLayerFactory.h>
using namespace SilverLining;
%}
#endif

#ifndef CLOUD_LAYER_FACTORY_H
#define CLOUD_LAYER_FACTORY_H

#include <FeSilverliningLib/Atmosphere.h>
#include <FeSilverliningLib/CloudTypes.h>
#include <iostream>
#include <FeSilverliningLib/Export.h>

#pragma pack(push)
#pragma pack(8)

namespace SilverLining
{
class CloudLayer;

/** A class factory for CloudLayer objects.

   Call CloudLayerFactory::Create to instantiate a CloudLayer object, which may then be configured,
   seeded, and submitted to the AtmosphericConditions. CloudLayer is a virtual base class, so this
   is the only mechanism for creating specific cloud layer types.
 */
class FESILVERLININGLIB_EXPORT CloudLayerFactory : public MemObject
{
public:
/** Instantiate a CloudLayer for a given cloud type. Currently supported #CloudTypes include:

   \li CloudTypes::CIRROCUMULUS - High planar cloud puffs.
   \li CloudTypes::CIRRUS_FIBRATUS - High, wispy cirrus clouds.
   \li CloudTypes::STRATUS - Low clouds represented as a slab.
   \li CloudTypes::CUMULONIMBUS_CAPPILATUS - A large thunderhead with lightning and rain.
   \li CloudTypes::CUMULUS_MEDIOCRIS - Small, puffy clouds. Use sparingly.
   \li CloudTypes::CUMULUS_CONGESTUS - Larger cumulus clouds with flattened bottoms, optimized for performance.
   \li CloudTypes::CUMULUS_CONGESTUS_HI_RES - Larger cumulus clouds with flattened bottoms, optimized for appearance.
   \li CloudTypes::TOWERING_CUMULUS - Large cumulus clouds with strong vertical growth - basically cumulus congestus clouds growing into cumulonimbus.
   \li CloudTypes::STRATOCUMULUS - Low, dense, puffy clouds with some sun breaks between them.
   \li CloudTypes::SANDSTORM - A "haboob" cloud of dust intended to be positioned at ground level.

   \param layerType The type of cloud deck to create, must be of the #CloudTypes enumeration.
   \return A pointer to a new CloudLayer for the specified type.
 */
    static CloudLayer * SILVERLINING_API Create(CloudTypes layerType);

#ifndef SWIG
/** Flattens a cloud layer to a stream buffer. */
    static bool SILVERLINING_API Serialize(CloudLayer *layer, std::ostream& stream);

/** Restores a cloud layer from a stream buffer written with Serialize() */
    static CloudLayer * SILVERLINING_API Unserialize(const Atmosphere& atm, std::istream& stream);
#endif
};
}

#pragma pack(pop)

#endif
