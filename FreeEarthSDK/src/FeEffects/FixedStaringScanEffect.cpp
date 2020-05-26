#include <FeEffects/FixedStaringScanEffect.h>
#include <FeUtils/CoordConverter.h>

namespace FeEffect
{

	static const char *vertSource = 
	{
		"varying vec4 vertexPos;\n"
		"void main(void)\n"
		"{\n"
		"    vertexPos = gl_Vertex;\n"
		"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"}\n"
	};

	static const char *fragSource =
	{
		"#define MAXNUMWAVE 10\n"
		"uniform float u_length;\n"
		"uniform float u_startlength[MAXNUMWAVE];\n"
		"uniform float u_waveWidth;\n"
		"uniform vec4 u_baseColor;\n"
		"uniform vec4 u_waveColor;\n"
		"uniform int  u_numWave;\n"
		"uniform bool u_isSmooth;\n"
		"varying vec4 vertexPos;\n"
		"void main(void)\n"
		"{\n"
		"float waveAlpha = 0.0;\n"
		"for(int i = 0; i < u_numWave; i++)\n"
		"{\n"
		"    float curLength = length(vertexPos.xyz);\n"
		"    float curWidth = u_waveWidth;\n"
		"    float endLength = u_startlength[i] + curWidth;\n"
		"    float startLength = u_startlength[i];\n"
		"    if(curLength > startLength && curLength < endLength)\n"
		"    {\n"
		"        if(u_isSmooth)\n"
		"        {\n"
		"           float halfWidth = curWidth / 2.0; \n"
		"           float centerLength = startLength + halfWidth; \n"
		"           float disToCenter = abs(curLength - centerLength); \n"
		"           float maxAlpha = 1.0; \n"
		"           float minAlpha = u_baseColor.a;\n"
		"           waveAlpha = (minAlpha - maxAlpha) * disToCenter / halfWidth + maxAlpha; \n"
		"        }\n"
		"        else\n"     
		"        {\n"
		"           waveAlpha = 1.0; \n"
		"        }\n"
		"        break; \n"
		"    }\n"
		"}\n"

		"if(waveAlpha > 0.0)\n"
		"{\n"
		"gl_FragColor = vec4(u_waveColor.xyz,waveAlpha);\n"
		"}\n"
		"else\n"
		"{\n"
		"gl_FragColor = u_baseColor;\n"
		"}\n"
		// " gl_FragColor = vec4(waveAlpha,0.0,0.0,1.0);\n"
		"}\n"
	};

	CFixedStaringScanEffect::CFixedStaringScanEffect( float dRadiu)
		:m_radius(dRadiu)
		,m_xAngle(dRadiu)
		,m_yAngle(dRadiu)
		,m_opGeode(NULL)
		,m_opOutLine(NULL)
		,m_opLeftFace(NULL)
		,m_opRightFace(NULL)
		,m_opDownFace(NULL)
		,m_opUpFace(NULL)
		,m_opFrontFace(NULL)
		,m_numVert(0)
		,m_edgeLength(0)
		,m_waveWidth(0)
		,m_baseColor(1.0,1.0,1.0,0.2)
		,m_waveColor(0.0,1.0,0.0,1.0)
		,m_numWave(10)
		,m_isSmooth(true)
		,m_waveStartOffset(0)
		,m_widthBetweenWave(0)
		,m_moveSpeed(0)
	{
		
		CreateEffect();
		
	}

	CFixedStaringScanEffect::CFixedStaringScanEffect( FeUtil::CRenderContext* pRenderContext,osg::MatrixTransform *pMT )
		:CFreeEffect()
		,m_radius(1)
		,m_xAngle(1)
		,m_yAngle(1)
		,m_opGeode(NULL)
		,m_opOutLine(NULL)
		,m_opLeftFace(NULL)
		,m_opRightFace(NULL)
		,m_opDownFace(NULL)
		,m_opUpFace(NULL)
		,m_opFrontFace(NULL)
		,m_numVert(0)
		,m_edgeLength(0)
		,m_waveWidth(0)
		,m_baseColor(0.0,1.0,0.0,0.2)
		,m_waveColor(0.0,1.0,0.0,1.0)
		,m_numWave(10)
		,m_isSmooth(true)
		,m_waveStartOffset(0)
		,m_widthBetweenWave(0)
		,m_moveSpeed(0)
		,m_opMT(pMT)
		,m_opRenderContext(pRenderContext)
	{
		CreateEffect();
	}

	bool CFixedStaringScanEffect::CreateEffect()
	{
		m_rpMT = new osg::MatrixTransform;
		addChild(m_rpMT);
		UpdateEffect();	
		return true;
	}

	bool CFixedStaringScanEffect::UpdateEffect()
	{
		initPram();
		createNode();
		m_rpCullCallback = new PyramidNodeCallback();
		m_rpMT->addCullCallback(m_rpCullCallback);

		m_rpUpdateLenCallback = new CUpdateLenCallback(m_opRenderContext.get(),m_opMT.get(),this);
		m_rpMT->addCullCallback(m_rpUpdateLenCallback);

		return true;
	}

	void CFixedStaringScanEffect::SetRotate( osg::Vec3d posture )
	{
		 m_rpMT->setMatrix(osg::Matrix::rotate(osg::inDegrees(posture.x()),osg::X_AXIS, osg::inDegrees(posture.y()), osg::Y_AXIS,osg::inDegrees(posture.z()), osg::Z_AXIS));
	}

	void CFixedStaringScanEffect::SetRotate( osg::Matrix mat )
	{
		m_rpMT->setMatrix(mat);
	}

	void CFixedStaringScanEffect::initPram()
	{
		m_waveWidth = m_radius * 0.05;
		m_widthBetweenWave = m_waveWidth * 5.0;
		m_moveSpeed = m_radius / 5.0;

		for (int i = 0; i < m_numWave; i++)
		{
			m_startLength[i] = m_waveStartOffset + i * (m_waveWidth + m_widthBetweenWave);
		}
	}

	void CFixedStaringScanEffect::createNode()
	{
		int binNum = 11;

		m_opLeftFace = new osg::Geometry();
		m_opLeftFace->setDataVariance(osg::Object::DYNAMIC);
		m_opLeftFace->setUseDisplayList(false);
		m_opLeftFace->setUseVertexBufferObjects(true);
		m_opLeftFace->getOrCreateStateSet()->setRenderBinDetails(binNum,"DepthSortedBin");

		m_opRightFace = new osg::Geometry();
		m_opRightFace->setDataVariance(osg::Object::DYNAMIC);
		m_opRightFace->setUseDisplayList(false);
		m_opRightFace->setUseVertexBufferObjects(true);
		m_opRightFace->getOrCreateStateSet()->setRenderBinDetails(binNum,"DepthSortedBin");

		m_opDownFace = new osg::Geometry();
		m_opDownFace->setDataVariance(osg::Object::DYNAMIC);
		m_opDownFace->setUseDisplayList(false);
		m_opDownFace->setUseVertexBufferObjects(true);
		m_opDownFace->getOrCreateStateSet()->setRenderBinDetails(binNum,"DepthSortedBin");

		m_opUpFace = new osg::Geometry();
		m_opUpFace->setDataVariance(osg::Object::DYNAMIC);
		m_opUpFace->setUseDisplayList(false);
		m_opUpFace->setUseVertexBufferObjects(true);
		m_opUpFace->getOrCreateStateSet()->setRenderBinDetails(binNum,"DepthSortedBin");

		m_opFrontFace = new osg::Geometry();
		m_opFrontFace->setDataVariance(osg::Object::DYNAMIC);
		m_opFrontFace->setUseDisplayList(false);
		m_opFrontFace->setUseVertexBufferObjects(true);
		m_opFrontFace->getOrCreateStateSet()->setRenderBinDetails(binNum,"DepthSortedBin");

		m_opOutLine = new osg::Geometry();
		m_opOutLine->setDataVariance(osg::Object::DYNAMIC);
		m_opOutLine->setUseDisplayList(false);
		m_opOutLine->setUseVertexBufferObjects(true);

		osg::Vec3 center(0,0,0);

		float height = m_radius * std::tan(m_yAngle / 2.0);
		float width = m_radius * std::tan(m_xAngle / 2.0);

		osg::Vec3 leftUp(-100000,100000,-m_radius - 2000000);
		osg::Vec3 rightUp(100000,100000,-m_radius- 2000000);
		osg::Vec3 rightDown(100000,-100000,-m_radius- 2000000);
		osg::Vec3 leftDown(-100000,-100000,-m_radius- 2000000);

		osg::Vec3Array* leftVa = new osg::Vec3Array();
		osg::DrawElementsUByte* leftIa = new osg::DrawElementsUByte(osg::PrimitiveSet::TRIANGLES);
		leftVa->push_back(center);
		leftVa->push_back(leftDown);
		leftVa->push_back(leftUp);
		leftIa->push_back(0);
		leftIa->push_back(1);
		leftIa->push_back(2);
		m_opLeftFace->setVertexArray(leftVa);
		m_opLeftFace->addPrimitiveSet(leftIa);

		osg::Vec3Array* rightVa = new osg::Vec3Array();
		osg::DrawElementsUByte* rightIa = new osg::DrawElementsUByte(osg::PrimitiveSet::TRIANGLES);
		rightVa->push_back(center);
		rightVa->push_back(rightUp);
		rightVa->push_back(rightDown);
		rightIa->push_back(0);
		rightIa->push_back(1);
		rightIa->push_back(2);
		m_opRightFace->setVertexArray(rightVa);
		m_opRightFace->addPrimitiveSet(rightIa);

		osg::Vec3Array* upVa = new osg::Vec3Array();
		osg::DrawElementsUByte* upIa = new osg::DrawElementsUByte(osg::PrimitiveSet::TRIANGLES);
		upVa->push_back(center);
		upVa->push_back(leftUp);
		upVa->push_back(rightUp);
		upIa->push_back(0);
		upIa->push_back(1);
		upIa->push_back(2);
		m_opUpFace->setVertexArray(upVa);
		m_opUpFace->addPrimitiveSet(upIa);

		osg::Vec3Array* downVa = new osg::Vec3Array();
		osg::DrawElementsUByte* downIa = new osg::DrawElementsUByte(osg::PrimitiveSet::TRIANGLES);
		downVa->push_back(center);
		downVa->push_back(leftDown);
		downVa->push_back(rightDown);
		downIa->push_back(0);
		downIa->push_back(1);
		downIa->push_back(2);
		m_opDownFace->setVertexArray(downVa);
		m_opDownFace->addPrimitiveSet(downIa);

		osg::Vec3Array* frontVa = new osg::Vec3Array();
		osg::DrawElementsUByte* frontIa = new osg::DrawElementsUByte(osg::PrimitiveSet::QUADS);
		frontVa->push_back(leftUp);
		frontVa->push_back(rightUp);
		frontVa->push_back(rightDown);
		frontVa->push_back(leftDown);
		m_opFrontFace->setVertexArray(frontVa);
		m_opFrontFace->addPrimitiveSet(frontIa);

		osg::Vec3Array* lineVa = new osg::Vec3Array();
		osg::DrawElementsUByte* lineIa = new osg::DrawElementsUByte(osg::PrimitiveSet::LINES);
		osg::Vec4Array* lineCa = new osg::Vec4Array();		
		lineVa->push_back(center);
		lineVa->push_back(leftUp);
		lineVa->push_back(rightUp);
		lineVa->push_back(rightDown);
		lineVa->push_back(leftDown);
		lineIa->push_back(0);
		lineIa->push_back(1);
		lineIa->push_back(0);
		lineIa->push_back(2);
		lineIa->push_back(0);
		lineIa->push_back(3);
		lineIa->push_back(0);
		lineIa->push_back(4);
		lineIa->push_back(1);
		lineIa->push_back(2);
		lineIa->push_back(2);
		lineIa->push_back(3);
		lineIa->push_back(3);
		lineIa->push_back(4);
		lineIa->push_back(4);
		lineIa->push_back(1);
		osg::Vec4 lineColor(m_baseColor.x(),m_baseColor.y(),m_baseColor.z(),0.6);
		lineCa->push_back(lineColor);
		m_opOutLine->setVertexArray(lineVa);
		m_opOutLine->addPrimitiveSet(lineIa);
		m_opOutLine->setColorArray(lineCa,osg::Array::BIND_OVERALL);
		m_opOutLine->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

		m_opGeode = new osg::Geode();
		m_opGeode->addDrawable(m_opLeftFace.get());
		m_opGeode->addDrawable(m_opRightFace.get());
		m_opGeode->addDrawable(m_opUpFace.get());
		m_opGeode->addDrawable(m_opDownFace.get());
		m_opGeode->addDrawable(m_opFrontFace.get());
		m_opGeode->addDrawable(m_opOutLine.get());

		m_opGeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		m_opGeode->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		osg::Program* prog = new osg::Program();
		prog->addShader(new osg::Shader(osg::Shader::VERTEX,vertSource));
		prog->addShader(new osg::Shader(osg::Shader::FRAGMENT,fragSource));
		m_opGeode->getOrCreateStateSet()->setAttributeAndModes(prog,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED |  osg::StateAttribute::OVERRIDE);


		m_edgeLength = leftDown.length();
		osg::Uniform* u_length = new osg::Uniform("u_length",m_edgeLength);
		m_opGeode->getOrCreateStateSet()->addUniform(u_length);
		osg::Uniform* u_waveWidth = new osg::Uniform("u_waveWidth",m_waveWidth);
		m_opGeode->getOrCreateStateSet()->addUniform(u_waveWidth);
		osg::Uniform* u_startlength = new osg::Uniform(osg::Uniform::FLOAT,"u_startlength",(int)MAXNUMWAVE);
		m_opGeode->getOrCreateStateSet()->addUniform(u_startlength);
		osg::Uniform* u_baseColor = new osg::Uniform("u_baseColor",m_baseColor);
		m_opGeode->getOrCreateStateSet()->addUniform(u_baseColor);
		osg::Uniform* u_waveColor = new osg::Uniform("u_waveColor",m_waveColor);
		m_opGeode->getOrCreateStateSet()->addUniform(u_waveColor);
		osg::Uniform* u_numWave = new osg::Uniform("u_numWave",m_numWave);
		m_opGeode->getOrCreateStateSet()->addUniform(u_numWave);
		osg::Uniform* u_isSmooth = new osg::Uniform("u_isSmooth",m_isSmooth);
		m_opGeode->getOrCreateStateSet()->addUniform(u_isSmooth);

		m_rpMT->addChild(m_opGeode.get());
	}

	void CFixedStaringScanEffect::updateUniform( osg::NodeVisitor* nv )
	{
		if(m_opGeode.valid())
		{
			osg::StateSet* ss = m_opGeode->getStateSet();
			if(ss)
			{
				float time = (float)std::fmod(nv->getFrameStamp()->getSimulationTime(),(double)FLT_MAX);
				osg::Uniform* u_startlength = ss->getUniform("u_startlength");
				if(u_startlength)
				{
					for (int i = 0; i < m_numWave; i++)
					{
						float curStartLength = fmod(m_startLength[i] + time * m_moveSpeed,m_edgeLength);
						u_startlength->setElement(i,curStartLength);
					}
				}

				osg::Uniform* u_baseColor = ss->getUniform("u_baseColor");
				if(u_baseColor)
				{
					u_baseColor->set(m_baseColor);
				}

				osg::Uniform* u_waveColor = ss->getUniform("u_waveColor");
				if(u_waveColor)
				{
					u_waveColor->set(m_waveColor);
				}

				osg::Uniform* u_numWave = ss->getUniform("u_numWave");
				if(u_numWave)
				{
					u_numWave->set(m_numWave);
				}

				osg::Uniform* u_waveWidth = ss->getUniform("u_waveWidth");
				if(u_waveWidth)
				{
					u_waveWidth->set(m_waveWidth);
				}

				osg::Uniform* u_isSmooth = ss->getUniform("u_isSmooth");
				if(u_isSmooth)
				{
					u_isSmooth->set(m_isSmooth);
				}

			}
		}	
	}

	void CFixedStaringScanEffect::updataLength(osg::Vec3 scrpos,osg::Vec3 targetPos)
	{
		osg::Vec3 lenghtvec = scrpos - targetPos;
		double  length = lenghtvec.length();
		length -= 100000;
		osg::Vec3 leftUp(-100000,100000,-length);
		osg::Vec3 rightUp(100000,100000,-length);
		osg::Vec3 rightDown(100000,-100000,-length);
		osg::Vec3 leftDown(-100000,-100000,-length);

		osg::Vec3 center(0,0,0);
		osg::Vec3Array* leftarray = dynamic_cast<osg::Vec3Array*>(m_opLeftFace->getVertexArray());
		leftarray->clear();
		leftarray->push_back(center);
		leftarray->push_back(leftDown);
		leftarray->push_back(leftUp);
		leftarray->dirty();
		m_opLeftFace->setVertexArray(leftarray);
		m_opLeftFace->dirtyDisplayList();
		m_opLeftFace->dirtyBound();

		osg::Vec3Array* rightarray = dynamic_cast<osg::Vec3Array*>(m_opRightFace->getVertexArray());
		rightarray->clear();
		rightarray->push_back(center);
		rightarray->push_back(rightDown);
		rightarray->push_back(rightUp);
		rightarray->dirty();
		m_opRightFace->setVertexArray(rightarray);
		m_opRightFace->dirtyDisplayList();
		m_opRightFace->dirtyBound();

		osg::Vec3Array* uparray = dynamic_cast<osg::Vec3Array*>(m_opUpFace->getVertexArray());
		uparray->clear();
		uparray->push_back(center);
		uparray->push_back(leftUp);
		uparray->push_back(rightUp);
		uparray->dirty();
		m_opUpFace->setVertexArray(uparray);
		m_opUpFace->dirtyDisplayList();
		m_opUpFace->dirtyBound();

		osg::Vec3Array* downarray = dynamic_cast<osg::Vec3Array*>(m_opDownFace->getVertexArray());
		downarray->clear();
		downarray->push_back(center);
		downarray->push_back(leftDown);
		downarray->push_back(rightDown);
		downarray->dirty();
		m_opDownFace->setVertexArray(downarray);
		m_opDownFace->dirtyDisplayList();
		m_opDownFace->dirtyBound();


		osg::Vec3Array* lineVa = dynamic_cast<osg::Vec3Array*>(m_opOutLine->getVertexArray());
		lineVa->clear();
		lineVa->push_back(center);
		lineVa->push_back(leftUp);
		lineVa->push_back(rightUp);
		lineVa->push_back(rightDown);
		lineVa->push_back(leftDown);
		lineVa->dirty();
		m_opOutLine->setVertexArray(lineVa);
		m_opOutLine->dirtyDisplayList();
		m_opOutLine->dirtyBound();
	
	}

	void CFixedStaringScanEffect::SetRadiu( float dRadiu )
	{
		m_radius = dRadiu;
		m_xAngle = dRadiu;
		m_yAngle = dRadiu;
	}


	void PyramidNodeCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if(node && nv)
		{
			osg::MatrixTransform* tn = dynamic_cast<osg::MatrixTransform*>(node);
			if(tn)
			{
				osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
				if(cv)
				{
					CFixedStaringScanEffect* nodeeffect = dynamic_cast<CFixedStaringScanEffect*>(tn->getParent(0));
					if(nodeeffect)
						nodeeffect->updateUniform(nv);
				}
			}
		}
		traverse(node,nv);
	}

	CUpdateLenCallback::CUpdateLenCallback(FeUtil::CRenderContext* pRenderContext,osg::MatrixTransform *pMT,CFixedStaringScanEffect* pFixedScan )
		:osg::NodeCallback()
		,m_opRenderContext(pRenderContext)
		,m_opMT(pMT)
		,m_opFixedScan(pFixedScan)
	{

	}

	void CUpdateLenCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if(m_opRenderContext.valid() && m_opMT.valid())
		{
			osg::Vec3d vecPosition = m_opMT->getMatrix().getTrans();
			osg::Vec3d vecLLH;
			osg::Vec3d vecNewPosition;


			FeUtil::XYZ2DegreeLLH(m_opRenderContext.get(),vecPosition,vecLLH);
			//FeUtil::DegreeLL2LLH(m_opRenderContext.get(),vecLLH);
			vecLLH.z() = 0;
			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),vecLLH,vecNewPosition);

			if(m_opFixedScan.valid())
			{
				m_opFixedScan->updataLength(vecPosition, vecNewPosition);
			}

		}
		traverse(node,nv);
	}

}
