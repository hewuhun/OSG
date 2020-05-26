#include <osgEarth/MapNode>
#include <osgViewer/View>
#include <osgEarth/ElevationQuery>
#include <osgEarth/GeoMath>
#include <FeUtils/CoordConverter.h>

namespace FeUtil
{

    bool RadianLLH2XYZ( CRenderContext* pContext, const osg::Vec3d& vecLLH, osg::Vec3d& vecXYZ )
    {
        if(pContext && pContext->GetMapNode())
        {
            pContext->GetMapNode()->getMapSRS()->getEllipsoid()->convertLatLongHeightToXYZ(
                vecLLH.y(), vecLLH.x(), vecLLH.z(), vecXYZ.x(), vecXYZ.y(), vecXYZ.z());
            return true;
        }

        return false;
    }

    bool DegreeLLH2XYZ( CRenderContext* pContext, const osg::Vec3d& vecLLH, osg::Vec3d& vecXYZ )
    {
        if(pContext && pContext->GetMapNode())
        {
            osg::Vec3d vecRadianLLH(osg::DegreesToRadians(vecLLH.x()), osg::DegreesToRadians(vecLLH.y()), vecLLH.z());
            return RadianLLH2XYZ(pContext, vecRadianLLH, vecXYZ);
        }

        return false;
    }

    bool XYZ2RadianLLH( CRenderContext* pContext, const osg::Vec3d& vecXYZ, osg::Vec3d& vecLLH )
    {
        if(pContext && pContext->GetMapNode())
        {
            pContext->GetMapNode()->getMapSRS()->getEllipsoid()->convertXYZToLatLongHeight(
                vecXYZ.x(), vecXYZ.y(), vecXYZ.z(), vecLLH.y(), vecLLH.x(), vecLLH.z());
            return true;
        }

        return false;
    }

    bool XYZ2DegreeLLH( CRenderContext* pContext, const osg::Vec3d& vecXYZ, osg::Vec3d& vecLLH )
    {
        if(pContext && pContext->GetMapNode())
        {
            pContext->GetMapNode()->getMapSRS()->getEllipsoid()->convertXYZToLatLongHeight(
                vecXYZ.x(), vecXYZ.y(), vecXYZ.z(), vecLLH.y(), vecLLH.x(), vecLLH.z());
            vecLLH.x() = osg::RadiansToDegrees(vecLLH.x());
            vecLLH.y() = osg::RadiansToDegrees(vecLLH.y());
            return true;
        }

        return false;
    }

	bool ScreenXY2DegreeLLH( CRenderContext* pContext, float fX, float fY, osg::Vec3d& vecLLH )
	{
		if(ScreenXY2RadiaLLH(pContext, fX, fY, vecLLH))
		{
			vecLLH.x() = osg::RadiansToDegrees(vecLLH.x());
			vecLLH.y() = osg::RadiansToDegrees(vecLLH.y());
			return true;
		}

		return false;
	}

	bool ScreenXY2DegreeLLH(CRenderContext* pContext, float fX, float fY, double& dLon, double& dLat, double& dHei)
	{
		if(ScreenXY2RadiaLLH(pContext, fX, fY, dLon, dLat, dHei))
		{
			dLon = osg::RadiansToDegrees(dLon);
			dLat = osg::RadiansToDegrees(dLat);
			return true;
		}

		return false;
	}

	bool ScreenXY2RadiaLLH( CRenderContext* pContext, float fX, float fY, osg::Vec3d& vecLLH )
	{
		if(pContext && pContext->GetView() && pContext->GetMapNode())
		{
			osg::Vec3d vecWorld;
			if(pContext->GetMapNode()->getTerrain()->getWorldCoordsUnderMouse(pContext->GetView(), fX, fY, vecWorld))
			{
				pContext->GetMapNode()->getMapSRS()->getEllipsoid()->convertXYZToLatLongHeight(
					vecWorld.x(), vecWorld.y(), vecWorld.z(), vecLLH.y(), vecLLH.x(), vecLLH.z());

				return true;
			}
		}

		return false;
	}

	bool ScreenXY2RadiaLLH(CRenderContext* pContext, float fX, float fY, double& dLon, double& dLat, double& dHei)
	{
		if(pContext && pContext->GetView() && pContext->GetMapNode())
		{
			osg::Vec3d vecWorld;
			if(pContext->GetMapNode()->getTerrain()->getWorldCoordsUnderMouse(pContext->GetView(), fX, fY, vecWorld))
			{
				pContext->GetMapNode()->getMapSRS()->getEllipsoid()->convertXYZToLatLongHeight(
					vecWorld.x(), vecWorld.y(), vecWorld.z(), dLat, dLon, dHei);
				return true;
			}
		}

		return false;
	}

