
#ifndef FE_SILVERLINING_CONTEXT
#define FE_SILVERLINING_CONTEXT 1

#include <FeSilverlining/SilverLiningOptions.h>
#include <osg/Referenced>
#include <osg/Light>
#include <osg/Camera>
#include <osgEarth/ThreadingUtils>

#include <FeSilverlining/Export.h>

namespace SilverLining {
    class Atmosphere;
    class CloudLayer;
}
#if 0
enum CloudTypes
{
	CIRROCUMULUS,               /*!<  有效果 高空覆盖一层薄云；High planar cloud puffs */
	CIRRUS_FIBRATUS,            /*!<  有效果 高空覆盖一层薄云；High, thicker and fibrous clouds that signal changing weather */
	STRATUS,                    /*!<  无效果Low clouds represented as a slab */
	CUMULUS_MEDIOCRIS,          /*!<  崩溃，Low, puffy clouds on fair days */
	CUMULUS_CONGESTUS,          /*!<  有效果 大积云Large cumulus clouds, built for performance. */
	CUMULUS_CONGESTUS_HI_RES,   /*!<  有效果 大积云Large cumulus clouds represented with high-resolution puff textures. */
	CUMULONIMBUS_CAPPILATUS,    /*!<  无效果Big storm clouds. */
	STRATOCUMULUS,              /*!<  无效果 低云层，太阳光可穿过Low, dense, puffy clouds with some sun breaks between them. */
	TOWERING_CUMULUS,           /*!<  有效果 高积云Very large, tall cumulus clouds in the process of becoming a thunderstorm.*/
	SANDSTORM,                  /*!<  有效果 沙尘暴 A "haboob" cloud of dust intended to be positioned at ground level. */
	NUM_CLOUD_TYPES             /*!<  Total number of cl on fair daysoud types. */
};
#endif

namespace FeSilverLining
{
	
	/**
     * Contains all the SilverLining SDK pointers.
     */
    class  FESILVERLINING_EXPORT FeSilverLiningContext : public osg::Referenced
    {
    public:
        FeSilverLiningContext(const FeSilverLiningOptions& options,int nCloudType = 2);
        
        /** Sets the light source that will represent the sun */
        void setLight(osg::Light* light);

        /** Sets the spatial reference system of the map */
        void setSRS(const osgEarth::SpatialReference* srs);

        /** Sets the minimum ambient lighting value */
        void setMinimumAmbient(const osg::Vec4f& value);

    public: // accessors

        bool ready() const { return _initAttempted && !_initFailed; }

        ::SilverLining::Atmosphere* getAtmosphere() { return _atmosphere; }

        /** Spatial reference of the map */
        const osgEarth::SpatialReference* getSRS() const { return _srs.get(); }

        void setSkyBoxSize(double size) { _skyBoxSize = size; }
        double getSkyBoxSize() const { return _skyBoxSize; }

        void initialize(osg::RenderInfo& renderInfo);

        void updateLocation();

        void updateLight();

        /** Set/get the cached camers. NOT THREAD/MULTI-CAM SAFE. */
        /** TODO */
        void setCamera(osg::Camera* camera) { _camera = camera; }
        osg::Camera* getCamera() { return _camera.get(); }

        void setCameraPosition(const osg::Vec3d& pos) { _cameraPos = pos; }
        const osg::Vec3d& getCameraPosition() const { return _cameraPos; }

		//云层高度
		void   SetCloudsBaseAltitude(double dHight);
		double GetCloudsBaseAltitude();

		void SetCloudsBaseLength( double dLength );
		void SetCloudsBaseWidth( double dWidth  );

		//云层厚度
		void SetCloudsThickness(double dThickness);
		double GetCloudsThickness();

		//云层密度
		void SetCloudsDensity(double dDensity);
		double GetCloudsDensity();

		//云层透明度
		void SetCloudsAlpha(double dAlpha);
		double GetCloudsAlpha();
		void SetCloudType(int nIndexType);

		//风速
		void SetWindSpeed(double metersPerSecond);
		double GetWindSpeed();

		//风向
		void SetWindDirection(double degreesFromNorth);
		double GetWindDirection();

		//风向
		void SetWindAltitudeRange(double dWindMinAltitude,double dWindMaxAltitude);
		void GetWindAltitudeRange(double &dWindMinAltitude,double &dWindMaxAltitude);


    protected:

        virtual ~FeSilverLiningContext();

		void ReinstallWind();

    private:

        void setupClouds();
		

    private:
        ::SilverLining::Atmosphere* _atmosphere;
        ::SilverLining::CloudLayer* _clouds;

        double _skyBoxSize;

        osg::observer_ptr<osg::Light>                  _light;
        osg::ref_ptr<const osgEarth::SpatialReference> _srs;

        bool                       _initAttempted;
        bool                       _initFailed;
        osgEarth::Threading::Mutex _initMutex;

        double _maxAmbientLightingAlt;

        osg::observer_ptr<osg::Camera> _camera;
        osg::Vec3d                     _cameraPos; // eye point
        osg::Vec4f                     _minAmbient;

        FeSilverLiningOptions			_options;


		double						 m_dHight;
		double						 m_dThickness;
		double						 m_dLength ;
		double						 m_dWidth  ;
		double						 m_dDensity  ;
		double						 m_dAlpha  ;

		double						m_dMetersPerSecond;
		double						m_dDegreesFromNorth;
		double						m_dWindMinAltitude;
		double						m_dWindMaxAltitude;

		int							m_nCloudType;

    };
}
    

#endif