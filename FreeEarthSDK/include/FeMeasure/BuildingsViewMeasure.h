/**************************************************************************************************
* @file BuildingsViewMeasure.h
* @note 建筑群点对点通视分析
* @author g00034
* @data 2017-1-5
**************************************************************************************************/

#ifndef  HG_BUILDINGS_VIEW_MEASURE_H
#define  HG_BUILDINGS_VIEW_MEASURE_H 

#include <osg/Group>

#include <FeUtils/RenderContext.h>

#include <FeMeasure/Export.h>
#include <FeMeasure/Measure.h>

#include <FeMeasure/ViewAnalyseNode.h>

namespace FeMeasure
{
	/**
	  * @class CBuildingsViewResult
	  * @brief 建筑群视域分析结果类
	  * @note 分装了建筑群视域分析的计算过程和显示结果
	  * @author g00034
	*/
    class CBuildingsViewResult : public CMeasureResult
	{
		friend class CBuildingsViewMeasure;

	public:
		/**  
		  * @brief 构造函数
		  * @param pRenderContext [in] 当前渲染上下文
		*/
		CBuildingsViewResult(FeUtil::CRenderContext* pContext);
	
	private:
		/**
		*@note: 根据输入点计算测量结果
		*/
		virtual bool CalculateImplement(const osg::Vec3dArray& vecPointsLLH);

		/**
		*@note: 创建计算结果数据对象
		*/
		virtual CMeasureResultData* CreateResultData(){ return NULL; }

		/**  
		  * @brief 设置观察位置
		*/
		osg::Vec3 GetLookPosition();
		void SetLookPosition(const osg::Vec3& vecPosLLH);

		/**  
		  * @brief 设置线框颜色
		*/
		osg::Vec4 GetFrustumLineColor();
		void SetFrustumLineColor(const osg::Vec4& vecColor);

		/**  
		  * @brief 设置近裁剪面
		*/
		float GetNearClip();
		void SetNearClip(float fVal);

		/**  
		  * @brief 设置远裁剪面
		*/
		float GetFarClip();
		void SetFarClip(float fVal);
		
		/**  
		  * @brief 设置垂直视场角
		*/
		float GetFov();
		void SetFov(float fVal);

		/**  
		  * @brief 设置视口比例
		*/
		float GetFovAspect();
		void SetFovAspect(float fVal);

		/**  
		  * @brief 设置俯仰角
		*/
		float GetPitchAngle();
		void SetPitchAngle(float fVal);

	private:
		osg::ref_ptr<ViewAnalyseNode>    m_rpBuildingsViewNode;
		osg::Vec3d                       m_vecPosLLH;
		float                            m_fPitchAngle;
	};
}

namespace FeMeasure
{
	/**
	  * @class CBuildingsViewMeasure
	  * @note 建筑群视域分析类
	  * @author g00034
	*/
	class FEMEASURE_EXPORT CBuildingsViewMeasure : public CMeasure
	{
		friend class CBuildingViewFactory;

	public:
		/**  
		  * @brief 设置观察位置
		*/
		osg::Vec3 GetLookPosition(){return m_opResult.valid()?m_opResult->GetLookPosition():osg::Vec3();}
		void SetLookPosition(const osg::Vec3& vecPosLLH){if(m_opResult.valid()) m_opResult->SetLookPosition(vecPosLLH);}

		/**  
		  * @brief 设置线框颜色
		*/
		osg::Vec4 GetFrustumLineColor(){return m_opResult.valid()?m_opResult->GetFrustumLineColor():osg::Vec4();}
		void SetFrustumLineColor(const osg::Vec4& vecColor){if(m_opResult.valid()) m_opResult->SetFrustumLineColor(vecColor);}

		/**  
		  * @brief 设置近裁剪面
		*/
		float GetNearClip(){return m_opResult.valid()?m_opResult->GetNearClip():0;}
		void SetNearClip(float fVal){if(m_opResult.valid()) m_opResult->SetNearClip(fVal);}

		/**  
		  * @brief 设置远裁剪面
		*/
		float GetFarClip(){return m_opResult.valid()?m_opResult->GetFarClip():0;}
		void SetFarClip(float fVal){if(m_opResult.valid()) m_opResult->SetFarClip(fVal);}
		
		/**  
		  * @brief 设置垂直视场角
		*/
		float GetFov(){return m_opResult.valid()?m_opResult->GetFov():0;}
		void SetFov(float fVal){if(m_opResult.valid()) m_opResult->SetFov(fVal);}

		/**  
		  * @brief 设置视口比例
		*/
		float GetFovAspect(){return m_opResult.valid()?m_opResult->GetFovAspect():0;}
		void SetFovAspect(float fVal){if(m_opResult.valid()) m_opResult->SetFovAspect(fVal);}

		/**  
		  * @brief 设置俯仰角
		*/
		float GetPitchAngle(){return m_opResult.valid()?m_opResult->GetPitchAngle():0;}
		void SetPitchAngle(float fVal){if(m_opResult.valid()) m_opResult->SetPitchAngle(fVal);}

	private: 
		CBuildingsViewMeasure(FeUtil::CRenderContext* pContext);
		virtual CMeasureResult* CreateMeasureResult();

		/**
		*@note: 添加控制点具体实现
		*/
		virtual void AddCtrlPointImplement(const osg::Vec3d& vecPosLLH);
		
		/**
		*@note: 设置控制点集具体实现
		*/
		virtual void SetCtrlPointsImplement(const osg::Vec3dArray& vecPointsLLH);

	private:
		osg::observer_ptr<CBuildingsViewResult> m_opResult;
	};
};


#endif
