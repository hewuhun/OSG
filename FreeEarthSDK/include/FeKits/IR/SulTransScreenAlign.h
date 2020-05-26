// SulTransScreenAlign.h

#ifndef __SULTRANSSCREENALIGN_H__
#define __SULTRANSSCREENALIGN_H__

#include <osg/Referenced>
#include <FeKits/Export.h>
#include <osg/MatrixTransform>
#include <osg/Projection>

namespace FeKit
{

	class  CSulTransScreenAlign : public osg::Group
	{
	public:
		CSulTransScreenAlign( float left, float right, float bottom, float top );
		CSulTransScreenAlign( float w, float h );

		void	AddChild( osg::Node* pNode );
		osg::MatrixTransform*				getMatrixTransform();
		osg::Projection*					getProjection();

	private:
		osg::ref_ptr<osg::MatrixTransform>	m_rTrans;
		osg::ref_ptr<osg::Projection>		m_rProj;
	};

}

#endif 