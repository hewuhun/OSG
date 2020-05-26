/**************************************************************************************************
* @file VolumeRendering.h
* @note 体渲染
* @author h00017
* @data 2017-2-9
**************************************************************************************************/
#ifndef FE_VOLUME_RENDER_H
#define FE_VOLUME_RENDER_H

#include <osg/Group>
#include <osg/Drawable>
#include <osg/Texture2D>
#include <osg/Texture1D>
#include <osg/Texture3D>

#include <FeUtils/RenderContext.h>

namespace FeVolumeRender
{
	class CVolumeRenderNode;

	/**
	* @class CVolumeRenderGeometry
	* @brief 用于体渲染的立方体盒子
	* @note 可渲染对象，继承于osg::Geometry,用于绘制体渲染的立方体盒子
	* @author h00017
	*/
	class CVolumeRenderGeometry : public osg::Geometry
	{
	public:
		CVolumeRenderGeometry(FeUtil::CRenderContext* rc);

	private:
		void InitGeometry();

	private:
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext;
	};

	class MyVolumeRenderNodeCallback : public osg::NodeCallback
	{
	public:
		MyVolumeRenderNodeCallback(CVolumeRenderNode* vrn);

		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

	private:
		osg::observer_ptr<CVolumeRenderNode> m_opVolumeRenderNode;
	};

	/**
	* @class CVolumeRenderNode
	* @brief 体渲染类
	* @note 利用光线追踪技术，实现体数据的渲染
	* @author h00017
	*/
	class CVolumeRenderNode : public osg::Group
	{
		friend class MyVolumeRenderNodeCallback;

	public:
		CVolumeRenderNode(FeUtil::CRenderContext* rc);

	public:
		void SetStepSize(float fSize);

		void SetSampleNum(int nNum);

	private:
		/**
		* @brief 更新函数
		* @note 更新回调函数，负责参数的更新
		* @return 无返回值
		*/
		void Update();

		/**
		* @brief 添加hud对象
		* @note 测试渲染到纹理的效果
		* @return 无返回值
		*/
		void AddHud();

		/**
		* @brief 创建立方体盒子
		* @note 创建用于体渲染的立方体盒子
		* @return 无返回值
		*/
		void CreateGeometry();

		/**
		* @brief 初始化纹理相机
		* @note 初始化纹理相机，用于保存盒子背面的位置
		* @return 无返回值
		*/
		void InitTexCamera();

		/**
		* @brief 初始化体纹理
		* @note 创建体纹理对象，并将体数据存入体纹理中
		* @param fileName [in] 体数据文件名
		* @param w [in] 体数据的宽
		* @param h [in] 体数据的高
		* @param d [in] 体数据的深度
		* @return 无返回值
		*/
		void InitVolumeTex(std::string fileName,int w,int h,int d);

		/**
		* @brief 初始化颜色表纹理
		* @note 创建一维纹理对象，并将颜色表数据存入其中
		* @param fileName [in] 颜色表数据文件名
		* @return 无返回值
		*/
		void InitTransferTex(std::string fileName);

		/**
		* @brief 初始化立方体盒子的世界变换矩阵
		* @note 根据体数据的坐标信息，创建世界变换矩阵
		* @param fileName [in] 体数据文件名
		* @return 无返回值
		*/
		void InitWorldMat(std::string fileName);

		/**
		* @brief string转float
		* @note 将数据文件中的字符串解析为浮点数据
		* @return 无返回值
		*/
		void StringToFloats(std::string inStr,std::vector<float>& floatList);

	private:
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext;
		osg::ref_ptr<CVolumeRenderGeometry> m_rpVolumeRenderGeometry;
		osg::ref_ptr<osg::Camera> m_rpBoxCamera;
		osg::ref_ptr<osg::Texture1D> m_rpTransferTex;
		osg::ref_ptr<osg::Texture2D> m_rpBoxTex;
		osg::ref_ptr<osg::Texture3D> m_rpVolumeTex;
		osg::ref_ptr<osg::Camera> m_rpHud;
		osg::ref_ptr<MyVolumeRenderNodeCallback> m_rpVolumeRenderNodeCallback;
		osg::ref_ptr<osg::MatrixTransform> m_rpWorldMat;
		int m_texWidth;
		int m_texHeight;
		float m_stepSize;
		osg::Vec3 m_boxSize;
		std::string m_transferTexName;
		std::string m_volumeTexName;
		std::string m_eleDataName;
		float m_startLon;
		float m_endLon;
		float m_stepLon;
		float m_startLat;
		float m_endLat;
		float m_stepLat;
		float m_startAlt;
		float m_endAlt;
		float m_stepAlt;	
		float m_minVolumeValue;
		float m_maxVolumeValue;

		int m_uSampleNum;
	};
}


#endif