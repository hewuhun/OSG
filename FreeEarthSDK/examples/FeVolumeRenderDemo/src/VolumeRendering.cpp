#include <VolumeRendering.h>

#include <osg/CullFace>
#include <osg/TransferFunction>
#include <osgEarth/Viewpoint>

#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeKits/manipulator/ManipulatorUtil.h>

#include <fstream>
#include <iostream>
#include "osgDB/WriteFile"

#define testHead

using namespace FeVolumeRender;

static const char *BackFaceVertSource = {
	"varying vec3 vPos;\n"
	"void main(void)\n"
	"{\n"
	"	 vPos = gl_Vertex;	\n"
	"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"}\n"
};

static const char *BackFaceFragSource = {
	"varying vec3 vPos;\n"
	"void main(void)\n"
	"{\n"
	"	gl_FragColor = vec4(vPos,1.0);	\n"
	"}\n"
};

static const char *FrontFaceVertSource = {
	"varying vec3 vEntryPoint;\n"
	"varying vec4 vExitPointCoord;\n"
	"void main(void)\n"
	"{\n"
	"	 vEntryPoint = gl_Vertex;	\n"
	"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"    vExitPointCoord = gl_Position;\n"
	"}\n"
};

static const char *FrontFaceFragSource = {
	"varying vec3 vEntryPoint;\n"
	"varying vec4 vExitPointCoord;\n"
	"uniform sampler1D uTransferTex;\n"
	"uniform sampler2D uBackTex;\n"
	"uniform sampler3D uVolumeTex;\n"
	"uniform vec2 uScreenSize;\n"
	"uniform float uStepSize;\n"
	"uniform int uSampleNum;\n"
	"uniform float uMinVolumeValue;\n"
	"uniform float uVolumeValueRange;\n"
	"void main(void)\n"
	"{\n"
	"	vec3 exitPoint = texture2D(uBackTex,gl_FragCoord.xy/uScreenSize).xyz;\n"
	"   if(length(exitPoint - vEntryPoint) < 0.001)\n"
	"	    discard;							   \n"

	"   vec3 dir = exitPoint - vEntryPoint;                                                    \n"
	"   float len = length(dir);															   \n"
	"   vec3 deltaDir = normalize(dir) * uStepSize;											   \n"
	"   float deltaDirLen = length(deltaDir);												   \n"
	"   vec3 voxelCoord = vEntryPoint;														   \n"
	"   vec4 colorAcum = vec4(0.0); 														   \n"
	"   float alphaAcum = 0.0;																   \n"
	"   float intensity;																	   \n"
	"   float lengthAcum = 0.0;																   \n"
	"   vec4 colorSample;																	   \n"
	"   float alphaSample;																	   \n"
	"   vec4 bgColor = vec4(1.0, 1.0, 1.0, 0.0);											   \n"
	"   																					   \n"
	"   for(int i = 0; i < uSampleNum; i++)														   \n"
	"   {																					   \n"
	"   	intensity =  texture3D(uVolumeTex, voxelCoord).x;								   \n"
	"   																					   \n"
#ifdef testHead
	"   	colorSample = texture1D(uTransferTex, intensity);								   \n"
#else
	"   	colorSample = texture1D(uTransferTex, (intensity - uMinVolumeValue) / uVolumeValueRange);								   \n"
#endif
	"   																					   \n"
	"   	if (colorSample.a > 0.0)														   \n"
	"   	{																				   \n"
	"   		colorSample.a = 1.0 - pow(1.0 - colorSample.a, uStepSize * 200.0f);			   \n"
	"   		colorAcum.rgb += (1.0 - colorAcum.a) * colorSample.rgb * colorSample.a;		   \n"
	"   		colorAcum.a += (1.0 - colorAcum.a) * colorSample.a;							   \n"
	"   	}																				   \n"
	"   	voxelCoord += deltaDir;															   \n"
	"   	lengthAcum += deltaDirLen;														   \n"
	"   	if (lengthAcum >= len )															   \n"
	"   	{																				   \n"
	"   		colorAcum.rgb = colorAcum.rgb * colorAcum.a + (1 - colorAcum.a) * bgColor.rgb; \n"
	"   		break;																		   \n"
	"   	}																				   \n"
	"   	else if (colorAcum.a > 1.0)														   \n"
	"   	{																				   \n"
	"   		colorAcum.a = 1.0;															   \n"
	"   		break;																		   \n"
	"   	}																				   \n"
	"   }																					   \n"

	"	gl_FragColor = colorAcum;	\n"

	//"	gl_FragColor = vec4(exitPoint,1.0);	\n"
	"}\n"
};