	bool RadiaLLH2Matrix(CRenderContext* pContext, const osg::Vec3d& vecLLH,osg::Matrix& matrix )
	{
		if(pContext && pContext->GetView() && pContext->GetMapNode())
		{
			pContext->GetMapNode()->getMapSRS()->getEllipsoid()->
				computeLocalToWorldTransformFromLatLongHeight(vecLLH.y(),vecLLH.x(),vecLLH.z(),matrix);

			return true;
		}

		return false;
	}

	bool DegreeLLH2Matrix( CRenderContext* pContext, const osg::Vec3d& vecLLH,osg::Matrix& matrix )
	{
		return RadiaLLH2Matrix(pContext, osg::Vec3d(osg::DegreesToRadians(vecLLH.x()),osg::DegreesToRadians(vecLLH.y()),vecLLH.z()), matrix);
	}

	bool XYZ2Matrix( CRenderContext* pContext, const osg::Vec3d& vecXYZ,osg::Matrix& matrix )
	{
		osg::Vec3d vecLLHRadia;
		if(XYZ2RadianLLH(pContext, vecXYZ, vecLLHRadia))
		{
			return RadiaLLH2Matrix(pContext, vecLLHRadia, matrix);
		}

		return false;
	}

	bool RadiaLL2LLH(CRenderContext* pContext, osg::Vec3d& vecLLA)
	{
		osg::Vec3d vecLLAD(osg::RadiansToDegrees(vecLLA.x()),osg::RadiansToDegrees(vecLLA.y()),vecLLA.z());
		if(DegreeLL2LLH(pContext, vecLLA))
		{
			vecLLA.z() = vecLLAD.z();
			return true;
		}

		return false;
	}

	bool DegreeLL2LLH(CRenderContext* pContext, osg::Vec3d& vecLLA)
	{
		if(pContext->GetMapNode())
		{
			double dHeight = 0.0;
			if(pContext->GetMapNode()->getTerrain()->getHeight(pContext->GetMapNode()->getMapSRS(), vecLLA.x(),vecLLA.y(), &dHeight))
			{
				vecLLA.z() = dHeight;
				return true;
			}

			return true;
		}

		return false;
	}

	extern FEUTIL_EXPORT bool RadiaLL2RealLLH( CRenderContext* pContext, osg::Vec3d& vecLLA )
	{
		osg::Vec3d vecLLAD(osg::RadiansToDegrees(vecLLA.x()),osg::RadiansToDegrees(vecLLA.y()), vecLLA.z());
		if(DegreeLL2RealLLH(pContext, vecLLA))
		{
			vecLLA.z() = vecLLAD.z();
			return true;
		}

		return false;
	}

	extern FEUTIL_EXPORT bool DegreeLL2RealLLH( CRenderContext* pContext, osg::Vec3d& vecLLA )
	{
		if (NULL == pContext)
		{
			return false;			
		}

		osgEarth::MapNode* pMapNode = pContext->GetMapNode();
		if (NULL == pMapNode)
		{
			return false;
		}

		double _precisionLod = 10.0;

		//获取当前TileKey的高程
		osg::ref_ptr<osg::HeightField> pHf = new osg::HeightField();
		pHf->allocate(17, 17); //16*16的高程数据？
		osgEarth::ElevationLayerVector elv;
		pMapNode->getMap()->getElevationLayers(elv);
		osgEarth::TileKey tileKey;
		//默认是平顶数据
		double n = ::powf(2.0, _precisionLod);
		double deltax = 180.0f / n;
		double deltay = 180.0f / n;

		int x = (vecLLA.x() + 180.0) / deltax;
		int y = (90.0 - vecLLA.y()) / deltay;

		osgEarth::TileKey ref_key(_precisionLod, x, y, pMapNode->getMap()->getProfile());

		tileKey = ref_key;

		elv.populateHeightField(pHf, tileKey, pMapNode->getMap()->getProfileNoVDatum(), pMapNode->getMap()->getMapOptions().elevationInterpolation().get(), 0L);

		osgEarth::GeoHeightField ghf(pHf, tileKey.getExtent());

		float dHei = 0;
		ghf.getElevation(tileKey.getExtent().getSRS(), vecLLA.x(), vecLLA.y(), pMapNode->getMap()->getMapOptions().elevationInterpolation().get(), tileKey.getExtent().getSRS(), dHei);
		vecLLA.z() = dHei;

		return true;
	}

