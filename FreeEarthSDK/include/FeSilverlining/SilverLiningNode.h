
#ifndef FE_SILVERLINING_NODE
#define FE_SILVERLINING_NODE 1


#include <FeSilverlining/SilverLiningOptions.h>
#include <osgEarthUtil/Sky>
#include <osgEarth/Map>
#include <osgEarth/PhongLightingEffect>
#include <osg/Light>
#include <osg/LightSource>

#include <FeSilverlining/Export.h>
#include <FeSilverlining/SilverLiningContext.h>

namespace FeSilverLining
{

	//class FeSilverLiningContext;

    /**
     * Node that roots the silverlining adapter.
     */
    class  FESILVERLINING_EXPORT FeSilverLiningNode : public osgEarth::Util::SkyNode
    {
#if 1
    public:
        FeSilverLiningNode(
            const osgEarth::Map*       map,
            const FeSilverLiningOptions& options);

    public: // SkyNode
		/*
		*note 初始化 h00021 2016-10-14 
		* 解决光照冲突问题
		*/
		void Init();

		/*
		*note 设置光照 h00021 2016-10-14 解决光照冲突问题
		*/
		void setSunLight(osg::Light* pLight);
        osg::Light* getSunLight() ;

		/*
		*note 设置相机 h00021 2016-10-20 解决相机冲突问题
		*/
		void setCamera(osg::Camera* pCamera);
        osg::Camera* getCamera() ;

        void attach(osg::View* view, int lightNum);

        // callbacks from base class.
        void onSetDateTime();
        void onSetMinimumAmbient();

    public: // osg::Node

        void traverse(osg::NodeVisitor&);

	public:
		void SetSkyShow(bool blShow);
		bool IsSkyShow();

		void SetCloudsShow(bool blShow);
		bool IsCloudsShow();

		void SetDrawableShow(osg::Drawable *pDrawable,bool blShow);

		FeSilverLiningContext *GetSilverLiningContext();

		//云层高度
		void SetCloudsAltitude(double dAltitude);
		double GetCloudsAltitude();

		//云层厚度
		void SetCloudsThickness(double dThickness);
		double GetCloudsThickness();

		//云层密度
		void SetCloudsDensity(double dDensity);
		double GetCloudsDensity();

		//云层透明度
		void SetCloudsAlpha(double dAlpha);
		double GetCloudsAlpha();

		//风速
		void SetWindSpeed(double metersPerSecond);
		double GetWindSpeed();

		//风向
		void SetWindDirection(double degreesFromNorth);
		double GetWindDirection();

		//风向
		void SetWindAltitudeRange(double dWindMinAltitude,double dWindMaxAltitude);
		void GetWindAltitudeRange(double &dWindMinAltitude,double &dWindMaxAltitude);

		//设置云类型
		void SetCloudType(int nCloudType);
		int  GetCloudType();

		//设置本地时间
		void SetLocalDateTime(osgEarth::DateTime dateTime);
	private:
		void ReSet();

    protected:
        virtual ~FeSilverLiningNode();

        osg::ref_ptr<FeSilverLiningContext> _SL;
		osg::ref_ptr<osg::Geode> _geode;
        osg::ref_ptr<osg::LightSource> _lightSource;
        osg::ref_ptr<osg::Drawable> _skyDrawable;
		osg::ref_ptr<osg::Drawable> _cloudsDrawable;
        osg::ref_ptr<osg::Light> _light;
		osg::ref_ptr<osg::Camera> m_rpCamera; //h00021 2016-10-20
		osg::ref_ptr<osg::Node> m_rpAtmosphere; //h00021 2016-10-20
		double _lastAltitude;
		const FeSilverLiningOptions _options;
        osg::ref_ptr<osgEarth::PhongLightingEffect> _lighting;

		osg::observer_ptr<osgEarth::Map>       m_opMap;

		bool m_blSkyShow;
		bool m_blCloudsShow;
		int  m_nCloudType;
		osgEarth::DateTime	m_DateTime;
		bool				m_AutoTime;
#endif
    };

}

#endif
