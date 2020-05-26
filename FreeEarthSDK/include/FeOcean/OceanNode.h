/**************************************************************************************************
* @file OceanNode.h
* @note 海洋节点基类， 定义了通用的海洋接口
* @author g00034
* @data 2017-3-3
**************************************************************************************************/

#ifndef HG_OCEAN_NODE__HH__
#define HG_OCEAN_NODE__HH__

#include <osgEarth/MapNode>
#include <osgEarth/SpatialReference>
#include <osgEarth/CullingUtils>
#include <osgViewer/Viewer>
#include <FeOcean/Export.h>
#include <osgEarthDrivers/engine_mp/MPGeometry>
#include <FeUtils/RenderContext.h>


namespace FeOcean
{
	/**
	  * @class COceanContext
	  * @brief 海洋上下文基类， 定义了通用的海洋上下文接口
	  * @author g00034
	*/
	class COceanContext : public osg::Referenced
	{
	public:
		virtual ~COceanContext(){}
	
	public:
		/**
		  * @note: 初始化海洋资源
		  * @param contextID [in] 当前渲染上下文ID
		*/
		void Initialize(unsigned int contextID);

		/**
		  * @note: 判读是否初始化海洋成功
		*/
		bool IsReady() const { return m_bInitialized; }

		/**
		  * @note: 设置地球空间坐标系
		*/
		void SetSRS(const osgEarth::SpatialReference * srs) { m_opSRS = srs; }
		const osgEarth::SpatialReference * GetSRS() const { return m_opSRS.valid() ? m_opSRS.get() : 0; }
		
		/**
		  * @note: 更新海洋
		  * @param dSimTime [in] 模拟时间点（单位：秒）
		*/
		void Update(double dSimTime);

	private:
		/**
		  * @note: 初始化海洋资源
		  * @param contextID [in] 当前渲染上下文ID
		*/
		virtual bool InitilizeImpl(unsigned int contextID) = 0;

		/**
		  * @note: 更新海洋
		  * @param dSimTime [in] 模拟时间点（单位：秒）
		*/
		virtual void UpdateImpl(double dSimTime) = 0;

	protected:
		COceanContext();

	private:
		osg::observer_ptr<const osgEarth::SpatialReference>   m_opSRS;        // 地球坐标系
		bool                                                  m_bInitialized; // 是否初始化成功
		OpenThreads::Mutex                                    m_InitMutex;
	};
}


namespace FeOcean
{
	class TritonNode;

	/**
	  * @class COceanNode
	  * @brief 海洋结点基类， 定义了通用的海洋接口
	  * @author g00034
	*/
	class FEOCEAN_EXPORT COceanNode : public osg::Group
	{
	public:
		virtual ~COceanNode();

		/**
		  * @note: 初始化海洋
		  * @param contextID [in] 当前渲染上下文ID
		*/
		void Initialize(unsigned int contextID);

		/**
		  *@note: 判断海洋是否初始化成功
		*/
		bool IsInitialized() const { return m_rpOceanContext.valid() ? m_rpOceanContext->IsReady() : false; }

		/**
		  *@note: 更新海洋
		*/
		void Update();

		/**  
		  * @note 设置海洋最大可视距离（垂直距离）
		  * @param dValue [in] 距离（单位：米）
		  * @return 最大可视距离
		*/
		void SetOceanMaxVisibleDistance(double dValue);
		double GetOceanMaxVisibleDistance() const;

		/**  
		  * @note 设置反射支持
		  * @param bEnable [in] 是否支持反射
		  * @return 是否支持反射
		*/
		void SetReflection(bool bEnable) { m_bEnableReflection = bEnable; }
		bool IsEnableReflection() const { return m_bEnableReflection; }

		/**
		  *@note: 如果为Triton海洋，则返回海洋节点，否则返回0
		*/
		virtual TritonNode* AsTritonNode() { return 0; }

		/**  
		  * @note 设置海平面高度  
		  * @param dValue [in] 海平面高度（单位：米）
		  * @return 海平面高度
		*/
		virtual void SetSeaLevel(double dValue) = 0;
		virtual double GetSeaLevel() const = 0;
	
		/**
		  *@note: 显隐控制
		*/
		virtual void Show();
		virtual void Hide();
		virtual bool IsHide() const;

		/**  
		  * @note 设置风速 
		  * @param dSpeed [in] 风速 （单位： 场景单元/秒）
		  * @return 风速
		*/
		virtual void SetWindSpeed(double dSpeed) = 0;
		virtual double GetWindSpeed() const = 0;

		/**  
		  * @note 设置风向
		  * @param dDirection [in] 风向 （单位：弧度）
		  * @return 风向
		*/
		virtual void SetWindDirection(double dDirection) = 0;
		virtual double GetWindDirection() const = 0;

		/**  
		  * @note 设置风波长
		  * @param dFetchLenght [in] 风波长 （单位：米）
		  * @return 风波长
		*/
		virtual void SetWindLength(double dFetchLenght) = 0;
		virtual double GetWindLength() const = 0;	

	private:
		/**
		  * @note: 初始化海洋
		*/
		virtual void InitializeImpl() = 0;
		
		/**
		  *@note: 更新海洋
		*/
		virtual void UpdateImpl() = 0;

	protected:
		/**
		  *@note: 构造函数
		*/
		COceanNode(FeUtil::CRenderContext* rc);

		/**
		  *@note: 获取坐标空间
		*/
		const osgEarth::SpatialReference * GetSRS();

		/**
		  *@note: 设置海洋上下文
		*/
		void SetOceanContext(COceanContext* pOceanContext) { m_rpOceanContext = pOceanContext; }

	protected:
		osg::ref_ptr<COceanContext>				  m_rpOceanContext;    // 海洋环境上下文
		osg::observer_ptr<osgViewer::View>		  m_opViewer;          // 主视口
		double                                    m_dMaxAltitude;      // 最大显示距离
		bool                                      m_bIsInVisibleRange; // 海洋是否在可视范围
		bool                                      m_bCanReflecting;    // 当前视角是否可以反射
		bool                                      m_bEnableReflection; // 是否开启了反射


	private:
		/**
		  * @class COceanUpdateCallback
		  * @brief 场景回调，在场景更新时用于更新海洋节点。
		  * @author g00034
		*/
		class COceanUpdateCallback : public osgViewer::ViewerBase::UpdateCallback
		{
		public:
			COceanUpdateCallback(COceanNode* tn) : m_opOceanNode(tn){}
			virtual void customOperator(const osgViewer::ViewerBase* viewerBase)
			{
				if(m_opOceanNode.valid())
				{
					m_opOceanNode->Update();
				}
			}
		private:
			osg::observer_ptr<COceanNode> m_opOceanNode;
		};

		osg::ref_ptr<COceanUpdateCallback>        m_rpOceanUpdateCallback; // 海洋更新回调
	};

}
#endif
