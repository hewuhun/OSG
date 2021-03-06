﻿#ifndef FE_HDRLIGHTING__HH__
#define FE_HDRLIGHTING__HH__

#include <osg/Group>
#include <osg/Drawable>
#include <FeUtils/RenderContext.h>
#include <osg/Texture2D>
#include <GlareDef.h>

namespace FeHDRLighting
{
	class FeHDRLightingNode;

	struct MyGeometryDrawCallback : public osg::Drawable::DrawCallback
	{
		MyGeometryDrawCallback(osg::Node* n);
		virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const;
	private:
		osg::observer_ptr<osg::Node> m_opModelNode;
	};

	class MyUpdateCallback : public osg::NodeCallback
	{
	public:
		MyUpdateCallback(FeHDRLightingNode* ln);
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
	private:
		osg::observer_ptr<FeHDRLightingNode> m_opFeHDRLightingNode;
	};

	class GeometryVisitor : public osg::NodeVisitor
	{
	public:
		GeometryVisitor(osg::Node* n);
		virtual void apply(osg::Geometry& geometry);
	private:
		osg::observer_ptr<osg::Node> m_opModelNode;
	};

	struct MyPreUpdateCallback : public osgViewer::ViewerBase::UpdateCallback
	{
	public:
		MyPreUpdateCallback(FeHDRLightingNode* ln)
			:m_opHDRLightingNode(ln)
		{
		}
		virtual void customOperator(const osgViewer::ViewerBase* viewerBase);
	private:
		osg::observer_ptr<FeHDRLightingNode> m_opHDRLightingNode;
	};

	class MyStarCamGroupNodeCallback : public osg::NodeCallback
	{
	public:
		MyStarCamGroupNodeCallback(FeHDRLightingNode* ln)
			:m_opHDRLightingNode(ln)
		{
		}
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
	private:
		osg::observer_ptr<FeHDRLightingNode> m_opHDRLightingNode;
	};

	struct MyStarCamDrawCallback : public osg::Camera::DrawCallback
	{
	public:
		MyStarCamDrawCallback(FeHDRLightingNode* ln)
			:m_opHDRLightingNode(ln)
		{
		}

		virtual void operator () (osg::RenderInfo& renderInfo) const;
	private:
		osg::observer_ptr<FeHDRLightingNode> m_opHDRLightingNode;
	};

	struct MyBloomCamDrawCallback : public osg::Camera::DrawCallback
	{
	public:
		MyBloomCamDrawCallback(FeHDRLightingNode* ln)
			:m_opHDRLightingNode(ln)
		{
		}

		virtual void operator () (osg::RenderInfo& renderInfo) const;
	private:
		osg::observer_ptr<FeHDRLightingNode> m_opHDRLightingNode;
	};

	struct MySceneCamDrawCallback : public osg::Camera::DrawCallback
	{
	public:
		MySceneCamDrawCallback(FeHDRLightingNode* ln)
			:m_opHDRLightingNode(ln)
		{
		}

		virtual void operator () (osg::RenderInfo& renderInfo) const;
	private:
		osg::observer_ptr<FeHDRLightingNode> m_opHDRLightingNode;
	};

	struct MyModelSceneCamDrawCallback : public osg::Camera::DrawCallback
	{
	public:
		MyModelSceneCamDrawCallback(FeHDRLightingNode* ln)
			:m_opHDRLightingNode(ln)
		{
		}

		virtual void operator () (osg::RenderInfo& renderInfo) const;
	private:
		osg::observer_ptr<FeHDRLightingNode> m_opHDRLightingNode;
	};

	class LightSphereGeomerty : public osg::Geometry
	{
	public:
		LightSphereGeomerty(float r = 1,int row = 50,int col = 50);
		void SetColor(osg::Vec4 c);
	private:
		void CreateSphere();
	private:
		int m_row;
		int m_col;
		float m_radius;
		osg::Vec4 m_color;
	};

