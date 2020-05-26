
#include <math.h>
#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>

#include <FeEffects/Corridor.h>


namespace FeEffect
{

	COsgIdealChannel::COsgIdealChannel(FeUtil::CRenderContext* pContext)
		:osg::Group()
		,FeUtil::CRenderContextObserver(pContext)
	{
		Init();
	}

	COsgIdealChannel::~COsgIdealChannel(void)
	{

	}

	bool COsgIdealChannel::Init()
	{
		m_pRoot = new osg::MatrixTransform;
		m_pWireframeGeode = new osg::Geode;
		m_pArrayVertex =new osg::Vec4dArray;
		m_pLockFrameGeode = new osg::Geode;
		m_Vec4Color.set(0.0,0.0,1.0,1.0);

		m_pApproachMode1 = new osg::Group;
		m_pApproachMode2 = new osg::Group;
		m_pApproachMode3 = new osg::Group;

		m_pRoot->addChild(m_pApproachMode1);
		m_pRoot->addChild(m_pApproachMode2);
		m_pRoot->addChild(m_pApproachMode3);

		m_iHeight=1050;
		m_iWidth=1680;

		m_iChannelHeight=1050;
		m_iChannelWidth = 1680;

		LeftBottom = osg::Vec3d(-m_iChannelWidth/2,0,-m_iChannelHeight/2);
		RightBottom = osg::Vec3d(m_iChannelWidth/2,0,-m_iChannelHeight/2);
		RightTop = osg::Vec3d(m_iChannelWidth/2,0,m_iChannelHeight/2);
		LeftTop = osg::Vec3d(-m_iChannelWidth/2,0,m_iChannelHeight/2);

		m_bCreat = false;

		DrawWireframe();
		DrawLockFrame();
		addChild(m_pRoot);
		return true;
	}

