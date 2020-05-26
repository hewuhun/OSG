/**************************************************************************************************
* @file TritonNode.h
* @note Triton 海洋节点类
* @author g00034
* @data 2017-3-3
**************************************************************************************************/

#ifndef TRITON_NODE__HH__
#define TRITON_NODE__HH__

#include <OpenThreads/Thread>
#include <osgEarthUtil/Ocean>
#include <osgEarth/Map>
#include <FeOcean/OceanNode.h>
#include <FeOcean/TritonDrawable.h>
#include <FeUtils/RenderContext.h>
#include <osg/CullFace>
#include <osgViewer/Viewer>
#include <FeOcean/TritonContext.h>

namespace FeOcean
{
	/**
	  * @class TritonNode
	  * @brief Triton 海洋节点类，定义了 Triton 海洋的使用接口
	  * @author g00034
	*/
	class FEOCEAN_EXPORT TritonNode : public COceanNode
	{
		friend class TritonDrawable;

	public:
		/**  
		  * @note 构造函数  
		  * @param rc [in]  当前渲染上下文
		  * @param srs [in] 空间坐标系
		  * @param strTritonPath [in] 海洋数据路径
		*/
		TritonNode(FeUtil::CRenderContext * rc, const osgEarth::SpatialReference * srs, const std::string& strTritonPath);
		virtual ~TritonNode();

		/**
		  *@note: 如果为Triton海洋，则返回海洋节点，否则返回0
		*/
		virtual TritonNode* AsTritonNode() { return this; }

		/**  
		  * @note 设置海平面高度  
		  * @param dValue [in] 海平面高度（单位：米）
		  * @return 海平面高度
		*/
		virtual void SetSeaLevel(double dValue);
		virtual double GetSeaLevel() const;

		/**  
		  * @note 设置风速 
		  * @param dSpeed [in] 风速 （单位： 场景单元/秒）
		  * @return 风速
		*/
		virtual void SetWindSpeed(double dSpeed);
		virtual double GetWindSpeed() const;

		/**  
		  * @note 设置风向
		  * @param dDirection [in] 风向 （单位：弧度）
		  * @return 风向
		*/
		virtual void SetWindDirection(double dDirection);
		virtual double GetWindDirection() const;
		
		/**  
		  * @note 设置风波长
		  * @param dFetchLenght [in] 风波长 （单位：米）
		  * @return 风波长
		*/
		virtual void SetWindLength(double dFetchLenght);
		virtual double GetWindLength() const;	

		osg::BoundingSphere computeBound() const;

		/**  
		  * @note 获取海洋上下文
		  * @return 海洋上下文
		*/
		TritonContext * GetTritonContext() const;

		/**  
		  * @note 遍历更新
		*/
		void traverse(osg::NodeVisitor& nv);

		/**  
		  * @note 设置大气层节点
		*/
		void SetAtmosphere(osg::Node* n) { m_opAtmosphere = n; }

		/**  
		  * @note 设置太阳节点
		*/
		void SetSun(osg::Node* n) { m_opSun = n; }

		/**  
		  * @note 设置月亮节点
		*/
		void SetMoon(osg::Node* n) { m_opMoon = n; }

		/**  
		  * @note 计算射线与地球的交点
		*/
		bool intersectWithRay(const osg::Vec3d& start,const osg::Vec3d& end,osg::Vec3d& hitPoint);

	protected:
		/**  
		  * @note 将原始场景相机节点从场景中移除
		*/
		void removeOriginalSceneCamFromRoot();

		/**  
		  * @note 将原始场景相机节点添加到场景中
		*/
		void insertOriginalSceneCamToRoot();

		/**  
		  * @note 更新海底渲染节点
		*/
		void updatePostRenderNode();

		/**  
		  * @note 更新GodRay
		*/
		void updateGodRay(osg::Camera* cam);

		void  OnSetSeaLevel();

		/**  
		  * @note 初始化高度相机，设置海洋透明度掩码图像 只供内部使用
		*/
		void SetupHeightMap();

		/**  
		  * @note 初始化反射相机
		*/
		void SetupReflectionCamera();

		/**  
		  * @note 计算反射相机的MVP矩阵
		*/
		void setReflectionMatrix(bool isReflecting);

		/**  
		  * @note 初始化原始场景相机
		*/
		void CreateOriginalSceneCam(int width = 1024,int height = 1024);

		/**  
		  * @note 删除原始场景相机
		*/
		void DeleteOriginalSceneCam();