	bool ConvertLocalWorldCoordToScreen( CRenderContext* pContext, const osg::Vec3d& pos, osg::Vec2d& screenPos )
	{
		if(pContext && pContext->GetMapNode() && pContext->GetView())
		{
			osg::Camera* cam = pContext->GetView()->getCamera();
			osg::MatrixList worldMatrixList = cam->getWorldMatrices(pContext->GetMapNode());
			osg::Matrix worldMatrix = worldMatrixList.at(0);
			osg::Matrix viewMatrix = cam->getViewMatrix();
			osg::Matrix projMatrix = cam->getProjectionMatrix();
			//		osg::Matrix camMatrix = cam->getViewport()->computeWindowMatrix();
			osg::Vec4d in = osg::Vec4d(pos.x(), pos.y(), pos.z(), 1.0);
			osg::Vec4d out = in * worldMatrix ;
			out =out * viewMatrix;
			out =out  * projMatrix;
			//	out =out  * camMatrix;

			if (out.w() <= 0.0) return false;  //如果out.w()小于0说明在背面不被拣选

			out.x() /= out.w();
			out.y() /= out.w();
			out.z() /= out.w();

			out.x() = out.x() * 0.5 + 0.5;
			out.y() = out.y() * 0.5 + 0.5;
			out.z() = out.z() * 0.5 + 0.5;
			
			if(!pContext->GetCamera()->getViewport())
			{
				return false;
			}
			int nScreenW = pContext->GetCamera()->getViewport()->width();
			int nScreenH = pContext->GetCamera()->getViewport()->height();

			screenPos.x() = out.x() * nScreenW;
			screenPos.y() = out.y() * nScreenH;   

			return true;
		}
		return false;
	}

	double GetGeoDistance( CRenderContext* pContext, double dSLon, double dSLat, double dELon, double dELat )
	{
		if(pContext && pContext->GetMapNode())
		{
			return osgEarth::GeoMath::distance(dSLat,dSLon, dELat, dELon, pContext->GetMapNode()->getMapSRS()->getEllipsoid()->getRadiusEquator());
		}

		return  0.0; 
	}

	double GetGeoDistanceDegree( CRenderContext* pContext, double dSLon, double dSLat, double dELon, double dELat )
	{
		if(pContext && pContext->GetMapNode())
		{
			return osgEarth::GeoMath::distance(osg::DegreesToRadians(dSLat),osg::DegreesToRadians(dSLon), osg::DegreesToRadians(dELat), osg::DegreesToRadians(dELon), pContext->GetMapNode()->getMapSRS()->getEllipsoid()->getRadiusEquator());
		}

		return  0.0; 
	}

	double GetGeoRhumbDistanceDegree(CRenderContext* pContext, double dSLon, double dSLat, double dELon, double dELat)
	{
		if(pContext && pContext->GetMapNode())
		{
			return osgEarth::GeoMath::rhumbDistance(osg::DegreesToRadians(dSLat),osg::DegreesToRadians(dSLon), osg::DegreesToRadians(dELat), osg::DegreesToRadians(dELon), pContext->GetMapNode()->getMapSRS()->getEllipsoid()->getRadiusEquator());
		}

		return  0.0; 
	}

	osg::Vec3d GeoMidPointDegree(osg::Vec3d sLLH, osg::Vec3d eLLH)
	{
		osg::Vec3d llh;

		osgEarth::GeoMath::midpoint(osg::DegreesToRadians(sLLH.y()),osg::DegreesToRadians(sLLH.x()),
			osg::DegreesToRadians(eLLH.y()),osg::DegreesToRadians(eLLH.x()),
			llh.y(),llh.x());
		llh.z() = (sLLH.z() + eLLH.z()) * 0.5; 

		return llh;
	}

	double RhumbBearing(osg::Vec3d sLLH, osg::Vec3d eLLH)
	{
		return osgEarth::GeoMath::rhumbBearing(osg::DegreesToRadians(sLLH.y()),osg::DegreesToRadians(sLLH.x()),
			osg::DegreesToRadians(eLLH.y()),osg::DegreesToRadians(eLLH.x()));
	}

