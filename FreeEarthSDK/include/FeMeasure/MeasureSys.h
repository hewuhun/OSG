/**************************************************************************************************
* @file MeasureSys.h
* @note 定义了组成测量功能的模块
* @author g00034
* @data 2017-1-5
**************************************************************************************************/

#ifndef FE_MEASURE_SYSTEM_CALL_H
#define FE_MEASURE_SYSTEM_CALL_H

#include <FeMeasure/Export.h>
#include <FeMeasure/Measure.h>

#include <FeUtils/SceneSysCall.h>

#include <osgGA/GUIEventHandler>

using namespace osgGA;


namespace FeMeasure
{
	/**
	  * @class CMeasureEventHandler
	  * @brief 测量事件处理基类
	  * @note 测量事件处理基类，所有的测量事件处理器都从该类继承
	  * @author g00034
	*/
	class CMeasureEventHandler : public osg::Referenced
	{
	public:
		/// 鼠标事件枚举
		enum E_MOUSE_EVENT{
			E_MOUSE_LBUTTON_DOWN = 0,
			E_MOUSE_LBUTTON_UP,
			E_MOUSE_LBUTTON_CLICK,
			E_MOUSE_LBUTTON_DBCLICK,
			E_MOUSE_RBUTTON_DOWN,
			E_MOUSE_RBUTTON_UP,
			E_MOUSE_RBUTTON_CLICK,
			E_MOUSE_RBUTTON_DBCLICK,
			E_MOUSE_MOVE,
		};

	public:
		/**
		  *@note: 析构函数
		*/
		~CMeasureEventHandler(){}

		/**  
		  * @note 处理鼠标事件  
		  * @param pMeasure [in] 当前测量
		  * @param mouseEvent [in] 鼠标事件枚举
		  * @param vecMousePosLLH [in] 鼠标所在位置经纬高
		  * @return 成功处理返回true，否则返回false
		*/
		bool HandleMouseEvent(CMeasure* pMeasure, E_MOUSE_EVENT mouseEvent, osg::Vec3 vecMousePosLLH)
		{
			return HandleMouseEventImplement(pMeasure, mouseEvent, vecMousePosLLH);
		}

	private:
		/**  
		  * @note 鼠标事件具体处理函数  
		  * @param pMeasure [in] 当前测量
		  * @param mouseEvent [in] 鼠标事件枚举
		  * @param vecMousePosLLH [in] 鼠标所在位置经纬高
		  * @return 成功处理返回true，否则返回false
		*/
		virtual bool HandleMouseEventImplement(CMeasure* pMeasure, E_MOUSE_EVENT mouseEvent, osg::Vec3 vecMousePosLLH) = 0;
	};
	
	typedef std::vector< osg::ref_ptr<CMeasureEventHandler> >    MEASURE_EVENT_HANDLER_LIST;
}

namespace FeMeasure
{
	/**
	  * @class CDefaultMeasureEventHandler
	  * @brief 默认测量事件处理器
	  * @note 默认测量事件处理器，在外部不提供事件处理器时，默认使用该事件处理器
	  * @author g00034
	*/
	class CDefaultMeasureEventHandler : public CMeasureEventHandler
	{
	public:
		/**
		  *@note: 构造函数
		*/
		CDefaultMeasureEventHandler(){}

	private:
		/**  
		  * @note 鼠标事件具体处理函数  
		  * @param pMeasure [in] 当前测量
		  * @param mouseEvent [in] 鼠标事件枚举
		  * @param vecMousePosLLH [in] 鼠标所在位置经纬高
		  * @return 成功处理返回true，否则返回false
		*/
		virtual bool HandleMouseEventImplement(CMeasure* pMeasure, E_MOUSE_EVENT mouseEvent, osg::Vec3 vecMousePosLLH);
	};
}

namespace FeMeasure
{
	const std::string MEASURE_SYSTEM_CALL_DEFAULT_KEY("MeasureSystem");

	class CMeasureHandler;

	/**
	* @class CMeasureSys
	* @brief 测量功能的系统模块
	* @note 定义了测量功能的系统模块，主要管理多种测量的功能
	* @author g00034
	*/
	class FEMEASURE_EXPORT CMeasureSys : public FeUtil::CSceneSysCall
	{
	public:
		/**
		  *@note: 构造函数
		*/
		CMeasureSys();

	public:
		/**
		  *@note: 添加事件处理器
		*/
		void AddEventHandler(CMeasureEventHandler* eventHandler);

		/**
		  *@note: 移除事件处理器
		*/
		bool RemoveEventHandler(CMeasureEventHandler* eventHandler);

		/**
		  *@note: 清除所有事件处理器
		*/
		void ClearEventHandler();

		/**
		*@note: 设置当前活动的测量模块。
		*/
		void ActiveMeasure(FeMeasure::CMeasure* pMeasure);

		/**
		*@note:将当前的活跃测量重置为空，如果存在当前的活动模块，
		*	   则将其停止并从场景中移除。
		*/
		void DeactiveMeasure();

