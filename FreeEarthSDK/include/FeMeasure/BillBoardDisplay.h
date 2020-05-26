/**************************************************************************************************
* @file BillBoardDisplay.h
* @note 定义量算分析信息显示接口，实现通过标牌显示类
* @author g00034
* @data 2017-1-5
**************************************************************************************************/

#ifndef HG_BILLBOARD_DISPLAY_H
#define HG_BILLBOARD_DISPLAY_H 1

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
	  * @class CMeasuringInfoDisplay
	  * @brief 测量过程中的信息显示
	  * @note 定义了添加信息的接口，具体显示方式由子类实现
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CMeasuringInfoDisplay : public osg::Referenced
	{
	public:
		~CMeasuringInfoDisplay(){}

		/**
		  * @note 显示/隐藏操作
		*/
		virtual void SetVisible(bool bShow){}

		/**
		  * @note 添加一行显示信息
		*/
		void AddString(const std::string& str){ m_strInfoList.push_back(str); StringChanged(); }

		/**
		  * @note 设置第 unRow 行的字符串，如果没有第 unRow 行，则创建
				unRow 从 0 开始计数
		*/
		void SetOrCreateString(unsigned int unRow, const std::string& str);

		/**
		  * @note 清除显示信息
		*/
		void ClearString() { m_strInfoList.clear(); StringChanged(); }

		/**
		  * @note 设置显示位置，单位（经纬高）
		*/
		void SetPosition(const osg::Vec3& vecPosLLH);

		/**
		  * @note 获取显示位置，单位（经纬高）
		*/
		osg::Vec3 GetPosition() { return m_vecPosLLH; }

		/**
		  * @note 将显示对象当做 osg::Node 类型
		  * 如果派生类实现的显示方式为通过 osg::Node 节点显示，则返回该节点
		*/
		virtual osg::Node* GetDisplayAsOsgNode() { return 0; }

	protected:
		CMeasuringInfoDisplay(){}

		/**
		  * @note 供派生类使用的接口
		*/
		osg::Vec3& PosLLH() { return m_vecPosLLH; }
		std::vector< std::string >& StrList() { return m_strInfoList; }

		/**
		  * @note 将所有信息组织成多行返回，获取多行数据
		*/
		std::string GetMultiString();

		/**
		  * @note 获取信息中最大字符串长度
		*/
		double GetMaxStringLen();

	private:
		/**
		  * @note 位置发生变化
		*/
		virtual void PositionChanged(){}

		/**
		  * @note 信息发生变化
		*/
		virtual void StringChanged(){}

		/**
		  * @note 设置指定位置的字符串
		*/
		bool SetString(unsigned int unIndex, const std::string& str);
	
	private:
		typedef std::vector< std::string > STR_INFO_LIST;

		/// 每个 string 代表一行显示数据
		STR_INFO_LIST              m_strInfoList; 

		/// 显示的位置(单位：经纬高)
		osg::Vec3                  m_vecPosLLH;   
	};


	/**
	  * @class CDisplayByBillBoard
	  * @brief 通过标牌显示测量过程中的信息
	  * @note 创建标牌，用于显示测量过程中的信息
	  * @author g00034
	*/
	class CDisplayByBillBoard : public CMeasuringInfoDisplay
	{
	public:
		CDisplayByBillBoard(osgEarth::MapNode *pMapNode, float characterSize = 13.0 );

		/**
		  * @note 返回标牌节点，将显示对象当做 osg::Node 类型
		  * 如果派生类实现的显示方式为通过 osg::Node 节点显示，则返回该节点
		*/
		virtual osg::Node* GetDisplayAsOsgNode(){ return m_rpBillboardNode.valid() ? m_rpBillboardNode.get() : NULL; }

		/**
		  * @note 显示/隐藏操作
		*/
		virtual void SetVisible(bool bShow);
		
	protected:
		/**
		  * @note 创建内部显示节点
		*/
		void CreateElement();

		/**
		  * @note 创建内部文字节点
		*/
		void CreateText();

		/**
		  * @note 绘制外边框
		*/
		void DrawFixedGeometry();

	private:
		/**
		  * @note 位置发生变化
		*/
		virtual void PositionChanged();

		/**
		  * @note 信息发生变化
		*/
		virtual void StringChanged();

		/**
		  * @note 重新计算标牌大小
		*/
		void CalcQuadSize();

	private:
		/// 标牌底部指示线长度
		double m_dLineLen;

		/// 标牌尺寸
		double m_dQuadHeight;
		double m_dQuadWidth;

		/// 字体大小
		double m_dCharacterSize;

		/// 标牌根节点
		osg::ref_ptr<osg::Group>    m_pObjectGroup;

		/// 连接线宽度
		osg::ref_ptr<osg::LineWidth> m_pLineWidth;		

		/// 连接线绘制对象
		osg::ref_ptr<osg::Geode>    m_pFixedLineGeode;
		osg::ref_ptr<osg::Geometry> m_pFixedLineGeometry;
		osg::ref_ptr<osg::Geode>    m_pFixedStippleLineGeode;
		osg::ref_ptr<osg::Geometry> m_pFixedStippleLineGeometry;

		/// 文本绘制对象
		osg::ref_ptr<osgText::Text> m_rpText;

		/// 标牌节点
		osg::ref_ptr<osgEarth::Annotation::OrthoNode>  m_rpBillboardNode;
	};
	
}



#endif // HG_BILLBOARD_DISPLAY_H