	class DrawableVisitor : public osg::NodeVisitor
	{
	public:
		DrawableVisitor(bool e)
			:m_enableReflect(e)
		{
			setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
		}

		virtual void apply(osg::Drawable& drawable)
		{
			drawable.setEnableReflection(m_enableReflect);
		}
	private:
		bool m_enableReflect;

	};

	bool J2000XYZ2WGS84XYZ(CRenderContext* pContext,  float fX, float fY, double& dX, double& dY, double& dZ )
	{
		if (pContext)
		{
			osg::Vec3d vecWorld;
			if(pContext->GetMapNode()->getTerrain()->getWorldCoordsUnderMouse(pContext->GetView(), fX, fY, vecWorld))
			{
				double dAngle = UT12GMST() / (24*3600) * 2*(osg::PI);
				dX = osg::Vec3d(cos(dAngle), sin(dAngle), 0) * vecWorld;
				dY = osg::Vec3d(-sin(dAngle), cos(dAngle), 0) * vecWorld;
				dZ = osg::Vec3d(0, 0 ,1) * vecWorld;

				return true;
			}
		}
		return false;
	}

	double UT12GMST()
	{
		double dT0 = (ComputeJD0h() - 2451545) /36525;
		double dT = (ComputeJD() - 2451545) / 36525;

		osgEarth::DateTime dUtc = osgEarth::DateTime();
		double dUT1 = dUtc.hours()* 3600;

		double dGMST = 24110.54841 + 8640184.812866 * dT0 + 1.002737909350795 * dUT1 +
			0.093104 * dT * dT - 0.0000062 * dT * dT * dT;

		return dGMST;
	}


	double ComputeJD0h()
	{
		osgEarth::DateTime dUtc = osgEarth::DateTime();
		//计算儒略日
		int y, m, B;
		y = dUtc.year(); 
		m = dUtc.month(); 
		//如果日期在1月或2月，则被看作是在前一年的13月或14月
		if (dUtc.month() <= 2) 
		{ 
			y = dUtc.year() - 1; 
			m = dUtc.month() + 12; 
		}
		/* 对格里高利历(即1582年10月15日以后)，有
		B = 2 - Y/100 + Y/400.
		另外，对于儒略历(即1582年10月15日之前)，取B=0. */
		B = -2; 
		if (dUtc.year() > 1582 || (dUtc.year() == 1582 && (dUtc.month() > 10 || (dUtc 
			.month() == 10 && dUtc.day() >= 15)))) 
		{ 
			B = y / 400 - y / 100; 
		}
		return (floor(365.25 * y) + 
			floor(30.6001 * (m + 1)) + B + 1720996.5 + 
			dUtc.day()); 
	}

	double ComputeJD()
	{
		osgEarth::DateTime dUtc = osgEarth::DateTime();
		//计算儒略日
		int y, m, B;
		y = dUtc.year(); 
		m = dUtc.month(); 
		//如果日期在1月或2月，则被看作是在前一年的13月或14月
		if (dUtc.month() <= 2) 
		{ 
			y = dUtc.year() - 1; 
			m = dUtc.month() + 12; 
		}
		// 对格里高利历(即1582年10月15日以后)，有
		//	B = 2 - Y/100 + Y/400.
		//	另外，对于儒略历(即1582年10月15日之前)，取B=0. 
		B = -2; 
		if (dUtc.year() > 1582 || (dUtc.year() == 1582 && (dUtc.month() > 10 || (dUtc 
			.month() == 10 && dUtc.day() >= 15)))) 
		{ 
			B = y / 400 - y / 100; 
		}
		return (floor(365.25 * y) + 
			floor(30.6001 * (m + 1)) + B + 1720996.5 + 
			dUtc.day() + dUtc.hours() / 24.0 ); 
	}

	bool Quat2Rotate( const osg::Quat& quat, osg::Vec3& vecRotate )
	{
		double q0 = quat.w();
		double q1 = quat.x();
		double q2 = quat.y();
		double q3 = quat.z();

		vecRotate.x() = float(atan2(2*(q2*q3+q0*q1), (q0*q0-q1*q1-q2*q2+q3*q3)));
		vecRotate.y() = float(asin(-2*(q1*q3-q0*q2)));
		vecRotate.z() = float(atan2(2*(q1*q2+q0*q3), (q0*q0+q1*q1-q2*q2-q3*q3)));

		return true;
	}

}