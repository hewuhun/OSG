#include <FeEffects/Scan.h>

#include <osg/PrimitiveSet>
#include <osg/AnimationPath>
#include <osg/Depth>
#include <FeUtils/CoordConverter.h>
#include <osg/CullFace>
#include <osg/LineWidth>


namespace FeEffect
{

	CScan::CScan( osg::Vec3d pos ):
		osg::Group(),
		m_pos(pos),
		m_color(osg::Vec4d(0.3, 1.0, 1.0, 1.0))//(osg::Vec4(0.0, 1.0, 1.0, 1.0))
	{
		Init();
		this->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	}

	void CScan::Init()
	{
		this->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		this->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		osg::Depth* dp = new osg::Depth();
		dp->setWriteMask(false);
		this->getOrCreateStateSet()->setAttribute(dp, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		m_gnode = new osg::Geode;
		addChild(m_gnode);

		m_rpDrawCallback = new DrawableDrawCallback(m_pos);
    
		m_geom = new osg::Geometry;
		m_gnode->addDrawable(m_geom);
		m_geom->setDrawCallback(m_rpDrawCallback);
		m_geom->setUseDisplayList(false);
		m_geom->setUseVertexBufferObjects(true);

		//顶点序列
		osg::Vec3dArray* vertex = new osg::Vec3dArray;
		m_geom->setVertexArray(vertex);

		//颜色序列
		osg::Vec4dArray* colorArray = new osg::Vec4dArray;
		m_geom->setColorArray(colorArray);
		m_geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		//勾边,循环线
		osg::DrawElementsUInt* edge = new osg::DrawElementsUInt(GL_LINE_LOOP);
		//m_geom->addPrimitiveSet(edge);
		/// 不能使用索引构造边框线， 因为在绘制更新时清空了节点，导致其他地方遍历DrawElementsUInt时出错。（2017.4.12 g00034）
	

		//顶点设置，首先求出除卫星点，地球中心点之外的第三个点，该算法若看不懂需要杨总亲自解释
		osg::Vec3d thirdPos = osg::Matrix(osg::Matrix::translate(m_pos) * osg::Matrix::rotate(osg::inDegrees(50.0), osg::Vec3d(1.0, 0.0, 0.0))).getTrans() - m_pos;

		//勾边，三个点
		{
			vertex->push_back(m_pos);
			edge->push_back(0);
			colorArray->push_back(m_color);
			vertex->push_back(osg::Vec3d(0.0, 0.0, 0.0));
			edge->push_back(1);
			colorArray->push_back(m_color);
			vertex->push_back(thirdPos);
			edge->push_back(2);
			colorArray->push_back(m_color);

			m_geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_LOOP, 0, vertex->size()));
		}

		//开始中间分段，着色
		{
			osg::Vec3d center = osg::Vec3d(0.0, 0.0, 0.0);
			osg::Vec3d right = thirdPos;
			float deltaRadio = 0.01;
       

			//四个形状
			osg::Vec3d m0, m1, m2, m3;

			//起初m0和m2为起点
			m0 = m_pos;
			m2 = m_pos;

			float deltaAlpha = 0.01;
			float m02Alaph = 0.01;
			float m13Alpha = m02Alaph + deltaAlpha;

			for(float radio = deltaRadio; radio<1.0; radio+=deltaRadio)
			{
				//求出左中心
				m1 = m_pos + (center - m_pos) * radio;
				m3 = m_pos + (right - m_pos) * radio;

				m_geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, vertex->size(), 4));

				vertex->push_back(m0);
				colorArray->push_back(osg::Vec4d(m_color.r(), m_color.g(), m_color.b(), m02Alaph));

				vertex->push_back(m1);
				colorArray->push_back(osg::Vec4d(m_color.r(), m_color.g(), m_color.b(), m13Alpha));

				vertex->push_back(m2);
				colorArray->push_back(osg::Vec4d(m_color.r(), m_color.g(), m_color.b(), m02Alaph));

				vertex->push_back(m3);          
				colorArray->push_back(osg::Vec4d(m_color.r(), m_color.g(), m_color.b(), m13Alpha));

				m0 = m1;
				//float temp = m02Alaph;
				m02Alaph = m13Alpha;
				m2 = m3;
				m13Alpha += deltaAlpha;

				if(m13Alpha>0.3)
				{
					deltaAlpha = -0.04;
				}

				if(m13Alpha<0.01)
				{
					deltaAlpha = 0.01;
				}