		/**  
		  * @note 初始化海底显示节点
		*/
		void initPostRenderNode();

		/**  
		  * @note 初始化GodRay相机
		*/
		void initGodRayCamera();
		/**  
		  * @note 更新场景纹理尺寸
		*/
		void updateOriginalSceneTexSize();

	private:
		/**
		  * @note: 初始化海洋
		*/
		virtual void InitializeImpl();

		/**
		  *@note: 更新海洋
		*/
		virtual void UpdateImpl();

	private:
		/**
		  * @class TritonInitThread
		  * @brief 海洋节点初始化线程类
		  * @author g00034
		*/
		class TritonInitThread : public OpenThreads::Thread
		{
		public:
			TritonInitThread(TritonNode* tr):OpenThreads::Thread() ,m_opTritonNode(tr){}

		protected:
			virtual void run()
			{
				if(m_opTritonNode.valid())
				{
					//m_opTritonNode->initialize();
				}
			}

		private:
			osg::observer_ptr<TritonNode> m_opTritonNode;
		};

		/**
		  * @class HeightCamera
		  * @brief 高度相机，用于获取地形高度的纹理图
		  * @author g00034
		*/
		class HeightCamera : public osg::Camera
		{
			friend class TritonNode;
		public:
			HeightCamera();
		private:
			void setMatrix();
		private:
			osg::BoundingSphere m_boundSphere;
		};

		/**
		  * @class TileNodeVisitor
		  * @brief 地形节点遍历器，用于获取地形瓦片信息，高度相机使用
		  * @author g00034
		*/
		class TileNodeVisitor : public osg::NodeVisitor
		{
		public:
			TileNodeVisitor(TritonNode* tn,osgUtil::CullVisitor::TileNodeList& tl);
			float getDistanceToViewPoint(const osg::Vec3& pos, bool useLODScale) const;
			void apply(osg::Node& node);
			void apply(osg::Geometry& geometry);
			void apply(osg::Geode& node);
			void initBoundBox();
		private:
			osg::observer_ptr<TritonNode> m_opTritonNode;
			osg::BoundingSphere m_box;
			osgUtil::CullVisitor::TileNodeList& m_tileNodeList;
		};

		/**
		  * @class CTileNodeCollector
		  * @brief 地形瓦片节点搜集类
		  * @note 获取当前视口中的瓦片信息。 用于生成地形高程纹理图，实现海岸线过渡等效果
		  * @author g00034
		*/
		struct CTileNodeCollector : public osgUtil::CullVisitor::CollectTileNodeCallback
		{
			CTileNodeCollector(TritonNode* on);
			void collectTileNodeImplementation(osg::Drawable& dr, osgUtil::CullVisitor* cv);
		private:
			osg::observer_ptr<TritonNode> m_opOceanNode;
		};

		/**
		  * @class PassHeightMapToTritonCallback
		  * @brief 高度相机回调，用于将生成的高度纹理传给Triton海洋
		  * @author g00034
		*/
		struct PassHeightMapToTritonCallback : public osg::Camera::DrawCallback 
		{ 
			friend class TritonNode;

			PassHeightMapToTritonCallback(Triton::Environment* tritonEnvironment,TritonNode* tn) 
				: _tritonEnvironment(tritonEnvironment), 
				_enable(false), 
				_opHeightMap(NULL),
				_opTritonNode(tn)
			{ 
			}; 

			virtual void operator()( osg::RenderInfo& renderInfo ) const; 
			
			mutable bool _enable; 
			Triton::Matrix4 _heightMapMatrix; 
		private: 
			Triton::Environment* _tritonEnvironment; 
			mutable OpenThreads::Mutex _mutex1;	
			osg::observer_ptr<osg::Texture2D> _opHeightMap;
			osg::observer_ptr<TritonNode> _opTritonNode;

		}; 

		/**
		  * @class ReflectionCamera
		  * @brief 反射相机，用于获取反射纹理图
		  * @author g00034
		*/
		class ReflectionCamera : public osg::Camera
		{
		public:
			ReflectionCamera(TritonNode* tn)
				:m_opTritonNode(tn)
			{
			}
			void traverse(osg::NodeVisitor& nv);
		
		private:
			osg::observer_ptr<TritonNode> m_opTritonNode;
		};

