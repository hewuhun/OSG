/**************************************************************************************************
* @file TritonContext.h
* @note Triton 海洋上下文
* @author g00034
* @data 2017-3-3
**************************************************************************************************/

#ifndef TRITON_CONTEXT__HH__
#define TRITON_CONTEXT__HH__

#include <osg/Referenced>
#include <FeTriton/Triton.h>
#include <osgEarth/SpatialReference>
#include <osg/RenderInfo>
#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>
#include <osgEarthUtil/Ocean>
#include <FeOcean/OceanNode.h>

namespace FeOcean
{
	 /**
		* @class TritonContext
		* @brief 海洋上下文
		* @note 改类封装了海洋的资源管理器，环境管理器，以及海洋渲染器
		* @author c000010
	*/
	class TritonContext : public COceanContext
	{
	public:
		TritonContext(const std::string& strTritonPath);
		virtual ~TritonContext();
	
	public:
		/**
		  * @note: 获得海洋环境管理器
		*/
		Triton::Environment * GetEnvironment() const { return m_pEnvironment; }

		/**
		  * @note: 获得海洋渲染类指针
		*/
		Triton::Ocean * GetOcean() const { return m_pOcean; }

	private:
		/**
		  * @note: 初始化海洋资源
		  * @param contextID [in] 当前渲染上下文ID
		*/
		virtual bool InitilizeImpl(unsigned int contextID);

		/**
		  * @note: 更新海洋
		  * @param dSimTime [in] 模拟时间点（单位：秒）
		*/
		virtual void UpdateImpl(double dSimTime);

	private:
		Triton::ResourceLoader           * m_pResourceLoader; // 海洋资源加载器
		Triton::Environment              * m_pEnvironment;    // 海洋环境对象
		Triton::Ocean                    * m_pOcean;          // 海洋对象
		std::string                        m_strTritonPath;   // 海洋数据的路径
	};

}
#endif