				if(m13Alpha <0.0) m13Alpha = 0.0;
			}

		}
 
	}

	void CScan::SetPosition(osg::Vec3d vecPos)
	{
		m_pos = vecPos;
		if (m_rpDrawCallback.valid())
		{
			m_rpDrawCallback->SetPosition(m_pos);
		}
	}

	DrawableDrawCallback::DrawableDrawCallback( osg::Vec3d pos ):
	osg::Drawable::DrawCallback(),
		m_pos(pos),
		m_radius(0.0)
	{

	}

	void DrawableDrawCallback::drawImplementation( osg::RenderInfo& renderInfo,const osg::Drawable* drawable ) const
	{
		//两帧来一次
		static int frameTiaoGuo = 0;

		frameTiaoGuo ++;

		//if(frameTiaoGuo>2)
		{
			frameTiaoGuo = 0;

			osg::Geometry* m_geom = (osg::Geometry*)drawable;

			osg::Vec4dArray* m_colorArray = (osg::Vec4dArray*)m_geom->getColorArray();
			m_colorArray->dirty();

			//前三个点是外边，要跳过去
			unsigned int sizePoint = m_colorArray->size();
        
			sizePoint = sizePoint -3; //这样得到的是四个一对，四个一对的点
			int forZ = sizePoint/4;

			int baseColor = 3;
			//m1, m3要取m0, m2的颜色值
			//m0, m2要取下一个四个的m0, m2的颜色值，如果没有下一个，则m0, m2要取最开始的m1, m3的颜色值
			osg::Vec4d fromM13 = m_colorArray->at(baseColor + (forZ-1)*4 + 1);

			//还可以循环forZ次
			for(int i = forZ-1; i>=0; i--)
			{
				osg::Vec4d colorm02 = m_colorArray->at(baseColor + i*4);
				m_colorArray->at(baseColor + i*4+1).set(colorm02.r(), colorm02.g(), colorm02.b(), colorm02.a());
				m_colorArray->at(baseColor + i*4+3).set(colorm02.r(), colorm02.g(), colorm02.b(), colorm02.a());

				int iNext = i-1;
				osg::Vec4d colorNext02;

				if(iNext < 0) 
				{
					colorNext02 = fromM13;
				}
				else
				{
					colorNext02 = m_colorArray->at(baseColor + iNext*4);
				}

				m_colorArray->at(baseColor + i*4+0).set(colorNext02.r(), colorNext02.g(), colorNext02.b(), colorNext02.a());
				m_colorArray->at(baseColor + i*4+2).set(colorNext02.r(), colorNext02.g(), colorNext02.b(), colorNext02.a());

			}

			////////////////////////////
			//设置顶点
			osg::Vec3dArray* vertex = (osg::Vec3dArray*)m_geom->getVertexArray();
			vertex->clear();
			vertex->dirty();

			//顶点设置，首先求出除卫星点，地球中心点之外的第三个点，该算法若看不懂需要杨总亲自解释
			osg::Vec3d aix = m_pos;
			aix.normalize();

			osg::Vec3d thirdPos = osg::Matrixd(osg::Matrix::translate(m_pos) * osg::Matrix::rotate(osg::inDegrees(50.0), osg::Vec3d(1.0, 0.0, 0.0))* osg::Matrix::rotate(osg::inDegrees(m_radius), aix)).getTrans() - m_pos;
			m_radius+=1.0;

			//勾边，三个点
			{
				vertex->push_back(m_pos);
				vertex->push_back(osg::Vec3d(0.0, 0.0, 0.0));
				vertex->push_back(thirdPos);
			}

			//开始中间分段，着色
			{
				osg::Vec3d center = osg::Vec3d(0.0, 0.0, 0.0);
				osg::Vec3d right = thirdPos;
				float deltaRadio = 0.01;
        
				//四个形状
				osg::Vec3d m0, m1, m2, m3;

				//起初m0和m2为起点
				m0 = m_pos;
				m2 = m_pos;

				float deltaAlpha = 0.01;
				float m02Alaph = 0.01;
				float m13Alpha = m02Alaph + deltaAlpha;

				for(float radio = deltaRadio; radio<1.0; radio+=deltaRadio)
				{
					//求出左中心
					m1 = m_pos + (center - m_pos) * radio;
					m3 = m_pos + (right - m_pos) * radio;

					vertex->push_back(m0);
					vertex->push_back(m1);
					vertex->push_back(m2);
					vertex->push_back(m3); 

					m0 = m1;
					//float temp = m02Alaph;
					m02Alaph = m13Alpha;
					m2 = m3;
					m13Alpha += deltaAlpha;

					if(m13Alpha>0.3)
					{
						deltaAlpha = -0.04;
					}

					if(m13Alpha<0.01)
					{
						deltaAlpha = 0.01;
					}

					if(m13Alpha <0.0) m13Alpha = 0.0;
				}

			}
		}

		drawable->drawImplementation(renderInfo);
	}

	void DrawableDrawCallback::SetPosition(osg::Vec3d vecPos)
	{
		m_pos = vecPos;
	}

	void TestCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if(node && nv)
		{
			TestNode* tn = dynamic_cast<TestNode*>(node);
			if(tn)
			{
				osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
				if(cv)
				{
					osg::Camera* cam = cv->getCurrentCamera();
					if(cam)
					{
						osg::Vec3d eye,look,up;
						cam->getViewMatrix().getLookAt(eye,look,up);

					    osg::Vec3d lookDir = look - eye;
						tn->updateGeometry(lookDir);
						
					}
					tn->updateUniform(nv);
				}
			}
		}
		
		traverse(node,nv);
	}

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

	TestNode::TestNode(float length,float angle,osg::Vec3 up)
		:m_centerLength(length)
		,m_circleVertNum(100)
		,m_waveWidth(0.0)
		,m_updateCallback(NULL)
		,m_opFrontDrawable(NULL)
		,m_opBackDrawable(NULL)
		,m_angle(angle)
		,m_edgeLength(0.0)
		,m_moveSpeed(20000)
		,m_opGeode(NULL)
		,m_baseColor(1.0,1.0,1.0,0.1)
		,m_waveColor(0.0,1.0,0.0,1.0)
		,m_numWave(5)
		,m_widthBetweenWave(0.0)
		,m_isSmooth(true)
		,m_up(up)
		,m_waveStartOffset(0)
	{
		initPram();
		initNode();
	}

	void TestNode::initPram()
	{
		m_edgeLength = m_centerLength / std::cos(m_angle);
		m_waveWidth = m_edgeLength * 0.02;
		m_widthBetweenWave = m_waveWidth * 5.0;

		for (int i = 0; i < m_numWave; i++)
		{
			m_startLength[i] = m_waveStartOffset + i * (m_waveWidth + m_widthBetweenWave);
		}
	}

	void TestNode::initNode()
	{
		m_updateCallback = new TestCallback();
		addCullCallback(m_updateCallback);

		osg::Vec3 center(0,0,0);
		createGeode(center,m_up);

		addChild(m_opGeode.get());
	}

	void TestNode::createGeode( osg::Vec3 center,osg::Vec3 up )
	{
		m_opBackDrawable = new MyGeometry();
		m_opBackDrawable->setDataVariance(osg::Object::DYNAMIC);
		m_opBackDrawable->setUseDisplayList(false);
		m_opBackDrawable->setUseVertexBufferObjects(true);
		m_opBackDrawable->getOrCreateStateSet()->setRenderBinDetails(11,"DepthSortedBin");

		m_opFrontDrawable = new MyGeometry();
		m_opFrontDrawable->setDataVariance(osg::Object::DYNAMIC);
		m_opFrontDrawable->setUseDisplayList(false);
		m_opFrontDrawable->setUseVertexBufferObjects(true);
		m_opFrontDrawable->getOrCreateStateSet()->setRenderBinDetails(12,"DepthSortedBin");

		m_pointList.push_back(center);
		m_boundingBox.expandBy(center);

		float radius = m_centerLength * std::tan(m_angle);

		osg::Vec3 localUp(0,0,1);
		osg::Vec3 localForward(0,radius,0);
		float pAngle = osg::PI * 2.0 / (float)m_circleVertNum;	

		osg::Matrix rotateMat;
		rotateMat.makeIdentity();

		float dot = localUp * up;
		if(osg::equivalent(dot,1.0f) || osg::equivalent(dot,-1.0f))
		{
		}
		else
		{
			up.normalize();
			osg::Vec3 curCross = localUp ^ up;
			curCross.normalize();

			float dotAngle = std::acos(osg::clampBetween((float)(localUp * up),-1.0f,1.0f));

			osg::Quat q(dotAngle,curCross);
			rotateMat.setRotate(q);
		}	

		for (int i = 0; i < m_circleVertNum; i++)
		{
			float curAngle = pAngle * i;

			osg::Quat cq(curAngle,localUp);
			osg::Matrix cm(cq);

			cm = cm * rotateMat;

			osg::Vec3 curPoint = localForward * cm;
			curPoint += center;
			curPoint += up * m_centerLength;

			m_pointList.push_back(curPoint);
			m_boundingBox.expandBy(curPoint);
		}

		if(m_pointList.size() > 3)
		{
			m_pointList.push_back(m_pointList.at(1));
			m_boundingBox.expandBy(m_pointList.at(1));
		}

		m_opGeode = new osg::Geode();
		m_opGeode->addDrawable(m_opFrontDrawable.get());
		m_opGeode->addDrawable(m_opBackDrawable.get());

		m_opGeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		m_opGeode->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		m_opGeode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		osg::Program* prog = new osg::Program();
		prog->addShader(new osg::Shader(osg::Shader::VERTEX,vertSource));
		prog->addShader(new osg::Shader(osg::Shader::FRAGMENT,fragSource));
		m_opGeode->getOrCreateStateSet()->setAttributeAndModes(prog,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED |  osg::StateAttribute::OVERRIDE);

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
		
		
		m_opBackDrawable->setBoundBox(m_boundingBox);
		m_opBackDrawable->setVertexNum(m_circleVertNum * 3);
		m_opBackDrawable->setIndexNum(m_circleVertNum * 3);

		m_opFrontDrawable->setBoundBox(m_boundingBox);
		m_opFrontDrawable->setVertexNum(m_circleVertNum * 3);
		m_opFrontDrawable->setIndexNum(m_circleVertNum * 3);
	}

	void TestNode::updateGeometry(osg::Vec3d lookDir)
	{
		if(osg::equivalent((lookDir - m_lastLookDir).length2(),0.0))
			return;

		m_lastLookDir = lookDir;

		if(m_opBackDrawable.valid() && m_opFrontDrawable.valid() && !m_pointList.empty())
		{
			int nVerts = m_circleVertNum * 3;
			MeshVertex *bv = new MeshVertex[nVerts];
			GLuint *bi = new GLuint[nVerts];

			MeshVertex *fv = new MeshVertex[nVerts];
			GLuint *fi = new GLuint[nVerts];

			osg::MatrixList ml = m_opBackDrawable->getWorldMatrices();
			osg::Matrix mat;
			mat.makeIdentity();
			if(!ml.empty())
				mat = ml[0];

			unsigned int curBackSize;
			curBackSize = 0;
			unsigned int curFrontSize;
			curFrontSize = 0;

			int bIndex = 0;
		    int fIndex = 0;

			for (int i = 1; i < m_pointList.size() - 1; i++)
			{
				osg::Vec3d p0 = m_pointList[0] * mat;
				osg::Vec3d p1 = m_pointList[i] * mat;
				osg::Vec3d p2 = m_pointList[i + 1] * mat;

				osg::Vec3d dir0 = p1 - p0;
				dir0.normalize();

				osg::Vec3d dir1 = p2 - p1;
				dir1.normalize();

				osg::Vec3d norDir = dir0 ^ dir1;
				norDir.normalize();

				lookDir.normalize();
				double dotDir = lookDir * norDir;

				if(dotDir > 0)
				{
					bv[bIndex].x = m_pointList[0].x();
					bv[bIndex].y = m_pointList[0].y();
					bv[bIndex].z = m_pointList[0].z();
					bv[bIndex].w = 1.0;
					curBackSize++;
					bi[bIndex] = curBackSize - 1;
					bIndex++;

					bv[bIndex].x = m_pointList[i].x();
					bv[bIndex].y = m_pointList[i].y();
					bv[bIndex].z = m_pointList[i].z();
					bv[bIndex].w = 1.0;
					curBackSize++;
					bi[bIndex] = curBackSize - 1;
					bIndex++;

					bv[bIndex].x = m_pointList[i + 1].x();
					bv[bIndex].y = m_pointList[i + 1].y();
					bv[bIndex].z = m_pointList[i + 1].z();
					bv[bIndex].w = 1.0;
					curBackSize++;
					bi[bIndex] = curBackSize - 1;
					bIndex++;
				}
				else
				{
					fv[fIndex].x = m_pointList[0].x();
					fv[fIndex].y = m_pointList[0].y();
					fv[fIndex].z = m_pointList[0].z();
					fv[fIndex].w = 1.0;
					curFrontSize++;
					fi[fIndex] = curFrontSize - 1;
					fIndex++;

					fv[fIndex].x = m_pointList[i].x();
					fv[fIndex].y = m_pointList[i].y();
					fv[fIndex].z = m_pointList[i].z();
					fv[fIndex].w = 1.0;
					curFrontSize++;
					fi[fIndex] = curFrontSize - 1;
					fIndex++;

					fv[fIndex].x = m_pointList[i + 1].x();
					fv[fIndex].y = m_pointList[i + 1].y();
					fv[fIndex].z = m_pointList[i + 1].z();
					fv[fIndex].w = 1.0;
					curFrontSize++;
					fi[fIndex] = curFrontSize - 1;
					fIndex++;
				}
			}

			m_opBackDrawable->setNewBuffer((GLvoid*)bv,(GLvoid*)bi,NULL);
			m_opBackDrawable->setNumIndices(bIndex);
			m_opFrontDrawable->setNewBuffer((GLvoid*)fv,(GLvoid*)fi,NULL);
			m_opFrontDrawable->setNumIndices(fIndex);
		}
	}

	void TestNode::updateUniform(osg::NodeVisitor* nv)
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

	void TestNode::setNumWave( int num )
	{
		m_numWave = num;
		if(m_numWave > MAXNUMWAVE)
			m_numWave = MAXNUMWAVE;
	}

	void TestNode::setWaveColor( osg::Vec3 color )
	{
		m_waveColor.x() = color.x();
		m_waveColor.y() = color.y();
		m_waveColor.z() = color.z();
	}

	void TestNode::setBaseColor( osg::Vec4 color )
	{
		m_baseColor.x() = color.x();
		m_baseColor.y() = color.y();
		m_baseColor.z() = color.z();
		m_baseColor.a() = color.a();
	}

	void TestNode::setWaveWidth( float width )
	{
		m_waveWidth = width;
		for (int i = 0; i < m_numWave; i++)
		{
			m_startLength[i] = m_waveStartOffset + i * (m_waveWidth + m_widthBetweenWave);
		}
	}

	void TestNode::setWaveStartOffset( float offset )
	{
		m_waveStartOffset = offset;
		for (int i = 0; i < m_numWave; i++)
		{
			m_startLength[i] = m_waveStartOffset + i * (m_waveWidth + m_widthBetweenWave);
		}
	}

	void TestNode::setWidthBetweenWave( float width )
	{
		m_widthBetweenWave = width;
		for (int i = 0; i < m_numWave; i++)
		{
			m_startLength[i] = m_waveStartOffset + i * (m_waveWidth + m_widthBetweenWave);
		}
	}

	MyGeometry::MyGeometry()
		:m_meshVboID(0)
		,m_meshIdxID(0)
		,m_colorVboID(0)
		,m_curIndices(0)
		,m_needUpdateBuffer(false)
		,m_newVertexBuffer(NULL)
		,m_newIndexBuffer(NULL)
		,m_newColorBuffer(NULL)
		,m_pGLExtensions(NULL)
		,m_vertexNum(0)
		,m_indexNum(0)
	{
	}

	MyGeometry::~MyGeometry()
	{
		if (m_pGLExtensions != NULL && m_meshVboID != 0)
		{
			m_pGLExtensions->glDeleteBuffers(1, &m_meshVboID);
		}

		if (m_pGLExtensions != NULL && m_meshIdxID != 0) 
		{
			m_pGLExtensions->glDeleteBuffers(1, &m_meshIdxID);
		}

		if (m_pGLExtensions != NULL && m_colorVboID != 0) 
		{
			m_pGLExtensions->glDeleteBuffers(1, &m_colorVboID);
		}
	}

	void MyGeometry::drawImplementation( osg::RenderInfo& renderInfo ) const
	{
		m_mutex.lock();

		osg::State& state = *renderInfo.getState();
		m_pGLExtensions = state.get<osg::GLExtensions>();

		if(m_meshIdxID == 0 && m_meshVboID == 0)
		{
			createMesh(renderInfo);
		}

		if(m_needUpdateBuffer)
		{
			updateMesh(renderInfo);
			m_needUpdateBuffer = false;
		}

		pushAllState();

		if(m_pGLExtensions && m_curIndices > 0)
		{
			m_pGLExtensions->glBindBuffer(GL_ARRAY_BUFFER_ARB, m_meshVboID);
			m_pGLExtensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, m_meshIdxID);
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(4, GL_FLOAT, 0, 0);

			m_pGLExtensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,m_colorVboID);
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4,GL_FLOAT,0,0);

			glDrawElements(GL_TRIANGLES, m_curIndices, GL_UNSIGNED_INT, 0);
			//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

			m_pGLExtensions->glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
			m_pGLExtensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		}

		popAllState(renderInfo);

		m_mutex.unlock();
	}

	void MyGeometry::createMesh(osg::RenderInfo& renderInfo) const
	{
		if(m_pGLExtensions)
		{
			/*MeshVertex* vb = new MeshVertex[3];
			float r = 100000;
			vb[0].x = 0;
			vb[0].y = 0;
			vb[0].z = 0;
			vb[0].w = 1.0;

			vb[1].x = -r;
			vb[1].y = 0;
			vb[1].z = r;
			vb[1].w = 1.0;

			vb[2].x = r;
			vb[2].y = 0;
			vb[2].z = r;
			vb[2].w = 1.0;

			GLuint* ib = new GLuint[3];
			ib[0] = 0;
			ib[1] = 1;
			ib[2] = 2;

			MeshColor* cb = new MeshColor[3];
			cb[0].r = 1;
			cb[0].g = 0;
			cb[0].b = 0;

			cb[1].r = 1;
			cb[1].g = 0;
			cb[1].b = 0;

			cb[2].r = 1;
			cb[2].g = 0;
			cb[2].b = 0;*/

			m_pGLExtensions->glGenBuffers(1,&m_meshVboID);
			m_pGLExtensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,m_meshVboID);
			m_pGLExtensions->glBufferData(GL_ARRAY_BUFFER_ARB,m_vertexNum * sizeof(MeshVertex),NULL,GL_DYNAMIC_DRAW_ARB);

			m_pGLExtensions->glGenBuffers(1,&m_meshIdxID);
			m_pGLExtensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,m_meshIdxID);
			m_pGLExtensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,m_indexNum * sizeof(GLuint),NULL,GL_DYNAMIC_DRAW_ARB);

			m_pGLExtensions->glGenBuffers(1,&m_colorVboID);
			m_pGLExtensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,m_colorVboID);	
			m_pGLExtensions->glBufferData(GL_ARRAY_BUFFER_ARB,m_vertexNum * sizeof(MeshColor),NULL,GL_DYNAMIC_DRAW_ARB);


			m_pGLExtensions->glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
			m_pGLExtensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
			//m_pGLExtensions->glBindBuffer(GL_COLOR_ARRAY_BUFFER_BINDING_ARB, 0);

			/*m_pGLExtensions->glGenBuffers(1,&m_meshVboID);
			m_pGLExtensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,m_meshVboID);
			m_pGLExtensions->glBufferData(GL_ARRAY_BUFFER_ARB,m_vertexNum * sizeof(MeshVertex),NULL,GL_DYNAMIC_DRAW_ARB);

			m_pGLExtensions->glGenBuffers(1,&m_meshIdxID);
			m_pGLExtensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,m_meshIdxID);
			m_pGLExtensions->glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,m_indexNum * sizeof(GLuint),NULL,GL_DYNAMIC_DRAW_ARB);

			m_pGLExtensions->glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
			m_pGLExtensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);*/
		}
	}

	void MyGeometry::updateMesh( osg::RenderInfo& renderInfo) const
	{
		if(m_pGLExtensions && m_newIndexBuffer != NULL && m_newVertexBuffer != NULL)
		{
			m_pGLExtensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,m_meshVboID);
			m_pGLExtensions->glBufferSubData(GL_ARRAY_BUFFER_ARB,0,m_vertexNum * sizeof(MeshVertex),m_newVertexBuffer);

			if(m_newColorBuffer != NULL)
			{
				m_pGLExtensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,m_colorVboID);
				m_pGLExtensions->glBufferSubData(GL_ARRAY_BUFFER_ARB,0,m_vertexNum * sizeof(MeshColor),m_newColorBuffer);

				delete[] m_newColorBuffer;
				m_newColorBuffer = NULL;
			}

			m_pGLExtensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,m_meshIdxID);
			m_pGLExtensions->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER_ARB,0,m_indexNum * sizeof(GLuint),m_newIndexBuffer);

			m_pGLExtensions->glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
			m_pGLExtensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

			delete[] m_newVertexBuffer;
			m_newVertexBuffer = NULL;
			delete[] m_newIndexBuffer;
			m_newVertexBuffer = NULL;
		}		
	}

	void MyGeometry::pushAllState() const
	{
		GLint currentProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
		m_programStack.push(currentProgram);

		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
		glPushAttrib(GL_ALL_ATTRIB_BITS);
	}

	void MyGeometry::popAllState(osg::RenderInfo& renderInfo) const
	{
		if (!m_programStack.empty()) 
		{
			osg::State& state = *renderInfo.getState();
			const osg::GLExtensions* extensions = state.get<osg::GLExtensions>();
			if(extensions)
			{
				GLint savedProgram = m_programStack.top();
				extensions->glUseProgram(savedProgram);
				m_programStack.pop();
			}		
		}

		glPopAttrib();
		glPopClientAttrib();
	}

	void MyGeometry::setNewBuffer( GLvoid* vData,GLvoid* iData, GLvoid* cData)
	{
		m_newVertexBuffer = vData;
		m_newIndexBuffer = iData;
		m_newColorBuffer = cData;
		m_needUpdateBuffer = true;
	}

	void MyGeometry::setBoundBox( osg::BoundingBox box )
	{
		setInitialBound(box);
	}

	void MyGeometry::setNumIndices( unsigned int num )
	{
		m_curIndices = num;
	}

	static const char *screenVertSource = 
	{
		"varying vec4 vertexPos;\n"
		"void main(void)\n"
		"{\n"
		"    vertexPos = gl_Vertex;\n"
		"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"}\n"
	};

	static const char *screenFragSource =
	{
		"varying vec4 vertexPos;\n"
		"uniform vec4 u_backColor;\n"
		"void main(void)\n"
		"{\n"
		"gl_FragColor = u_backColor;\n"
		"}\n"

	};

	SearchScreen::SearchScreen(FeUtil::CRenderContext* ct,const SearchScreenPointList& pl,const osg::Vec3& sPos)
		:m_rpGeometry(NULL)
		,m_backColor(0.0,0.6,0.0,0.2)
		,m_searchColor(1.0,1.0,1.0,1.0)
		,m_outLineColor(0.0,1.0,0.0)
		,m_rpUpdateCallback(NULL)
		,m_pointList(pl)
		,m_spherePos(sPos)
		,m_renderContext(ct)
		,m_rpOutLine(NULL)
		,m_edgeLength(0.0)
		,m_searchRate(0.5)
		,m_rpSearcher(NULL)
		,m_searcherStartLength(0)
		,m_moveSpeed(500000)
	{
		initPatam();
		createGeometry();
	}

	SearchScreen::~SearchScreen()
	{
		m_pointList.clear();
		m_pointList.resize(0);

		m_screenLengthMap.clear();

		m_worldPosList.clear();
		m_worldPosList.resize(0);
	}

	void SearchScreen::initPatam()
	{
		m_rpUpdateCallback = new SearchScreenCallback();
		addUpdateCallback(m_rpUpdateCallback);
	}

	void SearchScreen::createGeometry()
	{
		if(m_pointList.size() < 2 && m_renderContext.valid())
			return;

		m_rpGeometry = new osg::Geometry();
		osg::Vec3Array* va = new osg::Vec3Array();
		osg::DrawElementsUInt* ia = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);

		m_rpOutLine = new osg::Geometry();
		osg::Vec3Array* la = new osg::Vec3Array();
		osg::DrawElementsUShort* li = new osg::DrawElementsUShort(osg::PrimitiveSet::LINES);
		osg::Vec4Array* lc = new osg::Vec4Array();
		lc->push_back(osg::Vec4(m_outLineColor,1.0));

		m_rpSearcher = new MyGeometry();
		m_rpSearcher->setDataVariance(osg::Object::DYNAMIC);
		m_rpSearcher->setUseDisplayList(false);
		m_rpSearcher->setUseVertexBufferObjects(true);

		osg::BoundingBox bb;

		osg::Vec3d wPos;
		FeUtil::DegreeLLH2XYZ(m_renderContext.get(),m_spherePos,wPos);

	    osg::Vec3 up = wPos;
		up.normalize();

		osg::Vec3 localUp(0,0,1);
		if(osg::equivalent(abs(localUp * up),1.0f))
			localUp = osg::Vec3(1,0,0);

		osg::Vec3 right = localUp ^ up;
		right.normalize();

		osg::Vec3 north = up ^ right;
		north.normalize();

		osg::Vec3 center(0,0,0);
		va->push_back(center);
		la->push_back(center);
		
		bb.expandBy(center);

		osg::Vec3 lastPos;
		for (int i = 0; i < m_pointList.size(); i++)
		{
			osg::Vec3 curPos = north;
			osg::Vec3 curRight = right;
			osg::Matrix mat0 = osg::Matrix::rotate(m_pointList[i].a,up);
			curPos = curPos * mat0;
			curPos.normalize();
			curRight = curRight * mat0;
			curRight.normalize();

			osg::Matrix mat1 = osg::Matrix::rotate(m_pointList[i].e,curRight);
			curPos = curPos * mat1;
			curPos.normalize();
			
			curPos = curPos * m_pointList[i].r;

			va->push_back(curPos);
			la->push_back(curPos);

			bb.expandBy(curPos);

			if(i > 0)
			{
				float addLength = (curPos - lastPos).length();
				m_edgeLength += addLength;
			}
			lastPos = curPos;

			m_screenLengthMap.insert(ScreenLengthMap::value_type(i,m_edgeLength));

			m_worldPosList.push_back(curPos);
		}

		la->push_back(osg::Vec3(0,0,0));

		for (int i = 0; i < m_pointList.size() - 1; i++)
		{
			ia->push_back(0);
			ia->push_back(i + 1);
			ia->push_back(i + 2);
		}

		for (int i = 0; i < la->asVector().size() - 1; i++)
		{
			li->push_back(i);
			li->push_back(i + 1);
		}

		m_rpOutLine->setVertexArray(la);
		m_rpOutLine->addPrimitiveSet(li);
		m_rpOutLine->setColorArray(lc,osg::Array::BIND_OVERALL);
		m_rpOutLine->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

		m_rpSearcher->setBoundBox(bb);
		m_rpSearcher->setVertexNum(va->asVector().size());
		m_rpSearcher->setIndexNum(ia->asVector().size());

		m_rpGeometry->setVertexArray(va);
		m_rpGeometry->addPrimitiveSet(ia);

		m_rpGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
		m_rpGeometry->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		m_rpGeometry->getOrCreateStateSet()->setRenderBinDetails(11,"DepthSortedBin");

		osg::Program* prog = new osg::Program();
		prog->addShader(new osg::Shader(osg::Shader::VERTEX,screenVertSource));
		prog->addShader(new osg::Shader(osg::Shader::FRAGMENT,screenFragSource));
		m_rpGeometry->getOrCreateStateSet()->setAttributeAndModes(prog,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

		osg::Uniform* u_backColor = new osg::Uniform("u_backColor",m_backColor);
		m_rpGeometry->getOrCreateStateSet()->addUniform(u_backColor);

		m_rpSearcher->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
		m_rpSearcher->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		m_rpSearcher->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

		m_rpSearcher->getOrCreateStateSet()->setRenderBinDetails(12,"DepthSortedBin");

		osg::Geode* gn = new osg::Geode();
		gn->addChild(m_rpGeometry.get());
		gn->addChild(m_rpOutLine.get());
		gn->addChild(m_rpSearcher.get());

		osg::MatrixTransform* mt = new osg::MatrixTransform();
		osg::Matrix transMat;
		transMat.setTrans(wPos);
		mt->setMatrix(transMat);
		mt->addChild(gn);

		addChild(mt);
	}

	void SearchScreen::updateUniform(osg::NodeVisitor* nv)
	{
		if(m_rpGeometry.valid() && nv)
		{
			float time = (float)std::fmod(nv->getFrameStamp()->getSimulationTime(),(double)FLT_MAX);
			m_searcherStartLength = fmod(time * m_moveSpeed,m_edgeLength);		
		}
	}

	void SearchScreen::updateSearcher()
	{
		if(m_rpSearcher.valid() && !m_worldPosList.empty())
		{
			int numVert = m_rpSearcher->getVertexNum();
			int numIndex = m_rpSearcher->getIndexNum();

			MeshVertex* mv = new MeshVertex[numVert];
			GLuint*     mi = new GLuint[numIndex];
			MeshColor*  mc = new MeshColor[numVert];

			int curVertSize = 0;
			int curIndexSize = 0;

			osg::Vec3 center(0,0,0);
			mv[curVertSize].x = center.x();
			mv[curVertSize].y = center.y();
			mv[curVertSize].z = center.z();
			mv[curVertSize].w = 1.0;

			mc[curVertSize].r = m_searchColor.x();
			mc[curVertSize].g = m_searchColor.y();
			mc[curVertSize].b = m_searchColor.z();
			mc[curVertSize].a = 0.0;
			curVertSize++;

			float endLength = m_searcherStartLength;
			float startLength = m_searcherStartLength - m_edgeLength * m_searchRate;

			int startIndex = 0;
			int endIndex = 0;

			startLength = osg::clampBetween(startLength,0.0f,m_edgeLength);
			endLength = osg::clampBetween(endLength,0.0f,m_edgeLength);

			float sAddLen = 0;
			float eAddLen = 0;

			if(!m_screenLengthMap.empty())
			{
				ScreenLengthMap::iterator it = m_screenLengthMap.begin();
				int mSize = m_screenLengthMap.size();
				for (int i = 0; i < mSize - 1; i++)
				{
					ScreenLengthMap::iterator nextIt = it;
					nextIt++;

					if(startLength >= it->second && startLength < nextIt->second)
					{
						startIndex = it->first;
						sAddLen = startLength - it->second;
					}

					if(endLength >= it->second && endLength < nextIt->second)
					{
						endIndex = it->first;
						eAddLen = endLength - it->second;
						break;
					}

					it++;
				}
			}

			startIndex = osg::clampBetween(startIndex,(int)0,(int)(m_worldPosList.size() - 2));
			endIndex = osg::clampBetween(endIndex,(int)0,(int)(m_worldPosList.size() - 2));

		    osg::Vec3d sLow = m_worldPosList[startIndex];
			osg::Vec3d sHigh = m_worldPosList[startIndex + 1];

			osg::Vec3d eLow = m_worldPosList[endIndex];
			osg::Vec3d eHigh = m_worldPosList[endIndex + 1];

			osg::Vec3d sDir = sHigh - sLow;
			sDir.normalize();
			osg::Vec3d eDir = eHigh - eLow;
			eDir.normalize();

			osg::Vec3 sPoint = sLow + sDir * sAddLen;
			osg::Vec3 ePoint = eLow + eDir * eAddLen;

			mv[curVertSize].x = sPoint.x();
			mv[curVertSize].y = sPoint.y();
			mv[curVertSize].z = sPoint.z();
			mv[curVertSize].w = 1.0;

			mc[curVertSize].r = m_searchColor.x();
			mc[curVertSize].g = m_searchColor.y();
			mc[curVertSize].b = m_searchColor.z();
			mc[curVertSize].a = 0.0;
			curVertSize++;

			if(endIndex > startIndex)
			{
				for (int i = startIndex + 1; i < endIndex + 1; i++)
				{
					mv[curVertSize].x = m_worldPosList[i].x();
					mv[curVertSize].y = m_worldPosList[i].y();
					mv[curVertSize].z = m_worldPosList[i].z();
					mv[curVertSize].w = 1.0;

					float curAlpha = -(m_searcherStartLength - m_screenLengthMap[i] )/ (m_edgeLength * m_searchRate) + 1.0;
					mc[curVertSize].r = m_searchColor.x();
					mc[curVertSize].g = m_searchColor.y();
					mc[curVertSize].b = m_searchColor.z();
					mc[curVertSize].a = curAlpha;
					curVertSize++;
				}
			}	

			mv[curVertSize].x = ePoint.x();
			mv[curVertSize].y = ePoint.y();
			mv[curVertSize].z = ePoint.z();
			mv[curVertSize].w = 1.0;

			mc[curVertSize].r = m_searchColor.x();
			mc[curVertSize].g = m_searchColor.y();
			mc[curVertSize].b = m_searchColor.z();
			mc[curVertSize].a = 1.0;
			curVertSize++;

			for (int i = 1; i < curVertSize - 1; i++)
			{
				mi[curIndexSize++] = 0;
				mi[curIndexSize++] = i;
				mi[curIndexSize++] = i + 1;
			}

			m_rpSearcher->setNewBuffer((GLvoid*)mv,(GLvoid*)mi,(GLvoid*)mc);
			m_rpSearcher->setNumIndices(curIndexSize);
		}
	}

	void SearchScreenCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if(node && nv)
		{
			SearchScreen* ss = dynamic_cast<SearchScreen*>(node);
			if(ss)
			{
				ss->updateUniform(nv);
				ss->updateSearcher();
			}
		}
		traverse(node,nv);
	}


	PyramidNode::PyramidNode( float radius,float xAngle,float yAngle )
		:m_radius(radius)
		,m_xAngle(xAngle)
		,m_yAngle(yAngle)
		,m_opGeode(NULL)
        ,m_opOutLine(NULL)
		,m_opLeftFace(NULL)
		,m_opRightFace(NULL)
		,m_opDownFace(NULL)
		,m_opUpFace(NULL)
		,m_opFrontFace(NULL)
		,m_numVert(0)
		,m_rpCullCallback(NULL)
		,m_edgeLength(0)
		,m_waveWidth(0)
		,m_baseColor(1.0,1.0,1.0,0.2)
		,m_waveColor(1.0,0.5,0.0,1.0)
		,m_numWave(5)
		,m_isSmooth(true)
		,m_waveStartOffset(0)
		,m_widthBetweenWave(0)
		,m_moveSpeed(0)
	{
		initPram();
		initNode();
	}

	void PyramidNode::initPram()
	{
		m_waveWidth = m_radius * 0.02;
		m_widthBetweenWave = m_waveWidth * 5.0;
		m_moveSpeed = m_radius / 5.0;

		for (int i = 0; i < m_numWave; i++)
		{
			m_startLength[i] = m_waveStartOffset + i * (m_waveWidth + m_widthBetweenWave);
		}
	}

	void PyramidNode::initNode()
	{
		createNode();

		m_rpCullCallback = new PyramidNodeCallback1();
		addCullCallback(m_rpCullCallback);
	}

	void PyramidNode::createNode()
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

		osg::Vec3 leftUp(-width,height,-m_radius);
		osg::Vec3 rightUp(width,height,-m_radius);
		osg::Vec3 rightDown(width,-height,-m_radius);
		osg::Vec3 leftDown(-width,-height,-m_radius);

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
		osg::Vec4 lineColor(m_baseColor.x(),m_baseColor.y(),m_baseColor.z(),1.0);
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

		addChild(m_opGeode.get());
	}

	void PyramidNode::updateUniform(osg::NodeVisitor* nv)
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


	void PyramidNodeCallback1::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if(node && nv)
		{
			PyramidNode* tn = dynamic_cast<PyramidNode*>(node);
			if(tn)
			{
				osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
				if(cv)
				{
					osg::Camera* cam = cv->getCurrentCamera();
					if(cam)
					{
						osg::Vec3d eye,look,up;
						cam->getViewMatrix().getLookAt(eye,look,up);

						/*osg::Vec3d lookDir = look - eye;
						tn->updateGeometry(lookDir);*/
					}

					tn->updateUniform(nv);
				}
			}
		}
		traverse(node,nv);
	}

	static const char *pipeOutLineVertSource = 
	{
		"#define MAXNUMMAT 20\n"
		"varying vec3 vertexNormal;\n"
		"uniform mat4 u_worldMat[MAXNUMMAT];\n"
		//"uniform mat4 u_worldMat;\n"
		"uniform int u_numMoveLine;\n"
		"void main(void)\n"
		"{\n"
		"    mat4 wvpMat = mat4(1.0);\n"
		"    if(gl_InstanceID < MAXNUMMAT) \n"
		"	 {\n"
		"		 wvpMat = u_worldMat[gl_InstanceID];\n"
		"	 }\n"
		"    gl_Position = wvpMat * gl_Vertex;\n"
		"    vertexNormal = gl_Normal;\n"
		//"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		//"    gl_Position = u_worldMat * gl_Vertex;\n"
		//"    gl_NormalMatrix * gl_Normal;\n"
		"}\n"
	};

	static const char *pipeOutLineFragSource =
	{
		"varying vec3 vertexNormal;\n"
		"uniform vec4 u_outLineColor;\n"
		"void main(void)\n"
		"{\n"
		"    gl_FragColor = u_outLineColor;\n"
		//"    gl_FragColor = vec4(vertexNormal,1.0);\n"
		"}\n"
	};

	static const char *pipeLineVertSource = 
	{
		"varying vec3 vertexNormal;\n"
		"varying vec4 vertexPos;\n"
		"void main(void)\n"
		"{\n"
		"    vertexNormal = gl_NormalMatrix * gl_Normal;\n"
		"    vertexPos = gl_ModelViewMatrix * gl_Vertex;\n"
		"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"}\n"
	};

	static const char *pipeLineFragSource =
	{
		"varying vec3 vertexNormal;\n"
		"varying vec4 vertexPos;\n"
		"uniform vec4 u_pipeColor;\n"
		"uniform vec3 u_diffColor;\n"
		"uniform bool u_enableLighting;\n"
		"void main(void)\n"
		"{\n"
		"    if(u_enableLighting) \n"
		"    { \n"
		"       vertexNormal = normalize(vertexNormal);\n"
		"       vec3 lightDir = -normalize(vertexPos.xyz);\n"
		"       float lightColor = dot(vertexNormal,lightDir);\n"
		"       lightColor = pow(lightColor,2);\n"
		"       lightColor = clamp(lightColor,0,1);\n"
		"       float diffPow = 0.7;\n"
		" 	    vec3 fColor = u_pipeColor.xyz * lightColor + u_diffColor;\n"	
		//"	    gl_FragColor = vec4(lightColor,lightColor,lightColor,1.0);\n"
		"	    gl_FragColor = vec4(fColor,u_pipeColor.a);\n"
		"    } \n"
		"    else \n"
		"    { \n"
		"	    gl_FragColor = vec4(u_pipeColor);\n"
		"    } \n"
		"}\n"
	};

	PipeLine::PipeLine( const CenterLineList& ll ,FeUtil::CRenderContext* ct)
		:m_rpPipeLineGeometry(NULL)
		,m_numPointPerRing(50)
		,m_opRenderContext(ct)
		,m_pipeColor(1.0,1.0,1.0,0.5)
		,m_rpOutLineGeometry(NULL)
		,m_rpPipeLineCallback(NULL)
		,m_rpBackPipeLineGeometry(NULL)
		,m_centerLineList(ll)
		,m_outLineColor(0.0,1.0,0.0,1.0)
		,m_moveSpeed(50000)
		,m_centerLineLength(0)
		,m_numMoveLine(10)
		,m_rpPreDrawCallback(NULL)
		,m_rpPreCamera(NULL)
		,m_diffColor(0.7,0.0,0.0)
		,m_needUpdatePolygonMode(false)
		,m_polygonMode(osg::PolygonMode::FILL)
		,m_enableLighting(true)
	{
		m_rpPipeLineCallback = new PipeLineCallback();
	    addCullCallback(m_rpPipeLineCallback);

		m_rpPreCamera = new osg::Camera();
		m_rpPreCamera->setRenderOrder(osg::Camera::PRE_RENDER);
		m_rpPreDrawCallback = new PreDrawCallback(this);
		m_rpPreCamera->setPreDrawCallback(m_rpPreDrawCallback);
		addChild(m_rpPreCamera);

		if(m_numMoveLine >= MAXNUMMAT)
			m_numMoveLine = MAXNUMMAT;

		createPipeNode();
	}

	PipeLine::~PipeLine()
	{
		m_localLineList.clear();
		m_localLineList.resize(0);
		m_centerLineList.clear();
		m_centerLineList.resize(0);
		m_centerLineLengthMap.clear();
		m_outLineTransMatList.clear();
		m_outLineTransMatList.resize(0);
		m_outLineStartLengthList.clear();
		m_outLineStartLengthList.resize(0);
	}

	void PipeLine::createPipeNode()
	{
		if(m_centerLineList.size() > 1 && m_opRenderContext.valid())
		{
			osg::Vec3d localUp(0,0,1);
			osg::Vec3d localNorth(0,1,0);
			float perAngle = osg::PI * 2.0 / (float)m_numPointPerRing;

			m_rpOutLineGeometry = new osg::Geometry();
			osg::Vec3Array* lva = new osg::Vec3Array();
			osg::DrawElementsUShort* lia = new osg::DrawElementsUShort(osg::PrimitiveSet::LINE_STRIP);
			lia->setNumInstances(m_numMoveLine);
			osg::Vec4Array* lca = new osg::Vec4Array();
			lca->push_back(m_outLineColor);

			for (int i = 0; i < m_numPointPerRing; i++)
			{
				osg::Matrix rm = osg::Matrix::rotate(perAngle * i,localUp);
				osg::Vec3d curPos = localNorth * rm;
				m_localLineList.push_back(curPos);
				lva->push_back(curPos);
				lia->push_back(i);
			}

			if(!m_localLineList.empty())
			{
				m_localLineList.push_back(m_localLineList[0]);
				lva->push_back(m_localLineList[0]);
				lia->push_back(m_numPointPerRing);
			}

			m_rpOutLineGeometry->setVertexArray(lva);
			m_rpOutLineGeometry->addPrimitiveSet(lia);
			m_rpOutLineGeometry->setColorArray(lca,osg::Array::BIND_OVERALL);
			osg::BoundingBox outLineBox;

			m_rpPipeLineGeometry = new osg::Geometry();

			osg::Vec3Array* va = new osg::Vec3Array();
			osg::Vec3Array* na = new osg::Vec3Array();

			osg::Vec3d localCenter(m_centerLineList[0].x(),m_centerLineList[0].y(),m_centerLineList[0].z());
			osg::Vec3d localCenterPos;
			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),localCenter,localCenterPos);

			float curCenterLineLength = 0;
			for (int i = 0; i < m_centerLineList.size(); i++)
			{
				osg::Matrix rotateMat;
				rotateMat.makeIdentity();
				osg::Vec3 faceDir;
				osg::Vec3d curPos(m_centerLineList[i].x(),m_centerLineList[i].y(),m_centerLineList[i].z());
				osg::Vec3d curWorPos;
				FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),curPos,curWorPos);

				if(i == 0)
				{
					osg::Vec3d secondPoint(m_centerLineList[i + 1].x(),m_centerLineList[i + 1].y(),m_centerLineList[i + 1].z());
					osg::Vec3d swp;
					FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),secondPoint,swp);

					faceDir = curWorPos - swp;
					faceDir.normalize();

					curCenterLineLength = 0;
				}
				else if(i == m_centerLineList.size() - 1)
				{
					osg::Vec3d secondPoint(m_centerLineList[i - 1].x(),m_centerLineList[i - 1].y(),m_centerLineList[i - 1].z());
					osg::Vec3d swp;
					FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),secondPoint,swp);

					faceDir = swp - curWorPos;

					curCenterLineLength += faceDir.length();

					faceDir.normalize();
				}
				else
				{
					osg::Vec3d fp(m_centerLineList[i - 1].x(),m_centerLineList[i - 1].y(),m_centerLineList[i - 1].z());
					osg::Vec3d fwp;
					FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),fp,fwp);

					osg::Vec3d ep(m_centerLineList[i + 1].x(),m_centerLineList[i + 1].y(),m_centerLineList[i + 1].z());
					osg::Vec3d ewp;
					FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),ep,ewp);

					osg::Vec3 fDir = fwp - curWorPos;

					curCenterLineLength += fDir.length();

					fDir.normalize();
					osg::Vec3 eDir = curWorPos - ewp;
					eDir.normalize();

					osg::Vec3 cDir = fDir ^ eDir;
					cDir.normalize();
					float rAngle = std::acos(fDir * eDir);
					osg::Matrix rm = osg::Matrix::rotate(rAngle / 2.0,cDir);

					faceDir = fDir * rm;
				}

				osg::Vec3 crossUp = localUp ^ faceDir;
				crossUp.normalize();
				float rAngle = std::acos(localUp * faceDir);

				rotateMat = osg::Matrix::rotate(rAngle,crossUp);

                osg::Matrix scaleMatrix;
				scaleMatrix.makeScale(m_centerLineList[i].w(),m_centerLineList[i].w(),m_centerLineList[i].w());

				osg::Matrix transMat;
				transMat.makeTranslate(curWorPos);

				osg::Matrix fMat = scaleMatrix * rotateMat * transMat;

				for (int j = 0; j < m_localLineList.size(); j++)
				{
					osg::Vec3d wPos = m_localLineList[j] * fMat;
					osg::Vec3 vPos = wPos - localCenterPos;

					outLineBox.expandBy(wPos);
					va->push_back(vPos);
					na->push_back(osg::Vec3(0,0,0));
				}

				osg::Vec4 curDirAndLength(faceDir,curCenterLineLength);
				m_centerLineLengthMap.insert(CenterLineLengthMap::value_type(i,curDirAndLength));
			}

			m_rpOutLineGeometry->setInitialBound(outLineBox);

			m_centerLineLength = curCenterLineLength;

			float perLength = 0;
			if(m_numMoveLine > 1)
			    perLength = m_centerLineLength / (float)(m_numMoveLine - 1);

			for (int i = 0; i < m_numMoveLine; i++)
			{
				float curLength = i * perLength;
				m_outLineStartLengthList.push_back(curLength);
			}

			osg::DrawElementsUShort* ia = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES);
			osg::Vec3 firstDir;
			osg::Vec3 endDir;
			for (int i = 0; i < m_centerLineList.size() - 1; i++)
			{	
				for (int j = 0; j < m_localLineList.size() - 1; j++)
				{
					ia->push_back(m_localLineList.size() * i + j);
					ia->push_back(m_localLineList.size() * (i + 1) + j);
					ia->push_back(m_localLineList.size() * (i + 1) + j + 1);

					osg::Vec3 p0 = va->asVector().at(m_localLineList.size() * i + j);
					osg::Vec3 p1 = va->asVector().at(m_localLineList.size() * (i + 1) + j);
					osg::Vec3 p2 = va->asVector().at(m_localLineList.size() * (i + 1) + j + 1);
					osg::Vec3 dir0 = p1 - p0;
					dir0.normalize();
					osg::Vec3 dir1 = p2 - p1;
					dir1.normalize();
					osg::Vec3 norDir = dir0 ^ dir1;
					norDir.normalize();
					na->asVector().at(m_localLineList.size() * i + j) += norDir;
					na->asVector().at(m_localLineList.size() * i + j).normalize();
					na->asVector().at(m_localLineList.size() * (i + 1) + j) += norDir;
					na->asVector().at(m_localLineList.size() * (i + 1) + j).normalize();
					na->asVector().at(m_localLineList.size() * (i + 1) + j + 1) += norDir;
					na->asVector().at(m_localLineList.size() * (i + 1) + j + 1).normalize();

					if(j == 0)
                       firstDir = norDir;
					else if(j == m_localLineList.size() - 2)
                       endDir = norDir;

					ia->push_back(m_localLineList.size() * i + j);
					ia->push_back(m_localLineList.size() * (i + 1) + j + 1);
					ia->push_back(m_localLineList.size() * i + j + 1);

					p0 = va->asVector().at(m_localLineList.size() * i + j);
					p1 = va->asVector().at(m_localLineList.size() * (i + 1) + j + 1);
					p2 = va->asVector().at(m_localLineList.size() * i + j + 1);
					dir0 = p1 - p0;
					dir0.normalize();
					dir1 = p2 - p1;
					dir1.normalize();
					norDir = dir0 ^ dir1;
					norDir.normalize();
					na->asVector().at(m_localLineList.size() * i + j) += norDir;
					na->asVector().at(m_localLineList.size() * i + j).normalize();
					na->asVector().at(m_localLineList.size() * (i + 1) + j + 1) += norDir;
					na->asVector().at(m_localLineList.size() * (i + 1) + j + 1).normalize();
					na->asVector().at(m_localLineList.size() * i + j + 1) += norDir;
					na->asVector().at(m_localLineList.size() * i + j + 1).normalize();
				}

				na->asVector().at(m_localLineList.size() * i) += endDir;
				na->asVector().at(m_localLineList.size() * i).normalize();
				na->asVector().at(m_localLineList.size() * i + m_localLineList.size() - 1) += firstDir;
				na->asVector().at(m_localLineList.size() * i + m_localLineList.size() - 1).normalize();
			}

			na->asVector().at(m_localLineList.size() * (m_centerLineList.size() - 1)) += endDir;
			na->asVector().at(m_localLineList.size() * (m_centerLineList.size() - 1)).normalize();
			na->asVector().at(m_localLineList.size() * (m_centerLineList.size() - 1) + m_localLineList.size() - 1) += firstDir;
			na->asVector().at(m_localLineList.size() * (m_centerLineList.size() - 1) + m_localLineList.size() - 1).normalize();

			osg::Vec4Array* ca = new osg::Vec4Array();
			ca->push_back(m_pipeColor);


			m_rpPipeLineGeometry->setVertexArray(va);
			m_rpPipeLineGeometry->addPrimitiveSet(ia);
			m_rpPipeLineGeometry->setNormalArray(na,osg::Array::BIND_PER_VERTEX);
			m_rpPipeLineGeometry->setColorArray(ca,osg::Array::BIND_OVERALL);

			m_rpPipeLineGeometry->setDataVariance(osg::Object::DYNAMIC);
			m_rpPipeLineGeometry->setUseDisplayList(false);
			m_rpPipeLineGeometry->setUseVertexBufferObjects(true);

			m_rpPipeLineGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
			m_rpPipeLineGeometry->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
			m_rpPipeLineGeometry->getOrCreateStateSet()->setRenderBinDetails(11,"DepthSortedBin");
			m_rpPipeLineGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK),osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

			/*osg::Stencil* pipeLineStencil = new osg::Stencil();
			pipeLineStencil->setFunction(osg::Stencil::NOTEQUAL,1,1);
			pipeLineStencil->setOperation(osg::Stencil::KEEP,osg::Stencil::KEEP,osg::Stencil::KEEP);
			m_rpPipeLineGeometry->getOrCreateStateSet()->setAttributeAndModes(pipeLineStencil,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);*/

			osg::Program* prog1 = new osg::Program();
			prog1->addShader(new osg::Shader(osg::Shader::VERTEX,pipeLineVertSource));
			prog1->addShader(new osg::Shader(osg::Shader::FRAGMENT,pipeLineFragSource));
			m_rpPipeLineGeometry->getOrCreateStateSet()->setAttributeAndModes(prog1,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

			osg::PolygonMode* pm = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK,m_polygonMode);
			m_rpPipeLineGeometry->getOrCreateStateSet()->setAttributeAndModes(pm,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

			osg::Uniform* u_pipeColor = new osg::Uniform("u_pipeColor",m_pipeColor);
			m_rpPipeLineGeometry->getOrCreateStateSet()->addUniform(u_pipeColor);
			osg::Uniform* u_diffColor = new osg::Uniform("u_diffColor",m_diffColor);
			m_rpPipeLineGeometry->getOrCreateStateSet()->addUniform(u_diffColor);
			osg::Uniform* u_enableLighting = new osg::Uniform("u_enableLighting",m_enableLighting);
			m_rpPipeLineGeometry->getOrCreateStateSet()->addUniform(u_enableLighting);
			
				
			m_rpBackPipeLineGeometry = osg::clone(m_rpPipeLineGeometry.get(), osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL));
			m_rpBackPipeLineGeometry->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT),osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
			m_rpBackPipeLineGeometry->getOrCreateStateSet()->setRenderBinDetails(10,"DepthSortedBin");

			m_rpOutLineGeometry->setDataVariance(osg::Object::DYNAMIC);
			m_rpOutLineGeometry->setUseDisplayList(false);
			m_rpOutLineGeometry->setUseVertexBufferObjects(true);
			m_rpOutLineGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
			m_rpOutLineGeometry->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
			m_rpOutLineGeometry->getOrCreateStateSet()->setRenderBinDetails(9,"DepthSortedBin");

			OutLineBoundBoxCallback* bbc = new OutLineBoundBoxCallback(this);
			m_rpOutLineGeometry->setComputeBoundingBoxCallback(bbc);

			/*osg::Stencil* outLineStencil = new osg::Stencil();
			outLineStencil->setFunction(osg::Stencil::ALWAYS);
			outLineStencil->setStencilPassAndDepthPassOperation(osg::Stencil::INCR);
			m_rpOutLineGeometry->getOrCreateStateSet()->setAttributeAndModes(outLineStencil,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);*/

			osg::LineWidth* outLineWidth = new osg::LineWidth(2);
			m_rpOutLineGeometry->getOrCreateStateSet()->setAttributeAndModes(outLineWidth,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

			osg::Program* prog = new osg::Program();
			prog->addShader(new osg::Shader(osg::Shader::VERTEX,pipeOutLineVertSource));
			prog->addShader(new osg::Shader(osg::Shader::FRAGMENT,pipeOutLineFragSource));
			m_rpOutLineGeometry->getOrCreateStateSet()->setAttributeAndModes(prog,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
			osg::Uniform* u_outLineColor = new osg::Uniform("u_outLineColor",m_outLineColor);
			m_rpOutLineGeometry->getOrCreateStateSet()->addUniform(u_outLineColor);
			osg::Matrix initMat;
			osg::Uniform* u_worldMat = new osg::Uniform(osg::Uniform::FLOAT_MAT4,"u_worldMat",m_numMoveLine);
			m_rpOutLineGeometry->getOrCreateStateSet()->addUniform(u_worldMat);
			osg::Uniform* u_numMoveLine = new osg::Uniform("u_numMoveLine",m_numMoveLine);
			m_rpOutLineGeometry->getOrCreateStateSet()->addUniform(u_numMoveLine);	

			osg::Geode* gn = new osg::Geode();
			gn->addDrawable(m_rpPipeLineGeometry);
			gn->addDrawable(m_rpBackPipeLineGeometry);

			osg::Matrix worldTransMat;
			worldTransMat.makeTranslate(localCenterPos);
			osg::MatrixTransform* mt = new osg::MatrixTransform();
			mt->setMatrix(worldTransMat);
			mt->addChild(gn);

			osg::Geode* lgn = new osg::Geode();
			lgn->addDrawable(m_rpOutLineGeometry);

			addChild(mt);
			addChild(lgn);	

			for (int i = 0;i < m_numMoveLine; i++)
			{
				osg::Matrix m;
				m_outLineTransMatList.push_back(m);
			}
			
		}
	}
	void PipeLine::updateOutLine(osg::NodeVisitor* nv)
	{
		if(m_rpOutLineGeometry.valid() && !m_centerLineLengthMap.empty())
		{	
			float time = (float)std::fmod(nv->getFrameStamp()->getSimulationTime(),(double)FLT_MAX);
			for (int j = 0; j < m_numMoveLine; j++)
			{
				float curStartLength = fmod(m_outLineStartLengthList[j] + time * m_moveSpeed,m_centerLineLength);

				CenterLineLengthMap::iterator it = m_centerLineLengthMap.begin();
				int mSize = m_centerLineLengthMap.size();
				int curIndex = -1;
				float curLengthFromStart = 0;
				float curNextLength = 0;
				for (int i = 0; i < mSize - 1; i++)
				{
					CenterLineLengthMap::iterator nextIt = it;
					nextIt++;

					if(curStartLength >= it->second.w() && curStartLength < nextIt->second.w())
					{
						curIndex = it->first;
						curLengthFromStart = curStartLength - it->second.w();
						curNextLength = nextIt->second.w() - it->second.w();
						break;
					}

					it++;
				}

				osg::Matrix transMat;
				transMat.makeIdentity();

				if(curIndex >= 0 && curIndex <= m_centerLineList.size() - 2 && curLengthFromStart >= 0)
				{
					int nextIndex = curIndex + 1;

					float startR = m_centerLineList[curIndex].w();
					float endR = m_centerLineList[nextIndex].w();

					osg::Vec3 startDir(m_centerLineLengthMap[curIndex].x(),m_centerLineLengthMap[curIndex].y(),m_centerLineLengthMap[curIndex].z());
					startDir.normalize();
					osg::Vec3 endDir(m_centerLineLengthMap[nextIndex].x(),m_centerLineLengthMap[nextIndex].y(),m_centerLineLengthMap[nextIndex].z());
					endDir.normalize();

					float curRate = curLengthFromStart / curNextLength;
					curRate = osg::clampBetween(curRate,0.0f,1.0f);

					float curR = startR + (endR - startR) * curRate;

					osg::Vec3 crossDir = startDir ^ endDir;
					crossDir.normalize();
					float rAngle = std::acos(startDir * endDir);

					float curAngle = rAngle * curRate;
					osg::Matrix dirMat;
					dirMat = osg::Matrix::rotate(curAngle,crossDir);
					osg::Vec3 curDir = startDir * dirMat;
					curDir.normalize();

					osg::Vec3d startSPos(m_centerLineList[curIndex].x(),m_centerLineList[curIndex].y(),m_centerLineList[curIndex].z());
					osg::Vec3d startWPos;
					FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),startSPos,startWPos);
					osg::Vec3d endSPos(m_centerLineList[nextIndex].x(),m_centerLineList[nextIndex].y(),m_centerLineList[nextIndex].z());
					osg::Vec3d endWPos;
					FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(),endSPos,endWPos);
					osg::Vec3d wDir = endWPos - startWPos;
					wDir.normalize();

					osg::Vec3d curWPos = startWPos + wDir * curLengthFromStart;


					osg::Matrix curScaleMat;
					curScaleMat.makeScale(curR,curR,curR);

					osg::Matrix curRotateMat;
					osg::Vec3 localUp(0,0,1);
					osg::Vec3 crossUp = localUp ^ curDir;
					crossUp.normalize();
					float rotateAngle = std::acos(localUp * curDir);
					curRotateMat = osg::Matrix::rotate(rotateAngle,crossUp);

					osg::Matrix curTranslateMat;
					curTranslateMat.makeTranslate(curWPos);

					transMat = curScaleMat * curRotateMat * curTranslateMat;
				}

				m_outLineTransMatList[j] = transMat;
			}
		}
	}

	void PipeLine::updateUniform()
	{
		if(m_rpOutLineGeometry.valid())
		{
			if(m_opRenderContext.valid())
			{
				osg::Camera* cam = m_opRenderContext->GetCamera();
				if(cam)
				{
					osg::StateSet* ss = m_rpOutLineGeometry->getStateSet();
					if(ss)
					{
						osg::Uniform* u_worldMat = ss->getUniform("u_worldMat");

						if(u_worldMat && !m_outLineTransMatList.empty())
						{	
							for (int i = 0; i < m_outLineTransMatList.size(); i++)
							{		
								u_worldMat->setElement(i,m_outLineTransMatList[i] * cam->getViewMatrix() * cam->getProjectionMatrix());
							}	
						}

						osg::Uniform* u_outLineColor = ss->getUniform("u_outLineColor");
						if(u_outLineColor)
						{
							u_outLineColor->set(m_outLineColor);
						}
					}	
				}
			}
		}
		if(m_rpBackPipeLineGeometry.valid() && m_rpPipeLineGeometry.valid())
		{	
			osg::StateSet* pls = m_rpPipeLineGeometry->getStateSet();
			osg::StateSet* bps = m_rpBackPipeLineGeometry->getStateSet();
			if(pls && bps)
			{
				if(m_needUpdatePolygonMode)
				{
					osg::PolygonMode* pm = dynamic_cast<osg::PolygonMode*>(pls->getAttribute(osg::StateAttribute::POLYGONMODE));
					osg::PolygonMode* bpm = dynamic_cast<osg::PolygonMode*>(bps->getAttribute(osg::StateAttribute::POLYGONMODE));
					if(pm && bpm)
					{
						pm->setMode(osg::PolygonMode::FRONT_AND_BACK,m_polygonMode);
						bpm->setMode(osg::PolygonMode::FRONT_AND_BACK,m_polygonMode);
					}

					m_needUpdatePolygonMode = false;
				}
					

				osg::Uniform* u_pipeColor0 = pls->getUniform("u_pipeColor");
				osg::Uniform* u_pipeColor1 = bps->getUniform("u_pipeColor");
				if(u_pipeColor0 && u_pipeColor1)
				{
					u_pipeColor0->set(m_pipeColor);
					u_pipeColor1->set(m_pipeColor);
				}

				osg::Uniform* u_diffColor0 = pls->getUniform("u_diffColor");
				osg::Uniform* u_diffColor1 = bps->getUniform("u_diffColor");
				if(u_diffColor0 && u_diffColor1)
				{
					u_diffColor0->set(m_diffColor);
					u_diffColor1->set(m_diffColor);
				}

				osg::Uniform* u_enableLighting0 = pls->getUniform("u_enableLighting");
				osg::Uniform* u_enableLighting1 = bps->getUniform("u_enableLighting");
				if(u_enableLighting0 && u_enableLighting1)
				{
					u_enableLighting0->set(m_enableLighting);
					u_enableLighting1->set(m_enableLighting);
				}
		   }
		}
		
	}

	void PipeLine::setOutLineVisible( bool flag )
	{
		if(m_rpOutLineGeometry.valid())
		{
			m_rpOutLineGeometry->setNodeMask(flag ? 1 : 0);
		}
	}


	void PipeLineCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
	{
		if(node && nv)
		{
			PipeLine* pl = dynamic_cast<PipeLine*>(node);
			if(pl)
			{
				pl->updateOutLine(nv);
			}
		}
		traverse(node,nv);
	}


	OutLineBoundBoxCallback::OutLineBoundBoxCallback(PipeLine* pl)
		:m_opPipeLine(pl)
	{
	}

	osg::BoundingBox OutLineBoundBoxCallback::computeBound( const osg::Drawable& dr) const
	{
		return dr.getInitialBound();
	}

	void PreDrawCallback::operator()( osg::RenderInfo& renderInfo ) const
	{
		if(m_opPipeLine.valid())
		{
			m_opPipeLine->updateUniform();
		}
	}


	static const char *testVertSource = 
	{
		"void main(void)\n"
		"{\n"
		"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"}\n"
	};

	static const char *testFragSource =
	{
		"uniform float u_length[2];\n"
		"void main(void)\n"
		"{\n"
		"    gl_FragColor = vec4(u_length[0],u_length[1],0.0,1.0);\n"
		"}\n"
	};

	TestShaderNode::TestShaderNode()
		:m_rpGeometry(NULL)
	{
		m_rpGeometry = new TestGeometry();
		osg::Geode* gn = new osg::Geode();
		gn->addDrawable(m_rpGeometry.get());
		addChild(gn);
	}

	TestGeometry::TestGeometry()
		:m_vertexBuffer(NULL)
	    ,m_indexBuffer(NULL)
		,m_meshIdxID(0)
		,m_meshVboID(0)
		,m_glewInit(1)
		,m_vertShader(0)
		,m_fragShader(0)
		,m_programCast(0)
		,m_uLength(-1)
		,m_hasProgramUniforms(false)
		,m_opGLExtension(NULL)
	{
		setDataVariance(osg::Object::DYNAMIC);
		setUseDisplayList(false);
		setUseVertexBufferObjects(true);

		m_floatArr[0] = 0.0;
		m_floatArr[1] = 1.0;
	}

	TestGeometry::~TestGeometry()
	{
		if ( m_meshVboID != 0 && m_opGLExtension.valid())
		{
			m_opGLExtension->glDeleteBuffers(1, &m_meshVboID);
		}

		if ( m_meshIdxID != 0 ) 
		{
			m_opGLExtension->glDeleteBuffers(1, &m_meshIdxID);
		}
	}

	void TestGeometry::drawImplementation( osg::RenderInfo& renderInfo ) const
	{
		m_mutex.lock();

		osg::State& state = *renderInfo.getState();
		m_opGLExtension = state.get<osg::GLExtensions>();

		if(m_vertexBuffer == NULL || m_indexBuffer == NULL)
		{
			createGeometry();
		}

		if(m_vertShader == 0 || m_fragShader == 0)
		{
			loadShaders();
		}	

		pushAllState();

		if (m_programCast) 
		{
			m_opGLExtension->glUseProgram(m_programCast);
		}
		
		if ( m_uLength != -1)
		{
			m_opGLExtension->glUniform1fv(m_uLength,2,m_floatArr);
		}

		m_opGLExtension->glBindBuffer(GL_ARRAY_BUFFER_ARB, m_meshVboID);
		m_opGLExtension->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, m_meshIdxID);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(4, GL_FLOAT, 0, 0);

		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

		m_opGLExtension->glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
		m_opGLExtension->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
		
		m_opGLExtension->glUseProgram(NULL);

		popAllState(renderInfo);

		m_mutex.unlock();
	}

	void TestGeometry::createGeometry() const
	{
		MeshVertex* vb = new MeshVertex[3];
		float r = 100000;
		vb[0].x = 0;
		vb[0].y = 0;
		vb[0].z = 0;
		vb[0].w = 1.0;

		vb[1].x = -r;
		vb[1].y = 0;
		vb[1].z = r;
		vb[1].w = 1.0;

		vb[2].x = r;
		vb[2].y = 0;
		vb[2].z = r;
		vb[2].w = 1.0;

		GLuint* ib = new GLuint[3];
		ib[0] = 0;
		ib[1] = 1;
		ib[2] = 2;

		m_opGLExtension->glGenBuffers(1,&m_meshVboID);
		m_opGLExtension->glBindBuffer(GL_ARRAY_BUFFER_ARB,m_meshVboID);
		m_opGLExtension->glBufferData(GL_ARRAY_BUFFER_ARB,3 * sizeof(MeshVertex),vb,GL_DYNAMIC_DRAW_ARB);

		m_opGLExtension->glGenBuffers(1,&m_meshIdxID);
		m_opGLExtension->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,m_meshIdxID);
		m_opGLExtension->glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB,3 * sizeof(GLuint),ib,GL_DYNAMIC_DRAW_ARB);

		m_opGLExtension->glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
		m_opGLExtension->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

		delete[] vb;
		delete[] ib;
	}

	void TestGeometry::pushAllState() const
	{
		GLint currentProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
		m_programStack.push(currentProgram);

		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
		glPushAttrib(GL_ALL_ATTRIB_BITS);
	}

	void TestGeometry::popAllState( osg::RenderInfo& renderInfo ) const
	{
		if (!m_programStack.empty()) 
		{
			osg::State& state = *renderInfo.getState();
			
			GLint savedProgram = m_programStack.top();
			m_opGLExtension->glUseProgram(savedProgram);
			m_programStack.pop();			
		}
		glPopAttrib();
		glPopClientAttrib();
	}

	GLuint TestGeometry::loadShader(bool vertexShader,const char* source) const
	{
		char *data = 0;
	    GLuint shader = 0;
		
		const char *sources[3];
		sources[0] = source;

	    shader = m_opGLExtension->glCreateShader(vertexShader ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);

		m_opGLExtension->glShaderSource(shader, 1, sources, NULL);

		m_opGLExtension->glCompileShader(shader);

		GLint ok;
		m_opGLExtension->glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
		if (!ok) 
		{
			return 0;
		}

	    return shader;
	}

	void TestGeometry::loadShaders() const
	{
		m_vertShader = loadShader(true,testVertSource);
		m_fragShader = loadShader(false,testFragSource);

		if (m_vertShader && m_fragShader)
		{
			m_programCast = m_opGLExtension->glCreateProgram();

			m_opGLExtension->glAttachShader(m_programCast, m_vertShader);
			m_opGLExtension->glAttachShader(m_programCast, m_fragShader);

			m_opGLExtension->glLinkProgram(m_programCast);

			GLint linkStatus;
			m_opGLExtension->glGetProgramiv(m_programCast, GL_LINK_STATUS, &linkStatus);
			if (linkStatus == GL_FALSE) 
			{
				return;
			}

			m_uLength = m_opGLExtension->glGetUniformLocation(m_programCast, "u_length");
		}
	}

}


