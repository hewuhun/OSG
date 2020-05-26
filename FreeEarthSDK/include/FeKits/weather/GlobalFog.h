/**************************************************************************************************
* @file GlobalFog.h
* @note 全局雾效果
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef GLOBALFOG_H
#define GLOBALFOG_H

#include <osg/Uniform>
#include <osg/Node>
#include <osg/observer_ptr>

#include <osgEarthUtil/Common>
#include <osgEarth/TerrainEffect>

#include <FeKits/Export.h>

namespace FeKit
{
     /**
     * Utility class for injecting fog capabilities into a VirtualProgram
     */
    class FEKIT_EXPORT CGlobalFogEffect : public osg::Referenced
    {
    public:
        /**
         * Creates a new CGlobalFogEffect
         */         
        CGlobalFogEffect();

        /**
         * Creates a new  CGlobalFogEffect and attaches it to the stateset.
         */
        CGlobalFogEffect(osg::StateSet* stateSet );

	public:
        /**
         * Attaches this CGlobalFogEffect to the given StateSet
         */
        void attach(osg::StateSet* stateSet );

        /**
         * Detatches this CGlobalFogEffect from the given StateSet
         */
        void detach(osg::StateSet* stateSet );

        /**
         * Detaches this CGlobalFogEffect from all attached StateSets
         */
        void detach();

    protected:
        ~CGlobalFogEffect();

        typedef std::list< osg::observer_ptr<osg::StateSet> > StateSetList;
        StateSetList _statesets;
    };
}

#endif // OSGEARTHUTIL_FOG_H
