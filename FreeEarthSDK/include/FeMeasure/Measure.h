/**************************************************************************************************
* @file Measure.h
* @note 量算分析基类，定义了量算的使用接口
* @author g00034
* @data 2017-1-5
**************************************************************************************************/

#ifndef FE_MEASURE_H
#define FE_MEASURE_H

#include <osg/Group>
#include <osgEarthFeatures/Feature>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthAnnotation/LabelNode>

#include <osgEarth/MapNodeObserver>

#include <osgDB/ConvertUTF>

#include <FeMeasure/Export.h>

#include <FeUtils/StrUtil.h>
#include <FeUtils/RenderContext.h>


namespace FeMeasure
{
	/// 工具函数，输出格式化字符串
	std::string ReviseDistanceCalculate( double dDistance );
	std::string ReviseAreaCalculate(double dArea);

	/// 标记需要释放的指针
	#define _ptr_need_free(ptr)    ptr*

	/// 标记引用指针，无需释放
	#define _ptr_observer(ptr)     ptr*


	/// 量算分析的类型
	typedef enum EMeasureType
	{
		EM_SurfDist = 0,					// 地表距离测量
		EM_SpaceDist = 1,				    // 直线距离测量
		EM_SpaceArea = 2, 		            // 空间面积测量
		EM_ShadowArea = 3,			        // 投影面积测量
		EM_HeiDist = 4, 					// 高差分析
		EM_LineVisible = 5, 			    // 可视分析
		EM_RadialLineVisible = 6,	        // 视域分析
		EM_ProfileAnalysis  = 7,		    // 剖面分析
		EM_BuildingsView = 8,				// 建筑群视域分析
		EM_GradientAnalysis = 9,			// 坡度坡向分析
		EM_FillCutAnalysis = 10,   			// 填挖方分析
		EM_FlootAnalysis = 11,     			// 淹没分析
		EM_Contour = 12,                    // 等高线
		EM_Nothing,                			// 空
	};

	/// 测量的关键动作
	typedef enum EMeasureAction
	{
		EM_BEGIN = 0,					// 开始测量
		EM_UPDATE = 1,				    // 测量更新
		EM_END = 2,						// 结束测量
		EM_RESULT_CHANGED = 3,          // 测量结果发生变化。在结束测量后，通过特定接口更新了测量结果。如，水淹模拟等
	};
}

namespace FeMeasure
{
	/**
	  * @class CMeasureResultData
	  * @brief 测量结果数据基类，不同的测量类型可定制不同的数据
	  * @note  作为单纯的数据类来使用
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CMeasureResultData
	{
	public:
		/**
		  *@note: 析构函数
		*/
		virtual ~CMeasureResultData(){}

	protected:
		/**
		  *@note: 构造函数
		*/
		CMeasureResultData(){}
	};
}

namespace FeMeasure
{
	class CMeasure;

	/**
	  * @class CMeasureResult
	  * @brief 测量结果基类，定义了通用的测量结果生成接口，所有测量结果类都从该类继承
	  * @note 测量结果类对外应该只提供结果获取接口，其他接口由测量类提供
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CMeasureResult : public osg::Group
	{
		/// 测量类作为友元，可以访问其私有计算接口
		friend class CMeasure;

	public:
		/**
		*@note: 获取测量结果数据
		*/
		template < typename T>
		const T* GetResultData() const { return dynamic_cast<const T*>(m_pResultData); }
		
	protected:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CMeasureResult(FeUtil::CRenderContext* pRenderContext);

		/**
		*@note: 析构函数
		*/
		virtual ~CMeasureResult();

		/**
		*@note: 获取当前渲染上下文
		*/
		FeUtil::CRenderContext* GetRenderContext(){ return m_opRenderContext.valid() ? m_opRenderContext.get() : NULL; }
		
	private:
		/**
		*@note: 根据输入点计算测量结果
		*/
		bool Calculate(const osg::Vec3dArray& vecPointsLLH);