		/**
		*@note: 清除测量的结果，将场景中的结果直接删除,此函数仅对连续模式适用
		*/
		void ClearMesureResult();

	private:
		/**  
		  * @note 鼠标事件处理函数  
		  * @param pMeasure [in] 当前测量
		  * @param mouseEvent [in] 鼠标事件枚举
		  * @param vecMousePosLLH [in] 鼠标所在位置经纬高
		  * @return 成功处理返回true，否则返回false
		*/
		bool HandleEvent(CMeasure* pMeasure, CMeasureEventHandler::E_MOUSE_EVENT mouseEvent, osg::Vec3 vecMousePosLLH);

	public:
		/**
		*@note: 事件处理函数
		*/
		bool handle(const GUIEventAdapter& ea, GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*);

	protected:
		/**
		*@note: 实现本系统模型需要的初始化函数
		*/
		virtual bool InitiliazeImplement();

		/**
		*@note: 实现本系统模型需要的反初始化函数
		*/
		virtual bool UnInitiliazeImplement();

		/**
		*@note: 打开事件循环
		*/
		bool RunEventHandler();

		/**
		*@note: 关闭事件循环
		*/
		bool StopEventHandler();

	protected:
		/**  
		  * @brief 鼠标按下事件处理
		  * @param fMouseDownX [in] 鼠标所在位置屏幕坐标 x
		  * @param fMouseDownY [in] 鼠标所在位置屏幕坐标 y
		  * @param mk [in] 鼠标按键标识
		  * @return 成功处理返回true， 否则返回false
		*/
		virtual bool MouseDown(float fMouseDownX, float fMouseDownY, GUIEventAdapter::MouseButtonMask mk); 

		/**  
		  * @brief 鼠标抬起事件处理
		  * @param fMouseDownX [in] 鼠标所在位置屏幕坐标 x
		  * @param fMouseDownY [in] 鼠标所在位置屏幕坐标 y
		  * @param mk [in] 鼠标按键标识
		  * @return 成功处理返回true， 否则返回false
		*/
		virtual bool MouseUp(float fMouseDownX, float fMouseDownY, GUIEventAdapter::MouseButtonMask mk); 

		/**  
		  * @brief 鼠标移动事件处理
		  * @param fMouseDownX [in] 鼠标所在位置屏幕坐标 x
		  * @param fMouseDownY [in] 鼠标所在位置屏幕坐标 y
		  * @param mk [in] 鼠标按键标识
		  * @return 成功处理返回true， 否则返回false
		*/
		virtual bool MouseMove(float fMouseDownX, float fMouseDownY, GUIEventAdapter::MouseButtonMask mk); 

		/**  
		  * @brief 鼠标双击事件处理
		  * @param fMouseDownX [in] 鼠标所在位置屏幕坐标 x
		  * @param fMouseDownY [in] 鼠标所在位置屏幕坐标 y
		  * @param mk [in] 鼠标按键标识
		  * @return 成功处理返回true， 否则返回false
		*/
		virtual bool MouseDCheck(float fMouseDownX, float fMouseDownY, GUIEventAdapter::MouseButtonMask mk); 

	protected:
		/**
		  *@note: 析构函数
		*/
		virtual ~CMeasureSys();

	protected:
		/**
		* @class CMeasureHandler
		* @brief 测量内部事件处理类
		* @note 用户内部的事件捕获
		* @author g00034
		*/
		class CMeasureHandler : public GUIEventHandler
		{
		public:
			/**  
			  * @brief 构造函数
			  * @param pMeasureSys [in] 测量系统对象
			*/
			CMeasureHandler(CMeasureSys* pMeasureSys);

			/**  
			  * @brief 析构函数
			*/
			virtual ~CMeasureHandler();

			/**  
			  * @brief 事件处理函数
			  * @param ea [in] 事件适配器对象
			  * @param aa [in] 动作适配器对象
			  * @param pO [in] 发生事件的对象
			  * @param pNV [in] 节点遍历器对象
			  * @return 成功处理返回true，否则返回false
			*/
			virtual bool handle(const GUIEventAdapter& ea, GUIActionAdapter& aa, osg::Object* pO, osg::NodeVisitor* pNV);

		protected:
			/// 测量系统对象
			osg::observer_ptr<CMeasureSys>  m_opMeasureSys;
		};

	protected:
		/// 事件处理器
		osg::ref_ptr<CMeasureHandler>					m_rpHandler;   

		/// 当前执行的测量
		osg::ref_ptr<FeMeasure::CMeasure>				m_opCurMeasure; 

		/// 鼠标按下的X位置
		float											m_fMouseDownX;  

		/// 鼠标按下的Y位置
		float											m_fMouseDownY;  
		
		/// 测量事件处理器列表
		MEASURE_EVENT_HANDLER_LIST                      m_handlerList;    

		/// 默认测量事件处理器
		osg::ref_ptr<CMeasureEventHandler>              m_rpDefaultHandler; 
	};

}

#endif//HG_MEASURE_SYSTEM_CALL_H
