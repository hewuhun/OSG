/**************************************************************************************************
* @file HgNebula.h
* @note 星云背景特效节点
* @author c00005
* @data 2017-4-1
**************************************************************************************************/
#ifndef HG_EARTH_SKY_H
#define HG_EARTH_SKY_H

#include <osg/MatrixTransform>

#include <osgEarth/MapNode>

#include <FeKits/Export.h>

namespace FeKit
{
	class CNebulaNode;

	/**
	* @class CNebula
	* @brief 星云背景
	* @note  主要提供了天空球和天空盒的实现，通过制定一张图片来创建天空球以及通过六张图片创建天空盒，
	*        如果图片读取失败，会默认禁止显示天空星云背景。
	* @author l00008
	*/
	class FEKIT_EXPORT CHgNebula : public osg::Group
	{
	public:
		/**  
		* @brief 星云背景构造函数
		* @param pMapNode [in] 地球节点
		* @param strImageFile [in] 星云背景贴图
		*/
		CHgNebula(osgEarth::MapNode* pMapNode, const std::string& strImageFile);

	public:
		/**
		*@note: 设置显隐的标识量
		*/
		void SetNebulaVisible(bool bShow);

		/**
		*@note: 获得显隐的标识量
		*/
		bool GetNebulaVisible() const;

		/**
		*@note: 设置当前的时间
		*/
		void SetDateTime(const osgEarth::DateTime& dateTime);

		/**
		*@note: 获得当前的时间
		*/
		const osgEarth::DateTime& GetDateTime() const;

	protected:
		virtual ~CHgNebula(void);

		/**
		*@note 初始化天空节点，主要根据配置的信息创建天空特效的各种要素。
		*/
		void Initialize(osgEarth::MapNode* pMapNode);

		/*
		*note: 设置不同的时间星云背景的移动
		*/
		void OnSetDateTime();

		/*
		*note: 每一帧遍历的处理
		*/
		virtual void traverse( osg::NodeVisitor& nv );

	protected:
		///远地星云半径
		float                                   m_fNebulaRadius; 

		///星云是否显示
		bool                                    m_bNebulaVisible;   

		///当前的时间
		osgEarth::DateTime						m_dateTime;  

		//星空贴图
		std::string                             m_strNebulaImage;    

		//星云的变换矩阵
		osg::ref_ptr<osg::MatrixTransform>      m_rpNebulaXform;   
	};
}
#endif //HG_EARTH_SKY_H