FeVolumeRender::CVolumeRenderGeometry::CVolumeRenderGeometry( FeUtil::CRenderContext* rc )
	:m_opRenderContext(rc)
{
	setUseDisplayList(false);
	setUseVertexBufferObjects(true);

	InitGeometry();

	getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
	getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);  
	getOrCreateStateSet()->setRenderBinDetails(20,"DepthSortedBin");
}

void FeVolumeRender::CVolumeRenderGeometry::InitGeometry()
{
	osg::Vec3Array* va = new osg::Vec3Array();	
	float boxSize = 1;
	va->push_back(osg::Vec3(0,0,0));
	va->push_back(osg::Vec3(0,0,boxSize));
	va->push_back(osg::Vec3(boxSize,0,boxSize));
	va->push_back(osg::Vec3(boxSize,0,0));
	va->push_back(osg::Vec3(0,boxSize,0));
	va->push_back(osg::Vec3(0,boxSize,boxSize));
	va->push_back(osg::Vec3(boxSize,boxSize,boxSize));
	va->push_back(osg::Vec3(boxSize,boxSize,0));

	osg::DrawElementsUByte* ia = new osg::DrawElementsUByte(osg::PrimitiveSet::TRIANGLES);
	ia->push_back(0);
	ia->push_back(2);
	ia->push_back(1);
	ia->push_back(0);
	ia->push_back(3);
	ia->push_back(2);
	ia->push_back(4);
	ia->push_back(5);
	ia->push_back(6);
	ia->push_back(4);
	ia->push_back(6);
	ia->push_back(7);
	ia->push_back(1);
	ia->push_back(6);
	ia->push_back(5);
	ia->push_back(1);
	ia->push_back(2);
	ia->push_back(6);
	ia->push_back(0);
	ia->push_back(4);
	ia->push_back(7);
	ia->push_back(0);
	ia->push_back(7);
	ia->push_back(3);
	ia->push_back(0);
	ia->push_back(1);
	ia->push_back(5);
	ia->push_back(0);
	ia->push_back(5);
	ia->push_back(4);
	ia->push_back(3);
	ia->push_back(7);
	ia->push_back(6);
	ia->push_back(3);
	ia->push_back(6);
	ia->push_back(2);

	osg::Vec4Array* ca = new osg::Vec4Array();
	ca->push_back(osg::Vec4(1.0,1.0,0.0,1.0));

	this->setVertexArray(va);
	this->addPrimitiveSet(ia);
	this->setColorArray(ca,osg::Array::BIND_OVERALL);
}

FeVolumeRender::MyVolumeRenderNodeCallback::MyVolumeRenderNodeCallback( CVolumeRenderNode* vrn )
	:m_opVolumeRenderNode(vrn)
{

}

void FeVolumeRender::MyVolumeRenderNodeCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
	if(m_opVolumeRenderNode.valid())
	{
		m_opVolumeRenderNode->Update();
	}
}