	struct HDRLightParam
	{
		HDRLightParam()
			:lightPos(0.0,0.0,0.0)
			,lightPow(0)
			,lightRadius(0)
			,lightColor(1.0,1.0,1.0)
		{
		}
		//灯光位置，局部坐标
		osg::Vec3 lightPos;
		//灯光强度0-1
		float lightPow;
		//灯光球体半径
		float lightRadius;
		//灯光颜色
		osg::Vec3 lightColor;
	};

	class FeHDRLightingNode : public osg::Group
	{
		#define NUM_LIGHTS            8   
		#define MAX_SAMPLES           16   
		#define NUM_TONEMAP_TEXTURES  4
		#define NUM_BLOOM_TEXTURES    3  
		#define NUM_STAR_TEXTURES     12 

		friend class MyUpdateCallback;
		friend struct MyPreUpdateCallback;
		friend class MyStarCamGroupNodeCallback;
		friend struct MyStarCamDrawCallback;
		friend struct MyBloomCamDrawCallback;
		friend struct MySceneCamDrawCallback;
		friend struct MyModelSceneCamDrawCallback;
	public:
		FeHDRLightingNode(FeUtil::CRenderContext* rc,osg::Node* lightingModel = NULL,EGLARELIBTYPE lt = GLT_NATURAL,float starLengthScale = 1.0,float starScale = 0.01,float bloomScale = 1.0);
		~FeHDRLightingNode();

		void setLight(unsigned int index,const HDRLightParam& p);
		HDRLightParam getLight(unsigned int index);

		//设置漫反射半径
		void setLightDiffRadius(float r)
		{
			m_lightDiffRadius = r;
		}
		float getLightDiffRadius()
		{
			return m_lightDiffRadius;
		}

		//设置漫反射强度
		void setDiffuseCoefficient(float d)
		{
			m_diffuseCoefficient = d;
		}
		float getDiffuseCoefficient()
		{
			return m_diffuseCoefficient;
		}

		//设置镜面光强度
		void setPhongCoefficient(float p)
		{
			m_phongCoefficient = p;
		}
		float getPhongCoefficient()
		{
			return m_phongCoefficient;
		}
	private:
		void creatScene();
		void initUniform();
		void updateUniform();
		void updateLight();
		void initSceneCamera();
		void initModelSceneCamera();
		void addHud();
		void insertSceneCamToRoot();
		void removeSceneCamFromRoot();
		bool checkSceneCamWithRoot();
		void preUpdate();
		osg::Geode* creatScreenQuadNode(float leftU,float rightU,float topV,float bottomV);
		void initDownScale4x4Cam();
		void getTextureCoords(osg::ref_ptr<osg::Texture2D>& rpSrcTex,osg::Vec4d* srcRect,osg::ref_ptr<osg::Texture2D>& rpDestTex,osg::Vec4d* destRect,osg::Vec4d& outCoord);
		void getSampleOffsets_DownScale4x4( int dwWidth, int dwHeight, osg::Vec2 avSampleOffsets[] );
		void measureLuminance();
		void initAdaptation();
		void updateAdaptationTarget();
		void initSceneScaledToBrightPass();
		void addChildToModelSceneCam();
		void initBrightPassToStarSource();
		void getSampleOffsetsGaussBlur5x5(unsigned int dwD3DTexWidth, unsigned int dwD3DTexHeight, osg::Vec2* avTexCoordOffset,
			                              osg::Vec4* avSampleWeights, float fMultiplier = 1.0f);
		float gaussianDistribution( float x, float y, float rho );
		void initStarSourceToBloomSource();
		void getSampleOffsetsDownScale2x2( unsigned int dwWidth, unsigned int dwHeight, osg::Vec2 avSampleOffsets[] );
		void initBloom();
		void getSampleOffsetsBloom( unsigned int dwD3DTexSize, float afTexCoordOffset[15], osg::Vec4* avColorWeight,
			                        float fDeviation, float fMultiplier = 1.0f );
		void initStar();
		void initFinalSceneNode();
		void createBloomTextures();
		void deleteBloomTextures();
		void updateBloomTextureSize();

	private:
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext;

