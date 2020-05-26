#ifndef __ABC_BANG__
#define __ABC_BANG__

#include <FeEffects/Export.h>
#include <FeEffects/FreeEffect.h>

#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/NodeCallback>
#include <osg/Drawable>
#include <osg/ImageSequence>
#include <osg/MatrixTransform>

using namespace osg;

namespace FeEffect
{
	class FEEFFECTS_EXPORT CGeoTile : public CFreeEffect
	{
	public:
		CGeoTile();

		~CGeoTile();

	public:
		virtual bool CreateEffect();

		virtual bool StartEffect();

		virtual bool ClearEffect();

	private:
		osg::ref_ptr<osg::ImageSequence>            m_rpImageSequence;
		osg::ref_ptr<osg::MatrixTransform>			m_pTransMT;
		osg::ref_ptr<osg::MatrixTransform>			m_pScaleMT;
		osg::ref_ptr<osg::MatrixTransform>			m_pRotateMT;
	};
}

#endif