FeVolumeRender::CVolumeRenderNode::CVolumeRenderNode( FeUtil::CRenderContext* rc )
	:m_rpVolumeRenderGeometry(NULL)
	,m_opRenderContext(rc)
	,m_rpBoxCamera(NULL)
	,m_rpBoxTex(NULL)
	,m_texWidth(0)
	,m_texHeight(0)
	,m_rpVolumeRenderNodeCallback(NULL)
	,m_rpVolumeTex(NULL)
	,m_stepSize(0.001f)
	,m_rpTransferTex(NULL)
	,m_rpWorldMat(NULL)
	,m_startLon(0)
	,m_endLon(0)
	,m_stepLon(0)
	,m_startLat(0)
	,m_endLat(0)
	,m_stepLat(0)
	,m_startAlt(0)
	,m_endAlt(0)
	,m_stepAlt(0)
	,m_minVolumeValue(FLT_MAX)
	,m_maxVolumeValue(FLT_MIN)
	,m_uSampleNum(1600)
{
	m_transferTexName = FeFileReg->GetFullPath("texture/volume/tff.dat");
	m_volumeTexName = FeFileReg->GetFullPath("texture/volume/head256.raw");
	m_eleDataName = FeFileReg->GetFullPath("texture/volume/EleDensity.txt");

	m_rpVolumeRenderNodeCallback = new MyVolumeRenderNodeCallback(this);
	setUpdateCallback(m_rpVolumeRenderNodeCallback);

	InitWorldMat(m_eleDataName);
	//InitTexCamera();
	CreateGeometry();	

	//AddHud();
}

void FeVolumeRender::CVolumeRenderNode::Update()
{
	if(m_opRenderContext.valid())
	{
		osg::Camera* cam = m_opRenderContext->GetCamera();
		if(cam)
		{
			osg::Viewport* vp = cam->getViewport();
			if(vp)
			{
				m_texWidth = vp->width();
				m_texHeight = vp->height();
			}
		}
	}


	if(m_rpBoxTex.valid() && m_rpBoxCamera.valid())
	{
		if(m_rpBoxTex->getTextureWidth() != m_texWidth || m_rpBoxTex->getTextureHeight() != m_texHeight)
		{
			if(m_rpWorldMat.valid())
			{
				m_rpWorldMat->removeChild(m_rpBoxCamera);
				m_rpBoxCamera = NULL;
				m_rpBoxTex = NULL;
			}
		}

		if(m_rpVolumeRenderGeometry.valid())
		{
			osg::Uniform* uScreenSize = m_rpVolumeRenderGeometry->getStateSet()->getUniform("uScreenSize");
			if(uScreenSize)
			{
				uScreenSize->set(osg::Vec2(m_texWidth,m_texHeight));
			}
		}
	}
	else
	{
		InitTexCamera();
		if(m_rpVolumeRenderGeometry.valid())
		{
			int textUnit0 = 0;
			m_rpVolumeRenderGeometry->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit0,m_rpBoxTex,osg::StateAttribute::ON);
		}
	}

}

void FeVolumeRender::CVolumeRenderNode::AddHud()
{
	float w = 800;
	float h = 600;

	m_rpHud = new osg::Camera();
	m_rpHud->setProjectionMatrix(osg::Matrix::ortho2D(0,w,0,h));
	m_rpHud->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	m_rpHud->setViewMatrix(osg::Matrix::identity());
	m_rpHud->setClearMask(GL_DEPTH_BUFFER_BIT);
	m_rpHud->setRenderOrder(osg::Camera::POST_RENDER);
	m_rpHud->setAllowEventFocus(false);

	osg::Geometry* gm = new osg::Geometry();

	float sc = 3.0;
	osg::Vec3Array* va = new osg::Vec3Array();
	va->push_back(osg::Vec3(0,0,0));
	va->push_back(osg::Vec3(w / sc,0,0));
	va->push_back(osg::Vec3(w / sc,h / sc,0));
	va->push_back(osg::Vec3(0,h / sc,0));

	osg::Vec2Array* ta = new osg::Vec2Array();
	ta->push_back(osg::Vec2(0,0));
	ta->push_back(osg::Vec2(1,0));
	ta->push_back(osg::Vec2(1,1));
	ta->push_back(osg::Vec2(0,1));

	osg::DrawElementsUByte* ia = new osg::DrawElementsUByte(osg::PrimitiveSet::TRIANGLES);
	ia->push_back(0);
	ia->push_back(1);
	ia->push_back(2);
	ia->push_back(0);
	ia->push_back(2);
	ia->push_back(3);

	gm->setVertexArray(va);
	gm->addPrimitiveSet(ia);
	gm->setTexCoordArray(0,ta);

	gm->getOrCreateStateSet()->setTextureAttributeAndModes(0,m_rpBoxTex);
	gm->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	osg::Geode* gn = new osg::Geode();
	gn->addDrawable(gm);

	m_rpHud->addChild(gn);

	if(m_opRenderContext.valid())
		m_opRenderContext->GetRoot()->addChild(m_rpHud);
}

