#include <FeMeasure/EarthworkAnalyze.h>

#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/RenderSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthSymbology/TextSymbol>

namespace FeMeasure
{
    CCEarthworkAnalyze::CCEarthworkAnalyze(osgEarth::MapNode* pMapNode)
        :CMeasure(pMapNode)
    {
        SetKey("earthwork");
        SetTitle("土方分析");
    }

    CCEarthworkAnalyze::~CCEarthworkAnalyze()
    {

    }

	CMeasureFeature* CCEarthworkAnalyze::CreateMeasureFeature()
	{
		return new CEarthworkFeature();
	}

    //////////////////////////////////////////////////////////////////////////

    CEarthworkFeature::CEarthworkFeature()
        :CAreaMFeature()
    {

    }

    CEarthworkFeature::~CEarthworkFeature()
    {

    }

    void CEarthworkFeature::Calculate()
    {

    }

    double CEarthworkFeature::TriangleArea( const osg::Vec3d& p1, const osg::Vec3d& p2, const osg::Vec3d& p3 )
    {
        double dA = osgEarth::GeoMath::distance(
            osg::DegreesToRadians(p1.y()), 
            osg::DegreesToRadians(p1.x()), 
            osg::DegreesToRadians(p2.y()), 
            osg::DegreesToRadians(p2.x()));
        double dB = osgEarth::GeoMath::distance(
            osg::DegreesToRadians(p3.y()), 
            osg::DegreesToRadians(p3.x()), 
            osg::DegreesToRadians(p2.y()), 
            osg::DegreesToRadians(p2.x()));
        double dC = osgEarth::GeoMath::distance(
            osg::DegreesToRadians(p1.y()), 
            osg::DegreesToRadians(p1.x()), 
            osg::DegreesToRadians(p3.y()), 
            osg::DegreesToRadians(p3.x()));

        if(((dA+dB) > dC) && ((dA+dB) > dC) && ((dA+dB) > dC) )
        {
			//dA
        }

        return 0.0;
    }

    double CEarthworkFeature::TriangleArea( const std::vector< osg::Vec3d > &pArray )
    {
        double dArea = 0.0;

        if(pArray.size() >= 3)
        {
            osg::Vec3dArray::const_iterator itr = pArray.begin() + 1;
            for(; itr != pArray.end() - 1; ++itr)
            {
                TriangleArea(*(pArray.begin()), (*itr), *(itr + 1));
            }
        }

        return dArea;
    }

}