// SulFilterPass.h

#ifndef __SULFILTERPASS_H__
#define __SULFILTERPASS_H__

#include <FeKits/Export.h>
#include <FeKits/IR/SulTexCam.h>
#include <FeKits/IR/SulTransScreenAlign.h>

#include <osg/Group>
#include <osg/MatrixTransform>


namespace FeKit
{
	class  CSulFilterPass : public osg::Referenced
	{
    public:
        CSulFilterPass();
        ~CSulFilterPass();
	public:
		osg::Group*		create( osg::Texture2D* pTexIn, osg::Texture2D* pTexOut, const std::string& sNameRTT );

		CSulTexCam*		getTexCam();
		osg::Group*		getGroup();

    public:
		osg::ref_ptr<CSulTexCam>	m_rTexCam;
		osg::ref_ptr<osg::Group>	m_rGroup;
        osg::ref_ptr<CSulTransScreenAlign> m_rpAlign;
	};
}
#endif // __SULFILTERPASS_H__