void FeVolumeRender::CVolumeRenderNode::CreateGeometry()
{
	m_rpVolumeRenderGeometry = new CVolumeRenderGeometry(m_opRenderContext.get());
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK),osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

	InitTransferTex(m_transferTexName);
#ifdef testHead
	InitVolumeTex(m_volumeTexName,256, 256, 225);
#else
	InitVolumeTex(m_eleDataName,256, 256, 225);
#endif


	int textUnit0 = 0;
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit0,m_rpBoxTex,osg::StateAttribute::ON);
	int textUnit1 = 1;
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit1,m_rpVolumeTex,osg::StateAttribute::ON);
	int textUnit2 = 2;
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->setTextureAttributeAndModes(textUnit2,m_rpTransferTex,osg::StateAttribute::ON);

	osg::Program* program = new osg::Program;
	program->addShader( new osg::Shader( osg::Shader::VERTEX, FrontFaceVertSource ) );
	program->addShader( new osg::Shader( osg::Shader::FRAGMENT, FrontFaceFragSource ) );
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->setAttributeAndModes( program, osg::StateAttribute::ON );
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->addUniform( new osg::Uniform("uBackTex", textUnit0) );
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->addUniform( new osg::Uniform("uVolumeTex", textUnit1) );
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->addUniform( new osg::Uniform("uTransferTex", textUnit2) );
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->addUniform( new osg::Uniform("uStepSize", m_stepSize) );
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->addUniform( new osg::Uniform("uMinVolumeValue", m_minVolumeValue) );
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->addUniform( new osg::Uniform("uVolumeValueRange",m_maxVolumeValue - m_minVolumeValue) );
	m_rpVolumeRenderGeometry->getOrCreateStateSet()->addUniform( new osg::Uniform("uScreenSize", osg::Vec2(m_texWidth,m_texHeight)) );


	m_rpVolumeRenderGeometry->getOrCreateStateSet()->addUniform( new osg::Uniform("uSampleNum", m_uSampleNum) );

	osg::Geode* gn = new osg::Geode();
	gn->addDrawable(m_rpVolumeRenderGeometry);

	if(m_rpWorldMat.valid())
		m_rpWorldMat->addChild(gn);
}

