/**************************************************************************************************
* @file CoordConverter.h
* @note 坐标转换工具
* @author
* @data 2015-7-8
**************************************************************************************************/
#ifndef COORDINATE_CONVERTER_H
#define COORDINATE_CONVERTER_H

#include <osg/Vec3d>
#include <osg/Matrix>
#include <osg/observer_ptr>

#include <FeUtils/Export.h>
#include <FeUtils/RenderContext.h>


namespace FeUtil
{
	/**
	*@note: 转换本地经纬坐标为世界坐标系，本地坐标采用弧度表示
	*/
	extern FEUTIL_EXPORT bool RadianLLH2XYZ(CRenderContext* pContext, const osg::Vec3d& vecLLH, osg::Vec3d& vecXYZ);

	/**
	*@note: 转换本地经纬坐标为世界坐标系，本地坐标采用角度表示
	*/
	extern FEUTIL_EXPORT bool DegreeLLH2XYZ(CRenderContext* pContext, const osg::Vec3d& vecLLH, osg::Vec3d& vecXYZ);

	/**
	*@note: 转换世界坐标系为本地经纬坐标，本地坐标采用弧度表示
	*/
	extern FEUTIL_EXPORT bool XYZ2RadianLLH(CRenderContext* pContext, const osg::Vec3d& vecXYZ, osg::Vec3d& vecLLH);

	/**
	*@note: 转换世界坐标系为本地经纬坐标，本地坐标采用角度表示
	*/
	extern FEUTIL_EXPORT bool XYZ2DegreeLLH(CRenderContext* pContext, const osg::Vec3d& vecXYZ, osg::Vec3d& vecLLH);

	/**
	*@note: 转换屏幕坐标为本地经纬坐标，本地坐标采用角度表示
	*/
	extern FEUTIL_EXPORT bool ScreenXY2DegreeLLH(CRenderContext* pContext, float fX, float fY, osg::Vec3d& vecLLH);
	extern FEUTIL_EXPORT bool ScreenXY2DegreeLLH(CRenderContext* pContext, float fX, float fY, double& dLon, double& dLat, double& dHei);

	/**
	*@note: 转换屏幕坐标系为本地经纬坐标，本地坐标采用弧度表示
	*/
	extern FEUTIL_EXPORT bool ScreenXY2RadiaLLH(CRenderContext* pContext, float fX, float fY, osg::Vec3d& vecLLH);
	extern FEUTIL_EXPORT bool ScreenXY2RadiaLLH(CRenderContext* pContext, float fX, float fY, double& dLon, double& dLat, double& dHei);

	/**
	*@note: 转换本地经纬坐标为Matrix，本地坐标采用弧度表示
	*/
	extern FEUTIL_EXPORT bool RadiaLLH2Matrix(CRenderContext* pContext, const osg::Vec3d& vecLLH, osg::Matrix& matrix);

	/**
	*@note: 转换本地经纬坐标为Matrix，本地坐标采用角度表示
	*/
	extern FEUTIL_EXPORT bool DegreeLLH2Matrix(CRenderContext* pContext, const osg::Vec3d& vecLLH, osg::Matrix& matrix);

	/**
	*@note: 转换XYZ坐标为Matrix，本地坐标采用角度表示
	*/
	extern FEUTIL_EXPORT bool XYZ2Matrix(CRenderContext* pContext, const osg::Vec3d& vecXYZ, osg::Matrix& matrix);

	/**
	*@note: 由经纬度求得当前地形高度,经纬采用弧度计算
	*/
	extern FEUTIL_EXPORT bool DegreeLL2LLH(CRenderContext* pContext, osg::Vec3d& vecLLA);

	/**
	*@note: 由经纬度求得当前地形高度,经纬采用弧度计算
	*/
	extern FEUTIL_EXPORT bool RadiaLL2LLH(CRenderContext* pContext, osg::Vec3d& vecLLA);
	
	/**
	*@note: 由经纬度求得实际高度,经纬采用弧度计算
	*/
	extern FEUTIL_EXPORT bool DegreeLL2RealLLH(CRenderContext* pContext, osg::Vec3d& vecLLA);

	/**
	*@note: 由经纬度求得实际高度,经纬采用弧度计算
	*/
	extern FEUTIL_EXPORT bool RadiaLL2RealLLH(CRenderContext* pContext, osg::Vec3d& vecLLA);

	/**
	*@note: 四元数转欧拉角
	*/
	extern FEUTIL_EXPORT bool Quat2Rotate( const osg::Quat& quat, osg::Vec3& vecRotate );
	
	/**
	*@note: 局部世界坐标转成屏幕坐标
	*/
	extern FEUTIL_EXPORT bool ConvertLocalWorldCoordToScreen(CRenderContext* pContext, const osg::Vec3d& pos, osg::Vec2d& screenPos);

	/**
	*@note: 获得GeoDistance距离，获得地球坐标下的两点距离,输入的经纬度采用弧度
	*/
	extern FEUTIL_EXPORT double GetGeoDistance(CRenderContext* pContext, double dSLon, double dSLat, double dELon, double dELat);

	/**
	*@note: 获得GeoDistance距离，获得地球坐标下的两点距离,输入的经纬度采用角度表示
	*/
	extern FEUTIL_EXPORT double GetGeoDistanceDegree(CRenderContext* pContext, double dSLon, double dSLat, double dELon, double dELat);

	/**
	*@note: 获得GeoDistance距离，获得地球坐标下的两点距离,输入的经纬度采用角度表示
	*/
	extern FEUTIL_EXPORT double GetGeoRhumbDistanceDegree(CRenderContext* pContext, double dSLon, double dSLat, double dELon, double dELat);

	/**
	*@note: 获得两点geopos的中心坐标，sLLH、eLLH采用角度表示,返回值采用弧度
	*/
	extern FEUTIL_EXPORT osg::Vec3d GeoMidPointDegree(osg::Vec3d sLLH, osg::Vec3d eLLH);

	/**
	*@note: 获得两点geopos的方位角，,输入输出LLH采用角度表示
	*/
	extern FEUTIL_EXPORT double RhumbBearing(osg::Vec3d sLLH, osg::Vec3d eLLH);

	/**
	*@note: 转换J2000世界坐标到WGS84世界坐标
	*/
	extern FEUTIL_EXPORT bool J2000XYZ2WGS84XYZ(float fX, float fY, double& dX, double& dY, double& dZ);

	/**
	*@note: 计算自2000年1月1.5日起算的世界时转平恒星时
	*/
	extern FEUTIL_EXPORT double UT12GMST();

	/**
	*@note: 计算当天0时自2000年1月1.5日起算的世界时（只保留日期）儒略世纪数
	*/
	extern FEUTIL_EXPORT double ComputeJD0h();

	/**
	*@note: 计算自2000年1月1.5日起算的世界时（去掉日期保留时分秒，转换成的秒值）儒略世纪数
	*/
	extern FEUTIL_EXPORT double ComputeJD();
}


#endif //COORDINATE_CONVERTER_H
