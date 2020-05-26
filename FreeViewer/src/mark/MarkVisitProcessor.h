/**************************************************************************************************
* @file MarkVisitProcessor.h
* @note 场景标记遍历处理器
		主要用于配合场景管理对象，集中处理不同场景标记的特殊操作

* @author g00034
* @data 2017-02-09
**************************************************************************************************/
#ifndef MARK_VISIT_PROCESSOR_H
#define MARK_VISIT_PROCESSOR_H 1

#include <FeExtNode/ExternNodeVisitor.h>

namespace FeExtNode
{
	class CExFeatureNode;
	class CExLocalizedNode;
}

namespace FreeViewer
{
	 /**
      * @class CMarkVisitProcessor
      * @note 场景标记遍历处理器
      * @author g00034
    */
	class CMarkVisitProcessor : public FeExtNode::CExternNodeVisitor
	{
	public:
		/// 标记的处理状态宏，处理器根据当前状态进行相应处理
		enum E_Mark_Process_Type
		{
			E_MARK_DO_NOTHING = 0,    // 什么都不做

			E_MARK_GET_ICON_PATH,     // 获取标记对应的icon路径
			E_MARK_INITIALIZE,        // 标记的初始化操作

			E_MARK_SET_POSITION,      // 设置位置，处理鼠标左键点击
			E_MARK_SET_SIZE,          // 设置尺寸位置
			E_MARK_UPDATE_POSITION,   // 更新位置，处理绘制过程中鼠标移动
			E_MARK_STOP_DRAW,         // 结束绘制，处理鼠标右键点击结束绘制后事件
		};

	public:
		/// 数据基类，主要用于辅助转换
		struct CMarkDataBase{virtual ~CMarkDataBase(){}};

		/// 数据模板类，便于处理不同的数据
		template <class T>
		struct CProcessData : public CMarkDataBase
		{
			CProcessData(T& data): m_pData(&data) {}
		
			/// 要处理的数据
			T*    m_pData;       
		};

	public:
		/**  
		  * @brief 构造函数
		*/
		CMarkVisitProcessor();

		/**  
		  * @brief 获取标记是否绘制标识
		*/
		bool GetFlag();
		
		/**  
		  * @brief 执行指定操作
		*/
		void DoProcess(FeExtNode::CExternNode* pMarkNode, E_Mark_Process_Type eProcessType);

		/**  
		  * @brief 执行指定操作，携带处理数据
		*/
		template <class T>
		void DoProcess(FeExtNode::CExternNode* pMarkNode, E_Mark_Process_Type eProcessType, CProcessData<T>& data)
		{
			m_pProcessData = &data;
			DoProcess(pMarkNode, eProcessType);
			m_pProcessData = 0;
		}

		/**  
		  * @brief 根据类型进入具体类型访问器进行不同的处理
		  * @param 参数 [in] externNode
		  * @return bool
		*/
		virtual bool VisitExit(FeExtNode::CExComposeNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExPlaceNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExTextNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExTiltModelNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExLineNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExPolygonNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExBillBoardNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExOverLayNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExLodModelNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExArcNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExEllipseNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExRectNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExSectorNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExCircleNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExLabelNode& externNode);
		//virtual bool VisitEnter(FeExtNode::CExAssaultNode& externNode);
		//virtual bool VisitEnter(FeExtNode::CExAttackNode& externNode);
		//virtual bool VisitEnter(FeExtNode::CExStraightArrowNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExParticleNode& externNode);

	private:
		/**  
		  * @brief 属于一大类型标记的通用处理函数
		*/
		void CommonTypeHandle(FeExtNode::CExternNode& externNode){}
		void CommonTypeHandle(FeExtNode::CExPointNode& externNode);
		void CommonTypeHandle(FeExtNode::CExLocalizedNode& externNode);
		void CommonTypeHandle(FeExtNode::CExFeatureNode& externNode);
		//void CommonTypeHandle(FeExtNode::CExStraightArrowNode& externNode);

		/**  
		  * @brief 获取要处理的具体数据
		*/
		template <class T>
		T* GetMarkData() 
		{
			CProcessData<T>* pData = dynamic_cast< CProcessData<T>* >(m_pProcessData); 
			return pData ? pData->m_pData : 0;
		}

	private:
		/// 处理类型
		E_Mark_Process_Type  m_eProcessStyle;

		/// 当前处理的数据
		CMarkDataBase*       m_pProcessData;

		// 标记是否绘制标识
		bool					m_bMarkFlag;
		
	};
}





#endif // MARK_VISIT_PROCESSOR_H
