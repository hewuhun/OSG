#include <FeMeasure/FloodSurface.h>

#include <osg/Node>
#include <osg/Material>
#include <osg/LineWidth>
#include <osgDB/ReadFile>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TexMat>
#include <osg/TexGen>
#include <osg/Stencil>
#include <osg/TextureCubeMap>
#include <osgUtil/Tessellator>

#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/PathRegistry.h>

namespace FeMeasure
{
	static const char *vertexShadersource = {
		"varying vec3  Normal;\n"
		"varying vec3  EyeDir;\n"
		"\n"
		"varying vec2 texNormal0Coord;\n"
		"varying vec2 texColorCoord;\n"
		"varying vec2 texFlowCoord;\n"
		"\n"
		"uniform float osg_FrameTime;\n"
		"uniform mat4 osg_ViewMatrixInverse;\n"
		"varying float myTime;\n"
		"void main(void)\n"
		"{\n"
		"    gl_Position    = ftransform();\n"
		"    Normal         = normalize(gl_NormalMatrix * gl_Normal);\n"
		"    vec4 pos       = gl_ModelViewMatrix * gl_Vertex;\n"
		"    EyeDir         = vec3(osg_ViewMatrixInverse*vec4(pos.xyz,0));\n"
		"    texNormal0Coord   = gl_MultiTexCoord0.xy;\n"
		"    texColorCoord = gl_MultiTexCoord3.xy;\n"
		"    texFlowCoord = gl_MultiTexCoord2.xy;\n"
		"    myTime = 0.01 * osg_FrameTime;\n"
		"}\n"
	};

	static const char *fragShader = {
		"uniform sampler2D normalMap;\n"
		"uniform sampler2D colorMap;\n"
		"uniform sampler2D flowMap;\n"
		"uniform samplerCube cubeMap;\n"
		"uniform float waveLength;\n"
		"uniform float waveSpheed;\n"
		"varying vec3  Normal;\n"
		"varying vec3  EyeDir;\n"
		"varying vec2 texNormal0Coord;\n"
		"varying vec2 texColorCoord;\n"
		"varying vec2 texFlowCoord;\n"
		"varying float myTime;\n"
		"void main (void)\n"
		"{\n"
		"    float texScale = 10.0;\n"
		"    float texScale2 = waveLength;\n" 
		"    float myangle;\n"
		"    float transp;\n"
		"    vec3 myNormal;\n"
		"    vec2 mytexFlowCoord = texFlowCoord * texScale;\n"
		" 	  vec2 ff =  abs(2*(frac(mytexFlowCoord)) - 1) -0.5;\n" 	  
		"	  ff = 0.5-4*ff*ff*ff;\n"
		"	  vec2 ffscale = sqrt(ff*ff + (1-ff)*(1-ff));\n"
		"	  vec2 Tcoord = texNormal0Coord  * texScale2;\n"
		"		vec2 offset = vec2(myTime*waveSpheed,0);\n"
		"    vec3 sample = texture2D( flowMap, floor(mytexFlowCoord)/ texScale).rgb;\n"
		"    vec2 flowdir = sample.xy -0.5;\n"    
		"    flowdir *= sample.b;\n"
		"    mat2 rotmat = mat2(flowdir.x, -flowdir.y, flowdir.y ,flowdir.x);\n"
		"    vec2 NormalT0 = texture2D(normalMap, rotmat * Tcoord - offset).rg;\n"
		"    sample = texture2D( flowMap, floor((mytexFlowCoord + vec2(0.5,0)))/ texScale ).rgb;\n"
		"    flowdir = sample.b * (sample.xy - 0.5);\n"
		"    rotmat = mat2(flowdir.x, -flowdir.y, flowdir.y ,flowdir.x);\n"
		"	  vec2 NormalT1 = texture2D(normalMap, rotmat * Tcoord - offset*1.06+0.62).rg ;\n" 
		"	  vec2 NormalTAB = ff.x * NormalT0 + (1-ff.x) * NormalT1;\n"
		"    sample = texture2D( flowMap, floor((mytexFlowCoord + vec2(0.0,0.5)))/ texScale ).rgb;\n"
		"    flowdir = sample.b * (sample.xy - 0.5);\n"
		"    rotmat = mat2(flowdir.x, -flowdir.y, flowdir.y ,flowdir.x);\n"	      
		"    NormalT0 = texture2D(normalMap, rotmat * Tcoord - offset*1.33+0.27).rg;\n"
		"	  sample = texture2D( flowMap, floor((mytexFlowCoord + vec2(0.5,0.5)))/ texScale ).rgb;\n"
		"    flowdir = sample.b * (sample.xy - 0.5);\n"
		"    rotmat = mat2(flowdir.x, -flowdir.y, flowdir.y ,flowdir.x);\n"
		"	  NormalT1 = texture2D(normalMap, rotmat * Tcoord - offset*1.24).rg ;\n"
		"	  vec2 NormalTCD = ff.x * NormalT0 + (1-ff.x) * NormalT1;\n"
		"	  vec2 NormalT = ff.y * NormalTAB + (1-ff.y) * NormalTCD;\n"
		"    NormalT = (NormalT - 0.5) / (ffscale.y * ffscale.x);\n"
		"    NormalT *= 0.3;\n" 
		"    transp = texture2D( flowMap, texFlowCoord ).a;\n"
		"    NormalT *= transp*transp;\n"
		"    myNormal = vec3(NormalT,sqrt(1-NormalT.x*NormalT.x - NormalT.y*NormalT.y));\n"
		"    vec3 reflectDir = reflect(EyeDir, myNormal);\n"
		"    vec3 envColor = vec3 (textureCube(cubeMap, -reflectDir));\n" 
		"    myangle = dot(myNormal,normalize(EyeDir));\n"
		"    myangle = 0.95-0.6*myangle*myangle;\n"
		"    vec3 base = texture2D(colorMap,texColorCoord + (myNormal.xy/texScale2)*0.03*transp).rgb;\n"
		"    gl_FragColor = vec4 (mix(base,envColor,myangle*transp),1.0 );\n"
		"}\n"
	};
	
