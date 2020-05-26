/**************************************************************************************************
* @file FlameGeom.h
* @note 始终面向屏幕的正方形状的帧动画效果，用户可以设置自己的一系列图片实现需要效果
* @author c00005
* @data 2015-11-9
**************************************************************************************************/
#ifndef FE_FLAME_GEOM_H
#define FE_FLAME_GEOM_H

#include <osg/MatrixTransform>
#include <osg/ImageSequence>

#include <FeEffects/Export.h>

using namespace osg;

namespace FeEffect
{

	/**
	  * @class CFlameGeom
	  * @note 始终面向屏幕的正方形状的帧动画效果，用户可以设置自己的一系列图片实现需要效果,
	  *		  此种方法可以实现尾焰效果、爆炸效果以及其他的粒子效果。
	  * @author c00005
	*/
	class FEEFFECTS_EXPORT CFlameGeom:public osg::MatrixTransform
	{
	public:
		CFlameGeom(double dSize);

		~CFlameGeom();

	public:
		void Play();

		void Stop();

		void SetSize(double dSize);

		void SetLoopMode(osg::ImageStream::LoopingMode mode);

		void AddImage(osg::Image* pImage);

		void AddImagePath(std::string strImagePath);

		osg::ImageSequence* GetImageSequence();

		void AddExplosionImage();

	protected:
		void Init();
		
	protected:
		osg::ref_ptr<osg::ImageSequence> m_rpImageSequence;

		osg::ref_ptr<osg::Geometry> m_rpGeom;

		double m_dSize;
	};

};

#endif // FE_FLAME_GEOM_H