#ifndef __FE_LINECONNECT__
#define __FE_LINECONNECT__

/*

卫星常规的扫瞄地球的类，由（卫星位置【世界坐标】）来共同决定卫星扫瞄波的形状
所有参数一次成形，不接受修改，临时方案

若对角度大小不满意，调整Scan.cpp中的10.0 
osg::Vec3d thirdPos = osg::Matrix(osg::Matrix::translate(m_pos) * osg::Matrix::rotate(osg::inDegrees(10.0), osg::Vec3d(1.0, 0.0, 0.0))).getTrans() - m_pos;

*/

#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/NodeCallback>
#include <osg/Drawable>

#include <FeEffects/Export.h>

using namespace osg;

namespace FeEffect
{
	class LineDrawCallback;
	class FEEFFECTS_EXPORT CLineConnect : public osg::Group
	{
	public: 
		//pos位置-世界坐标，扫瞄角速度-度/秒
		CLineConnect(osg::Vec3d start, osg::Vec3d end, float startAlpha, float endAlpha, int lineWidth);

		void SetStart(osg::Vec3d start);

		void SetEnd(osg::Vec3d end);
		//重置start和end
		void SetStartAndEnd(osg::Vec3d start, osg::Vec3d end);
		//设置颜色
		void SetColor(osg::Vec4d color);
		//初始化
		void Init();

	protected:
		void Update();
	public:
		osg::ref_ptr<osg::MatrixTransform>	m_rpTransMT;

	private:
		//
		osg::Vec3d m_start;
		osg::Vec3d m_end;
		float m_startAlpha;
		float m_endAlpha;

	private:
		//
		osg::Geode* m_gnode;
		osg::Geometry* m_geom;
		osg::Vec4d m_color;

		osg::ref_ptr<LineDrawCallback> m_rpLineDrawCallback; 
	};

	class LineDrawCallback : public osg::Drawable::DrawCallback
	{
	public:
		LineDrawCallback(CLineConnect *pLinkLine,osg::Vec3d start, osg::Vec3d end,osg::Vec4d vecColor=osg::Vec4d(0.0,1.0,1.0,1.0));
		virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const;

		void SetStartAndEnd(osg::Vec3d start, osg::Vec3d end);

		void SetLineColor(osg::Vec4d vecColor);

	private:
		mutable osg::Vec3d m_start;
		mutable osg::Vec3d m_end;
		osg::observer_ptr<CLineConnect> m_opLinkLine;

		mutable int m_nframeTiaoGuo;
		mutable osg::Vec4d m_vecColor;
		mutable double		m_dTime;

	};
}

#endif //__FE_LINECONNECT__