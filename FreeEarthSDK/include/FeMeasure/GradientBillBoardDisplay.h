/**************************************************************************************************
* @file GradientBillBoardDisplay.h
* @note 坡度坡向标牌节点
* @author g00034
* @data 2015-9-11
**************************************************************************************************/

#ifndef HG_GRADIENT_BILLBOARD_DISPLAY_H
#define HG_GRADIENT_BILLBOARD_DISPLAY_H 1

#include <osg/Group>
#include <osg/LineWidth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osgText/Text>

#include <osgEarth/MapNode>
#include <osgEarth/GeoData>
#include <osgEarthAnnotation/OrthoNode>

#include <FeMeasure/Export.h>

namespace FeMeasure
{
	/**
	  * @class CGradientBillBoardDisplay
	  * @brief 坡度坡向信息显示的标牌节点
	  * @author w00024
	*/
	class FEMEASURE_EXPORT CGradientBillBoardDisplay : public osgEarth::Annotation::OrthoNode
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pMapNode [in] 地球节点
		  * @param position [in] 位置
		  * @param characterSize [in] 字体大小
		  * @param textRow [in] 行数
		  * @param lineLen [in] 每行长度
		  * @param characterNum [in] 字符数
		*/
		CGradientBillBoardDisplay(osgEarth::MapNode *pMapNode, const osgEarth::GeoPoint &position,
			float characterSize = 13.0, int textRow = 5, 
			double lineLen = 0.0, double characterNum = 9.5);
		virtual ~CGradientBillBoardDisplay() {}

	public:
		/**  
		  * @brief 绘制标牌几何形状
		*/
		void DrawFixedGeometry();

		/**  
		  * @brief 设置文字
		  * @param textRow [in] 行索引
		  * @param strText [in] 文本内容
		  * @param text_encoding [in] 文本编码格式
		*/
		void SetText(int textRow, std::string strText, osgText::String::Encoding text_encoding = osgText::String::ENCODING_UTF8);

		/**  
		  * @brief 设置标牌显隐
		*/
		void SetVisible(bool bShow);

		/**  
		  * @brief 判断是否显示
		*/
		bool IsVisible();

	protected:
		/**  
		  * @brief 创建几何图元
		*/
		void CreateElement();

		/**  
		  * @brief 创建文字节点
		*/
		void CreateText();

	private:
		/// 连接线长度
		double m_dLineLen;

		/// 标牌高度
		double m_dQuadHeight;

		/// 标牌宽度
		double m_dQuadWidth;

		/// 字体大小
		double m_dCharacterSize;

		/// 文本行数
		int m_iTextRow;

		/// 根节点
		osg::ref_ptr<osg::Group>    m_pObjectGroup;

		/// 线段宽度
		osg::ref_ptr<osg::LineWidth> m_pLineWidth;		

		/// 连接线节点
		osg::ref_ptr<osg::Geode>    m_pFixedLineGeode;
		osg::ref_ptr<osg::Geometry> m_pFixedLineGeometry;

		/// 文本节点
		std::vector<osg::ref_ptr<osgText::Text> > m_vecText;
	};
}

#endif // HG_BILLBOARD_DISPLAY_H

