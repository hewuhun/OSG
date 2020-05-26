/**************************************************************************************************
* @file CoordConverter.h
* @note 坐标转换工具
* @author w00024
* @data 2016-8-17
**************************************************************************************************/
#ifndef ALG_COORDINATE_CONVERTER_H
#define ALG_COORDINATE_CONVERTER_H 1

#include <osg/Vec3d>
#include <osg/Matrix>

#include <FeAlg/Export.h>

namespace FeAlg
{
	/**
	*@note: 转换本地经纬坐标为世界坐标系，本地坐标采用弧度表示
	*/
	extern FEALG_EXPORT osg::Vec3d RadianLLH2XYZ(const osg::Vec3d& vecLLH);

	/**
	*@note: 转换本地经纬坐标为世界坐标系，本地坐标采用角度表示
	*/
	extern FEALG_EXPORT osg::Vec3d DegreeLLH2XYZ(const osg::Vec3d& vecLLH);

	/**
	*@note: 转换世界坐标系为本地经纬坐标，本地坐标采用弧度表示
	*/
	extern FEALG_EXPORT osg::Vec3d XYZ2RadianLLH(const osg::Vec3d& vecXYZ);

	/**
	*@note: 转换世界坐标系为本地经纬坐标，本地坐标采用角度表示
	*/
	extern FEALG_EXPORT osg::Vec3d XYZ2DegreeLLH(const osg::Vec3d& vecXYZ);

	/**
	*@note: 转换本地经纬坐标为Matrix，本地坐标采用弧度表示
	*/
	extern FEALG_EXPORT void RadiaLLH2Matrix(const osg::Vec3d& vecLLH, osg::Matrix& matrix);

	/**
	*@note: 转换本地经纬坐标为Matrix，本地坐标采用角度表示
	*/
	extern FEALG_EXPORT void DegreeLLH2Matrix(const osg::Vec3d& vecLLH, osg::Matrix& matrix);

	/**
	*@note: 转换XYZ坐标为Matrix，本地坐标采用角度表示
	*/
	extern FEALG_EXPORT void XYZ2Matrix(const osg::Vec3d& vecXYZ, osg::Matrix& matrix);
}

#endif //COORDINATE_CONVERTER_H
