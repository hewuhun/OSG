/**************************************************************************************************
* @file ExStraightArrowNode.h
* @note 直箭头图元的基类
* @author w00040
* @data 2016-11-30
**************************************************************************************************/
#ifndef FE_EXTERN_STRAIGHT_ARROW_NODE_H
#define FE_EXTERN_STRAIGHT_ARROW_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExFeatureNode.h>
#include <FeAlg/FlightMotionAlgorithm.h>

namespace FeExtNode
{
	class FEEXTNODE_EXPORT CExStraightArrowNodeOption : public CExFeatureNodeOption
	{
	public:
		/**  
		  * @note 直箭头图元节点的配置选项的构造函数
		*/
		CExStraightArrowNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExStraightArrowNodeOption();
	};
}

namespace FeExtNode
{
	class FEEXTNODE_EXPORT CExStraightArrowNode : public CExFeatureNode
	{
	public:
		/**  
		  * @note 线图元节点基类的构造函数
		  * @param pMapNode [in] 地球节点
		  * @param opt [in] 线节点的配置属性集合
		*/
		CExStraightArrowNode(FeUtil::CRenderContext* pRender, CExStraightArrowNodeOption* opt);

		/**  
		  * @note 析构函数
		*/
		~CExStraightArrowNode(void);

	public:
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExStraightArrowNodeOption* GetOption();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**  
		  * @note 线不提供填充色功能
		*/
		virtual void SetFillColor(const osg::Vec4d& vecFillColor){}

		/**  
		  * @note 设置线、面图元的线颜色  
		*/
		virtual void SetLineColor(const osg::Vec4d& vecLineColor);
		
		/**  
		  * @note 设置线、面图元的线宽  
		*/
		virtual void SetLineWidth(const int& nLineWidth);

		/**  
		  * @note 设置线、面图元的线型  
		*/
		virtual void SetLineStipple(const unsigned int& unLineStipple);


		/**  
		  * @note 设置线、面图元的顶点集合  
		*/
		virtual void SetVertex(osg::Vec3dArray* pCoord);

		/**  
		  * @note 替换线、面图元指定的末尾处的顶点   
		*/
		virtual void ReplaceBackVertex(const osg::Vec3d& vecCoord);

		/**  
		  * @note 向线、面图元后面中压入顶点
		*/
		virtual void PushBackVertex(const osg::Vec3d& vecCoord);

		/**  
		  * @note 初始化图形节点 m_rpFeatureNode 对象，由子类调用进行初始化
		          调用该函数的子类必须同时实现 GetFeature 和 GetFeatureStyle 方法
		  * @return 成功true, 失败false
		*/
		virtual bool InitFeatureNode();

		/**  
		  * @note 更新编辑器
		*/
		virtual void UpdateEditor();

	private:
		/**  
		  * @note 获取矢量图形的基本构建对象，不同的子类实现不同的对象
		*/
		virtual osgEarth::Features::Feature* GetFeature();

		/**  
		  * @note 获取矢量图形的初始样式，子类可实现该方法返回不同的样式
		*/
		virtual osgEarth::Style GetFeatureStyle();

		/**
		  * @note 计算直箭头控制点坐标
		*/
		void ComputeCoordinate(const osg::Vec3d startPoint, const osg::Vec3d endPoint, osg::Vec3d &leftPoint, osg::Vec3d &rightPoint);

		/**
		  * @note 绘制直箭头
		*/
		void DrawStraightArrow();

		/**
		  * @note 绘制内部图元细节
		*/
		virtual void DrawInternalExtra() { DrawStraightArrow(); }

	protected:
		/// 矢量图形的基本构建对象
		osg::ref_ptr<osgEarth::Features::Feature>		 m_rpLeftFeature;
		osg::ref_ptr<osgEarth::Features::Feature>		 m_rpRightFeature;

		/// 包含图形构建对象的节点
		osg::ref_ptr<osgEarth::Annotation::FeatureNode>  m_rpLeftFeatureNode;
		osg::ref_ptr<osgEarth::Annotation::FeatureNode>  m_rpRightFeatureNode;

		/// 鼠标移动标识
		bool m_bMoveFlag;
	};
}

#endif