		/**
		*@note: 计算最终结果。部分测量为了提高效率，在测量结束时才进行计算
		*/
		bool FinalCalculate();

		/**
		*@note: 根据输入点计算测量结果
		*/
		virtual bool CalculateImplement(const osg::Vec3dArray& vecPointsLLH) = 0;
		
		/**
		*@note: 计算最终结果。部分测量为了提高效率，不会实时更新，而只有在测量结束时才显示结果
		*/
		virtual bool FinalCalculateImplement() { return true; }

		/**
		*@note: 创建计算结果数据对象
		*/
		virtual CMeasureResultData* CreateResultData() = 0;
	
	private:
		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>  m_opRenderContext;

		/// 测量结果数据
		_ptr_need_free(CMeasureResultData)         m_pResultData;

		/// 保护计算的互斥对象
		OpenThreads::Mutex                         m_mutexCalc;
	};

	/// 测量结果集类型
	typedef std::vector< osg::ref_ptr<CMeasureResult> > MEASURE_RESULT_LIST;
}


namespace FeMeasure
{
	class CMeasure;
	
	/**
	  * @class CMeasureCallback
	  * @brief 测量的事件回调类
	  * @note 测量的事件回调类，是一个虚基类，前端与分析结果的通信主要通过继承该回调类
	  * @author c00005
	*/
	class FEMEASURE_EXPORT CMeasureCallback : public osg::Referenced
	{
	public:
		/**
		  *@note: 构造函数
		*/
		CMeasureCallback(){};

		/**
		  *@note: 析构函数
		*/
		~CMeasureCallback(){};

	public:
		/**  
		  * @brief 发送测量消息
		  * @note 派生类通过继承此函数来接收测量事件，并进行相关处理  
		  * @param msg [in] 测量消息类型
		  * @param pMeasure [in] 当前测量对象
		  * @return 无返回值
		  */
		virtual void SendMsg(EMeasureAction msg, CMeasure* pMeasure){};
	};

	typedef std::vector< osg::observer_ptr<CMeasureCallback> > MEASURE_OBSERVER_LIST;
}


namespace FeMeasure
{
	/**
	  * @class CMeasure
	  * @brief 量算分析基类
	  * @note 量算分析基类，定义了统一的测量接口和结果获取接口，所有具体测量都从该类继承
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CMeasure : public osg::Group
	{
	public:
		/**
		*@note: 析构函数
		*/
		virtual ~CMeasure();

		/**
		*@note: 设置和获得唯一的标示符
		*/
		void SetKey(const std::string& strKey){m_strKey = strKey;}
		std::string GetKey() const {return m_strKey;}

		/**
		*@note: 设置和获得显示的标签
		*/
		void SetTitle(const std::string& strTitle){m_strTitle = strTitle;}
		std::string GetTitle() const {return m_strTitle;}

		/**
		*@note: 获取当前测量类型
		*/
		EMeasureType GetMeasureType() const { return m_measureType; }

		/**  
		* @brief 开始一次新的测量
		* @note 开始一次新的测量，只有停止了上一次测量才能开始新的测量
		* @return 是否执行成功
		*/
		bool BeginMeasure();

		/**  
		* @brief 结束一次测量
		* @note 结束正在进行的测量
		* @return 是否执行成功
		*/
		bool EndMeasure();

		/**
		*@note: 是否正在进行测量
		*/
		bool IsMeasuring() const { return m_bDoingMeasure; }

		/**
		*@note: 添加控制点
		*/
		void AddCtrlPoint(const osg::Vec3d& vecPosLLH);
		
		/**
		*@note: 设置控制点集
		*/
		void SetCtrlPoints(const osg::Vec3dArray& vecPointsLLH);

