/**************************************************************************************************
* @file ViewAnalyseNode.h
* @note 城市可视域分析示例
* @author h00017
* @data 2016-07-12
**************************************************************************************************/


#ifndef VIEWANALYSENODE__HH__
#define VIEWANALYSENODE__HH__

#include <FeUtils/CoordConverter.h>
#include <FeUtils/RenderContext.h>
#include <osg/GLExtensions>


namespace FeMeasure
{

	class ViewAnalyseNode;

	struct EndDepthCallback : public osg::Camera::DrawCallback 
	{ 
		EndDepthCallback(ViewAnalyseNode* vn);

		void operator()( osg::RenderInfo& renderInfo ) const;
	
	private: 
		osg::observer_ptr<ViewAnalyseNode> m_opViewAnalyseNode;
	}; 

	struct BeginDepthCallback : public osg::Camera::DrawCallback 
	{ 
		BeginDepthCallback(ViewAnalyseNode* vn);
		void operator()( osg::RenderInfo& renderInfo ) const;
	private: 
		osg::observer_ptr<ViewAnalyseNode> m_opViewAnalyseNode;
	}; 

	struct ViewAnalyseDrawCallback : public osg::Drawable::DrawCallback
	{
		ViewAnalyseDrawCallback(ViewAnalyseNode* vn);
		~ViewAnalyseDrawCallback();

		void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const;
	private:
		unsigned int loadShader(bool vertexShader) const;
		bool loadShaders(unsigned int id) const;
		void pushAllState() const;
		void popAllState() const;
		void initUniforms() const;
		void updateUniforms(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const;
	private:
		osg::observer_ptr<ViewAnalyseNode> m_opViewAnalyseNode;
		mutable osg::observer_ptr<osg::GLExtensions> m_opGLExtensions;
		mutable bool                             m_shaderInit;
		mutable std::stack<GLint>                m_programStack;
		mutable GLint                            m_hasTex;
		mutable GLint                            m_mvpMat;
		mutable GLint                            m_isDepth;
		mutable GLint                            m_texMat;
		mutable GLint                            m_tex0;
		mutable bool                             m_isTexUniformSet;
		mutable GLint                            m_depthTex;
		mutable GLint                            m_texMVMat;

		mutable unsigned int m_vertShader;
		mutable unsigned int m_fragShader;
		mutable GLuint       m_programCast;
	};

	class ViewAnalyseUpdateCallback : public osg::NodeCallback
	{
	public:
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
	};

	class DepthCameraCullCallback : public osg::NodeCallback
	{
	public:
		DepthCameraCullCallback(ViewAnalyseNode* vn);
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
	private:
		osg::observer_ptr<ViewAnalyseNode> m_opViewAnalyseNode;
	};

	class ViewAnalyseNodeVisitor : public::osg::NodeVisitor
	{
	public:
		ViewAnalyseNodeVisitor(ViewAnalyseNode* vn);
		void apply(osg::Drawable& drawable);
	private:
		osg::observer_ptr<ViewAnalyseNode> m_opViewAnalyseNode;
	};

	/**
	* @class ViewAnalyseNode
	* @brief 城市可视域分析结点类
	* @note 本类包括城市模型的管理及可视域分析效果的实现
	* @author h00017
	*/
	class ViewAnalyseNode : public osg::Group
	{
		friend struct ViewAnalyseCullCallback;
		friend struct EndDepthCallback;
		friend struct BeginDepthCallback;
		friend class ViewAnalyseUpdateCallback;
		friend class DepthCameraCullCallback;
		friend struct ViewAnalyseDrawCallback;
	public:
		ViewAnalyseNode(FeUtil::CRenderContext* rc);
		~ViewAnalyseNode();
		/**
		* @brief 初始化函数
		* @note  创建深度相机，设置模型的渲染回调
		* @return 无返回值
		*/
		void init();
		/**
		* @brief 设置观察点的位置
		* @note 设置可视域分析时观察点的位置（世界坐标）
		* @param data [in] 观察点位置
		* @return 无返回值
		*/
		void setEyePos(const osg::Vec3d& pos);
		const osg::Vec3d& getEyePos(){return m_eyePos;}

		/**
		* @brief 设置观察方向点的位置
		* @note 设置观察方向点的位置，即指定朝哪一个点看（世界坐标）
		* @param data [in] 观察方向点的位置
		* @return 无返回值
		*/
		void setLookPos(const osg::Vec3d& pos);
		const osg::Vec3d& getLookPos(){return m_lookPos;}

		/**  
		  * @note 设置近平面距离  
		  * @param fVal [in] 设置值
		  * @return 无返回值
		*/
		void setNearClip(const float& fVal);
		float getNearClip(){return m_nearClip;}

		/**  
		  * @note 设置远平面距离  
		  * @param fVal [in] 设置值
		  * @return 无返回值
		*/
		void setFarClip(const float& fVal);
		float getFarClip(){return m_farClip;}

		/**  
		  * @note 设置垂直视场角  
		  * @param fVal [in] 设置值
		  * @return 无返回值
		*/
		void setFov(const float& fVal);
		float getFov(){return m_fov;}

		/**  
		  * @note 设置水平视场角与垂直视场角的比例值 
		  * @param fVal [in] 设置值
		  * @return 无返回值
		*/
		void setFovAspect(const float& fVal);
		float getFovAspect(){return m_aspect;}

		/**  
		  * @note 设置平截头体线框颜色
		  * @param fVal [in] 颜色值
		  * @return 无返回值
		*/
		void setFrustumLineColor(const osg::Vec4& vecColor);
		const osg::Vec4& getFrustumLineColor(){return m_frustumLineColor;}

	private:
		void initDepthCamera();
		void updateDepthCamera();
		osg::Geometry* createCameraGeometry();
		void addHud();
	
	private:
		osg::ref_ptr<ViewAnalyseNodeVisitor> m_rpNodeVisitor;
		osg::ref_ptr<osg::Texture2D>     m_rpDepthMap;
		osg::ref_ptr<osg::Camera>        m_rpDepthCamera;
		bool                             m_isDepth;
		osg::Matrix                      m_depthViewMatrix;
		osg::Matrix                      m_depthProjMatrix;
		float                            m_nearClip;
		float                            m_farClip;
		float                            m_fov;
		float                            m_aspect;
		osg::Vec3d                       m_eyePos;
		osg::Vec3d                       m_lookPos;
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext;
		osg::ref_ptr<ViewAnalyseUpdateCallback> m_rpUpdateCallback;
		osg::ref_ptr<DepthCameraCullCallback> m_rpDepthCameraCullCallback;
		osg::ref_ptr<osg::Group>         m_rpCamGeometry;
		osg::Vec4                        m_frustumLineColor;

		osg::ref_ptr<osg::Camera>        m_rpHud;

		bool                             m_bInit;
	};


}





#endif