		/**
		  * @class FinalReflectionMapDrawCallback
		  * @brief 反射相机回调，用于将生成的反射纹理传给Triton海洋
		  * @author g00034
		*/
		struct FinalReflectionMapDrawCallback : public osg::Camera::DrawCallback 
		{ 
			FinalReflectionMapDrawCallback(TritonNode* tn)
				:_opTritonNode(tn)
			{
			}
			virtual void operator()( osg::RenderInfo& renderInfo ) const;
		private:
			osg::observer_ptr<TritonNode> _opTritonNode;
		}; 


		/**
		  * @class MyReflectionCullCallback
		  * @brief 反射裁剪回调，检测图形是否支持反射
		  * @author g00034
		*/
		struct MyReflectionCullCallback : public osgUtil::CullVisitor::ReflectionCallback
		{
			MyReflectionCullCallback(TritonNode* tn)
				:m_opTritonNode(tn)
			{
			}
			bool checkNeedReflection(osg::Drawable& dr, osgUtil::CullVisitor* cv)
			{
				return dr.getEnableReflection();
			}
		private:
			osg::observer_ptr<TritonNode> m_opTritonNode;
		};


		/**
		  * @class MyPostRenderNode
		  * @brief 最后渲染的相机节点
		  * @author g00034
		*/
		class MyPostRenderNode : public osg::Camera
		{
		public:
			MyPostRenderNode()
			{
				setRenderOrder( osg::Camera::POST_RENDER,-1 );
				setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );
			}
		    virtual void traverse(osg::NodeVisitor& nv)
			{
				if ( nv.getVisitorType() == nv.CULL_VISITOR)
				{
					osg::Camera::traverse(nv);
				}
			}
		};


		/**
		  * @class MyOriginalSceneCamera
		  * @brief  RTT相机，渲染原始场景，与主相机位置相同，用于海底效果显示
		  * @author g00034
		*/
		class MyOriginalSceneCamera : public osg::Camera
		{
		public:
			MyOriginalSceneCamera(TritonNode* tn)
				:m_opTritonNode(tn)
			{
			}
		private:
			osg::observer_ptr<TritonNode> m_opTritonNode;
		};


		/**
		  * @class MyGodRayNode
		  * @brief RTT相机，渲染 GodRay，与主相机位置相同，用于海底效果显示
		  * @author g00034
		*/
		class MyGodRayNode : public osg::Camera
		{
		public:
			virtual void traverse(osg::NodeVisitor& nv)
			{
				if ( nv.getVisitorType() == nv.CULL_VISITOR)
				{
					osg::Camera::traverse(nv);
				}
			}
		};

	
	private:
		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext;

		/// Triton海洋上下文
		osg::observer_ptr<TritonContext>  m_opTritonContext;

		/// Triton海洋绘制节点
		TritonDrawable                    * _drawable; 
		TritonInitThread*                 m_pTritonInitThread;

		/// 地形高度纹理
		osg::ref_ptr<osg::Texture2D>      m_heightMap;    
		osg::ref_ptr<HeightCamera>        m_pHeightCamera; 
		
		/// 反射效果
		osg::ref_ptr<ReflectionCamera>    m_rpReflectionCam;
		osg::ref_ptr<osg::Texture2D>      m_rpReflectionTexture;
		Triton::Matrix3                   m_reflectionTextureMat;
		osg::ref_ptr<MyReflectionCullCallback> m_rpReflectionCullCallback;
		bool                              m_isReflectionCamTraverse;

		/// 固有反射节点
		osg::observer_ptr<osg::Node>      m_opSun;
		osg::observer_ptr<osg::Node>      m_opAtmosphere;
		osg::observer_ptr<osg::Node>      m_opMoon;

		/// 海底效果
		osg::ref_ptr<MyOriginalSceneCamera> m_rpOriginalSceneCam;
		osg::ref_ptr<osg::Texture2D>        m_rpOriginalSceneTex;
		osg::ref_ptr<MyPostRenderNode>      m_rpPostRenderNode;
		osg::ref_ptr<osg::Texture2D>        m_rpGodRayTex;
		osg::ref_ptr<MyGodRayNode>          m_rpGodRayCamera;

		//mutable OpenThreads::Mutex        m_mutex;
		
	private:
		typedef std::map<osgEarth::Drivers::MPTerrainEngine::MPGeometry*,int> MPGeometryMap;
		MPGeometryMap                     m_mpGeometryMap;    // 存放地球瓦片几何图形

		osg::ref_ptr<CTileNodeCollector>  m_pTileNodeCallback;  // 地球瓦片拣选回调
		
	};
}

#endif

