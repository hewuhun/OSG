/**************************************************************************************************
* @file ExLabelCullEventHandler.h
* @note 标牌裁剪事件处理类
* @author g00034
* @data 2016-9-8
**************************************************************************************************/

#ifndef EX_LABEL_CULL_EVENT_HANDLE
#define EX_LABEL_CULL_EVENT_HANDLE

#include <osgViewer/View>
#include <osgGA/GUIEventHandler>

#include <FeUtils/RenderContext.h>

#include <FeExtNode/Export.h>

namespace FeExtNode
{
	class CExLabelNode;

	/**
	  * @class CExLabelCullEventHandler
	  * @note 标牌裁剪类，用于对标牌的绑定点进行裁剪操作，进而对标牌进行显隐操作
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExLabelCullEventHandler : public osgGA::GUIEventHandler
	{
	public:
		CExLabelCullEventHandler(FeUtil::CRenderContext* pRender);

		~CExLabelCullEventHandler();

		/**  
		  * @note 添加要处理的标牌节点  
		  * @param pNode [in] 标牌节点  
		  * @return 是否添加成功
		*/
		bool AddLabel(CExLabelNode* pNode);

		/**  
		  * @note 移除标牌节点  
		  * @param pNode [in] 标牌节点  
		  * @return 是否移除成功
		*/
		bool RemoveLabel(CExLabelNode* pNode);

		/**  
		  * @note 获取当前有效标牌节点数
		  * @return 返回当前标牌节点数
		*/
		unsigned int GetValidLabelNum();

	protected:
		/**  
		  * @note 处理函数  
		*/
		virtual bool handle (const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) ;

		/**  
		  * @note 检测 XYZ 点是否不被裁剪  
		  * @param XYZ [in] 检测点
		  * @param eye [in] 视点
		  * @return 被裁剪返回false， 否则返回true
		*/
		bool IsNotCull(const osg::Vec3d& XYZ, const osg::Vec3d& eye);
		bool IsNotCull(CExLabelNode* pLabelNode, const osg::Vec3d& eye);

	protected:
		/// 当前视景器
		osg::observer_ptr<osgViewer::View> m_view;

		/// 标牌节点列表
        typedef std::list<osg::observer_ptr<CExLabelNode> > BillBoardNodeList;
		BillBoardNodeList m_NodeList;

		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>  m_opRenderContext;

		/// 互斥量
		OpenThreads::Mutex m_mutex;

		/// 视点参数
		osg::Vec3 m_eye;
		osg::Vec3 m_center;
		osg::Vec3 m_up;

		/// 视口参数
		int m_viewportWidth;
		int m_viewportHeight;
		bool m_bfirstFrame;
	};

}

#endif