	bool COsgIdealChannel::RedrawIdealChannel( osg::Vec4dArray* pKeysVertex,double dKeysRotateAngle)
	{
		if (NULL==pKeysVertex)
		{
			return false;
		}
		//线框之间的连线之一左下角连线的顶点数组
		osg::Vec3dArray * _pVertexLeftBottomLine = new osg::Vec3dArray;
		//线框之间的连线之一右下角连线的顶点数组
		osg::Vec3dArray * _pVertexRightBottomLine = new osg::Vec3dArray;
		//线框之间的连线之一右上角连线的顶点数组
		osg::Vec3dArray * _pVertexRightTopLine =new osg::Vec3dArray;
		//线框之间的连线之一左上角的连线的顶点数组
		osg::Vec3dArray * _PVertexLeftTopLine = new osg::Vec3dArray;

		//绘制面所需顶点的数组
		osg::Vec3dArray * _pVertexFaceLeft = new osg::Vec3dArray;
		//绘制面所需顶点的数组
		osg::Vec3dArray * _pVertexFaceRight = new osg::Vec3dArray;
		//绘制面所需顶点的数组
		osg::Vec3dArray * _pVertexFaceTop = new osg::Vec3dArray;
		//绘制面所需顶点的数组
		osg::Vec3dArray * _pVertexFaceBottom = new osg::Vec3dArray;

		///存放所有线框的Group节点
		osg::ref_ptr<osg::Group> _pAllWireframeGroup = new osg::Group;
		///存放所有锁定框的Group节点
		osg::ref_ptr<osg::Group> _pAllLockFrameGroup = new osg::Group;

		//存放理想航道中开始进入平滑过渡阶段的起始点
		osg::Vec4dArray *_pStartSmooth =new osg::Vec4dArray;
		//存放理想航道中平滑过渡阶段结束的点
		osg::Vec4dArray *_pEndSmooth = new osg::Vec4dArray;
		//B样条插值时需要向起点和终点之间插入值的个数
		int InserValueCountB = 20;
		//记录平滑过渡阶段两个相邻线框之间旋转角度的插值
		double deltaRotateAngle = dKeysRotateAngle/(InserValueCountB/2);
		osg::Vec4dArray* pFinalLinePoint = getFinalLinePoint(pKeysVertex,InserValueCountB,_pStartSmooth,_pEndSmooth);	
		//记录理想通道四个角的连线的点的个数,同时根据count的值算出画上下左右四个面的点的个数
		int count = 0;	
		osg::Vec4dArray::iterator iter = pFinalLinePoint->begin();
		osg::Vec4dArray::iterator iterStartSmooth = _pStartSmooth->begin();
		osg::Vec4dArray::iterator iterEndSmooth = _pEndSmooth->begin();
		//标记理想航道是否处于平滑过渡阶段
		bool bIsSmooth =false;
		///平滑阶段线框应该旋转的角度
		double dRotateAngle_Smooth=0;
		//记录平滑过渡阶段已经做过旋转的线框的个数，
		int Rotate_WireFram_Count=0;
		for (;iter!=pFinalLinePoint->end();iter++)
		{
			//通道进近模式一用到的平移变换矩阵，变换矩阵将组成模式一的线框移动到地球表面
			osg::ref_ptr<osg::MatrixTransform> _pTranslateMode1  = new osg::MatrixTransform;
			//通道进近模式一用到的旋转变换矩阵
			osg::ref_ptr<osg::MatrixTransform> _pRotateMode1 = new osg::MatrixTransform;
			//通道进近模式二用到的平移变换矩阵，变换矩阵将组成模式二的线框移动到地球表面
			osg::ref_ptr<osg::MatrixTransform> _pTranslateMode2  = new osg::MatrixTransform;
			//通道进近模式二用到的旋转变换矩阵
			osg::ref_ptr<osg::MatrixTransform> _pRotateMode2 = new osg::MatrixTransform;

			_pRotateMode1->addChild(m_pWireframeGeode);
			_pRotateMode2->addChild(m_pLockFrameGeode);
			_pTranslateMode1->addChild(_pRotateMode1); 
			_pTranslateMode2->addChild(_pRotateMode2);

			_pAllWireframeGroup->addChild(_pTranslateMode1);
			_pAllLockFrameGroup->addChild(_pTranslateMode2);

			//获得奖线框从世界坐标的原点移动到地球表面所需的变换矩阵
			osg::Matrix matrix;
			FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(), osg::Vec3d(iter->x(),iter->y(),iter->z()),matrix);

			_pTranslateMode1->setMatrix(matrix);
			_pTranslateMode2->setMatrix(matrix);

			//获得线框从原点移到地表后旋转所需的航向角和俯仰角
			double YawAngle=0.0;//航向角
			double PitchAngle=0.0;//俯仰角
			double RollAngle=0.0;
			osg::Vec3d first(iter->x(),iter->y(),iter->z());
			if (iter+1!=pFinalLinePoint->end())
			{
				osg::Vec3d second((iter+1)->x(),(iter+1)->y(),(iter+1)->z());
				GetAngle(first,second,YawAngle,PitchAngle);
				//getFlyPosture(first,second,PitchAngle,RollAngle,YawAngle);
			}
			else
			{
				osg::Vec3d second((iter-1)->x(),(iter-1)->y(),(iter-1)->z());
				GetAngle(first,second,YawAngle,PitchAngle);
				YawAngle=-YawAngle;
				//RollAngle=osg::PI;
			}
			if (iterStartSmooth!=_pStartSmooth->end())
			{
				if (*iter==*iterStartSmooth)
				{
					dRotateAngle_Smooth=0;
					bIsSmooth = true;
					iterStartSmooth++;
				}
			}
			if (iterEndSmooth!=_pEndSmooth->end())
			{
				if (*iter==*iterEndSmooth)
				{
					dRotateAngle_Smooth=0;
					bIsSmooth=false;
					Rotate_WireFram_Count=0;
					iterEndSmooth++;
				}
			}
			if (true==bIsSmooth)
			{
				if (Rotate_WireFram_Count<=InserValueCountB/2)
				{
					dRotateAngle_Smooth=dRotateAngle_Smooth+deltaRotateAngle;
				}
				else
				{
					dRotateAngle_Smooth=dRotateAngle_Smooth-deltaRotateAngle;
				}
				Rotate_WireFram_Count++;
			}
			else
			{
				dRotateAngle_Smooth =0;
			}
			RollAngle = dRotateAngle_Smooth;
			osg::Matrix RotateMatrix = osg::Matrix::rotate(PitchAngle,osg::X_AXIS,RollAngle,osg::Y_AXIS,YawAngle,osg::Z_AXIS);
			//做旋转
			_pRotateMode1->setMatrix(RotateMatrix);
			_pRotateMode2->setMatrix(RotateMatrix);

			///线框被移动到地球上后，四个角顶点的坐标
			osg::Vec3d leftbottomline(LeftBottom*RotateMatrix*matrix);
			osg::Vec3d rightbottomline(RightBottom*RotateMatrix* matrix);
			osg::Vec3d righttopline(RightTop*RotateMatrix*matrix);
			osg::Vec3d lefttopline(LeftTop*RotateMatrix*matrix);

			///绘制线框之间的连线需要顶点，下面将绘制线框连线所需顶点压入到响应的数组中
			_pVertexLeftBottomLine->push_back(leftbottomline);
			_pVertexRightBottomLine->push_back(rightbottomline);
			_pVertexRightTopLine->push_back(righttopline);
			_PVertexLeftTopLine->push_back(lefttopline);

			if (count!=0)
			{
				///让如绘制通道四个面所需顶点
				_pVertexFaceLeft->push_back(leftbottomline);
				_pVertexFaceLeft->push_back(lefttopline);
				_pVertexFaceLeft->push_back(lefttopline);
				_pVertexFaceLeft->push_back(leftbottomline);

				_pVertexFaceRight->push_back(rightbottomline);
				_pVertexFaceRight->push_back(righttopline);
				_pVertexFaceRight->push_back(righttopline);
				_pVertexFaceRight->push_back(rightbottomline);

				_pVertexFaceTop->push_back(righttopline);
				_pVertexFaceTop->push_back(lefttopline);
				_pVertexFaceTop->push_back(lefttopline);
				_pVertexFaceTop->push_back(righttopline);

				_pVertexFaceBottom->push_back(rightbottomline);
				_pVertexFaceBottom->push_back(leftbottomline);
				_pVertexFaceBottom->push_back(leftbottomline);
				_pVertexFaceBottom->push_back(rightbottomline);	

			}
			else 
			{
				///让如绘制通道四个面所需顶点
				_pVertexFaceLeft->push_back(lefttopline);
				_pVertexFaceLeft->push_back(leftbottomline);

				_pVertexFaceRight->push_back(righttopline);
				_pVertexFaceRight->push_back(rightbottomline);

				_pVertexFaceTop->push_back(lefttopline);
				_pVertexFaceTop->push_back(righttopline);

				_pVertexFaceBottom->push_back(leftbottomline);
				_pVertexFaceBottom->push_back(rightbottomline);
			}
			count++;
		}
		osg::Geode* _pWireLineGeo =DrawWireframeLigature(_pVertexLeftBottomLine,_PVertexLeftTopLine
			,_pVertexRightBottomLine,_pVertexRightTopLine,count);
		osg::Geode* _pFaceGeo =DrawChannelFace(_pVertexFaceLeft,_pVertexFaceRight,_pVertexFaceTop,
			_pVertexFaceBottom,count*4-2);

