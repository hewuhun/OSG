#ifndef FE_HISTORY_TRACK_EFFECT_H
#define FE_HISTORY_TRACK_EFFECT_H

/**************************************************************************************************
* @file HistoryTrackEffect.h
* @note 历史航迹效果
* @author z00013
* @data 2015-12-24
**************************************************************************************************/

#include <osg/Geometry>
#include <osg/Array>
#include <osg/Group>
#include <osg/Geode>

#include <FeEffects/Export.h>
#include <FeEffects/FreeEffect.h>

namespace FeEffect
{
	class CHistoryTrackEffect;
	class FEEFFECTS_EXPORT CHistoryTrackCallback : public osg::NodeCallback
	{
	public:
		CHistoryTrackCallback(osg::Geometry *pGeom,osg::DrawArrays *pDrawArrays,CHistoryTrackEffect *pTrack,osg::Vec3d vecFirstPos);

		CHistoryTrackCallback(osg::Geometry *pGeom,osg::DrawArrays *pDrawArrays,CHistoryTrackEffect *pTrack,osg::Vec3d vecFirstPos,osg::Vec4d vecColor= osg::Vec4d(1.0,0.0,0.0,1.0),osg::Vec3d vecPosr= osg::Vec3d(0.0,0.0,0.0),double dWide = 10.0);
		~CHistoryTrackCallback();

		void SetColor( osg::Vec4d vecColor );

		void SetCenter(osg::Vec3d vecCenter);

		void SetRibbonWide(double dWide);

	protected:
		virtual void operator()(osg::Node *pNode,osg::NodeVisitor *pNv);


	protected:
		osg::ref_ptr<osg::Geometry>								m_rpGeom;
		osg::ref_ptr<osg::Vec3dArray>							m_rpVertexArray;
		osg::Vec4d												m_vecColor;
		osg::Vec3d												m_vecPos;
		double													m_dWide;
		osg::observer_ptr<osg::DrawArrays>						m_opDrawArrays;
		osg::observer_ptr<CHistoryTrackEffect>							m_opTrack;
		osg::Vec3d												m_vecFirstPos;
		
	};
}

namespace FeEffect
{

	class FEEFFECTS_EXPORT CHistoryTrackEffect : public CFreeEffect
	{
	public:
		CHistoryTrackEffect(osg::MatrixTransform *pMt=NULL);

		virtual ~CHistoryTrackEffect();

	public:

		virtual void ClearLine();

		virtual bool IsDrawed() const;

		void AddTrackCallback();

		void RemoveTrackCallback();

		void SetMatrixTransform(osg::MatrixTransform *pMT);

	public:
		virtual bool CreateEffect();

		virtual bool UpdataEffect();

		virtual bool ClearEffect();

		virtual void SetColor(osg::Vec4d vecColor);

		virtual void SetCenter(osg::Vec3d vecCenter);
		void SetWide( double dWide );
		void ContinueDrawLine(std::vector<osg::Vec3d> vecHistoryVertex);

		//zs 第一个位置（世界坐标） 
		void SetFirstPostion(osg::Vec3d vecPostion);

		void AddKeyPos(osg::Vec3d vecPos);

	protected:
		osg::MatrixTransform								 *m_pMt;
		osg::ref_ptr<osg::Group>							  m_pPath;
		osg::ref_ptr<osg::Geometry>							  m_rpGeometry;
		osg::ref_ptr<osg::Vec3dArray>						  m_rpVertexArray;
		osg::ref_ptr<osg::Vec4dArray>						  m_rpColorArray;
		osg::ref_ptr<osg::DrawArrays>						  m_rpDrawArrays;

		float												  m_fLineWidth;

		osg::ref_ptr<CHistoryTrackCallback>					  m_pTrackCallbck;


		osg::Vec3d											m_vecXYZPos;

		std::vector<osg::Vec3d>									m_vecHistoryVertex;
		osg::ref_ptr<osg::MatrixTransform>					m_rpTransMt;

		osg::Vec3d											m_vecOldKey;
	};
}


#endif //FE_HISTORY_TRACK_EFFECT_H