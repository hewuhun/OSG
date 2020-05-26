/**************************************************************************************************
* @file MeasureFactory.h
* @note 量算分析工厂类的定义
* @author g00034
* @data 2017-1-5
**************************************************************************************************/

#ifndef FE_MEASURE_FACTORY_H
#define FE_MEASURE_FACTORY_H

#include <FeMeasure/Export.h>
#include <FeMeasure/Measure.h>


namespace FeMeasure
{
	/**
	  * @brief 量算分析工厂类接口
	  * @note 定义了创建量算对象的接口
	  * @author g00034
	*/
	class FEMEASURE_EXPORT IMeasureFactory : public osg::Referenced
	{
	public:
		/**
		  *@note: 析构函数
		*/
		~IMeasureFactory(){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create() = 0;

	protected:
		/**
		  *@note: 构造函数
		*/
		IMeasureFactory(){}
	};

	/**
	  * @brief 量算分析具体工厂基类
	  * @note 工厂基类，所有其他量算工厂从此类集成
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CMeasureFactory : public IMeasureFactory
	{
	public:
		/**
		  *@note: 析构函数
		*/
		~CMeasureFactory(){}

	protected:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CMeasureFactory(FeUtil::CRenderContext* pContext):m_opRenderContex(pContext){}

		/**
		  *@note: 获取当前渲染上下文
		*/
		FeUtil::CRenderContext* RenderContex(){ return m_opRenderContex.get(); }

	private:
		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContex;
	};



	/**
	  * @brief 空间距离工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CSpaceDistFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CSpaceDistFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 地表距离工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CSurfaceDistFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CSurfaceDistFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 高差分析工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CHeiDistFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CHeiDistFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 空间面积工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CSpaceAreaFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CSpaceAreaFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 投影面积工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CShadowAreaFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CShadowAreaFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 通视分析工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CIntervisibleFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CIntervisibleFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 视域分析工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CRadialLineFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CRadialLineFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 建筑物通视分析工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CBuildingViewFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CBuildingViewFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 坡度坡向分析工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CGradientFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CGradientFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 淹没分析工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CFloodFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CFloodFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 剖面分析工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CProfileFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CProfileFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 填挖方计算工厂
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CFillCutFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CFillCutFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};

	/**
	  * @brief 等高线工厂
	  * @author g00037
	*/
	class FEMEASURE_EXPORT CContourFactory : public CMeasureFactory
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CContourFactory(FeUtil::CRenderContext* pContext):CMeasureFactory(pContext){}

		/**
		  *@note: 量算对象创建接口
		*/
		virtual CMeasure* Create();
	};
}

#endif //FE_MEASURE_FACTORY_H