		/**
		* @note: 替换控制点
		* @param vecPosLLH [in] 要替换的位置值（单位：经纬高）
		* @param nIndex [in] 要替换的控制点索引，默认为-1，则替换末尾控制点
		*/
		bool ReplaceCtrlPoint(const osg::Vec3d& vecPosLLH, int nIndex = -1);

		/**
		*@note: 获取控制点集
		*/
		const osg::Vec3dArray& GetCtrlPoints();

		/**
		*@note: 清除控制点集
		*/
		void ClearCtrlPoints();

		/**
		*@note: 计算生成新的测量结果，该函数会根据已有控制点生成新的测量结果
		*/
		void CalcNewResult();

		/**
		*@note: 更新当前测量结果
		*/
		void UpdateResult();

		/**
		*@note: 获取指定索引的测量结果对象, 索引从0开始
		*/
		CMeasureResult* GetMeasureResult(unsigned int uIndex)
		{
			return (uIndex < GetMeasureResultCount()) ? m_listResult.at(uIndex) : NULL;
		}

		/**
		*@note: 获取测量结果数量
		*/
		unsigned int GetMeasureResultCount() const { return m_listResult.size(); }

		/**
		*@note: 获取当前测量结果对象，如果目前还未生成测量结果，则创建一个新的测量结果。
		*/
		const CMeasureResult* GetCurrentResult() { return GetOrCreateCurrentResult(); }

		/**
		*@note: 清除所有测量结果
		*/
		void ClearResult();

		/**
		*@note：添加监视器，监视器作为测量事件的通知器使用
		*/
		void RegisterObserver(CMeasureCallback* pObserver);

		/**
		*@note：删除监视器
		*/
		void RemoveObserver(CMeasureCallback* pObserver);

	protected:
		/**
		*@note: 构造函数
		*/
		CMeasure(FeUtil::CRenderContext* pRenderContext);

		/**
		*@note: 数据成员读取函数，供派生类使用
		*/
		osg::Vec3dArray& Vertexes() { return *m_rpVertexes; }
		FeUtil::CRenderContext* GetRenderContext() { return m_opRenderContext.valid() ? m_opRenderContext.get() : NULL; }
		EMeasureType& MeasureType() { return m_measureType; }

		/**
		*@note: 通知所有的监视器当前测量的事件
		*/
		void SendMeasureMsg(EMeasureAction action);

		/**
		*@note: 创建新的测量结果
		*/
		void CreateNewResult()
		{ 	
			m_listResult.push_back(CreateMeasureResult());
			addChild(m_listResult.back().get());
		}

		/**
		*@note: 获取当前测量结果对象，如果目前还未生成测量结果，则创建一个新的测量结果。
		*/
		CMeasureResult* GetOrCreateCurrentResult() 
		{
			if(m_listResult.empty()) CreateNewResult();
			return m_listResult.back();
		}

	private:
		/**
		*@note: 创建测量结果，派生类实现该接口，返回自己的测量结果对象
		*/
		virtual CMeasureResult* CreateMeasureResult() = 0;

		/**
		*@note: 添加控制点具体实现
		*/
		virtual void AddCtrlPointImplement(const osg::Vec3d& vecPosLLH);
		
		/**
		*@note: 设置控制点集具体实现
		*/
		virtual void SetCtrlPointsImplement(const osg::Vec3dArray& vecPointsLLH);
		
	private:
		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>  m_opRenderContext;

		/// 控制点列表
		osg::ref_ptr<osg::Vec3dArray>    m_rpVertexes;

		/// 测量结果列表
		MEASURE_RESULT_LIST              m_listResult;

		/// 监视器列表
		MEASURE_OBSERVER_LIST            m_listObserver;

		/// 测量功能名称
		std::string                      m_strKey; 

		/// 测量功能的显示名称
		std::string                      m_strTitle; 

		/// 测量类型
		EMeasureType                     m_measureType;

		/// 是否正在进行一次测量
		bool                             m_bDoingMeasure;
	};
}


#endif //FE_MEASURE_H
