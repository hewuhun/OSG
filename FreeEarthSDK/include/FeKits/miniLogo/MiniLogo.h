/**************************************************************************************************
* @file MiniLogo.h
* @note FreeEarth的Logo标识
* @author c00005
* @data 2015-7-8
**************************************************************************************************/
#ifndef FE_MINILOGO_H
#define FE_MINILOGO_H

#include <osg/NodeVisitor>
#include <osgText/Text>
#include <osgViewer/View>

#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>

#include <osgEarth/MapNode>

#include <FeKits/ScreenKits.h>

#include <FeKits/Export.h>

namespace FeKit
{
	/**
	* @class CMiniLogo
	* @brief FreeEarth的Logo标识
	* @note FreeEarth的Logo标识
	* @author c00005
	*/
	class FEKIT_EXPORT CMiniLogo : public CScreenKits
	{
	public:
		/**
		* @brief FreeEarth的Logo标识
		* @note FreeEarth的Logo标识
		* @param pView [in]（添加Logo的View）
		* @param nXOffset [in] X偏移
		* @param nYOffset [in] Y偏移
		* @param nWidth [in] 宽度
		* @param nHeight [in] 高度
		* @param ePlace [in] 位置
		*/
		CMiniLogo(
			int nXOffset, 
			int nYOffset, 
			int nWidth, 
			int nHeight, 
			CPlaceStrategy::KIT_PLACE ePlace, 
			osgViewer::View* pView, 
			const std::string& strTitle,
			const std::string& strIconPath="");

	public:
		/**  
		* @brief 设置Logo的图标路径
		*/
		void SetIconPath(const std::string& strPath);

		/**  
		* @brief 获取Logo的图标路径
		*/
		std::string GetIconPath();

	protected:
		~CMiniLogo(void);

		/**  
		* @brief 创建Logo节点
		*/
		osg::Geode* CreateLogoUI(const std::string& strTitle);

	protected:
		///Logo显示文本的节点
		osg::ref_ptr<osgText::Text>             m_rpTextLogo;

		///Logo图标路径
		std::string                             m_strIconPath;
	};
}

#endif //FE_MINILOGO_H
