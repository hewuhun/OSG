/**************************************************************************************************
* @file ExBillBoardNode.h
* @note 标记节点(可以带图标)的定义，用于在场景中显示标记
* @author c00005
* @data 2016-8-3
**************************************************************************************************/
#ifndef FE_EXTERN_BILLBOARD_NODE_H
#define FE_EXTERN_BILLBOARD_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExPointNode.h>

#include <FeUtils/RenderContext.h>

#include <osgEarth/GeoData>


namespace FeExtNode
{
	/**
	* @class CExBillBoardNodeOption
	* @note 标记节点的配置选项
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExBillBoardNodeOption : public CExPointNodeOption
	{
	public:
		/**  
		  * @note Text节点的配置选项的构造函数
		*/
		CExBillBoardNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExBillBoardNodeOption();

	public:
		/**  
		  * @note 获取标记的图片的路径
		*/
		std::string& imagePath() { return m_strImage; }
		
		/**  
		  * @note 获取标记的图片的大小
		*/
		osg::Vec2& imageSize() { return m_vecSize; }

	protected:
		///标记的图片
		std::string					m_strImage;

		///标记的图片的大小
		osg::Vec2					m_vecSize;
	};
}


namespace FeExtNode
{
	using namespace osgEarth::Annotation;

	/**
	* @class CExBillBoardNode
	* @note 标记节点类，主要用于绘制标记对象
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExBillBoardNode : public CExPointNode
	{
	public:
		/**  
		  * @note 点节点的构造函数
		  * @param pRender [in] 当前渲染上下文
		  * @param opt [in] Lod节点的配置属性集合
		*/
		CExBillBoardNode(FeUtil::CRenderContext* pRender, CExBillBoardNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		~CExBillBoardNode();

	public:
		/**  
		  * @note 设置标记的内容
		  * @param strName [in] 标记的内容
		*/
		virtual void SetName(const std::string& strName);

		/**  
		  * @note 获取标记的图标的路径
		  * @return std::string 标记的路径
		*/
		virtual std::string GetImagePath();

		/**  
		  * @note 设置标记的图标的路径
		  * @param strImage [in] 标记的图标的路径
		*/
		virtual void SetImagePath(const std::string& strImage);

		/**  
		  * @note 获取标记的图标的大小
		  * @return osg::Vec2 标记图标的大小
		*/
		virtual osg::Vec2 GetImageSize();

		/**  
		  * @note 设置标记图标的大小
		  * @param osg::Vec2 [in] 标记图标的大小
		*/
		virtual void SetImageSize(const osg::Vec2& vecSize);

		/**  
		  * @note 设置标记显示的位置
		  * @param geoPosition [in] 标记显示的位置，其中可以设置绝对高度和相对高度
		*/
		virtual void SetPosition(const osgEarth::GeoPoint& geoPosition);

		/**  
		  * @note 获取标记显示的位置  
		  * @return osgEarth::GeoPoint 返回的是标记显示的位置
		*/
		virtual osgEarth::GeoPoint GetPosition();


	public:
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExBillBoardNodeOption* GetOption();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**
		  * @note 创建编辑工具
		*/
		FeNodeEditor::CFeNodeEditTool* CreateEditorTool();

		/**
		  * @note 开始拖拽编辑
		*/
		virtual void DraggerStart();

		/**
		  * @note 拖拽编辑器矩阵变化量
		*/
		virtual void DraggerUpdateMatrix(const osg::Matrix& matrix);

		/**
		  * @note 获取编辑点位置
		*/
		virtual osg::Vec3d GetEditPointPositionXYZ();

	protected:
		/**  
		  * @note 当位置改变或者大小改变后调用，将放大后的图片半径增加到高度上
		*/
		virtual void UpdatePosition();

		/**  
		  * @note 更新图像显示，在改变了图像路径后调用
		*/
		virtual void UpdateImage();

	protected:
		/// 广告牌节点
		osg::ref_ptr<osg::Billboard>			  m_rpBillBoardNode;

		/// 图片节点
		osg::ref_ptr<osg::Image>			      m_rpImageNode; 

		/// 平移矩阵
		osg::ref_ptr<osg::MatrixTransform>		  m_opTransformMT;

		/// 缩放矩阵
		osg::ref_ptr<osg::MatrixTransform>		  m_opScaleMT;

		/// 图像初始高度
		double									  m_dOriginHeight;

		/// 图像初始宽度
		double									  m_dOriginWidth;
	};
}


#endif //FE_EXTERN_BILLBOARD_NODE_H