		osg::ref_ptr<osg::Group> m_rpModelNode;
		osg::ref_ptr<osg::MatrixTransform> m_rpLightNode[NUM_LIGHTS];
		bool m_enableLighting;
		bool m_enableBlooming;
		osg::ref_ptr<MyUpdateCallback> m_rpUpdateCallback;

		int m_minLightBaseValue;
		int m_maxLightBaseValue;
		unsigned int m_lightBaseValue[NUM_LIGHTS];
		float m_lightPow[NUM_LIGHTS];
		int m_nLightLogIntensity[NUM_LIGHTS];
		int m_nLightMantissa[NUM_LIGHTS];
		osg::Vec4 m_avLightIntensity[NUM_LIGHTS];
		osg::Vec4 m_avLightPosition[NUM_LIGHTS];
		osg::Vec3 m_lightColor[NUM_LIGHTS];
		float m_lightRadius[NUM_LIGHTS];
		float m_emissiveCoefficient;
		osg::Vec3 m_emissive;
		float m_phongCoefficient;
		float m_diffuseCoefficient;
		float m_phongExponent;
        float m_lightDiffRadius;
		bool m_islastFrameActive;
		float m_visibleMultiple;
		int m_curRenderBinNum;
		bool m_adaptationInvalid;
		double m_lastFrameTime;
		float m_keyValue; 
		float m_minKeyValue;
		float m_maxKeyValue;
		float m_starScale;
		float m_bloomScale;
		float m_starLengthScale;
		CGlareDef m_glareDef; 
		EGLARELIBTYPE m_glareType;
		osg::ref_ptr<MyStarCamGroupNodeCallback> m_starCamGroupNodeCallback;
		osg::observer_ptr<osg::Node> m_rpLightingModel;

		osg::ref_ptr<osg::Camera> m_rpSceneCamera;
		osg::ref_ptr<osg::Texture2D> m_rpSceneTex;
		osg::ref_ptr<osg::Camera> m_rpHud;
		osg::ref_ptr<MyPreUpdateCallback> m_rpPreUpdateCallback;
		osg::ref_ptr<osg::Camera> m_rpDownScale4x4Cam;
		osg::ref_ptr<osg::Texture2D> m_rpDownScale4x4Tex;
		osg::ref_ptr<osg::Texture2D> m_apTexToneMap[NUM_TONEMAP_TEXTURES];
		osg::ref_ptr<osg::Camera>    m_toneMapCam[NUM_TONEMAP_TEXTURES];
		osg::ref_ptr<osg::Texture2D> m_texAdaptedLuminanceLast;
		osg::ref_ptr<osg::Texture2D> m_texAdaptedLuminanceCur;
		osg::ref_ptr<osg::Camera> m_AdaptedLuminanceCam;
		osg::ref_ptr<osg::Texture2D> m_texBrightPass;
		osg::ref_ptr<osg::Camera> m_texBrightPassCam;
		osg::ref_ptr<osg::Texture2D> m_modelSceneTex;
		osg::ref_ptr<osg::Camera> m_rpModelSceneCam;
		osg::ref_ptr<osg::Texture2D> m_texStarSource;
		osg::ref_ptr<osg::Camera> m_texStarSourceCam;
		osg::ref_ptr<osg::Texture2D> m_texBloomSource;
		osg::ref_ptr<osg::Camera> m_texBloomSourceCam;
		osg::ref_ptr<osg::Texture2D> m_texBloom[NUM_BLOOM_TEXTURES];
		osg::ref_ptr<osg::Camera> m_texBloomCam[NUM_BLOOM_TEXTURES];
	    osg::ref_ptr<osg::Texture2D> m_texStar[NUM_STAR_TEXTURES];
		osg::ref_ptr<osg::Camera> m_texStarCam[NUM_STAR_TEXTURES];
		osg::ref_ptr<osg::Camera> m_texStarAddCam[8][2];
		osg::ref_ptr<osg::Texture2D> m_texStarAdd[8][2];
		osg::ref_ptr<osg::Group> m_texStarCamGroup;
		osg::ref_ptr<osg::Camera> m_finalSceneCam;
	};
}

























#endif