	CFloodSurface::CFloodSurface(FeUtil::CRenderContext* pContext)
		:osg::MatrixTransform()
		,m_opRenderContext(pContext)
	{
		m_pWaterBorderRoot = new osg::Group;

		m_dMinHeight = 0.0;
		m_dMaxHeight = 0.0;

		setDataVariance(Object::DYNAMIC);
	}

	CFloodSurface::~CFloodSurface()
	{

	}

	void CFloodSurface::Reset()
	{
		m_basicPosition.z() = m_dMinHeight;
	}

	bool CFloodSurface::Raise( double dHei )
	{
		bool bStop = false;
		m_basicPosition.z() += dHei;
		if (m_basicPosition.z() >= m_dMaxHeight)
		{
			m_basicPosition.z() = m_dMaxHeight;
			bStop = true;
		}

		osg::Vec3d xyzPos;
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), m_basicPosition, xyzPos);
		setMatrix(osg::Matrix::translate(xyzPos));

		return bStop;
	}

	void CFloodSurface::SetHeight( double dMaxHei, double dMinHei )
	{
		m_dMaxHeight = dMaxHei;
		m_dMinHeight = dMinHei;
	}

	void CFloodSurface::SetFloodCoords( osg::Vec3dArray* pLLHCoords )
	{
		m_rpFloodCoords = pLLHCoords;
		if (m_rpFloodCoords->size() > 0)
		{
			m_basicPosition = osg::Vec3d(
				m_rpFloodCoords->at(0).x(), 
				m_rpFloodCoords->at(0).y(), 
				m_dMinHeight);

			osg::Vec3d xyzPos;
			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),m_basicPosition, xyzPos);
			setMatrix(osg::Matrix::translate(xyzPos));
		}
	}

	void CFloodSurface::Hide()
	{
		setNodeMask(0x00000000);
	}

	void CFloodSurface::Show()
	{
		setNodeMask(~0x00000000);
	}

	void CFloodSurface::InitFloodSurface()
	{
		this->removeChildren(0, this->getNumChildren());
		addChild(m_pWaterBorderRoot.get());

		if (false == m_rpFloodCoords.valid())
		{
			return ;
		}

		//用于保存水面边界的顶点
		osg::ref_ptr<osg::Vec3dArray> pWaterBorderCoord = new osg::Vec3dArray;
		osg::ref_ptr<osg::Vec3Array> pWaterCoord = new osg::Vec3Array;

		osg::Vec3d xyzBasixPos;
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), m_basicPosition, xyzBasixPos);

		for (int i = 0; i < m_rpFloodCoords->size(); i++)
		{
			osg::Vec3d xyzMinHeiPoint, xyzDeltaHeiPoint;
			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),
				osg::Vec3d(
					m_rpFloodCoords->at(i).x(),
					m_rpFloodCoords->at(i).y(), 
					m_dMinHeight),
				xyzMinHeiPoint);

			//1000.0用于消除误差带来的效果影响
			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),
				osg::Vec3d(
				m_rpFloodCoords->at(i).x(),
				m_rpFloodCoords->at(i).y(), 
				m_dMinHeight - (m_dMaxHeight-m_dMinHeight) - 1000.0),
				xyzDeltaHeiPoint);

			pWaterCoord->push_back(xyzMinHeiPoint-xyzBasixPos);
			pWaterBorderCoord->push_back(xyzDeltaHeiPoint-xyzBasixPos);
			pWaterBorderCoord->push_back(xyzMinHeiPoint-xyzBasixPos);
		}

		//绘制水面的边界
		{
			//绘制边界线
			{
				osg::ref_ptr<osg::Geode> pBorderLineGeode = new osg::Geode;
				osg::ref_ptr<osg::Geometry> pBorderLineGeom = new osg::Geometry;
				osg::ref_ptr<osg::Vec4dArray> pBorderLineColor = new osg::Vec4dArray;
				osg::ref_ptr<osg::LineWidth> pLineWidth = new osg::LineWidth(2.0);

				pBorderLineGeom->setVertexArray(pWaterBorderCoord.get());
				pBorderLineGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, pWaterBorderCoord->size()));

				pBorderLineColor->push_back(osg::Vec4d(45.0/255.0, 187.0/255.0, 159.0/255.0, 1.0));
				pBorderLineGeom->setColorArray(pBorderLineColor.get());
				pBorderLineGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

				pBorderLineGeode->addDrawable(pBorderLineGeom.get());
				pBorderLineGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
				pBorderLineGeode->getOrCreateStateSet()->setAttribute(pLineWidth.get());

				m_pWaterBorderRoot->addChild(pBorderLineGeode.get());
			}
			
			//绘制边界面
			{
				osg::ref_ptr<osg::Geode> pWaterBorderGeode = new osg::Geode;
				osg::ref_ptr<osg::Geometry> pWaterBorderGeometry = new osg::Geometry;
				osg::ref_ptr<osg::Vec4dArray> pColor = new osg::Vec4dArray;

				if (pWaterBorderCoord->size() > 1)
				{
					pWaterBorderCoord->push_back(pWaterBorderCoord->at(0));
					pWaterBorderCoord->push_back(pWaterBorderCoord->at(1));
				}
				pWaterBorderGeometry->setVertexArray(pWaterBorderCoord.get());
				pWaterBorderGeometry->addPrimitiveSet(new osg::DrawArrays(GL_QUAD_STRIP, 0, pWaterBorderCoord->size()));

				pColor->push_back(osg::Vec4d(0.0/255.0, 0.0/255.0, 200.0/255.0, 0.75));
				pWaterBorderGeometry->setColorArray(pColor.get());
				pWaterBorderGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

				pWaterBorderGeode->addDrawable(pWaterBorderGeometry);
				pWaterBorderGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
				pWaterBorderGeode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
				m_pWaterBorderRoot->addChild(pWaterBorderGeode.get());
				m_pWaterBorderRoot->getOrCreateStateSet()->setRenderBinDetails(1, "RenderBin");
			}
		}


		osg::ref_ptr<osg::Geode> pWaterGeode = new osg::Geode;
		osg::ref_ptr<osg::Geometry> pWaterGeometry = new osg::Geometry;
		pWaterGeometry->setVertexArray(pWaterCoord);
		pWaterGeometry->addPrimitiveSet(new osg::DrawArrays(GL_POLYGON, 0, pWaterCoord->size()));

		osg::ref_ptr<osgUtil::Tessellator> tscx = new osgUtil::Tessellator();
		tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
		tscx->setBoundaryOnly(false);
		tscx->setWindingType(osgUtil::Tessellator::TESS_WINDING_ODD);
		tscx->setWindingType(osgUtil::Tessellator::TESS_WINDING_POSITIVE);
		tscx->retessellatePolygons(*pWaterGeometry);

		pWaterGeode->addDrawable(pWaterGeometry);
				

		//状态控制
		//osg::StateSet* stateSetGeometry = pWaterGeometry->getOrCreateStateSet();
		//stateSetGeometry->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
		//stateSetGeometry->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

		osg::StateSet* stateSetGeometry = pWaterGeometry->getOrCreateStateSet();
		//stateSetGeometry->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
		//stateSetGeometry->setMode(GL_LIGHTING, osg::StateAttribute::ON);
		
		//设置材质
		//osg::Material* material = new osg::Material;
		//material->setDataVariance(osg::Object::STATIC);
		//material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(0.588235,0.588235,0.588235,0.38));
		//material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(1,1,1,0.38));
		//material->setShininess(osg::Material::FRONT_AND_BACK, 0.0);
		//material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
		//material->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,0.38));
		//material->setColorMode(osg::Material::OFF);
		//stateSetGeometry->setAttribute(material,osg::StateAttribute::ON);

		//设置颜色混合
		//osg::BlendFunc* blendFunc = new osg::BlendFunc;
		//blendFunc->setSource(osg::BlendFunc::SRC_ALPHA);
		//blendFunc->setDestination(osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
		//stateSetGeometry->setAttributeAndModes(blendFunc,osg::StateAttribute::ON| osg::StateAttribute::OVERRIDE);

		//设置2d纹理贴图
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setDataVariance(osg::Object::STATIC);
		texture->setImage(osgDB::readImageFile(FeFileReg->GetFullPath("texture/river/wave.tga")));
		texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
		texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );
		texture->setWrap( osg::Texture2D::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
		texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
		texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
		texture->setMaxAnisotropy(1.0);
		texture->setBorderColor(osg::Vec4d(0.0, 0.0, 0.0, 0.0));
		texture->setBorderWidth(0);
		texture->setResizeNonPowerOfTwoHint(true);
		texture->setUseHardwareMipMapGeneration(false);
		stateSetGeometry->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON); 
		osg::TexMat* texmat = new osg::TexMat(osg::Matrixd::identity());
		stateSetGeometry->setTextureAttribute(0, texmat);


		osg::TextureCubeMap *tcm = new osg::TextureCubeMap;
		tcm->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		tcm->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		tcm->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
		tcm->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);  
		tcm->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);  

		tcm->setImage(osg::TextureCubeMap::POSITIVE_X, osgDB::readImageFile(FeFileReg->GetFullPath("texture/river/p_x_grijs.tga")));
		tcm->setImage(osg::TextureCubeMap::NEGATIVE_X, osgDB::readImageFile(FeFileReg->GetFullPath("texture/river/n_x_grijs.tga")));
		tcm->setImage(osg::TextureCubeMap::POSITIVE_Y, osgDB::readImageFile(FeFileReg->GetFullPath("texture/river/p_y_grijs.tga")));
		tcm->setImage(osg::TextureCubeMap::NEGATIVE_Y, osgDB::readImageFile(FeFileReg->GetFullPath("texture/river/n_y_grijs.tga")));
		tcm->setImage(osg::TextureCubeMap::POSITIVE_Z, osgDB::readImageFile(FeFileReg->GetFullPath("texture/river/p_z_grijs.tga")));
		tcm->setImage(osg::TextureCubeMap::NEGATIVE_Z, osgDB::readImageFile(FeFileReg->GetFullPath("texture/river/n_z_grijs.tga")));
		tcm->setMaxAnisotropy(1.0);
		tcm->setBorderColor(osg::Vec4d(0.0, 0.0, 0.0, 0.0));
		tcm->setBorderWidth(0);
		tcm->setResizeNonPowerOfTwoHint(true);

		stateSetGeometry->setTextureAttributeAndModes(1, tcm, osg::StateAttribute::ON); //
		stateSetGeometry->setTextureMode(1, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
		stateSetGeometry->setTextureMode(1, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
		stateSetGeometry->setTextureMode(1, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
		stateSetGeometry->setTextureMode(1, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON);

		osg::TexGen *tg = new osg::TexGen;
		tg->setMode(osg::TexGen::REFLECTION_MAP);
		stateSetGeometry->setTextureAttribute(1, tg, osg::StateAttribute::ON); //

		//textureUnit 2
		osg::Texture2D* texture2 = new osg::Texture2D;
		texture2->setDataVariance(osg::Object::STATIC);

		osg::Image* image = osgDB::readImageFile(FeFileReg->GetFullPath("texture/river/hedao.png"));

		texture2->setImage(image);
		texture2->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE );
		texture2->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
		texture2->setWrap( osg::Texture2D::WRAP_R, osg::Texture2D::CLAMP_TO_EDGE);
		texture2->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
		texture2->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
		texture2->setMaxAnisotropy(0.0);
		texture2->setBorderColor(osg::Vec4d(0.0, 0.0, 0.0, 0.0));
		texture2->setBorderWidth(0);
		texture2->setResizeNonPowerOfTwoHint(false);
		texture2->setUseHardwareMipMapGeneration(false);
		stateSetGeometry->setTextureAttributeAndModes(2,texture2,osg::StateAttribute::ON); //
		osg::TexMat* texmat2 = new osg::TexMat(osg::Matrixd::identity());
		stateSetGeometry->setTextureAttribute(2, texmat2);

		//pWaterGeometry->setUseDisplayList(true);
		//pWaterGeometry->setUseVertexBufferObjects(false);

		osg::Vec3Array* normal = new osg::Vec3Array;
		osg::Vec2Array* coord0 = new osg::Vec2Array;
		osg::Vec2Array* coord1 = new osg::Vec2Array;
		osg::Vec2Array* coord2 = new osg::Vec2Array;

		double minX=DBL_MAX, minY=DBL_MAX, maxX=-DBL_MAX, maxY=-DBL_MAX;

		osg::ref_ptr<osg::Vec3Array> _pVertex = dynamic_cast<osg::Vec3Array*>(pWaterGeometry->getVertexArray()); 
		osg::Matrixd matrixd;
		osg::Vec3d position(0,0,0);
		FeUtil::XYZ2Matrix(m_opRenderContext.get(),position, matrixd);

		osg::Matrixd InverseMatrix = osg::Matrixd::inverse(matrixd);
		osg::ref_ptr<osg::Vec3dArray> _pVertexExact = new osg::Vec3dArray;
		for(osg::Vec3Array::iterator iter = _pVertex->begin(); iter!= _pVertex->end(); iter++)
		{
			osg::Vec3 pos = (osg::Matrix::translate(*iter + position)*InverseMatrix).getTrans();//减小误差 
			_pVertexExact->push_back(pos);
			if(minX > pos.x())
			{
				minX = pos.x();
			}
			if(minY > pos.y())
			{
				minY = pos.y();
			}
			if(maxX < pos.x())
			{
				maxX = pos.x();
			}
			if(maxY < pos.y())
			{
				maxY = pos.y();
			}
		}

		//纹理坐标范围确定
		double xLength = maxX - minX;
		double yLength = maxY - minY;
		osg::Vec2Array* coord = new osg::Vec2Array;
		for(osg::Vec3dArray::iterator iter = _pVertexExact->begin(); iter != _pVertexExact->end(); iter++)
		{
			//判断与X距离
			double coordx =  (iter->x() - minX)/xLength;
			double coordy = (iter->y()-minY)/yLength;

			coord0->push_back((osg::Vec2d(coordx,coordy)));
			coord1->push_back((osg::Vec2d(coordx,coordy)));
			coord2->push_back((osg::Vec2d(coordx,coordy)));
			normal->push_back(osg::Vec3d(0.0,0.0,1.0));
		}		

		pWaterGeometry->setNormalArray(normal);
		pWaterGeometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

		pWaterGeometry->setTexCoordArray(0, coord0);
		pWaterGeometry->setTexCoordArray(1, coord1);
		pWaterGeometry->setTexCoordArray(2, coord2);

		//设置shader
		osg::StateSet* stateSetPmt = pWaterGeode->getOrCreateStateSet();
		osg::Program *program = new osg::Program;
		stateSetPmt->setAttributeAndModes(program, osg::StateAttribute::ON);
		program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShadersource));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShader));
		stateSetPmt->addUniform(new osg::Uniform("Normal0Map", 0));
		stateSetPmt->addUniform(new osg::Uniform("cubeMap", 1));
		stateSetPmt->addUniform(new osg::Uniform("colorMap", 3));
		stateSetPmt->addUniform(new osg::Uniform("flowMap", 2));

		//控制波长
		osg::Uniform* waveLength = new osg::Uniform("waveLength", 4.0f);
		//控制波速
		osg::Uniform* waveSpheed = new osg::Uniform("waveSpheed", 2.0f);

		waveLength->setUpdateCallback(new CFloatCallBack(4.0f));
		waveSpheed->setUpdateCallback(new CFloatCallBack(2.0f));

		stateSetPmt->addUniform(waveLength);
		stateSetPmt->addUniform(waveSpheed);

		addChild(pWaterGeode);
	}

	double CFloodSurface::GetCurrentHeight()
	{
		return m_basicPosition.z();
	}

	void CFloodSurface::ShowBorder( bool bShow /*= true*/ )
	{
		if(bShow)
		{
			m_pWaterBorderRoot->setNodeMask(~0x00000000);
		}
		else
		{
			m_pWaterBorderRoot->setNodeMask(0x00000000);
		}
	}

	bool CFloodSurface::IsShowBorder()
	{
		if (m_pWaterBorderRoot->getNodeMask())
		{
			return true;
		}
		
		return false;
	}

}
