/**************************************************************************************************
* @file SulIR.h
* @note 红外效果对外接口
* @author c00005
* @data 2017-3-7
**************************************************************************************************/
#ifndef SUL_IR_H
#define SUL_IR_H

#include <FeKits/KitsCtrl.h>

#include <FeKits/IR/SulTexCam.h>
#include <FeKits/IR/SulPostFilterNightVision.h>

#include <osg/observer_ptr>

namespace FeKit
{
	/**
	  * @class CSulIR
	  * @brief 红外效果对外接口
	  * @note 用于初始化红外效果，设置显示的比例以及红外效果的具体实体等参数
	  * @author c00005
	*/
	class FEKIT_EXPORT CSulIR: public osg::Group, public CKitsCtrl
	{
	public:
		/**  
		  * @note 通过入参构造红外效果  
		  * @param pRenderContext [in] 渲染上下文，用于初始化视口等
		  * @param pNode [in] 实现红外效果的场景实体
		  * @param dWidthRadio [in] 红外效果显示窗口宽度占总视口的比例
		  * @param dHeightRadio [in] 红外效果显示窗口高度占总视口的比例
		  * @param dIR [in] 红外参数
		  * @param nSamplerTex [in] 红外参数
		  * @param setup [in] 红外参数
		*/
		CSulIR(
			FeUtil::CRenderContext* pRenderContext,
			osg::Node* pNode, 
			double dWidthRadio,
			double dHeightRadio, 
			float dIR = 0.6,
			sigma::uint32 nSamplerTex = 32,
			FeKit::CSulTexCam::ESETUP setup  = CSulTexCam::STANDARD);
		
		virtual ~CSulIR();

	public:
		void Show();

		void Hide();

		void AddNode(osg::Node* obj);

		virtual void ResizeKit(int w, int h);

	protected:
		void CreateIR(int w, int h);

	protected:
		double										m_dWidthRadio;
		double										m_dHeightRadio;

		float										m_fIR;

		FeKit::CSulTexCam::ESETUP					m_eSetup;
		sigma::uint32								m_nSamplerTex;

		osg::ref_ptr<osg::Group>					m_rpIR;

		osg::observer_ptr<osg::Node>				m_opEarthNode;

		osg::observer_ptr<osgViewer::Viewer>		m_opViewer;

		osg::observer_ptr<FeUtil::CRenderContext>	m_opRC;

		osg::ref_ptr<FeKit::CSulTexCam>				m_rpRtt;

		osg::ref_ptr<osg::Group>					m_rpCameraGroup;
	};
}

#endif