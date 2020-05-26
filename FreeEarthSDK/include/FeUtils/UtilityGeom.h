/**************************************************************************************************
* @file TextureGeom.h
* @note 定义了场景中的贴图几何体
* @author l00008
* @data 2014-04-30
**************************************************************************************************/

#ifndef FE_TEXTURE_GEOME_H
#define FE_TEXTURE_GEOME_H

#include <osg/Geode>
#include <osgText/Text>

#include <FeUtils/Export.h>

namespace FeUtil
{
	/**
	* @brief 创建可贴图的面板，作为组件的面板
	* @note 创建可贴图的面板，作为组件的面板
	* @param strImagePath [in]（图片的路径名称）
	* @param vecCorner [in]（面板的偏移坐标）
	* @param dWidthRate [in]（宽度比例）
	* @param dHeightRate [in]（高度比例）
	* @param unWidth [in]（宽度）
	* @return 创建的Geode 
	*/
	extern FEUTIL_EXPORT osg::Geode* CreateBasePlate(
		const std::string& strImagePath, 
		const osg::Vec3d& vecCorner, 
		double dWidthRate,
		double dHeightRate,
		const osg::Vec4d& vecColor = osg::Vec4d(1.0, 1.0, 1.0, 0.5)
		);

	/**
	* @brief 创建一个矩形，可以设置宽度和颜色
	* @note 创建一个矩形，可以设置宽度和颜色
	* @param strImagePath [in]（图片的路径名称）
	* @param vecCorner [in]（面板的偏移坐标）
	* @param dWidthRate [in]（宽度比例）
	* @param dHeightRate [in]（高度比例）
	* @param unWidth [in]（宽度）
	* @return 创建的Geode 
	*/
	extern FEUTIL_EXPORT osg::Geometry* CreateRectangle(
		double dPosX,
		double dPosY,
		double dWidth, 
		double dHeight, 
		const osg::Vec4d& vecColor,
		float fLineWidth = 1.0,
		bool bType = false,
		const osg::Vec4d& vecFillColor = osg::Vec4d(1.0,1.0,0.0,0.5));

	/**  
	* @brief 创建一个卫星绕地球运行轨道以及赤道平面等
	* @note 创建一个卫星绕地球运行轨道以及赤道平面等
	* @param dHeight [in] 轨迹距离地面的高度
	* @param dOffsetAngle [in] 相对于赤道平面的偏移角度
	* @param dLineWidth [in] 轨迹线宽
	* @param bIsFill [in] 是否填充
	* @param colorSurface [in] 填充面的颜色值
	* @param colorLine [in] 轨迹线的颜色值
	* @return osg::Node* 返回轨道的Node
	*/
	extern FEUTIL_EXPORT osg::Node* CreateParallelSphere(
		double dHeight,
		double dOffsetAngle = 0.0, 
		double dLineWidth = 1.0, 
		bool bIsFill = true,
		osg::Vec4 colorSurface = osg::Vec4(1.0, 1.0, 0.0, 0.1), 
		osg::Vec4 colorLine = osg::Vec4(1.0, 1.0, 0.0, 1.0));

	/**  
	* @brief 创建文字显示的节点
	* @note 通过传入的参数构建文字显示的节点
	* @param corner [in] 文字显示的中心点
	* @param dWidth [in] 文字显示的宽度
	* @param dHeight [in] 文字显示的高度
	* @param strText [in] 要进行显示的文字
	* @param strImage [in] 图片，暂时无用
	* @return osgText::Text* 文字显示节点
	*/
	extern FEUTIL_EXPORT osgText::Text* CreateImageAndText(
		osg::Vec3d corner,
		double dWidth,
		double dHeight,
		std::string strText,
		std::string strImage="");

	/**  
	* @brief 创建三角形的节点
	* @note 通过传入的参数构建三角形
	* @param dPosX [in] 三角形左下角点的X坐标
	* @param dPosY [in] 三角形左下角点的Y坐标
	* @param dWidth [in] 三角形的宽度
	* @param dHeight [in] 三角形的高度
	* @param vecColor [in] 三角形三条边的颜色值
	* @param fLineWidth [in] 三角形三条边的线宽
	* @param bType [in] 图片，三角形的三个点的连接方式，默认false为LINE_LOOP，否则为TRIANGLE_STRIP
	* @return osg::Geometry* 三角形的多边形节点
	*/	
	extern FEUTIL_EXPORT osg::Geometry* CreateTrilateral(
		double dPosX,
		double dPosY,
		double dWidth, 
		double dHeight,
		const osg::Vec4d& vecColor,
		float fLineWidth = 1.0,
		bool bType = false);

	/**  
	* @brief 创建圆的节点
	* @note 通过传入的参数构建圆
	* @param dPosX [in] 圆的中心点的X坐标
	* @param dPosY [in] 圆的中心点的Y坐标
	* @param dRadius [in] 圆的半径
	* @param vecColor [in] 圆的颜色值
	* @param fLineWidth [in] 圆的线宽
	* @param bType [in] 图片，圆的顶点的连接方式，默认false为LINE_LOOP，否则为TRIANGLE_FAN
	* @return osg::Geometry* 三角形的多边形节点
	*/	
	extern FEUTIL_EXPORT osg::Geometry* CreateCircle( 
		double dPosX,
		double dPosY,
		double dRadius,  
		const osg::Vec4d& vecColor, 
		float fLineWidth = 1.0,
		bool bType = false);


	/**
	* @note 绘制军标模型
	* @param 参数 [in] corner 中心点
	* @param 参数 [in] width 宽度
	* @param 参数 [in] height 高度
	* @param 参数 [in] image 军标图
	* return 军标模型
	*/
	extern FEUTIL_EXPORT osg::Drawable* CreateImageSquare(const osg::Vec3& corner, const osg::Vec3& width, const osg::Vec3& height, osg::Image* image);
}

#endif //HG_HUD_CAMERA_H
