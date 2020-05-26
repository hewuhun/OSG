/**************************************************************************************************
* @file GradientMapEffect.h
* @note 地形的坡度特效
* @author w00024
* @data 2016-7-11
**************************************************************************************************/
#ifndef FE_GRADIENT_MAP_EFFECT_H
#define FE_GRADIENT_MAP_EFFECT_H 1

#include <osgEarthUtil/Common>
#include <osgEarth/TerrainEffect>
#include <osgEarth/ImageLayer>
#include <osgEarth/MapNode>
#include <osg/observer_ptr>
#include <osgEarth/ShaderLoader>

#include <osg/Texture3D>
#include <osg/NodeCallback>

#include <FeMeasure/Export.h>

namespace FeMeasure
{
	/**
	* @class CGradientMapEffect
	* @note 地形的GradientMap坡度坡向图的效果实现,继承自osgEarth::TerrainEffect
	* @author w00024 
	*/
	class FEMEASURE_EXPORT CGradientMapEffect : public osgEarth::TerrainEffect
	{
	public:
		/**  
		  * @brief 构造函数
		  * @param pMapNode [in] 地球节点
		*/
		CGradientMapEffect(osgEarth::MapNode *pMapNode);

		virtual ~CGradientMapEffect();

		/**  
		  * @brief 设置光照图层效果参数
		  * @param conf [in] 光照图层配置
		  * @param map [in] 地图节点
		*/
		void SetNormalMapConfig(const osgEarth::Config& conf, osgEarth::Map* map);

		/**  
		  * @brief 设置经纬度范围
		  * @param lonRange [in] 经度范围
		  * @param latRange [in] 纬度范围
		*/
		void SetLonLatRange(osg::Vec3f lonRange, osg::Vec3f latRange);
		
		/**  
		  * @brief 获取经纬度范围
		  * @param lonRange [out] 经度范围
		  * @param latRange [out] 纬度范围
		*/
		void GetLonLatRange(osg::Vec3f& lonRange, osg::Vec3f& latRange);

		/**  
		  * @brief 设置经纬宽度
		  * @param lonLatWidth [in] 经纬宽度
		*/
		void SetLonLatWidth(osg::Vec2f lonLatWidth);

		/**  
		  * @brief 获取经纬宽度
		  * @param lonLatWidth [out] 经纬宽度
		*/
		void GetLonLatWidth(osg::Vec2f& lonLatWidth);

		/**  
		  * @brief 设置箭头大小
		  * @param arrowSize [in] 箭头大小
		*/
		void SetArrowSize(float arrowSize);

		/**  
		  * @brief 设置边框线宽度
		  * @param size [in] 边框线宽度
		*/
		void SetBorderLineSize(float size){ m_rpBorderLineSize->set(size); }

		/**  
		  * @brief 计算指定点的坡度坡向信息
		  * @param llDegree [in] 指定点经纬高（单位：角度）
		  * @param gradientInfo [in] 坡度坡向信息
		*/
		void CalculateGradientInfo(const osg::Vec2d llDegree, osg::Vec2d &gradientInfo);

	public:
		/**  
		  * @brief 加载坡度效果
		  * @param engine [in] 地形引擎
		*/
		void onInstall(osgEarth::TerrainEngineNode* engine);

		/**  
		  * @brief 卸载坡度效果
		  * @param engine [in] 地形引擎
		*/
		void onUninstall(osgEarth::TerrainEngineNode* engine);

		/**  
		  * @brief 判断是否已经加载坡度效果
		*/
		bool IsOnInstall() { return m_bOnInstall; }

		/**  
		  * @brief 计算两点之间的夹角
		*/
		double CalculatePitchDegree(osg::Vec3d& first, osg::Vec3d& second);

		/**  
		  * @brief 获取图层
		*/
		osgEarth::ImageLayer* GetImgLayer() { return m_opLayer.get(); }

	protected:
		/**  
		  * @brief 初始化
		*/
		void Init();

		/// 地球节点
		osg::observer_ptr<osgEarth::MapNode> m_opMapNode;

		/// 共享图层的名称
		osgEarth::optional<std::string> m_strLayerName;		

		/// 共享图层的指针
		osg::observer_ptr<osgEarth::ImageLayer> m_opLayer;  

		/// 绑定sample到image单元
		osg::ref_ptr<osg::Uniform> m_rpSamplerUniform;     

		/// 加载shader库
		osgEarth::ShaderPackage m_shaderPackage;		

		/// 是否已经加载效果
		bool m_bOnInstall;

		// 输入的经纬度范围
		osg::Vec3f m_longituteRange, m_latitudeRange;
		osg::Vec2f m_lonLatWidth;

		/// shader中使用的变量
		osg::ref_ptr<osg::Uniform> m_rpLongituteRange;
		osg::ref_ptr<osg::Uniform> m_rpLatitudeRange;
		osg::ref_ptr<osg::Uniform> m_rpLonLatWidth;
		osg::ref_ptr<osg::Uniform> m_rpArrowSize;		
		osg::ref_ptr<osg::Uniform> m_rpBorderLineSize;

		/// 箭头大小
		float m_fArrowSize;
		
		/// 纹理对象
		osg::ref_ptr<osg::Texture3D>          m_rpNoise3DTexture;
		osg::ref_ptr<osg::Uniform>			  m_rpUniformNoise;
	};

	/**
	* @class CGradientArrowCullCallback
	* @note 效果裁剪回调，主要用于处理箭头大小
	* @author w00024 
	*/
	class FEMEASURE_EXPORT CGradientArrowCullCallback : public osg::NodeCallback
	{
	public:
		CGradientArrowCullCallback(FeMeasure::CGradientMapEffect *pEffect) : m_opEffect(pEffect) { }

		/**  
		  * @brief 更新回调函数，主要用于根据视点范围处理箭头大小
		*/
		void operator()(osg::Node* node, osg::NodeVisitor* nv)
		{
			if (m_opEffect.valid() )
			{
				osg::Vec3 eye = nv->getEyePoint();
				float fRange = osg::clampAbove( eye.length() - 6356752.3142f, 0.0f );

				if (fRange > 1.5e+7)
				{
					m_opEffect->SetLonLatWidth(osg::Vec2f(1.8e+5, 1.8e+5));
					m_opEffect->SetArrowSize(1.9e+4);
				}
				else if (fRange > 1.5e+6)
				{
					m_opEffect->SetLonLatWidth(osg::Vec2f(1.5e+5, 1.5e+5));
					m_opEffect->SetArrowSize(1.5e+4);
				}
				else if (fRange > 4.0e+5)
				{
					m_opEffect->SetLonLatWidth(osg::Vec2f(35000.0, 35000.0));
					m_opEffect->SetArrowSize(6000.0);
				}
				else if (fRange > 80000.0)
				{
					m_opEffect->SetLonLatWidth(osg::Vec2f(20000.0, 20000.0));
					m_opEffect->SetArrowSize(2000.0);
				}
				else {
					m_opEffect->SetLonLatWidth(osg::Vec2f(5000.0, 5000.0));
					m_opEffect->SetArrowSize(500.0);
				}
				m_opEffect->SetBorderLineSize(fRange);
			}

			traverse(node, nv);
		}

	protected:
		/// 坡度坡向效果
		osg::observer_ptr<FeMeasure::CGradientMapEffect> m_opEffect; 
	};

} 

#endif // FE_NORMAL_MAP_EFFECT_H