void FeVolumeRender::CVolumeRenderNode::InitTexCamera()
{
	if(!m_rpBoxTex.valid())
	    m_rpBoxTex = new osg::Texture2D();
	m_rpBoxTex->setTextureSize(m_texWidth, m_texHeight); 
	m_rpBoxTex->setInternalFormat(GL_RGBA16F_ARB); 
	m_rpBoxTex->setSourceFormat(GL_LUMINANCE); 
	m_rpBoxTex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
	m_rpBoxTex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
	m_rpBoxTex->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
	m_rpBoxTex->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);
	m_rpBoxTex->setResizeNonPowerOfTwoHint(false);

	if(!m_rpBoxCamera.valid())
	    m_rpBoxCamera = new osg::Camera();
	m_rpBoxCamera->setName("hdr_light_camera");
	m_rpBoxCamera->setReferenceFrame(osg::Transform::RELATIVE_RF); 
	m_rpBoxCamera->setViewport(0, 0, m_texWidth, m_texHeight); 
	m_rpBoxCamera->setRenderOrder(osg::Camera::PRE_RENDER); 
	m_rpBoxCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT ); 
	m_rpBoxCamera->attach(osg::Camera::COLOR_BUFFER, m_rpBoxTex); 
	m_rpBoxCamera->setAllowEventFocus(false);
	m_rpBoxCamera->setInheritanceMask(osg::CullSettings::COMPUTE_NEAR_FAR_MODE |
		osg::CullSettings::CULLING_MODE                            |
		osg::CullSettings::LOD_SCALE                               |
		osg::CullSettings::SMALL_FEATURE_CULLING_PIXEL_SIZE        |
		osg::CullSettings::CLAMP_PROJECTION_MATRIX_CALLBACK        |
		osg::CullSettings::NEAR_FAR_RATIO                          |
		osg::CullSettings::IMPOSTOR_ACTIVE                         |
		osg::CullSettings::DEPTH_SORT_IMPOSTOR_SPRITES             |
		osg::CullSettings::IMPOSTOR_PIXEL_ERROR_THRESHOLD          |
		osg::CullSettings::NUM_FRAMES_TO_KEEP_IMPOSTORS_SPRITES    |
		osg::CullSettings::CULL_MASK                               |
		osg::CullSettings::CULL_MASK_LEFT                          |
		osg::CullSettings::CULL_MASK_RIGHT                         |
		osg::CullSettings::CLEAR_COLOR                             |
		osg::CullSettings::CLEAR_MASK                              |
		osg::CullSettings::LIGHTING_MODE                           |
		osg::CullSettings::LIGHT                                   );

	CVolumeRenderGeometry* vrg = new CVolumeRenderGeometry(m_opRenderContext.get());
	vrg->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT),osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
	osg::Program* program = new osg::Program;
	program->addShader( new osg::Shader( osg::Shader::VERTEX, BackFaceVertSource ) );
	program->addShader( new osg::Shader( osg::Shader::FRAGMENT, BackFaceFragSource ) );
	vrg->getOrCreateStateSet()->setAttributeAndModes( program, osg::StateAttribute::ON );
	osg::Geode* gn = new osg::Geode();
	gn->addDrawable(vrg);
	m_rpBoxCamera->addChild(gn);

	if(m_rpWorldMat.valid())
		m_rpWorldMat->addChild(m_rpBoxCamera);
}

void FeVolumeRender::CVolumeRenderNode::InitVolumeTex( std::string fileName,int w,int h,int d )
{
#ifdef testHead
	FILE *fp;
	size_t size = w * h * d;
	GLubyte *data = new GLubyte[size];			  // 8bit
	if (!(fp = fopen(fileName.c_str(), "rb")))
	{
		return;
	}

	size_t readSize = fread(data, sizeof(char), size, fp);
	fclose(fp);

	if ( readSize != size) 
	{
		return;
	}


	osg::Image* img = new osg::Image();
	img->setImage(w, h, d,GL_INTENSITY,
		GL_LUMINANCE, GL_UNSIGNED_BYTE,
		data,
		osg::Image::USE_NEW_DELETE);
#else
	std::vector<float> valueList;
	std::ifstream inFile(fileName, std::ifstream::in);
	if(inFile.is_open())
	{
		const unsigned int maxLength = 1024 * 10;
		char buffer[maxLength];

		inFile.getline(buffer,maxLength);
		inFile.getline(buffer,maxLength);
		inFile.getline(buffer,maxLength);

		while(!inFile.eof())
		{
			inFile.getline(buffer,maxLength);
			std::string valueStr(buffer);

			StringToFloats(valueStr,valueList);
		}	
	}

	w = (int)((m_endLon - m_startLon) / m_stepLon) + 1;
	h = (int)((m_endAlt - m_startAlt) / m_stepAlt) + 1;
	d = (int)((m_endLat - m_startLat) / m_stepLat) + 1;

	osg::Image* img = new osg::Image();
	img->allocateImage(w,h,d,GL_LUMINANCE,GL_FLOAT);
	img->setInternalTextureFormat(GL_INTENSITY32F_ARB);
	int noPixels = w * h * d;

	if(valueList.size() > noPixels)
	{
		int valueIndex = 0;
		float* dataPtr = (float*)img->data();
		for (int i = 0;i < w;i++)
		{
			for (int j = 0;j < h;j++)
			{
				for (int k = 0;k < d;k++)
				{
					*dataPtr++ = valueList[i];
					if(valueList[i] < m_minVolumeValue)
					{
						m_minVolumeValue = valueList[i];
					}
					if(valueList[i] > m_maxVolumeValue)
					{
						m_maxVolumeValue = valueList[i];
					}
					valueIndex++;
				}
			}
		}
	}
#endif

	m_rpVolumeTex = new osg::Texture3D();
	m_rpVolumeTex->setImage(img);
	m_rpVolumeTex->setResizeNonPowerOfTwoHint(false);

	m_rpVolumeTex->setFilter(osg::Texture3D::MIN_FILTER, osg::Texture3D::LINEAR);
	m_rpVolumeTex->setFilter(osg::Texture3D::MAG_FILTER, osg::Texture3D::LINEAR);
	m_rpVolumeTex->setWrap(osg::Texture::WRAP_S,osg::Texture::REPEAT);
	m_rpVolumeTex->setWrap(osg::Texture::WRAP_T,osg::Texture::REPEAT);
	m_rpVolumeTex->setWrap(osg::Texture::WRAP_R,osg::Texture::REPEAT);
}

