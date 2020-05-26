/**************************************************************************************************
* @file CEarthworkAnalyze.h
* @note 土方分析
* @author z00013
* @data 2015-08-18
**************************************************************************************************/
#ifndef FE_EARTHWORK_ANALYZE_H
#define FE_EARTHWORK_ANALYZE_H

#include <osgEarth/Containers>
#include <FeMeasure/AreaMeasure.h>


namespace FeMeasure
{
    class FEMEASURE_EXPORT CCEarthworkAnalyze : public CMeasure
    {
    public:
        CCEarthworkAnalyze(osgEarth::MapNode* pMapNode);

    protected:
        virtual ~CCEarthworkAnalyze();

		virtual CMeasureFeature* CreateMeasureFeature();
    };

//////////////////////////////////////////////////////////////////////////

    class CEarthworkFeature : public CAreaMFeature
    {
    public:
        CEarthworkFeature();

    protected:
        virtual ~CEarthworkFeature();

        virtual void Calculate();

        double TriangleArea(const osg::Vec3d& p1, const osg::Vec3d& p2, const osg::Vec3d& p3);

        double TriangleArea(const std::vector< osg::Vec3d > &pArray);

    };
}

#endif //FE_EARTHWORK_ANALYZE_H