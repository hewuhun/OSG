// SulTransScreenAlign.cpp

#include<FeKits/IR/SulTransScreenAlign.h>

namespace FeKit
{
	CSulTransScreenAlign::CSulTransScreenAlign( float left, float right, float bottom, float top )
	{
	
		m_rTrans = new osg::MatrixTransform;
		m_rTrans->setName( "CSulTransScreenAlign-T" );
		m_rTrans->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
		m_rTrans->setMatrix( osg::Matrix::identity() );

	
		m_rProj = new osg::Projection();
		m_rProj->setName( "CSulTransScreenAlign-P" );
		osg::Matrixd mOrtho = osg::Matrix::ortho2D( left, right, bottom, top );
		m_rProj->addChild( m_rTrans.get() );
		m_rProj->setMatrix( mOrtho );

        addChild(m_rProj);
	}

	CSulTransScreenAlign::CSulTransScreenAlign( float w, float h )
	{

		m_rTrans = new osg::MatrixTransform;
		m_rTrans->setName( "CSulTransScreenAlign-T" );
		m_rTrans->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
		m_rTrans->setMatrix( osg::Matrix::identity() );

		m_rProj = new osg::Projection();
		m_rProj->setName( "CSulTransScreenAlign-P" );
		osg::Matrixd mOrtho = osg::Matrix::ortho2D( 0, w, 0, h );
		m_rProj->addChild( m_rTrans.get() );
		m_rProj->setMatrix( mOrtho );
	}

	void CSulTransScreenAlign::AddChild( osg::Node* pNode )
	{
		m_rTrans->addChild( pNode );
	}

	osg::MatrixTransform* CSulTransScreenAlign::getMatrixTransform()
	{
		return m_rTrans;
	}

	osg::Projection* CSulTransScreenAlign::getProjection()
	{
		return m_rProj;
	}
}