void FeVolumeRender::CVolumeRenderNode::InitTransferTex( std::string fileName )
{
#ifdef testHead
	std::ifstream inFile(fileName, std::ifstream::in);
	if (!inFile)
	{
		return;
	}

	const int MAX_CNT = 10000;
	GLubyte *tff = (GLubyte *) calloc(MAX_CNT, sizeof(GLubyte));
	inFile.read(reinterpret_cast<char *>(tff), MAX_CNT);
	if (inFile.eof())
	{
		size_t bytecnt = inFile.gcount();
		*(tff + bytecnt) = '\0';
	}
	else if(inFile.fail())
	{
		return;
	}
	else
	{
		return;
	}    

	osg::Image* img = new osg::Image();
	img->setImage(256,1,1,GL_RGBA8,GL_RGBA,GL_UNSIGNED_BYTE,tff,osg::Image::USE_NEW_DELETE);
#else
	float alphaPow = 0.1;
	osg::ref_ptr<osg::TransferFunction1D> xfer = new osg::TransferFunction1D();
	xfer->setColor(  0.0000, osg::Vec4f(14.0/255.0,46.0/255.0,133.0/255.0, 0.1 * alphaPow), false);
	xfer->setColor(  0.0062, osg::Vec4f(8.0/255.0,103.0/255.0,183.0/255.0, 0.2 * alphaPow), false);
	xfer->setColor(  0.1250, osg::Vec4f(39.0/255.0,196.0/255.0,231.0/255.0, 0.3 * alphaPow), false);
	xfer->setColor(  0.3250, osg::Vec4f(143.0/255.0,228.0/255.0,224.0/255.0, 0.4 * alphaPow), false);
	xfer->setColor(  0.7500, osg::Vec4f(215.0/255.0,237.0/255.0,63.0/255.0, 0.5 * alphaPow), false);
	xfer->setColor(  0.9000, osg::Vec4f(156.0/255.0,80.0/255.0,24.0/255.0, 0.6 * alphaPow), false);
	xfer->setColor(  1.0000, osg::Vec4f(255.0/255.0,0.0/255.0,0.0/255.0, 0.7 * alphaPow), false);
	xfer->updateImage();
#endif

	m_rpTransferTex = new osg::Texture1D();
#ifdef testHead
	m_rpTransferTex->setImage(img);
#else
	m_rpTransferTex->setImage(xfer->getImage());
#endif

	m_rpTransferTex->setResizeNonPowerOfTwoHint(false);

	m_rpTransferTex->setFilter(osg::Texture1D::MIN_FILTER, osg::Texture1D::NEAREST);
	m_rpTransferTex->setFilter(osg::Texture1D::MAG_FILTER, osg::Texture1D::NEAREST);
	m_rpTransferTex->setWrap(osg::Texture::WRAP_S,osg::Texture::REPEAT);   
}