		//理想通道模式一的Group节点,此模式中有线框及连接线框的四条线组成
		m_pApproachMode1->removeChildren(0,m_pApproachMode1->getNumChildren());
		m_pApproachMode1->addChild(_pAllWireframeGroup);
		m_pApproachMode1->addChild(_pWireLineGeo);
		//理想通道模式二的Group节点,此模式中只有锁定框
		m_pApproachMode2->removeChildren(0,m_pApproachMode2->getNumChildren());
		m_pApproachMode2->addChild(_pAllLockFrameGroup);
		//理想通道模式二的Group节点,此模式中含有锁定框、四条连线、四个面
		m_pApproachMode3->removeChildren(0,m_pApproachMode3->getNumChildren());
		m_pApproachMode3->addChild(_pAllLockFrameGroup);
		m_pApproachMode3->addChild(_pWireLineGeo);
		m_pApproachMode3->addChild(_pFaceGeo);

		m_pApproachMode1->setNodeMask(0x00);
		m_pApproachMode2->setNodeMask(0x00);
		m_pApproachMode3->setNodeMask(0x00);

		return true;
	}

	osg::Vec4dArray * COsgIdealChannel::getFinalLinePoint( osg::Vec4dArray * pKeysVertex,double dInsertCount
		,osg::Vec4dArray *_pStartSmooth,osg::Vec4dArray *_pEndSmooth)
	{
		osg::Vec4dArray* interpolation = new osg::Vec4dArray;
		///对关键点进行第一次插值
		FeMath::Interpolation(pKeysVertex,interpolation);

		///存放将关键点数组pKeysVertex经过线性插值算法和B样条插值法俩个插值算法插值后的点的数组,即存放最总组成理想航线的所有点的数组
		osg::Vec4dArray* pFinalLinePointVertex = new osg::Vec4dArray;

		osg::Vec4dArray::iterator iterKeys = pKeysVertex->begin();
		iterKeys++;
		osg::Vec4dArray::iterator iter = interpolation->begin();
		for (;iter!=interpolation->end();)
		{
			if (iter+1!=interpolation->end()&&iter+2!=interpolation->end()
				&&iterKeys!=pKeysVertex->end()&&iter+3!=interpolation->end()&&iter+4!=interpolation->end())
			{
				if (*(iter+2)==*iterKeys)
				{
					///两个点之间用B样条插值法后得到的数组
					osg::ref_ptr<osg::Vec4dArray> pOutArrayBLine = new osg::Vec4dArray;
					///用B样条插值法在两个点之间插值时要出入的点的个数
					int iBLineCount = dInsertCount;
					///第二次插值
					ComputerBLine(*iter,*(iter+4),*(iter+2),pOutArrayBLine,iBLineCount);
					osg::Vec4dArray::iterator iterBLine = pOutArrayBLine->begin();
					for(;iterBLine+1!=pOutArrayBLine->end();iterBLine++)
					{
						pFinalLinePointVertex->push_back(*iterBLine);
					}
					iterKeys++;
					///将理想航道中开始进入平滑阶段的点压入_pStartSmooth中
					_pStartSmooth->push_back(*iter);
					iter=iter+4;
					///将理想航道结束某段平滑过渡的点压入_pEndSmooth中
					_pEndSmooth->push_back(*iter);
				}
				else
				{
					pFinalLinePointVertex->push_back(*iter);
					iter++;
				}
			}
			else
			{
				pFinalLinePointVertex->push_back(*iter);
				iter++;
			}
		}
		return pFinalLinePointVertex;
	}

	osg::Geode * COsgIdealChannel::DrawWireframeLigature( osg::Vec3dArray * _pLeftBottomLine,osg::Vec3dArray * _pLeftTopLine,
		osg::Vec3dArray * _pRightBottomLine,osg::Vec3dArray * _pRightTopLine,int PointCount )
	{
		///连线组成理想航道线框的四条线的Geometry节点
		osg::ref_ptr<osg::Geometry> _pLeftBottomLineGeom = new osg::Geometry;
		osg::ref_ptr<osg::Geometry> _pLeftTopLineGeom = new osg::Geometry;
		osg::ref_ptr<osg::Geometry> _pRightBottomLineGeom = new osg::Geometry;
		osg::ref_ptr<osg::Geometry> _pRightTopLineGeom = new osg::Geometry;

		///连线组成理想航道线框的四条线的geode节点
		osg::Geode* _pWireLineGeo = new osg::Geode;


		osg::Vec4dArray *_pColorLine = new osg::Vec4dArray;

		for (int i =0;i<PointCount;i++)
		{
			_pColorLine->push_back(m_Vec4Color);
		}

		///左下角线的绘制
		_pLeftBottomLineGeom->setColorArray(_pColorLine);
		_pLeftBottomLineGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
		_pLeftBottomLineGeom->setVertexArray(_pLeftBottomLine);

		_pLeftBottomLineGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,0,PointCount));

		_pWireLineGeo->addDrawable(_pLeftBottomLineGeom);

		///左上角线的绘制
		_pLeftTopLineGeom->setColorArray(_pColorLine);
		_pLeftTopLineGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
		_pLeftTopLineGeom->setVertexArray(_pLeftTopLine);

		_pLeftTopLineGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,0,PointCount));

		_pWireLineGeo->addDrawable(_pLeftTopLineGeom);

		///右下角线的绘制
		_pRightBottomLineGeom->setColorArray(_pColorLine);
		_pRightBottomLineGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
		_pRightBottomLineGeom->setVertexArray(_pRightBottomLine);

		_pRightBottomLineGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,0,PointCount));

		_pWireLineGeo->addDrawable(_pRightBottomLineGeom);

		///右上角线的绘制
		_pRightBottomLineGeom->setColorArray(_pColorLine);
		_pRightTopLineGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
		_pRightTopLineGeom->setVertexArray(_pRightTopLine);

		_pRightTopLineGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,0,PointCount));

		_pWireLineGeo->addDrawable(_pRightTopLineGeom);

		_pWireLineGeo->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
		_pWireLineGeo->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		_pWireLineGeo->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
		_pWireLineGeo->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		return _pWireLineGeo;
	}

	osg::Geode * COsgIdealChannel::DrawChannelFace( osg::Vec3dArray * _pVertexLeftFace,osg::Vec3dArray * _pVertexRightFace 
		,osg::Vec3dArray * _pVertexTopFace,osg::Vec3dArray * _pVertexBottomFace,int PointCount )
	{
		//进近模式三，面模式
		osg::ref_ptr<osg::Geometry> _pLeftFaceGeome =new osg::Geometry;
		osg::ref_ptr<osg::Geometry> _pRightFaceGeom =new osg::Geometry;
		osg::ref_ptr<osg::Geometry> _pTopFaceGeome =new osg::Geometry;
		osg::ref_ptr<osg::Geometry> _pBottomFaceGeome =new osg::Geometry;

		//存放理想航道四个面的geode节点
		osg::Geode* _pFaceGeo = new osg::Geode ;

		//底面颜色数组
		osg::ref_ptr<osg::Vec4dArray > _pColorBottomFace = new osg::Vec4dArray;
		//左面颜色数组
		osg::ref_ptr<osg::Vec4dArray> _pColorLeftFace = new osg::Vec4dArray ;
		//上面颜色数组
		osg::ref_ptr<osg::Vec4dArray> _pColorTopFace = new osg::Vec4dArray;
		//右面颜色数组
		osg::ref_ptr<osg::Vec4dArray> _pColorRightFace = new osg::Vec4dArray ;

		osg::Vec4d ColorBottomFace(0,1,1,0.5);
		osg::Vec4d ColorLeftFace(1,0,1,0.5);
		osg::Vec4d ColorTopFace(0,1,1,0.5);
		osg::Vec4d ColorRight(1,0,1,0.5);

		for (int i=0;i<PointCount;i++)
		{
			_pColorBottomFace->push_back(ColorBottomFace);
			_pColorLeftFace->push_back(ColorLeftFace);
			_pColorTopFace->push_back(ColorTopFace);
			_pColorRightFace->push_back(ColorRight);
		}
		//底面 
		_pBottomFaceGeome->setColorArray(_pColorBottomFace);
		_pBottomFaceGeome->setColorBinding(osg::Geometry::BIND_PER_VERTEX); 
		_pBottomFaceGeome->setVertexArray(_pVertexBottomFace);

		_pBottomFaceGeome->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,PointCount));

		_pFaceGeo->addDrawable(_pBottomFaceGeome);

		//左面
		_pLeftFaceGeome->setColorArray(_pColorLeftFace);
		_pLeftFaceGeome->setColorBinding(osg::Geometry::BIND_PER_VERTEX); 
		_pLeftFaceGeome->setVertexArray(_pVertexLeftFace);

		_pLeftFaceGeome->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,PointCount));

		_pFaceGeo->addDrawable(_pLeftFaceGeome);

		//上面
		_pTopFaceGeome->setColorArray(_pColorTopFace);
		_pTopFaceGeome->setColorBinding(osg::Geometry::BIND_PER_VERTEX); 
		_pTopFaceGeome->setVertexArray(_pVertexTopFace);

		_pTopFaceGeome->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,PointCount));

		_pFaceGeo->addDrawable(_pTopFaceGeome);

		//右面
		_pRightFaceGeom->setColorArray(_pColorRightFace);
		_pRightFaceGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX); 
		_pRightFaceGeom->setVertexArray(_pVertexRightFace);

		_pRightFaceGeom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,PointCount));

		_pFaceGeo->addDrawable(_pRightFaceGeom);

		_pFaceGeo->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
		_pFaceGeo->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		_pFaceGeo->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
		_pFaceGeo->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		return _pFaceGeo;
	}

	bool COsgIdealChannel::DrawWireframe()
	{
		osg::ref_ptr<osg::Geometry> _pGeometry = new osg::Geometry;

		osg::ref_ptr<osg::Vec3dArray> _pVertexArray =new osg::Vec3dArray;
		osg::ref_ptr<osg::Vec4dArray> _pColorArray = new osg::Vec4dArray;

		//压入绘制线框需要的5个顶点
		_pVertexArray->push_back(LeftBottom);
		_pColorArray->push_back(m_Vec4Color);

		_pVertexArray->push_back(RightBottom);
		_pColorArray->push_back(m_Vec4Color);

		_pVertexArray->push_back(RightTop);
		_pColorArray->push_back(m_Vec4Color);

		_pVertexArray->push_back(LeftTop);
		_pColorArray->push_back(m_Vec4Color);

		_pVertexArray->push_back(LeftBottom);
		_pColorArray->push_back(m_Vec4Color);


		_pGeometry->setColorArray(_pColorArray);
		_pGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		_pGeometry->setVertexArray(_pVertexArray);

		_pGeometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,0,5));

		_pGeometry->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
		_pGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		_pGeometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
		_pGeometry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		m_pWireframeGeode->addDrawable(_pGeometry);

		return true;
	}

	bool COsgIdealChannel::GetAngle( osg::Vec3d First,osg::Vec3d Second,double &YawAngle,double &PitchAngle )
	{
		//记录第一个点转换为世界坐标后的值
		osg::Vec3d FirstXYZ;
		//记录第二个点转换为世界坐标后的值
		osg::Vec3d SecondXYZ;

		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), First,FirstXYZ);
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), Second,SecondXYZ);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//首先计算俯仰角
		if (FeMath::Equal(First.z(),Second.z()))
		{
			PitchAngle =0;
		}
		else
		{
			double distance =(FirstXYZ - SecondXYZ).length();
			double detal = First.z()-Second.z();
			//20140602,w00009
			while (abs(detal)>=abs(distance))
			{
				if (distance>0)
				{
					distance = distance+0.001;
				}
				else
				{
					distance = distance - 0.001;//因为经纬高都是double类型的计算距离等于0时可能是取的近似值。
				}
			}
			PitchAngle = asin(detal/distance);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//计算航向角
		//如果第一个点和第二个点的纬度相同，则航向角为90度或者-90度
		if (FeMath::Equal(First.y(),Second.y()))
		{
			double detal = Second.x()-First.x();//计算经度差
			if (detal<-180||(detal>0&&detal<180))
			{
				//在其右侧180度内
				YawAngle  = osg::DegreesToRadians(-90.0);//逆时针转为正向，所以在右侧时转的角度为负值。
			}
			else
			{
				//在其左侧180度内
				YawAngle = osg::DegreesToRadians(90.0);
			}
		}
		//如果第一个点的经度和第二个点的经度相同则航向角为0度或者180度
		else if (FeMath::Equal(First.x(),Second.x()))
		{
			double detal = Second.y()-First.y();
			if (detal>0)
			{
				YawAngle = osg::DegreesToRadians(0.0);
			}
			else
			{
				YawAngle = osg::DegreesToRadians(180.0);
			}
		}
		//第一个点和第二个点的经度和维度都不同，此种情况下只考虑xy坐标就可以了不用考虑z即只在XOY平面做计算就可以了
		else 
		{
			double detalX = Second.x()-First.x();

			osg::Vec3d Second1 (Second.x(),Second.y(),First.z());
			osg::Vec3d Second1XYZ;
			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), Second1,Second1XYZ);

			double dDistance1=(FirstXYZ - Second1XYZ).length();

			osg::Vec3d three (First.x(),Second.y(),Second1.z());
			osg::Vec3d threeXYZ;
			FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), three,threeXYZ);
			double dDistance2 = (FirstXYZ - threeXYZ).length();
			//2014年6月2号,w00009
			while(abs(dDistance2)>=abs(dDistance1))
			{
				if (dDistance1>0)
				{
					dDistance1=dDistance1+0.001;
				}
				else
				{
					dDistance1=dDistance1-0.001;
				}
			}

			double Angle = acos(dDistance2/dDistance1);
			//如果第二个点在第一个点的右边
			if (detalX<-180||(detalX>0&&detalX<180))
			{
				if (Second.y()>First.y())
				{
					YawAngle = -Angle;
				}
				else
				{
					YawAngle = -(osg::PI-Angle);
				}
			}
			//第二点在第一个点的左侧
			else
			{
				if (Second.y()>First.y())
				{
					YawAngle = Angle;
				}
				else
				{
					YawAngle = osg::PI-Angle;
				}
			}
		}
		return true;
	}

	bool COsgIdealChannel::DrawLockFrame()
	{
		//绘制理想航道四个角的锁定框,仍然是根据四个角的坐标来绘制
		osg::ref_ptr<osg::Geometry> _pLockFrameGoemetry =new osg::Geometry;

		osg::Vec3dArray * _pVertexArray = new osg::Vec3dArray ;
		osg::Vec4dArray * _pColorArray = new osg::Vec4dArray;

		osg::ref_ptr<osg::LineWidth> _pLineWidth = new osg::LineWidth(float(2.0));

		//左下角 锁定框
		_pVertexArray->push_back(osg::Vec3d(LeftBottom.x(),LeftBottom.y(),LeftBottom.z()+100));
		_pVertexArray->push_back(osg::Vec3d(LeftBottom));
		_pVertexArray->push_back(osg::Vec3d(LeftBottom.x()+100,LeftBottom.y(),LeftBottom.z()));


		//右下角锁定框
		_pVertexArray->push_back(osg::Vec3d(RightBottom.x()-100,RightBottom.y(),RightBottom.z()));
		_pVertexArray->push_back(osg::Vec3d(RightBottom));
		_pVertexArray->push_back(osg::Vec3d(RightBottom.x(),RightBottom.y(),RightBottom.z()+100));

		//右上角锁定框
		_pVertexArray->push_back(osg::Vec3d(RightTop.x(),RightTop.y(),RightTop.z()-100));
		_pVertexArray->push_back(RightTop);
		_pVertexArray->push_back(osg::Vec3d(RightTop.x()-100,RightTop.y(),RightTop.z()));

		//左上角锁定框
		_pVertexArray->push_back(osg::Vec3d(LeftTop.x()+100,LeftTop.y(),LeftTop.z()));
		_pVertexArray->push_back(LeftTop);
		_pVertexArray->push_back(osg::Vec3d(LeftTop.x(),LeftTop.y(),LeftTop.z()-100));

		for (int i=0;i<12;i++)
		{
			_pColorArray->push_back(m_Vec4Color);
		}

		_pLockFrameGoemetry->setColorArray(_pColorArray);
		_pLockFrameGoemetry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

		_pLockFrameGoemetry->setVertexArray(_pVertexArray);

		_pLockFrameGoemetry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,0,3));
		_pLockFrameGoemetry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,3,3));
		_pLockFrameGoemetry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,6,3));
		_pLockFrameGoemetry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP,9,3));

		_pLockFrameGoemetry->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
		_pLockFrameGoemetry->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
		_pLockFrameGoemetry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
		_pLockFrameGoemetry->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		_pLockFrameGoemetry->getOrCreateStateSet()->setAttribute(_pLineWidth.get(),osg::StateAttribute::ON);

		m_pLockFrameGeode->addDrawable(_pLockFrameGoemetry);

		return true;
	}

	void COsgIdealChannel::getFlyPosture(osg::Vec3d first,osg::Vec3d second,double& xAngleFuYang,double& yAngleHengGun,double& zAngleHangXiang)
	{
		osg::Vec3d firstXYZ;
		osg::Vec3d secondXYZ;
		yAngleHengGun = 0.0; //横滚角暂不计算

		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), first, firstXYZ);
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), second, secondXYZ);
		/////////////////////////////////////////////////////
		//入参为经纬高，出参为弧度，first不能与second相等
		/////////////////////////////////////////////////////
		//首先计算俯仰角，如果first和second的高度相同，我们认为其俯仰角为0
		if(FeMath::Equal(first.z(), second.z()))
		{
			xAngleFuYang = 0.0;
		}
		else
		{
			//高度不同，根据常识，俯仰都不会超过90度，根据高度差与两点之间的距离的反正弦可计算得
			double dis_first_second = (firstXYZ - secondXYZ).length();
			double delta_height = second.z() - first.z();

			while(abs(delta_height) >= abs(dis_first_second))
			{
				if(dis_first_second > 0)
				{
					dis_first_second += 0.001;
				}
				else
				{
					dis_first_second -= 0.001;
				}
			}

			xAngleFuYang = asin(delta_height/dis_first_second);
		}

		/////////////////////////////////////////////////////
		//其次计算HangXiang，计算航向与高度没有关系，因此将高度调整为一致
		second.set(second.x(), second.y(), first.z());
		FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), second, secondXYZ);

		//1、如果其纬度相同，则看其经度，若second在first右侧，则航向为-90，在左侧为90，不可能重合。
		//判断second的方向性
		if(FeMath::Equal(first.y(), second.y()))
		{
			double deltaX = second.x() - first.x();
			if((deltaX <-180) || ((deltaX > 0) && (deltaX < 180)))
			{
				//在其右侧
				zAngleHangXiang = osg::DegreesToRadians(-90.0);
			}
			else
			{
				zAngleHangXiang = osg::DegreesToRadians(90.0);
			}
		}
		else
		{
			//2、如果经度相同，则看其纬度，则second在first上侧，则航向角为0，在下侧则航向角为180度
			if(FeMath::Equal(first.x(), second.x()))
			{
				double deltaY = second.y() - first.y();
				if(deltaY > 0) //123.61 25.89
				{
					zAngleHangXiang = 0.0;
				}
				else
				{
					zAngleHangXiang = osg::DegreesToRadians(180.0);
				}
			}
			else
			{
				//3、经度与纬度都不相同，则要需要计算实际角度
				//斜边
				double dis_first_second = (firstXYZ - secondXYZ).length();
				//cos的直角边，将second的经度设置的与first相同
				osg::Vec3d secondJingdu = osg::Vec3d(first.x(), second.y(), second.z());
				osg::Vec3d secondJingduXYZ;
				FeUtil::DegreeLLH2XYZ(m_opRenderContext.get(), secondJingdu, secondJingduXYZ);
				double dis_first_secondJingdu =(firstXYZ - secondJingduXYZ).length();


				while(abs(dis_first_secondJingdu) >= abs(dis_first_second))
				{
					if(dis_first_second > 0)
					{
						dis_first_second += 0.001;
					}
					else
					{
						dis_first_second -= 0.001;
					}
				}

				double angle = acos(dis_first_secondJingdu/dis_first_second);

				//分四个象限来分别得出最终的zAngleHangXiang
				//以second在first左右分为一个if
				double deltaX = second.x() - first.x();
				if((deltaX <-180) || ((deltaX > 0) && (deltaX < 180)))
				{
					//下面为第一第四象限
					//如果为第一象限
					if(second.y() > first.y())
					{
						zAngleHangXiang = -angle;
					}
					else
					{
						zAngleHangXiang = angle-osg::PI/*osg::PI + angle*/;
					}
				}
				else
				{
					//下面为第二第三象限
					//如果为第二象限
					if(second.y() > first.y())
					{
						zAngleHangXiang = angle;
					}
					else
					{
						zAngleHangXiang = osg::PI - angle;
					}
				}
			}
		}
	}

	bool COsgIdealChannel::CloseIdealChannel()
	{
		m_pApproachMode1->setNodeMask(0x00);
		m_pApproachMode2->setNodeMask(0x00);
		m_pApproachMode3->setNodeMask(0x00);
		m_pRoot->setNodeMask(0x00);
		return true;
	}

	bool COsgIdealChannel::OpenIdealChannelMode1()
	{
		m_pApproachMode1->setNodeMask(~0x00);
		m_pApproachMode2->setNodeMask(0x00);
		m_pApproachMode3->setNodeMask(0x00);
		m_pRoot->setNodeMask(~0x00);
		return true;
	}

	bool COsgIdealChannel::OpenIdealChannelMode2()
	{
		m_pApproachMode1->setNodeMask(0x00);
		m_pApproachMode2->setNodeMask(~0x00);
		m_pApproachMode3->setNodeMask(0x00);
		m_pRoot->setNodeMask(~0x00);
		return true;
	}

	bool COsgIdealChannel::OpenIdealChannelMode3()
	{
		m_pApproachMode1->setNodeMask(0x00);
		m_pApproachMode2->setNodeMask(~0x00);
		m_pApproachMode3->setNodeMask(~0x00);
		m_pRoot->setNodeMask(~0x00);
		return true;
	}

	void COsgIdealChannel::ComputerBLine( osg::Vec4d startPoint, osg::Vec4d endPoint, osg::Vec4d controlPoint
		,osg::Vec4dArray *outArray,int insertPointNum )
	{
		if (outArray == NULL)
		{
			return;
		}
		double t = 0.0;
		double delta_t = 1.0/ (float)insertPointNum;
		double x = 0.0, y = 0.0, z = 0.0,w=0.0;

		for (int n=0 ; n <=insertPointNum; n++)
		{
			x = (1-t)*(1-t)*startPoint.x() + 2*(1-t)*t*controlPoint.x() + t*t*endPoint.x();
			y = (1-t)*(1-t)*startPoint.y() + 2*(1-t)*t*controlPoint.y() + t*t*endPoint.y();
			z = /*(1-t)*(1-t)*startPoint.z() + 2*(1-t)*t*controlPoint.z() + t*t*endPoint.z()*/startPoint.z();
			w = startPoint.w();
			outArray->push_back(osg::Vec4d(x,y,z,w));
			t += delta_t;
		}
	}

	bool COsgIdealChannel::ReDrawIdealChannelForApproach( osg::Vec4dArray* pKeysVertex )
	{
		if (NULL==pKeysVertex)
		{
			return false;
		}
		//线框之间的连线之一左下角连线的顶点数组
		osg::Vec3dArray * _pVertexLeftBottomLine = new osg::Vec3dArray;
		//线框之间的连线之一右下角连线的顶点数组
		osg::Vec3dArray * _pVertexRightBottomLine = new osg::Vec3dArray;
		//线框之间的连线之一右上角连线的顶点数组
		osg::Vec3dArray * _pVertexRightTopLine =new osg::Vec3dArray;
		//线框之间的连线之一左上角的连线的顶点数组
		osg::Vec3dArray * _PVertexLeftTopLine = new osg::Vec3dArray;

		//绘制面所需顶点的数组
		osg::Vec3dArray * _pVertexFaceLeft = new osg::Vec3dArray;
		//绘制面所需顶点的数组
		osg::Vec3dArray * _pVertexFaceRight = new osg::Vec3dArray;
		//绘制面所需顶点的数组
		osg::Vec3dArray * _pVertexFaceTop = new osg::Vec3dArray;
		//绘制面所需顶点的数组
		osg::Vec3dArray * _pVertexFaceBottom = new osg::Vec3dArray;

		///存放所有线框的Group节点
		osg::ref_ptr<osg::Group> _pAllWireframeGroup = new osg::Group;
		///存放所有锁定框的Group节点
		osg::ref_ptr<osg::Group> _pAllLockFrameGroup = new osg::Group;

		osg::Vec4dArray* _pInterpolation = new osg::Vec4dArray;
		///对关键点进行第一次插值
		FeMath::Interpolation(pKeysVertex,_pInterpolation);

		//记录理想通道四个角的连线的点的个数,同时根据count的值算出画上下左右四个面的点的个数
		int count = 0;	
		for (osg::Vec4dArray::iterator iter= _pInterpolation->begin();iter!=_pInterpolation->end();iter++)
		{
			//通道进近模式一用到的平移变换矩阵，变换矩阵将组成模式一的线框移动到地球表面
			osg::ref_ptr<osg::MatrixTransform> _pTranslateMode1  = new osg::MatrixTransform;
			//通道进近模式一用到的旋转变换矩阵
			osg::ref_ptr<osg::MatrixTransform> _pRotateMode1 = new osg::MatrixTransform;
			//通道进近模式二用到的平移变换矩阵，变换矩阵将组成模式二的线框移动到地球表面
			osg::ref_ptr<osg::MatrixTransform> _pTranslateMode2  = new osg::MatrixTransform;
			//通道进近模式二用到的旋转变换矩阵
			osg::ref_ptr<osg::MatrixTransform> _pRotateMode2 = new osg::MatrixTransform;

			_pRotateMode1->addChild(m_pWireframeGeode);
			_pRotateMode2->addChild(m_pLockFrameGeode);
			_pTranslateMode1->addChild(_pRotateMode1); 
			_pTranslateMode2->addChild(_pRotateMode2);

			_pAllWireframeGroup->addChild(_pTranslateMode1);
			_pAllLockFrameGroup->addChild(_pTranslateMode2);

			//获得奖线框从世界坐标的原点移动到地球表面所需的变换矩阵
			osg::Matrix matrix;
			FeUtil::DegreeLLH2Matrix(m_opRenderContext.get(), osg::Vec3d(iter->x(),iter->y(),iter->z()),matrix);

			_pTranslateMode1->setMatrix(matrix);
			_pTranslateMode2->setMatrix(matrix);

			//获得线框从原点移到地表后旋转所需的航向角和俯仰角
			double YawAngle=0.0;//航向角
			double PitchAngle=0.0;//俯仰角
			double RollAngle=0.0;
			osg::Vec3d first(iter->x(),iter->y(),iter->z());
			if (iter+1!=_pInterpolation->end())
			{
				osg::Vec3d second((iter+1)->x(),(iter+1)->y(),(iter+1)->z());
				GetAngle(first,second,YawAngle,PitchAngle);

				//getFlyPosture(first,second,PitchAngle,RollAngle,YawAngle);
			}
			else
			{
				osg::Vec3d second((iter-1)->x(),(iter-1)->y(),(iter-1)->z());
				GetAngle(first,second,YawAngle,PitchAngle);
				YawAngle=-YawAngle;
				//RollAngle=osg::PI;
			}
			osg::Matrix RotateMatrix = osg::Matrix::rotate(PitchAngle,osg::X_AXIS,RollAngle,osg::Y_AXIS,YawAngle,osg::Z_AXIS);
			//做旋转
			_pRotateMode1->setMatrix(RotateMatrix);
			_pRotateMode2->setMatrix(RotateMatrix);

			///线框被移动到地球上后，四个角顶点的坐标
			osg::Vec3d leftbottomline(LeftBottom*RotateMatrix*matrix);
			osg::Vec3d rightbottomline(RightBottom*RotateMatrix* matrix);
			osg::Vec3d righttopline(RightTop*RotateMatrix*matrix);
			osg::Vec3d lefttopline(LeftTop*RotateMatrix*matrix);

			///绘制线框之间的连线需要顶点，下面将绘制线框连线所需顶点压入到响应的数组中
			_pVertexLeftBottomLine->push_back(leftbottomline);
			_pVertexRightBottomLine->push_back(rightbottomline);
			_pVertexRightTopLine->push_back(righttopline);
			_PVertexLeftTopLine->push_back(lefttopline);

			if (count!=0)
			{
				///让如绘制通道四个面所需顶点
				_pVertexFaceLeft->push_back(leftbottomline);
				_pVertexFaceLeft->push_back(lefttopline);
				_pVertexFaceLeft->push_back(lefttopline);
				_pVertexFaceLeft->push_back(leftbottomline);

				_pVertexFaceRight->push_back(rightbottomline);
				_pVertexFaceRight->push_back(righttopline);
				_pVertexFaceRight->push_back(righttopline);
				_pVertexFaceRight->push_back(rightbottomline);

				_pVertexFaceTop->push_back(righttopline);
				_pVertexFaceTop->push_back(lefttopline);
				_pVertexFaceTop->push_back(lefttopline);
				_pVertexFaceTop->push_back(righttopline);

				_pVertexFaceBottom->push_back(rightbottomline);
				_pVertexFaceBottom->push_back(leftbottomline);
				_pVertexFaceBottom->push_back(leftbottomline);
				_pVertexFaceBottom->push_back(rightbottomline);	

			}
			else 
			{
				///让如绘制通道四个面所需顶点
				_pVertexFaceLeft->push_back(lefttopline);
				_pVertexFaceLeft->push_back(leftbottomline);

				_pVertexFaceRight->push_back(righttopline);
				_pVertexFaceRight->push_back(rightbottomline);

				_pVertexFaceTop->push_back(lefttopline);
				_pVertexFaceTop->push_back(righttopline);

				_pVertexFaceBottom->push_back(leftbottomline);
				_pVertexFaceBottom->push_back(rightbottomline);
			}
			count++;
		}
		osg::Geode* _pWireLineGeo =DrawWireframeLigature(_pVertexLeftBottomLine,_PVertexLeftTopLine
			,_pVertexRightBottomLine,_pVertexRightTopLine,count);
		osg::Geode* _pFaceGeo =DrawChannelFace(_pVertexFaceLeft,_pVertexFaceRight,_pVertexFaceTop,
			_pVertexFaceBottom,count*4-2);

		//理想通道模式一的Group节点,此模式中有线框及连接线框的四条线组成
		m_pApproachMode1->removeChildren(0,m_pApproachMode1->getNumChildren());
		m_pApproachMode1->addChild(_pAllWireframeGroup);
		m_pApproachMode1->addChild(_pWireLineGeo);
		//理想通道模式二的Group节点,此模式中只有锁定框
		m_pApproachMode2->removeChildren(0,m_pApproachMode2->getNumChildren());
		m_pApproachMode2->addChild(_pAllLockFrameGroup);
		//理想通道模式二的Group节点,此模式中含有锁定框、四条连线、四个面
		m_pApproachMode3->removeChildren(0,m_pApproachMode3->getNumChildren());
		m_pApproachMode3->addChild(_pAllLockFrameGroup);
		m_pApproachMode3->addChild(_pWireLineGeo);
		m_pApproachMode3->addChild(_pFaceGeo);

		m_pApproachMode1->setNodeMask(0x00);
		m_pApproachMode2->setNodeMask(0x00);
		m_pApproachMode3->setNodeMask(0x00);

		return true;
	}

	osg::Vec4dArray * COsgIdealChannel::getArrayVertex()
	{
		return m_pArrayVertex;
	}

}