#include <IFeLayerFactory.h>

#include <FeLayerValid.h>
#include <FeLayerImage.h>
#include <FeLayerModel.h>
#include <FeLayerElevation.h>
#include <FeEffectLayer.h>

namespace FeLayers
{
	class CFeLayerFactory : public ILayerFactory
	{
	public:
		/**  
		  * @brief 创建一个地图图层
		  * @note: 内部根据图层配置属性判断创建的具体图层类型
		  * @param cfg [in] 图层配置信息
		  * @return 返回创建的图层
		*/
		virtual IFeLayer* CreateLayer(const CFeLayerConfig& cfg)
		{
			switch (cfg.GetLayerType())
			{
			case E_LAYER_IMAGE: 
				{
					CFeLayerImageConfig layerCfg(cfg.getConfig());
					return CreateLayer(layerCfg);
				};
			case E_LAYER_MODEL: 
				{
					CFeLayerModelConfig layerCfg(cfg.getConfig());
					return CreateLayer(layerCfg);
				};
			case E_LAYER_ELEVATION: 
				{
					CFeLayerElevationConfig layerCfg(cfg.getConfig());
					return CreateLayer(layerCfg);
				};
			case E_LAYER_EFFECT: 
				{
					CFeLayerEffectConfig layerCfg(cfg.getConfig());
					return CreateLayer(layerCfg);
				};
			}

			OE_WARN << "CFeLayerFactory.CreateLayer 创建图层失败" << std::endl;
			return NULL;
		}

		IFeLayer* CreateLayer(const CFeLayerImageConfig& cfg)
		{
			CFeLayerImageConfig layerCfg(cfg);
			CLayerConfigValidator cfgValidator(layerCfg);

			// 检测并校验图层配置信息
			if( !cfgValidator.CheckAndValid() )
			{
				return NULL;
			}

			return new CFeLayerImage(layerCfg);
		}

		IFeLayer* CreateLayer(const CFeLayerModelConfig& cfg)
		{
			CFeLayerModelConfig layerCfg(cfg);
			CLayerConfigValidator cfgValidator(layerCfg);

			// 检测并校验图层配置信息
			if( !cfgValidator.CheckAndValid() )
			{
				return NULL;
			}

			return new CFeLayerModel(layerCfg);
		}

		IFeLayer* CreateLayer(const CFeLayerElevationConfig& cfg)
		{
			CFeLayerElevationConfig layerCfg(cfg);
			CLayerConfigValidator cfgValidator(layerCfg);

			// 检测并校验图层配置信息
			if( !cfgValidator.CheckAndValid() )
			{
				return NULL;
			}

			return new CFeLayerElevation(layerCfg);
		}

		IFeLayer* CreateLayer(const CFeLayerEffectConfig& cfg)
		{
			return new CFeEffectLayer(cfg);
		}
	};

	ILayerFactory* LayerFactorySingle() { static CFeLayerFactory layerFactory;  return &layerFactory; }
}