void FeVolumeRender::CVolumeRenderNode::InitWorldMat( std::string fileName )
{
#ifdef testHead
	m_rpWorldMat = new osg::MatrixTransform();
	addChild(m_rpWorldMat);
#else
	std::ifstream inFile(fileName, std::ifstream::in);
	if(inFile.is_open())
	{
		const int maxLength = 256;
		char buffer[maxLength];

		inFile.getline(buffer,maxLength);
		std::string lonStr(buffer);

		std::vector<float> floatList;
		StringToFloats(lonStr,floatList);
		if(floatList.size() >= 3)
		{
			m_startLon = floatList[0];
			m_endLon = floatList[1];
			m_stepLon = floatList[2];
		}
		floatList.clear();

		inFile.getline(buffer,maxLength);
		std::string latStr(buffer);

		StringToFloats(latStr,floatList);
		if(floatList.size() >= 3)
		{
			m_startLat = floatList[0];
			m_endLat = floatList[1];
			m_stepLat = floatList[2];
		}
		floatList.clear();

		inFile.getline(buffer,maxLength);
		std::string altStr(buffer);

		StringToFloats(altStr,floatList);
		if(floatList.size() >= 3)
		{
			m_startAlt = floatList[0];
			m_endAlt = floatList[1];
			m_stepAlt = floatList[2];
		}
		floatList.clear();

		osg::Vec3d leftDown(m_startLon,m_startLat,m_startAlt);
		osg::Vec3d leftDownPos;
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),leftDown,leftDownPos);

		osg::Vec3d rightDown(m_endLon,m_startLat,m_startAlt);
		osg::Vec3d rightDownPos;
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),rightDown,rightDownPos);

		osg::Vec3d leftDownBack(m_startLon,m_endLat,m_startAlt);
		osg::Vec3d leftDownBackPos;
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),leftDownBack,leftDownBackPos);

		osg::Vec3d leftUp(m_startLon,m_startLat,m_endAlt);
		osg::Vec3d leftUpPos;
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),leftUp,leftUpPos);

		float boxWidth = (rightDownPos - leftDownPos).length();
		float boxHeight = (leftUpPos - leftDownPos).length();
		float boxDepth = (leftDownBackPos - leftDownPos).length();

		m_rpWorldMat = new osg::MatrixTransform();
		osg::Matrix scaleMat;
		scaleMat.makeScale(boxWidth,boxDepth,boxHeight);
		osg::Matrix transMat;
		FeUtil::XYZ2Matrix(m_opRenderContext.get(),leftDownPos,transMat);
		m_rpWorldMat->setMatrix(scaleMat * transMat);

		addChild(m_rpWorldMat);
	}
#endif
}

void FeVolumeRender::CVolumeRenderNode::StringToFloats( std::string inStr,std::vector<float>& floatList )
{
	std::string::size_type lastIndex = 0;

	while(1)
	{
		std::string::size_type startIndex = inStr.find_first_not_of(' ',lastIndex);
		if(startIndex == std::string::npos)
		{
			break;
		}

		bool needBreak = false;

		std::string::size_type endIndex = inStr.find_first_of(' ',startIndex);
		if(endIndex == std::string::npos)
		{
			needBreak = true;
			endIndex = inStr.length() + 1;
		}

		std::string subStr = inStr.substr(startIndex,endIndex - startIndex);
		float subFloat = atof(subStr.c_str());
		floatList.push_back(subFloat);

		if(needBreak)
			break;

		lastIndex = endIndex;
	}
}

void FeVolumeRender::CVolumeRenderNode::SetStepSize( float fSize )
{
	m_stepSize = fSize;

	osg::Uniform* pUniform = m_rpVolumeRenderGeometry->getOrCreateStateSet()->getUniform("uStepSize");
	if (pUniform)
	{
		pUniform->set(fSize);
	}
}

void FeVolumeRender::CVolumeRenderNode::SetSampleNum( int nNum )
{
	m_uSampleNum = nNum;

	osg::Uniform* pUniform = m_rpVolumeRenderGeometry->getOrCreateStateSet()->getUniform("uSampleNum");
	if (pUniform)
	{
		pUniform->set(nNum);
	}
}
