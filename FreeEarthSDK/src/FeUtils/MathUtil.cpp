#include <string>
#include <algorithm>
#include <ctype.h>

#include <osgDB/ConvertUTF>
#include <osg/TriangleFunctor>
#include <osg/MatrixTransform>
#include <osgEarthDrivers/engine_mp/MPGeometry>
#include <osgEarth/GeoMath>

#include <float.h>
#include <MathUtil.h>

#include <FeUtils/CoordConverter.h>

namespace FeMath
{

	bool LinearInterpolation(const osg::Vec3d& first, const osg::Vec3d& last, osg::Vec3dArray * pOutArray,double dDelta /*= 0.02*/ )
	{
		if(!pOutArray)
		{
			return false;
		}

		if(EqualZero(dDelta))
		{
			return false;
		}

		double dDistance = (last - first).length();
		if(dDistance <= dDelta)
		{
			return false;
		}

		double dLon(0.0), dLat(0.0), dHei(0.0);
		int MAX = dDistance/dDelta;
		double dDeltaIndex = 1.0 / MAX;
		double dT(0.0);

		for(int nIndex = 1; nIndex < MAX; ++nIndex)
		{
			dT = nIndex * dDeltaIndex;

			dLon = last.x() * dT + (1-dT) * first.x();
			dLat = last.y() * dT + (1-dT) * first.y();
			dHei = last.z() * dT + (1-dT) * first.z();

			pOutArray->push_back(osg::Vec3d(dLon, dLat, dHei));
		}

		return true;
	}

	bool LinearInterpolation( const osg::Vec3dArray* pVertexs,osg::Vec3dArray* pOutArray,double delta /*= 0.02*/ )
	{
		if(pVertexs && pOutArray) 
		{
			if(pVertexs->size() <= 0)
			{
				return false;
			}
			unsigned unVertexNum = pVertexs->size() - 1;
			for(unsigned unIndex = 0; unIndex != unVertexNum; ++unIndex)
			{
				osg::Vec3 vecLLaFrom = osg::Vec3(pVertexs->at(unIndex));
				osg::Vec3 vecLLaTo = osg::Vec3(pVertexs->at(unIndex+1));

				pOutArray->push_back(vecLLaFrom);

				osg::Vec3d vecTemp;

				if(vecLLaFrom.x() - vecLLaTo.x() > 180.0)
				{
					vecTemp.x() = 180.0;
					vecTemp.y() = vecLLaFrom.y() - (vecLLaFrom.x() - 180.0)/(vecLLaFrom.x() - vecLLaTo.x())*(vecLLaFrom.y()- vecLLaTo.y());
					vecTemp.z() = vecLLaFrom.z() - (vecLLaFrom.x() - 180.0)/(vecLLaFrom.x() - vecLLaTo.x())*(vecLLaFrom.z()- vecLLaTo.z());
					LinearInterpolation(vecLLaFrom,vecTemp,pOutArray,delta);
					vecTemp.x() = -180.0;
					LinearInterpolation(vecTemp,vecLLaTo,pOutArray,delta);
				}
				else if(vecLLaFrom.x() - vecLLaTo.x() < -180)
				{
					vecTemp.x() = -180.0;
					vecTemp.y() = vecLLaFrom.y() - (vecLLaFrom.x() + 180.0)/(vecLLaFrom.x() - vecLLaTo.x())*(vecLLaFrom.y()- vecLLaTo.y());
					vecTemp.z() = vecLLaFrom.z() - (vecLLaFrom.x() + 180.0)/(vecLLaFrom.x() - vecLLaTo.x())*(vecLLaFrom.z()- vecLLaTo.z());
					LinearInterpolation(vecLLaFrom,vecTemp,pOutArray,delta);
					vecTemp.x() = 180.0;
					LinearInterpolation(vecTemp,vecLLaTo,pOutArray,delta);
				}
				else
				{
					LinearInterpolation(vecLLaFrom,vecLLaTo,pOutArray,delta);
				}


				//NeighbourInter(vecLLaFrom,vecLLaTo,depedent);
			}

			osg::Vec3 vecPoint = osg::Vec3(pVertexs->at(unVertexNum));
			pOutArray->push_back(vecPoint);
		}

		return false;
	}

	void Interpolation( const osg::Vec3d first, const osg::Vec3d last,osg::Vec3dArray * outArray,double delta )
	{
		if(EqualZero(delta))
		{
			return;
		}

		outArray->push_back(first);

		//整体经度加180.0.让经度原来的区间[-180.0--180.0]变成[0.0--360.0]
		//整体纬度加90.0.让经度原来的区间[-90.0--90.0]变成[0.0--180.0]
		osg::Vec3d firstTemp(first.x()+180.0,first.y()+90.0,first.z());
		osg::Vec3d lastTemp(last.x()+180.0,last.y()+90.0,last.z());
		//计算两点之间经、纬、高的差值
		double deltaLon = lastTemp.x() - firstTemp.x();
		double deltaLat = lastTemp.y() - firstTemp.y();
		double deltaHei = lastTemp.z() - firstTemp.z();
		//
		double restDelta = 0.0;
		double deltaX = 0.0;
		double deltaY = 0.0;
		double deltaZ = 0.0;
		int numDelta = 0;


		if( deltaLon >= 180.0 )
		{
			//两个点的经度一个正值，一个是负值，即lastTemp原先的经度值为正值，firstTemp原先的经度值为负值，它们之间的实际经度的差值如下：
			deltaLon = deltaLon - 360.0 ;
		}
		else if ( deltaLon < -180.0 )
		{
			//两个点的经度一个正值，一个是负值，即lastTemp原先的经度值为负值，firstTemp原先的经度值为正值，它们之间的实际经度的差值如下：
			deltaLon = deltaLon + 360.0 ;
		}
		if (deltaLat>=90)
		{
			deltaLat=deltaLat-180;
		}
		if (deltaLat<-90)
		{
			deltaLat=deltaLat+180;
		}
		//
		if( abs(deltaLon) > 2*abs(deltaLat) )
		{
			int iFlagLon = abs(deltaLon)*100000;
			int iFlagDelta = abs(delta)*100000;

			if (iFlagLon<iFlagDelta)
			{
				return;
			}
			//delta的正负号由deltaLon确定
			delta = abs(delta) * deltaLon/(abs(deltaLon));
			//计算需要插入点的个数，numDelta的正负号由delta确定，即delta与numDelta正负号相同
			numDelta = (int)(abs(deltaLon) / delta);
			//因为numDelta为int型，所以((abs(deltaLon) - delta* numDelta)的值可能不为0；
			//经过（(abs(deltaLon) - delta* numDelta)/numDelta）计算后得出总的偏差应该分到每个delta区间的值是多少
			//后面乘以(delta/abs(delta))是为了保证restDelta与delta正负号相同
			restDelta = ((abs(deltaLon) - delta* numDelta)/numDelta)*(delta/abs(delta));
			//得到插值时X轴方向每次应该增加或者减少的值
			deltaX = delta + restDelta;
			//得到插值时Y轴方向每次应该增加或者减少的值
			deltaY = deltaLat/abs(numDelta);
			//得到插值时Z轴方向每次应该增加或者减少的值
			deltaZ = deltaHei/abs(numDelta);
		}
		else if(abs(deltaLon) < 2*abs(deltaLat))
		{
			int iFlagLat = abs(deltaLon)*100000;
			int iFlagDelta = abs(delta)*100000;

			if (iFlagLat<iFlagDelta)
			{
				return;
			}
			//delta的正负号由deltaLat确定
			delta = abs(delta) * deltaLat/(abs(deltaLat));
			//计算需要插入点的个数，numDelta的正负号由delta确定，即delta与numDelta正负号相同
			numDelta = (int)(abs(deltaLat) / delta);
			//因为numDelta为int型，所以(abs(deltaLat) - delta* numDelta)的值可能不为0, 不为0时就出现了偏差；
			//经过((abs(deltaLat) - delta* numDelta)/numDelta)计算后得出总的偏差应该分到每个delta区间的值是多少
			//后面乘以(delta/abs(delta))是为了保证restDelta与delta正负号相同
			restDelta = ((abs(deltaLat) - delta* numDelta)/numDelta) * (delta/abs(delta));
			//得到插值时Y轴方向每次应该增加或者减少的值
			deltaY = delta + restDelta;
			//得到插值时X轴方向每次应该增加或者减少的值
			deltaX = deltaLon/abs(numDelta);
			//得到插值时Z轴方向每次应该增加或者减少的值
			deltaZ = deltaHei/abs(numDelta);
		}
		else 
		{
			return;
		}

		//
		for(int n = 1 ; n < abs(numDelta) ; n++)
		{
			//
			osg::Vec3d temp(firstTemp.x()+deltaX*n,firstTemp.y()+deltaY*n,firstTemp.z()+deltaZ*n);
			//因为(firstTemp.x()+deltaX*numDelta)=lastTemp.x()不是恒成立的，即很多时候是不等的所以temp.x()>360.0是有可能成立的
			if(temp.x() >= 360.0)
			{
				temp.set(temp.x()-360.0 ,temp.y() ,temp.z());
			}
			//因为(firstTemp.x()+deltaX*numDelta)=lastTemp.x()不是恒成立的，即很多时候是不等的所以temp.x()<0.0是有可能成立的
			else if(temp.x() <= -0.0 )
			{
				temp.set(360.0 + temp.x() ,temp.y() ,temp.z());
			}
			if (temp.y()>=180)
			{
				temp.set(temp.x(),temp.y()-180,temp.z());
			}
			else if (temp.y()<-0.0)
			{
				temp.set(temp.x(),180+temp.y(),temp.z());
			}
			temp.set(temp.x() - 180.0 ,temp.y() - 90.0 ,temp.z());
			///2014年1月10号添加
			/*根据temp的x（经度）和y（纬度）得到当前经纬度确定的点的海拔高度（高层值）dCurrentHeight,
			比较dCurrentHeight与temp中z（高度）的大小，如果dCurrentHeight小于temp中z（高度）的大小，不做任何处理，
			如果dCurrentHeight的高度大于temp中z（高度），这时需要temp中z（高度）置为dCurrentHeight的值，否则画的航线就会有部分线段在地面下面导致航线不连续*/  
			//以下方法由经纬度得到的高度准确，推荐使用此方法
			//  		double dCurrentHeight =0;
			//   	    osgEarth::GeoPoint geop(CosgViewer::Instance()->getEarthMapNode()->getMapSRS(),temp.x(),temp.y());
			//   	    CosgViewer::Instance()->getElevationQuery()->getElevation(geop,dCurrentHeight);
			//  		if (dCurrentHeight>=temp.z())
			//  		{
			//  			temp.set(temp.x(),temp.y(),dCurrentHeight+300);
			// 		     }
			outArray->push_back(temp);
		}
	}

	void Interpolation( const osg::Vec3dArray* vertexs,osg::Vec3dArray* outArray,double delta)
	{
		outArray->clear();
		int num = vertexs->size();
		if( num <2)
		{
			return;
		}
		for( int n = 1 ; n < num ; n++ )
		{
			Interpolation(vertexs->at(n-1),vertexs->at(n),outArray);
		}
		outArray->push_back(vertexs->at(num-1));
	}

	void Interpolation( const osg::Vec4d first,const osg::Vec4d last,osg::Vec4dArray* outArray,double delta /*= 0.02*/ )
{
	outArray->push_back(first);

	//整体经度加180.0.让经度原来的区间[-180.0--180.0]变成[0.0--360.0]
	//整体纬度加90.0.让经度原来的区间[-90.0--90.0]变成[0.0--180.0]
	osg::Vec3d firstTemp(first.x()+180.0,first.y()+90.0,first.z());
	osg::Vec3d lastTemp(last.x()+180.0,last.y()+90.0,last.z());
	//计算两点之间经、纬、高的差值
	double deltaLon = lastTemp.x() - firstTemp.x();
	double deltaLat = lastTemp.y() - firstTemp.y();
	double deltaHei = lastTemp.z() - firstTemp.z();
	//
	double restDelta = 0.0;
	//
	double deltaX = 0.0;
	double deltaY = 0.0;
	double deltaZ = 0.0;

	//
	int numDelta = 0;

	
	if( deltaLon >= 180.0 )
	{
		//两个点的经度一个正值，一个是负值，即lastTemp原先的经度值为正值，firstTemp原先的经度值为负值，它们之间的实际经度的差值如下：
		deltaLon = deltaLon - 360.0 ;
	}
	else if ( deltaLon < -180.0 )
	{
		//两个点的经度一个正值，一个是负值，即lastTemp原先的经度值为负值，firstTemp原先的经度值为正值，它们之间的实际经度的差值如下：
		deltaLon = deltaLon + 360.0 ;
	}
	if (deltaLat>=90)
	{
		deltaLat=deltaLat-180;
	}
	if (deltaLat<-90)
	{
		deltaLat=deltaLat+180;
	}
	//
	if( abs(deltaLon) > 2*abs(deltaLat) )
	{
		int iFlagLon = abs(deltaLon)*100000;
		int iFlagDelta = abs(delta)*100000;

		if (iFlagLon<iFlagDelta)
		{
			return;
		}
		//delta的正负号由deltaLon确定
		delta = abs(delta) * deltaLon/(abs(deltaLon));
		//计算需要插入点的个数，numDelta的正负号由delta确定，即delta与numDelta正负号相同
		numDelta = (int)(abs(deltaLon) / delta);
		//因为numDelta为int型，所以((abs(deltaLon) - delta* numDelta)的值可能不为0；
		//经过（(abs(deltaLon) - delta* numDelta)/numDelta）计算后得出总的偏差应该分到每个delta区间的值是多少
		//后面乘以(delta/abs(delta))是为了保证restDelta与delta正负号相同
		restDelta = ((abs(deltaLon) - delta* numDelta)/numDelta)*(delta/abs(delta));
		//得到插值时X轴方向每次应该增加或者减少的值
		deltaX = delta + restDelta;
		//得到插值时Y轴方向每次应该增加或者减少的值
		deltaY = deltaLat/abs(numDelta);
		//得到插值时Z轴方向每次应该增加或者减少的值
		deltaZ = deltaHei/abs(numDelta);
	}
	else if(abs(deltaLon) < 2*abs(deltaLat))
	{
		int iFlagLat = abs(deltaLon)*100000;
		int iFlagDelta = abs(delta)*100000;

		if (iFlagLat<iFlagDelta)
		{
			return;
		}
		//delta的正负号由deltaLat确定
		delta = abs(delta) * deltaLat/(abs(deltaLat));
		//计算需要插入点的个数，numDelta的正负号由delta确定，即delta与numDelta正负号相同
		numDelta = (int)(abs(deltaLat) / delta);
		//因为numDelta为int型，所以(abs(deltaLat) - delta* numDelta)的值可能不为0, 不为0时就出现了偏差；
		//经过((abs(deltaLat) - delta* numDelta)/numDelta)计算后得出总的偏差应该分到每个delta区间的值是多少
		//后面乘以(delta/abs(delta))是为了保证restDelta与delta正负号相同
		restDelta = ((abs(deltaLat) - delta* numDelta)/numDelta) * (delta/abs(delta));
		//得到插值时Y轴方向每次应该增加或者减少的值
		deltaY = delta + restDelta;
		//得到插值时X轴方向每次应该增加或者减少的值
		deltaX = deltaLon/abs(numDelta);
		//得到插值时Z轴方向每次应该增加或者减少的值
		deltaZ = deltaHei/abs(numDelta);
	}
	else 
	{
		return;
	}

	//
	for(int n = 1 ; n < abs(numDelta) ; n++)
	{
		//
		osg::Vec4d temp(firstTemp.x()+deltaX*n,firstTemp.y()+deltaY*n,firstTemp.z()+deltaZ*n,first.w());
		//因为(firstTemp.x()+deltaX*numDelta)=lastTemp.x()不是恒成立的，即很多时候是不等的所以temp.x()>360.0是有可能成立的
		if(temp.x() >= 360.0)
		{
			temp.set(temp.x()-360.0 ,temp.y() ,temp.z(),temp.w());
		}
		//因为(firstTemp.x()+deltaX*numDelta)=lastTemp.x()不是恒成立的，即很多时候是不等的所以temp.x()<0.0是有可能成立的
		else if(temp.x() <= -0.0 )
		{
			temp.set(360.0 + temp.x() ,temp.y() ,temp.z(),temp.w());
		}
		if (temp.y()>=180)
		{
			temp.set(temp.x(),temp.y()-180,temp.z(),temp.w());
		}
		else if (temp.y()<-0.0)
		{
			temp.set(temp.x(),180+temp.y(),temp.z(),temp.w());
		}
		temp.set(temp.x() - 180.0 ,temp.y() - 90.0 ,temp.z(),temp.w());
		///2014年1月10号添加
		/*根据temp的x（经度）和y（纬度）得到当前经纬度确定的点的海拔高度（高层值）dCurrentHeight,
		比较dCurrentHeight与temp中z（高度）的大小，如果dCurrentHeight小于temp中z（高度）的大小，不做任何处理，
		如果dCurrentHeight的高度大于temp中z（高度），这时需要temp中z（高度）置为dCurrentHeight的值，否则画的航线就会有部分线段在地面下面导致航线不连续*/  
		//以下方法由经纬度得到的高度准确，推荐使用此方法
// 		double dCurrentHeight =0;
//   	    osgEarth::GeoPoint geop(CosgViewer::Instance()->getEarthMapNode()->getMapSRS(),temp.x(),temp.y());
//   	    CosgViewer::Instance()->getElevationQuery()->getElevation(geop,dCurrentHeight);
// 		if (dCurrentHeight>=temp.z())
// 		{
// 			temp.set(temp.x(),temp.y(),dCurrentHeight,temp.w());
// 		}
	  	outArray->push_back(temp);
	}
}

	void Interpolation( const osg::Vec4dArray* vertexs,osg::Vec4dArray* outArray,double delta /*= 0.02*/ )
{
	//
	outArray->clear();
	int num = vertexs->size();
	if( num <2)
	{
		return;
	}
	//
	for( int n = 1 ; n < num ; n++ )
	{
		//outArray->push_back(vertexs->at(n-1));
		Interpolation(vertexs->at(n-1),vertexs->at(n),outArray);
	}
	//
	outArray->push_back(vertexs->at(num-1));
}

}


namespace FeMath
{

	bool ContainPointInPolygon( osg::Vec3d point, osg::Vec3dArray* pArray )
	{
		if (pArray == NULL)
		{
			return false;
		}

		int nCrossings = 0;
		for (int i = 0; i < pArray->size(); i++)
		{
			osg::Vec3d first = pArray->at(i%pArray->size());
			osg::Vec3d second = pArray->at((i+1)%pArray->size());

			double dSlope = (second.y()-first.y())/(second.x() - first.x());
			bool bCond1 = (first.x() <= point.x()) && (point.x() < second.x());
			bool bCond2 = (second.x() <= point.x()) && (point.x() < first.x());
			bool bAbove = (point.y() < dSlope*(point.x()-first.x())+first.y());

			if ((bCond1 || bCond2) && bAbove)
			{
				nCrossings++;
			}
		}

		return (nCrossings % 2 != 0);
	}

	double PolygonArea( osg::Vec3dArray* pArray )
	{
		if (NULL == pArray)
		{
			return false;
		}

		int nCycle = 0, nCount = 0;
		double dArea = 0.0;

		nCount=pArray->size();

		for(nCycle = 0; nCycle < nCount; nCycle++)
		{    
			osg::Vec3d first = pArray->at(nCycle);
			osg::Vec3d second = pArray->at((nCycle+1)%nCount);

			dArea += first.x() * second.y() - second.x() * first.y();
		}

		return abs(0.5*dArea);
	}

	double ComputePolygonSpaceArea(osg::Vec3dArray* pArray)
	{
		if (pArray->size() <= 2)
		{
			return 0.0;
		}
		else
		{
			double dArea = 0.0;
			osg::ref_ptr<osg::Vec3dArray> array = new osg::Vec3dArray;
			if (pArray->size()%2 == 0) //偶数
			{
				int i = 1;
				for (i=1; i<pArray->size()-1;)
				{
					osg::Vec3d vecOA = pArray->at(i-1) - pArray->at(i);
					osg::Vec3d vecOB = pArray->at(i+1) - pArray->at(i);
					dArea += ComputeCross(vecOA, vecOB);
					array->push_back(pArray->at(i-1));
					i += 2;
				}
				osg::Vec3d vecOA = pArray->at(i-1) - pArray->at(i);
				osg::Vec3d vecOB = pArray->front() - pArray->at(i);
				dArea += ComputeCross(vecOA, vecOB);
				array->push_back(pArray->at(i-1));
				dArea += ComputePolygonSpaceArea(array);
			}
			else //奇数
			{
				int i = 1;
				for (i=1; i<pArray->size()-1;)
				{
					osg::Vec3d vecOA = pArray->at(i-1) - pArray->at(i);
					osg::Vec3d vecOB = pArray->at(i+1) - pArray->at(i);
					dArea += ComputeCross(vecOA, vecOB);
					array->push_back(pArray->at(i-1));
					i += 2;
				}
				array->push_back(pArray->at(i-1));
				dArea += ComputePolygonSpaceArea(array);
			}

			return abs(0.5*dArea);
		}
	}

	double ComputeCross(const osg::Vec3d &vecA, const osg::Vec3d &vecB)
	{
		double resTemp = vecA.y()*vecB.z() - vecA.z()*vecB.y()
						+ vecA.z()*vecB.x() - vecA.x()*vecB.z()
						+ vecA.x()*vecB.y() - vecA.y()*vecB.x();

		if (resTemp > 0) //凹顶点
		{
			osg::Vec3d vecTemp = vecA ^ vecB;

			return -sqrt(vecTemp.x()*vecTemp.x()
				+ vecTemp.y()*vecTemp.y()
				+ vecTemp.z()*vecTemp.z());
		}
		else //凸顶点
		{
			osg::Vec3d vecTemp = vecA ^ vecB;

			return sqrt(vecTemp.x()*vecTemp.x()
				+ vecTemp.y()*vecTemp.y()
				+ vecTemp.z()*vecTemp.z());
		}
		
	}

	double ComputePolygonShadowArea(const double *padX,const double* padY,int nCount)
	{
		double x1, y1, dx, dy;  
		double Qbar1, Qbar2;
		double Two_PI = osg::PI + osg::PI;
		double dSemiMajor = 6378140.0; //长半轴
		double dSemiMinor = 6356755.0; //短半轴
		double a2 = ( dSemiMajor * dSemiMajor );  
		double e2 = 1 - ( a2 / ( dSemiMinor * dSemiMinor ) ); 
		double AE = a2 * ( 1 - e2 );  

		double Qp = GetQ(osg::PI / 2);
		double E = 4 * osg::PI * Qp * AE;
		E = abs(E);

		if (NULL == padX || NULL == padY)  
		{  
			return 0;  
		}  


		if (nCount < 3)  
		{  
			return 0;  
		}  

		double x2 = Deg2Rad( padX[nCount-1] );  
		double y2 = Deg2Rad( padY[nCount-1] );  
		Qbar2 = GetQbar( y2 );  

		double area = 0.0;  

		for ( int i = 0; i < nCount; i++ )  
		{  
			x1 = x2;  
			y1 = y2;  
			Qbar1 = Qbar2;  

			x2 = Deg2Rad( padX[i] );  
			y2 = Deg2Rad( padY[i] );  
			Qbar2 = GetQbar( y2 );  

			if ( x1 > x2 )  
				while ( x1 - x2 > osg::PI )  
					x2 += Two_PI;  
			else if ( x2 > x1 )  
				while ( x2 - x1 > osg::PI )  
					x1 += Two_PI;  

			dx = x2 - x1;  
			area += dx * ( Qp - GetQ( y2 ) );  

			if (( dy = y2 - y1 ) != 0.0 )  
				area += dx * GetQ( y2 ) - ( dx / dy ) * ( Qbar2 - Qbar1 );  
		}  
		if (( area *= AE ) < 0.0 )  
			area = -area;  

		if ( area > E )  
			area = E;  
		if ( area > E / 2 )  
			area = E - area;  

		return area;
	}

	double GetQ(double x)  
	{
		double dSemiMajor = 6378140.0; //长半轴
		double dSemiMinor = 6356755.0; //短半轴
		double a2 = ( dSemiMajor * dSemiMajor );  
		double e2 = 1 - ( a2 / ( dSemiMinor * dSemiMinor ) );  
		double e4, e6;
		double sinx2;

		e4 = e2 * e2;  
		e6 = e4 * e2;

		double QA = ( 2.0 / 3.0 ) * e2;  
		double QB = ( 3.0 / 5.0 ) * e4;  
		double QC = ( 4.0 / 7.0 ) * e6;

		sinx2 = sin(x) * sin(x);  

		return sin(x) * (1 + sinx2 * (QA + sinx2 * (QB + sinx2 * QC)));  
	}  

	double GetQbar(double x)  
	{
		double dSemiMajor = 6378140.0; //长半轴
		double dSemiMinor = 6356755.0; //短半轴
		double a2 = ( dSemiMajor * dSemiMajor );  
		double e2 = 1 - ( a2 / ( dSemiMinor * dSemiMinor ) );  
		double e4, e6;
		double cosx2;

		e4 = e2 * e2;  
		e6 = e4 * e2;

		double QbarA = -1.0 - ( 2.0 / 3.0 ) * e2 - ( 3.0 / 5.0 ) * e4  - ( 4.0 / 7.0 ) * e6;  
		double QbarB = ( 2.0 / 9.0 ) * e2 + ( 2.0 / 5.0 ) * e4  + ( 4.0 / 7.0 ) * e6;  
		double QbarC = - ( 3.0 / 25.0 ) * e4 - ( 12.0 / 35.0 ) * e6;  
		double QbarD = ( 4.0 / 49.0 ) * e6;

		cosx2 = cos(x) * cos(x);  

		return cos(x) * (QbarA + cosx2 * (QbarB + cosx2 * (QbarC + cosx2 * QbarD)));  
	}

	double Deg2Rad(double x)
	{
		return ((x)*osg::PI/180);
	}
}


namespace FeMath
{
	bool Equal( double dValue1, double dValue2 )
	{
		return EqualZero(dValue1 - dValue2);
	}

	bool Equal( float fValue1, float fValue2 )
	{
		return EqualZero(fValue1 - fValue2);
	}

	bool EqualZero( double dValue )
	{
		if( (dValue >= -DBL_EPSILON) && (dValue <= DBL_EPSILON) )
		{
			return true;
		}

		return false;
	}

	bool EqualZero( float fValue )
	{
		if( (fValue >= -FLT_EPSILON) && (fValue <= FLT_EPSILON) )
		{
			return true;
		}

		return false;
	}

}

namespace FeUtil
{
	bool GetAngle(CRenderContext *pRenderContext, osg::Vec3d First, osg::Vec3d Second, double &YawAngle, double &PitchAngle)
	{
		// 记录第一个点转换为世界坐标后的值
		osg::Vec3d FirstXYZ;
		// 记录第二个点转换为世界坐标后的值
		osg::Vec3d SecondXYZ;

		FeUtil::DegreeLLH2XYZ(pRenderContext, First, FirstXYZ);
		FeUtil::DegreeLLH2XYZ(pRenderContext, Second, SecondXYZ);

		osg::Vec3d SecondBottom(Second.x(), Second.y(), First.z()), SecondBottomXYZ;
		FeUtil::DegreeLLH2XYZ(pRenderContext, SecondBottom, SecondBottomXYZ);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 首先计算俯仰角, 在高度方向上，通过世界坐标计算角度，初始时飞机方向朝北，计算角度后得人为调整。
		if (FeMath::Equal(First.z(), Second.z()))
		{
			PitchAngle = 0.0;
		}
		else
		{
			double distance = GetDis(FirstXYZ, SecondXYZ);
			double detal = GetDis(SecondXYZ, SecondBottomXYZ);
			if (abs(detal) >= abs(distance))
			{
				if (detal > 0.0) {
					PitchAngle = osg::DegreesToRadians(90.0);
				}
				else {
					PitchAngle = osg::DegreesToRadians(-90.0);
				}
			}
			else
			{
				PitchAngle = asin(detal / distance);
			}
			if (Second.z() < First.z() && PitchAngle > 0.0)
			{
				PitchAngle = -PitchAngle;
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 计算航向角
		// 如果第一个点和第二个点的纬度相同，则航向角为90度或者-90度
		if (FeMath::Equal(First.y(), Second.y()))
		{
			double detal = Second.x() - First.x();//计算经度差
			if (detal < -180.0 || (detal>0.0 && detal < 180.0))
			{
				// 在其右侧180度内
				YawAngle = osg::DegreesToRadians(-90.0);//逆时针转为正向，所以在右侧时转的角度为负值。
			}
			else
			{
				// 在其左侧180度内
				YawAngle = osg::DegreesToRadians(90.0);
			}
		}
		// 如果第一个点的经度和第二个点的经度相同则航向角为0度或者180度
		else if (FeMath::Equal(First.x(), Second.x()))
		{
			double detal = Second.y() - First.y();
			if (detal > 0.0)
			{
				YawAngle = osg::DegreesToRadians(0.0);
			}
			else
			{
				YawAngle = osg::DegreesToRadians(180.0);
			}
		}
		// 第一个点和第二个点的经度和维度都不同，此种情况下只考虑xy坐标就可以了不用考虑z即只在XOY平面做计算就可以了
		else
		{
			osg::Vec3d three(First.x(), Second.y(), First.z());
			osg::Vec3d threeXYZ;
			FeUtil::DegreeLLH2XYZ(pRenderContext, three, threeXYZ);
			double dDistance1 = GetDis(threeXYZ, SecondBottomXYZ);
			double dDistance2 = GetDis(FirstXYZ, threeXYZ);
			
			YawAngle = atan(dDistance1 / dDistance2);
			////如果第二个点在第一个点的右边
			double detalX = Second.x()-First.x();
			if (detalX<-180||(detalX>0&&detalX<180))
			{
				if (Second.y()>First.y())
				{
					YawAngle = -YawAngle;
				}
				else
				{
					YawAngle = -(osg::PI-YawAngle);
				}
			}
			//第二点在第一个点的左侧
			else
			{
				if (Second.y()<First.y())
				{
					YawAngle = osg::PI-YawAngle;
				}
			}
		}
		return true;
	}

	double GetDis( osg::Vec3d vecFrom, osg::Vec3d vecTo )
	{
		osg::Vec3d delta = vecTo - vecFrom;
		return sqrtl(delta.x() * delta.x() + delta.y() * delta.y() + delta.z() * delta.z());
	}

	double GetDis(osg::Vec3 vecFrom, osg::Vec3 vecTo)
	{
		return (vecTo - vecFrom).length();
	}

	double GetRunTime(osg::Vec3d vecFrom, osg::Vec3d vecTo, double dSpeed)
	{
		double dist = GetDis(vecFrom, vecTo);
		if (dSpeed == 0)
			return 1000000000;
		return dist / dSpeed;
	}

	void ComputeBLine(osg::Vec3d start, osg::Vec3d end, osg::Vec3d control, std::vector<osg::Vec3d>& output, int numInter)
	{
		double x = 0.0, y = 0.0, z = 0.0;

		double deltaInter = 1.0 / (numInter * 1.0);
		double deltaHei = end.z() - start.z();
		double t = deltaInter;
		for (int n = 1; n < numInter; n++)
		{
			osg::Vec3d tempBline;
			x = (1 - t)*(1 - t)*start.x() + 2 * (1 - t)*t*control.x() + t*t*end.x();
			y = (1 - t)*(1 - t)*start.y() + 2 * (1 - t)*t*control.y() + t*t*end.y();
			z = start.z() + deltaHei * 1.0 * n / (numInter * 1.0);
			tempBline.set(x, y, z);
			output.push_back(tempBline);
			t += deltaInter;
		}
	}

	void InterpolationSphere(const std::vector<osg::Vec3d>& keyPionts, std::vector<osg::Vec3d>& output, 
		double deltaDistance /*= 1000*/, bool BSEInter /*= true*/, int numBSEInter /*= 50*/, double disStartInsert /*= 50.0*/)
	{
		if (keyPionts.empty())
		{
			return;
		}

		// 判断输入距离的大小，拐角处开始平滑的距离必须小于球面插值两点之间的距离
		if (disStartInsert > deltaDistance)
		{
			disStartInsert = deltaDistance;
		}

		output.push_back(keyPionts.at(0));

		// 首先处理拐角需要平滑的点
		for (int i = 1; i < keyPionts.size() - 1; ++i) 
		{
			if (BSEInter)
			{
				osg::Vec3d startBSE, endBSE;
				// 寻找第一个平滑点，如果寻找成功，则在之前进行插值，如果寻找失败，选取中间点作为插入点。
				if (GetFirstInterpolation(keyPionts[i], keyPionts[i - 1], startBSE, disStartInsert))
				{
					InterpolationSphere(keyPionts[i-1], startBSE, output, deltaDistance);
				}
				else {
					startBSE = keyPionts[i] * 0.5 + keyPionts[i-1] * 0.5;
				}
				if (output.at(output.size() - 1) != startBSE) 
				{
					output.push_back(startBSE);
				}

				if (!GetFirstInterpolation(keyPionts[i], keyPionts[i + 1], endBSE, disStartInsert))
				{
					endBSE = keyPionts[i] * 0.5 + keyPionts[i + 1] * 0.5;
				}
				FeUtil::ComputeBLine(startBSE, endBSE, keyPionts[i], output, numBSEInter);
				output.push_back(endBSE);
			}
			else{
				InterpolationSphere(keyPionts[i-1], keyPionts[i], output, deltaDistance);
				output.push_back(keyPionts[i]);
			}
		}
		InterpolationSphere(output.at(output.size()-1), keyPionts[keyPionts.size()-1], output, deltaDistance);
		output.push_back(keyPionts[keyPionts.size()-1]);
	}

	void InterpolationSphere(const osg::Vec3d start, const osg::Vec3d end, std::vector<osg::Vec3d>& output, double deltaDistance)
	{
		std::vector<osg::Vec3d> vecArray;
		vecArray.push_back(start);
		vecArray.push_back(end);
		double dDistance = osgEarth::GeoMath::distance(vecArray);
		double deltaHei = end.z() - start.z();

		int nTotalNum = dDistance / deltaDistance;

		for (int i = 1; i < nTotalNum; i++)
		{
			double radLat, radLon;
			osgEarth::GeoMath::interpolate(
				osg::DegreesToRadians(start.y())
				, osg::DegreesToRadians(start.x())
				, osg::DegreesToRadians(end.y())
				, osg::DegreesToRadians(end.x())
				, double(i) / double(nTotalNum)
				, radLat
				, radLon);

			output.push_back(osg::Vec3d(osg::RadiansToDegrees(radLon), osg::RadiansToDegrees(radLat), start.z() + deltaHei*1.0*i / nTotalNum));
		}

		if (output.at(output.size() - 1) == end)
		{
			output.pop_back();
		}
	}

	bool GetFirstInterpolation(const osg::Vec3d start, const osg::Vec3d end, osg::Vec3d& output, double deltaDistance)
	{
		std::vector<osg::Vec3d> vecArray;
		vecArray.push_back(start);
		vecArray.push_back(end);
		double dDistance = osgEarth::GeoMath::distance(vecArray);
		double deltaHei = end.z() - start.z();

		int nTotalNum = dDistance / deltaDistance;

		for (int i = 1; i < nTotalNum; i++)
		{
			double radLat, radLon;
			osgEarth::GeoMath::interpolate(
				osg::DegreesToRadians(start.y())
				, osg::DegreesToRadians(start.x())
				, osg::DegreesToRadians(end.y())
				, osg::DegreesToRadians(end.x())
				, double(i) / double(nTotalNum)
				, radLat
				, radLon);

			output.set(osg::Vec3d(osg::RadiansToDegrees(radLon), osg::RadiansToDegrees(radLat), start.z() + deltaHei*1.0*i / nTotalNum));
			return true;
		}
		return false;
	}

	void GetInterpolation( const VecVertexs& source, VecVertexs& depedent )
	{
		if(source.size() <= 0) return;

		unsigned unVertexNum = source.size() - 1;
		for(unsigned unIndex = 0; unIndex < unVertexNum; ++unIndex)
		{
			osg::Vec3 vecLLaFrom = osg::Vec3(source.at(unIndex));
			osg::Vec3 vecLLaTo = osg::Vec3(source.at(unIndex+1));

			depedent.push_back(vecLLaFrom);

			InterpolationSphere(vecLLaFrom,vecLLaTo,depedent,1000.0);			
		}

		osg::Vec3 vecPoint = osg::Vec3(source.at(unVertexNum));
		depedent.push_back(vecPoint);
	}

	void GetDegreesAngle( osg::Quat quat,double& dPitch,double& dRoll,double& dHeading )
	{
		osg::Quat q = quat;

		double test = q.y() * q.z() + q.x() * q.w();
		if (test > 0.4999)
		{ // singularity at north pole
			dHeading = 2.0 * atan2(q.y(), q.w());
			dPitch   = osg::PI_2;
			dRoll    = 0.0;
			return;
		}
		if (test < -0.4999)
		{ // singularity at south pole
			dHeading = 2.0 * atan2(q.y(), q.w());
			dPitch   = -osg::PI_2;
			dRoll    = 0.0;
			return;
		}
		double sqx = q.x() * q.x();
		double sqy = q.y() * q.y();
		double sqz = q.z() * q.z();
		dHeading = atan2(2.0 * q.z() * q.w() - 2.0 * q.y() * q.x(), 1.0 - 2.0 * sqz - 2.0 * sqx);
		dPitch   = asin(2.0 * test);
		dRoll    = atan2(2.0 * q.y() * q.w() - 2.0 * q.z() * q.x(), 1.0 - 2.0 * sqy - 2.0 * sqx);

		dPitch = osg::RadiansToDegrees(dPitch  );
		dRoll = osg::RadiansToDegrees(dRoll    );
		dHeading = osg::RadiansToDegrees(dHeading );

	}

	void GetDegreesAngle(FeUtil::CRenderContext *pRenderContext, osg::Matrix matrix,double& dPitch,double& dRoll,double& dHeading )
	{
		//获取三大角度方法；
		osg::Vec3d vecXYZ;
		vecXYZ = matrix.getTrans();
		osg::Matrix matrixTemp;
		FeUtil::XYZ2Matrix(pRenderContext,vecXYZ,matrixTemp);

		osg::Matrix matrixInvertTemp;
		matrixInvertTemp.invert(matrixTemp);
		matrixTemp = matrix*matrixInvertTemp;
		osg::Quat quat = matrixTemp.getRotate();

		GetDegreesAngle(quat,dPitch,dRoll,dHeading);
	}


	bool CalculateSlopeAndPitchRadian(FeUtil::CRenderContext *pRenderContext, osg::Matrix &matrix, osg::Vec3d front, osg::Vec3d back, osg::Vec3d left, osg::Vec3d right,osg::Vec3d &vecPosture)
	{
		if (pRenderContext == NULL)
		{
			return false;
		}

		if((front.y() - back.y()) < 1.0)
		{
			front.y() += 1.0;
			back.y() -= 1.0;
		}

		if((right.x() - left.x()) < 1.0)
		{
			right.x() += 1.0;
			left.x() -= 1.0;
		}

		osg::Quat rotation;
		rotation.makeRotate(vecPosture.z(), osg::Z_AXIS);
		osg::Vec3d vecLLH;
		FeUtil::XYZ2DegreeLLH(pRenderContext,matrix.getTrans(),vecLLH);
		FeUtil::DegreeLL2LLH(pRenderContext,vecLLH);
		FeUtil::DegreeLLH2Matrix(pRenderContext, vecLLH,matrix);

		osg::Matrix curMatrix = matrix;
		curMatrix.preMultRotate(rotation);

		front = front * curMatrix;
		back = back * curMatrix;
		left = left * curMatrix;
		right = right * curMatrix;

		osg::Vec3d frontLLH, backLLH, leftLLH, rightLLH;
		FeUtil::XYZ2DegreeLLH(pRenderContext, front, frontLLH);
		FeUtil::XYZ2DegreeLLH(pRenderContext, back, backLLH);
		FeUtil::XYZ2DegreeLLH(pRenderContext, left, leftLLH);
		FeUtil::XYZ2DegreeLLH(pRenderContext, right, rightLLH);
		FeUtil::DegreeLL2LLH(pRenderContext, frontLLH);
		FeUtil::DegreeLL2LLH(pRenderContext, backLLH);
		FeUtil::DegreeLL2LLH(pRenderContext, leftLLH);
		FeUtil::DegreeLL2LLH(pRenderContext, rightLLH);

		double dTemp;
		GetAngle(pRenderContext, backLLH, frontLLH, dTemp,vecPosture.x());
		GetAngle(pRenderContext, rightLLH, leftLLH, dTemp,vecPosture.y());

		return true;
	}
}

// 军标相关算法
namespace FeUtil
{
	osg::Vec3dArray* CalculateVector( osg::Vec3d v, double a, double d )
	{
		if(!a) a =  osg::PI_2;
		if(!d) d = 1;

		//定义目标向量的头部   x 坐标
		double x_1;
		double x_2;
		//定义目标向量的头部   y 坐标
		double y_1;
		double y_2;
		//定义目标向量，一左一右
		osg::Vec3d v_l;
		osg::Vec3d v_r;

		//计算基准向量vec的模
		double d_v = std::sqrt(v.x()*v.x() + v.y()*v.y());

		//基准向量的斜率为0时，y值不能作为除数，所以需要特别处理
		if(v.y() == 0)
		{
			//计算x,会有两个值
			x_1 = x_2 = d_v*d*std::cos(a)/v.x();
			//根据vec.x()的正负判断目标向量的左右之分
			if(v.x()>0)
			{
				//计算y
				y_1 = std::sqrt(d*d-x_1*x_1);
				y_2 = -y_1;
			}
			else if(v.x()<0)
			{
				//计算y
				y_2 = std::sqrt(d*d-x_1*x_1);
				y_1 = -y_2;
			}
			v_l = osg::Vec3d(x_1,y_1,0.0);
			v_r = osg::Vec3d(x_2,y_2,0.0);
		}
		//此为大多数情况
		else
		{
			//转换为y=nx+m形式
			double n = -v.x()/v.y();
			double m = d*d_v*std::cos(a)/v.y();
			//
			//x*x + y*y = d*d
			//转换为a*x*x + b*x + c = 0
			double a = 1+n*n;
			double b = 2*n*m;
			double c = m*m - d*d;
			//计算x,会有两个值
			x_1 = (-b - std::sqrt(b*b-4*a*c))/(2*a);
			x_2 = (-b + std::sqrt(b*b-4*a*c))/(2*a);
			//计算y
			y_1 = n*x_1 + m;
			y_2 = n*x_2 + m;
			//当向量向上时
			if(v.y()>=0)
			{
				v_l = osg::Vec3d(x_1,y_1,0.0);
				v_r = osg::Vec3d(x_2,y_2,0.0);
			}
			//当向量向下时
			else if(v.y()<0)
			{
				v_l = osg::Vec3d(x_2,y_2,0.0);
				v_r = osg::Vec3d(x_1,y_1,0.0);
			}
		}

		if(v.y() == 0 && a == osg::PI_2)
		{
			x_1 = 0;
			x_2 = 0;
			y_2 = d;
			y_1 = -d;
			v_l = osg::Vec3d(x_1,y_1,0.0);
			v_r = osg::Vec3d(x_2,y_2,0.0);
		}
		osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray();
		rpPoints->push_back(v_l);
		rpPoints->push_back(v_r);

		return rpPoints.release();
	}

	osg::Vec3d CalculateAngularBisector( osg::Vec3d v1, osg::Vec3d v2 )
	{
		//计算角平分线的思想是取两个向量的单位向量，然后相加
		double d1 = std::sqrt(v1.x()*v1.x()+v1.y()*v1.y());
		double d2 = std::sqrt(v2.x()*v2.x()+v2.y()*v2.y());
		osg::Vec3d point = osg::Vec3d(v1.x()/d1+v2.x()/d2, v1.y()/d1+v2.y()/d2, 0.0);

		return point;
	}

	osg::Vec3d CalculateIntersection( osg::Vec3d v_1, osg::Vec3d v_2, osg::Vec3d point1, osg::Vec3d point2 )
	{
		//定义交点的坐标
		double x;
		double y;
		//如果向量v_1和v_2平行
		if(v_1.y()*v_2.x()-v_1.x()*v_2.y() == 0)
		{
			//平行也有两种情况
			//同向
			if(v_1.x()*v_2.x()>0 || v_1.y()*v_2.y()>0)
			{
				//同向直接取两个点的中点
				x = (point1.x()+point2.x())/2;
				y = (point1.y()+point2.y())/2;
			}
			//反向
			else
			{
				//如果反向直接返回后面的点位置
				x = point2.x();
				y = point2.y();
			}
		}
		else
		{
			//
			x = (v_1.x()*v_2.x()*(point2.y()-point1.y())+point1.x()*v_1.y()*v_2.x()-point2.x()*v_2.y()*v_1.x())/(v_1.y()*v_2.x()-v_1.x()*v_2.y());
			if(v_1.x()!=0)
			{
				y = (x-point1.x())*v_1.y()/v_1.x()+point1.y();
			}
			//不可能v_1.x和v_2.x同时为0
			else
			{
				y = (x-point2.x())*v_2.y()/v_2.x()+point2.y();
			}
		}
		osg::Vec3d point = osg::Vec3d(x, y, 0.0);

		return point;
	}
	extern FEUTIL_EXPORT osg::Vec3dArray* CreateBezier2( osg::Vec3dArray* vecCPoints,int part )
	{
		osg::ref_ptr<osg::Vec3dArray> rpBezierPoints = new osg::Vec3dArray();
		double dScale = 0.05;
		if (part>0)
		{
			dScale = 1.0/part;
		}
		for (int i=0;i<vecCPoints->size()-2;)
		{
			//起始点
			osg::Vec3d vecPointS = vecCPoints->at(i);

			osg::Vec3d vecPointC = vecCPoints->at(i+1);

			osg::Vec3d vecPointE = vecCPoints->at(i+2);
			rpBezierPoints->push_back(vecPointS);
			for (double t=0;t<1;)
			{
				double x_ = (1 - t) * (1 - t) * vecPointS.x() + 2 * t * (1 - t) * vecPointC.x() + t * t * vecPointE.x();
				double y_ = (1 - t) * (1 - t) * vecPointS.y() + 2 * t * (1 - t) * vecPointC.y() + t * t * vecPointE.y();
				osg::Vec3d vecPoint = osg::Vec3d(x_,y_,0.0);
				rpBezierPoints->push_back(vecPoint);
				t+=dScale;
			}
			i+=2;
			if (i>=vecCPoints->size())
			{
				rpBezierPoints->push_back(vecPointS);
			}

		}
		//需要判定一下最后一个点是否存在
		osg::Vec3d poRE = rpBezierPoints->at(rpBezierPoints->size()-1);
		osg::Vec3d popE = vecCPoints->at(vecCPoints->size()-1);
		if (!(poRE ==popE)) {
			rpBezierPoints->push_back(popE);
		}

		return rpBezierPoints.release();
	}

	extern FEUTIL_EXPORT osg::Vec3dArray* CalculateArrowLines( osg::Vec3d startP, osg::Vec3d endP,double ratio, double angle )
	{
		osg::ref_ptr<osg::Vec3dArray> arrowLines=new osg::Vec3dArray();

		double dictance=CalculateDistance(startP,endP);
		osg::Vec3d vector=ToVector(startP,endP);
		osg::ref_ptr<osg::Vec3dArray> vectorArrows=CalculateVector(vector,angle,dictance/ratio);
		osg::Vec3d arrowLineP_l=osg::Vec3d(vectorArrows->at(0).x()+endP.x(),vectorArrows->at(0).y()+endP.y(),0.0);
		osg::Vec3d arrowLineP_r=osg::Vec3d(vectorArrows->at(1).x()+endP.x(),vectorArrows->at(1).y()+endP.y(),0.0);
		arrowLines->push_back(endP);
		arrowLines->push_back(arrowLineP_l);
		arrowLines->push_back(endP);
		arrowLines->push_back(arrowLineP_r);
		arrowLines->push_back(endP);

		return arrowLines.release();
	}

	extern FEUTIL_EXPORT double CalculateDistance( osg::Vec3d pointA, osg::Vec3d pointB )
	{
		double distance =std::sqrt(std::pow(pointA.x() - pointB.x(), 2) + std::pow(pointA.y() - pointB.y(), 2));
		return distance;
	}

	extern FEUTIL_EXPORT osg::Vec3d ToVector( osg::Vec3d pointA,osg::Vec3d pointB )
	{
		return osg::Vec3d(pointA.x()-pointB.x(),pointA.y()-pointB.y(),0.0);
	}

	extern FEUTIL_EXPORT osg::Vec3dArray* CalculateIntersectionFromTwoCorner( osg::Vec3d pointS, osg::Vec3d pointE, double a_S/*=osg::PI/4*/, double a_E/*=osg::PI/4*/ )
	{
		//起始点、结束点、交点加起来三个点，形成一个三角形
		//斜边（起始点到结束点）的向量为
		osg::Vec3d v_SE = osg::Vec3d(pointE.x()-pointS.x(),pointE.y()-pointS.y(),0.0);
		//计算起始点、交点的单位向量
		osg::ref_ptr<osg::Vec3dArray> v_SI_lr = FeUtil::CalculateVector(v_SE,a_S,1);
		//获取
		osg::Vec3d v_SI_l = v_SI_lr->at(0);
		osg::Vec3d v_SI_r = v_SI_lr->at(1);
		//计算结束点、交点的单位向量
		osg::ref_ptr<osg::Vec3dArray> v_EI_lr = FeUtil::CalculateVector(v_SE,osg::PI-a_S,1);
		//获取
		osg::Vec3d v_EI_l = v_EI_lr->at(0);
		osg::Vec3d v_EI_r = v_EI_lr->at(1);
		//求左边的交点
		osg::Vec3d pointI_l = FeUtil::CalculateIntersection(v_SI_l,v_EI_l,pointS,pointE);
		//计算右边的交点
		osg::Vec3d pointI_r = FeUtil::CalculateIntersection(v_SI_r,v_EI_r,pointS,pointE);
		osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray;
		rpPoints->push_back(pointI_l);
		rpPoints->push_back(pointI_r);
		return rpPoints.release();
	}

	extern FEUTIL_EXPORT osg::Vec3dArray* CreateBezier3( osg::Vec3dArray* vecCPoints,int part )
	{
		osg::ref_ptr<osg::Vec3dArray> rpBezierPoints = new osg::Vec3dArray();
		double dScale = 0.05;
		if (part>0)
		{
			dScale = 1.0/part;
		}
		for (int i = 0; i < vecCPoints->size() - 3;) 
		{
			//起始点
			osg::Vec3d pointS = vecCPoints->at(i);
			//第一个控制点
			osg::Vec3d pointC1 = vecCPoints->at(i+1);
			//第二个控制点
			osg::Vec3d pointC2 = vecCPoints->at(i+2);
			//结束点
			osg::Vec3d pointE = vecCPoints->at(i+3);

			rpBezierPoints->push_back(pointS);
			for (double t = 0; t < 1;) {
				//三次贝塞尔曲线公式
				double x = (1 - t) * (1 - t) * (1 - t) * pointS.x() + 3 * t * (1 - t) * (1 - t) * pointC1.x() + 3 * t * t * (1 - t) * pointC2.x() + t * t * t * pointE.x();
				double y = (1 - t) * (1 - t) * (1 - t) * pointS.y() + 3 * t * (1 - t) * (1 - t) * pointC1.y() + 3 * t * t * (1 - t) * pointC2.y() + t * t * t * pointE.y();
				osg::Vec3d point = osg::Vec3d (x, y,0.0);
				rpBezierPoints->push_back(point);
				t += dScale;
			}

			i += 3;
			if (i >= vecCPoints->size()) {
				rpBezierPoints->push_back(pointS);
			}
		}
		//需要判定一下最后一个点是否存在
		osg::Vec3d poRE = rpBezierPoints->at(rpBezierPoints->size()-1);
		osg::Vec3d popE = vecCPoints->at(vecCPoints->size()-1);
		if (!(poRE==popE)) {
			rpBezierPoints->push_back(popE);
		}

		return rpBezierPoints.release();


	}
	extern FEUTIL_EXPORT osg::Vec3dArray* CreateBezierN( osg::Vec3dArray* vecCPoints,int part )
	{
		if (!part) part =vecCPoints->size()*8 ;

		//获取待拆分的点
		osg::ref_ptr<osg::Vec3dArray> rpBezierPoints = new osg::Vec3dArray();
		double dScale = 0.05;
		if (part>0)
		{
			dScale = 1.0/part;
		}
		for (double t = 0; t <= 1;) {
			double x = 0;
			double y = 0;
			int n = vecCPoints->size();
			for (int i = 0; i < vecCPoints->size(); i++) {
				double b = BEZ(n - 1, i, t);
				x += vecCPoints->at(i).x() * b;
				y += vecCPoints->at(i).y() * b;
			}
			osg::Vec3d point = osg::Vec3d (x, y,0.0);
			rpBezierPoints->push_back(point);
			t += dScale;
		}
		//需要判定一下最后一个点是否存在
		osg::Vec3d poRE = rpBezierPoints->at(rpBezierPoints->size()-1);
		osg::Vec3d popE = vecCPoints->at(vecCPoints->size()-1);
		if (!(poRE==popE)) {
			rpBezierPoints->push_back(popE);
		}

		return rpBezierPoints.release();
	}

	extern FEUTIL_EXPORT double Factorial( int n )
	{
		double result = 1;
		for (int i = 1; i <= n; i++) {
			result *= i;
		}
		return result;
	}

	extern FEUTIL_EXPORT double CombSort( int n,int k )
	{
		double son = Factorial(n);
		double mother = Factorial(k) * Factorial(n - k);
		return son / mother;
	}

	extern FEUTIL_EXPORT double BEZ  (int n, int k, double t) 
	{
		return CombSort(n, k) * std::pow(t, k) * std::pow(1 - t, n - k);
	}

	extern FEUTIL_EXPORT osg::Vec3dArray* CreateCardinalCurve( osg::Vec3dArray* vecCPoints )
	{
		if (vecCPoints == nullptr || vecCPoints->size() < 3) {
			return vecCPoints;
		}
		osg::ref_ptr<osg::Vec3dArray> cardinalPoints = new osg::Vec3dArray(500);

		//定义张力系数，取值在0<t<0.5
		double t = 0.4;
		//为端点张力系数因子，取值在0<b<1
		double b = 0.5;
		//误差控制，是一个大于等于0的数，用于三点非常趋近与一条直线时，减少计算量
		double e = 0.005;

		//传入的点数量，至少有三个，n至少为2
		int n = vecCPoints->size() - 1;
		//从开始遍历到倒数第二个，其中倒数第二个用于计算起点（终点）的插值控制点

		for (int k = 0; k <= n + 1 - 3; k++) {
			//三个基础输入点
			osg::Vec3d p0 = vecCPoints->at(k);
			osg::Vec3d p1 = vecCPoints->at(k+1);
			osg::Vec3d p2 = vecCPoints->at(k+2);
			//定义p1的左控制点和右控制点
			osg::Vec3d p1l = osg::Vec3d();
			osg::Vec3d p1r = osg::Vec3d();
			//通过p0、p1、p2计算p1点的做控制点p1l和又控制点p1r
			//计算向量p0_p1和p1_p2
			osg::Vec3d p0_p1 = osg::Vec3d(p1.x() - p0.x(), p1.y() - p0.y(),0.0);
			osg::Vec3d p1_p2 = osg::Vec3d(p2.x() - p1.x(), p2.y() - p1.y(),0.0);
			//并计算模
			double d01 = std::sqrt(p0_p1.x() * p0_p1.x() + p0_p1.y() * p0_p1.y());
			double d12 = std::sqrt(p1_p2.x() * p1_p2.x() + p1_p2.y() * p1_p2.y());
			//向量单位化
			osg::Vec3d p0_p1_1 = osg::Vec3d(p0_p1.x() / d01, p0_p1.y() / d01,0.0);
			osg::Vec3d p1_p2_1 = osg::Vec3d(p1_p2.x() / d12, p1_p2.y() / d12,0.0);
			//计算向量p0_p1和p1_p2的夹角平分线向量
			osg::Vec3d p0_p1_p2 = osg::Vec3d(p0_p1_1.x() + p1_p2_1.x(), p0_p1_1.y() + p1_p2_1.y(),0.0);
			//计算向量 p0_p1_p2 的模
			double d012 = std::sqrt(p0_p1_p2.x() * p0_p1_p2.x() + p0_p1_p2.y() * p0_p1_p2.y());
			//单位化向量p0_p1_p2
			osg::Vec3d p0_p1_p2_1 = osg::Vec3d(p0_p1_p2.x() / d012, p0_p1_p2.y() / d012,0.0);
			//判断p0、p1、p2是否共线，这里判定向量p0_p1和p1_p2的夹角的余弦和1的差值小于e就认为三点共线
			double cosE_p0p1p2 = (p0_p1_1.x() * p1_p2_1.x() + p0_p1_1.y() * p1_p2_1.y()) / 1;
			//共线
			if (std::abs(1 - cosE_p0p1p2) < e) {
				//计算p1l的坐标
				p1l.x() = p1.x() - p1_p2_1.x() * d01 * t;
				p1l.y() = p1.y() - p1_p2_1.y() * d01 * t;
				p1l.z()=0.0;
				//计算p1r的坐标
				p1r.x() = p1.x() + p0_p1_1.x() * d12 * t;
				p1r.y() = p1.y() + p0_p1_1.y() * d12 * t;
				p1r.z()=0.0;
			}
			//非共线
			else {
				//计算p1l的坐标
				p1l.x() = p1.x() - p0_p1_p2_1.x() * d01 * t;
				p1l.y() = p1.y() - p0_p1_p2_1.y() * d01 * t;
				p1l.z()=0.0;
				//计算p1r的坐标
				p1r.x() = p1.x() + p0_p1_p2_1.x() * d12 * t;
				p1r.y() = p1.y() + p0_p1_p2_1.y() * d12 * t;
				p1r.z()=0.0;
			}
			//记录下这三个控制点
			cardinalPoints->at(k * 3 + 2 + 0).set(p1l);
			cardinalPoints->at(k * 3 + 2 + 1).set(p1);
			cardinalPoints->at(k * 3 + 2 + 2).set(p1r);

			//当为起始点时需要计算第一个点的右控制点
			if (k == 0) {
				//定义p0的右控制点
				osg::Vec3d p0r = osg::Vec3d();

				//计算向量p0_p1l
				osg::Vec3d po_p1l = osg::Vec3d(p1l.x() - p0.x(), p1l.y() - p0.y(),0.0);
				//计算模
				double d01l = std::sqrt(po_p1l.x() * po_p1l.x() + po_p1l.y() * po_p1l.y());
				//单位化
				osg::Vec3d po_p1l_1 = osg::Vec3d(po_p1l.x() / d01l, po_p1l.y() / d01l,0.0);
				//计算p0r
				p0r.x() = p0.x() + po_p1l_1.x() * d01 * t * b;
				p0r.y() = p0.y() + po_p1l_1.y() * d01 * t * b;
				p0r.z()=0.0;
				cardinalPoints->at(k*3+0).set(p0);
				cardinalPoints->at(k*3+1).set(p0r);
			}
			//当为倒数第三个点时需要计算最后点的左控制点
			if (k == n + 1 - 3) {
				//定义 p2的做控制点p2l
				osg::Vec3d p2l;

				//计算向量p2_p1r
				osg::Vec3d p2_p1r = osg::Vec3d(p1r.x() - p2.x(), p1r.y() - p2.y(),0.0);
				//并取模
				double d21r = std::sqrt(p2_p1r.x() * p2_p1r.x() + p2_p1r.y() * p2_p1r.y());
				//单位化
				osg::Vec3d p2_p1r_1 = osg::Vec3d(p2_p1r.x() / d21r, p2_p1r.y() / d21r,0.0);
				//计算p2l
				p2l.x() = p2.x() + p2_p1r_1.x() * d12 * t * b;
				p2l.y() = p2.y() + p2_p1r_1.y() * d12 * t * b;
				p2l.z()=0.0;
				cardinalPoints->at(k * 3 + 2 + 3).set(p2l);
				cardinalPoints->at(k * 3 + 2 + 4).set(p2);
			}
		}
		return cardinalPoints.release();
	}

	extern FEUTIL_EXPORT osg::Vec3dArray* CalculateArc( osg::Vec3d center,double radius, double startAngle,double endAngle,int direction,double sides )
	{
		if(!direction ||(direction!=1 && direction!=-1)) direction=-1;
		if(!sides) sides=360;
		double step=osg::PI/sides/2;
		double stepDir= step*direction;
		double length=std::abs(endAngle-startAngle);
		osg::ref_ptr<osg::Vec3dArray> points= new osg::Vec3dArray;
		double radians =startAngle;
		for(double i = 0; i <length;i+=step)
		{
			osg::Vec3d circlePoint = osg::Vec3d(std::cos(radians) * radius + center.x(), std::sin(radians) * radius + center.y(),0.0);
			points->push_back(circlePoint);
			radians+=stepDir;
			radians=radians<0?(radians+2*osg::PI):radians;
			radians=radians> 2*osg::PI?(radians-2*osg::PI):radians;

		}
		return points.release();
	}

	extern FEUTIL_EXPORT osg::Vec3dArray* CreateCloseCardinal( osg::Vec3dArray* vecCPoints )
	{
		if (vecCPoints == nullptr || vecCPoints->size()< 3) {
			return vecCPoints;
		}
		osg::ref_ptr<osg::Vec3dArray> rpPoints = new osg::Vec3dArray();
		rpPoints->insert(rpPoints->end(),vecCPoints->begin(),vecCPoints->end());

		//获取起点，作为终点，以闭合曲线。
		osg::Vec3d lastP = vecCPoints->at(0);
		rpPoints->push_back(lastP);

		osg::ref_ptr<osg::Vec3dArray> cardinalPoints= new osg::Vec3dArray(500); 

		//定义张力系数，取值在0<t<0.5
		double t = 0.4;
		//为端点张力系数因子，取值在0<b<1
		double b = 0.5;
		//误差控制，是一个大于等于0的数，用于三点非常趋近与一条直线时，减少计算量
		double e = 0.005;
		int  n = rpPoints->size() - 1;
		//从开始遍历到倒数第二个，其中倒数第二个用于计算起点（终点）的插值控制点
		for (int k = 0; k <= n - 1; k++) {
			osg::Vec3d p0 ;
			osg::Vec3d p1 ;
			osg::Vec3d p2 ;
			//计算起点（终点）的左右控制点
			if (k == n - 1) {
				//三个基础输入点
				p0.set(rpPoints->at(n-1));
				p1.set(rpPoints->at(0));
				p2.set(rpPoints->at(1));
			}
			else {
				p0.set(rpPoints->at(k));
				p1.set(rpPoints->at(k+1));
				p2.set(rpPoints->at(k+2));
			}

			//定义p1的左控制点和右控制点
			osg::Vec3d p1l ;
			osg::Vec3d p1r ;
			//通过p0、p1、p2计算p1点的做控制点p1l和又控制点p1r
			//计算向量p0_p1和p1_p2
			osg::Vec3d p0_p1 = osg::Vec3d(p1.x() - p0.x(), p1.y() - p0.y(),0.0);
			osg::Vec3d p1_p2 = osg::Vec3d(p2.x() - p1.x(), p2.y() - p1.y(),0.0);
			//并计算模
			double d01 = std::sqrt(p0_p1.x() * p0_p1.x() + p0_p1.y() * p0_p1.y());
			double d12 = std::sqrt(p1_p2.x() * p1_p2.x() + p1_p2.y() * p1_p2.y());
			//向量单位化
			osg::Vec3d p0_p1_1 = osg::Vec3d(p0_p1.x() / d01, p0_p1.y() / d01,0.0);
			osg::Vec3d p1_p2_1 = osg::Vec3d(p1_p2.x() / d12, p1_p2.y() / d12,0.0);
			//计算向量p0_p1和p1_p2的夹角平分线向量
			osg::Vec3d p0_p1_p2 = osg::Vec3d(p0_p1_1.x() + p1_p2_1.x(), p0_p1_1.y() + p1_p2_1.y(),0.0);
			//计算向量 p0_p1_p2 的模
			double d012 = std::sqrt(p0_p1_p2.x() * p0_p1_p2.x() + p0_p1_p2.y() * p0_p1_p2.y());
			//单位化向量p0_p1_p2
			osg::Vec3d p0_p1_p2_1 = osg::Vec3d(p0_p1_p2.x() / d012, p0_p1_p2.y() / d012,0.0);
			//判断p0、p1、p2是否共线，这里判定向量p0_p1和p1_p2的夹角的余弦和1的差值小于e就认为三点共线
			double cosE_p0p1p2 = (p0_p1_1.x() * p1_p2_1.x() + p0_p1_1.y() * p1_p2_1.y()) / 1;
			//共线
			if (std::abs(1 - cosE_p0p1p2) < e) {
				//计算p1l的坐标
				p1l.x() = p1.x() - p1_p2_1.x() * d01 * t;
				p1l.y() = p1.y() - p1_p2_1.y() * d01 * t;
				p1l.z()=0.0;
				//计算p1r的坐标
				p1r.x() = p1.x() + p0_p1_1.x() * d12 * t;
				p1r.y() = p1.y() + p0_p1_1.y() * d12 * t;
				p1r.z()=0.0;

			}
			//非共线
			else {
				//计算p1l的坐标
				p1l.x() = p1.x() - p0_p1_p2_1.x() * d01 * t;
				p1l.y() = p1.y() - p0_p1_p2_1.y() * d01 * t;
				p1l.z()=0.0;
				//计算p1r的坐标
				p1r.x() = p1.x() + p0_p1_p2_1.x() * d12 * t;
				p1r.y() = p1.y() + p0_p1_p2_1.y() * d12 * t;
				p1r.z()=0.0;
			}

			//记录起点（终点）的左右插值控制点及倒数第二个控制点
			if (k == n - 1) {
				cardinalPoints->at(0).set(p1);
				cardinalPoints->at(1).set(p1r);
				cardinalPoints->at((n - 2) * 3 + 2 + 3).set(p1l) ;
				cardinalPoints->at((n - 2) * 3 + 2 + 4).set(rpPoints->at(n));
			}
			else {
				//记录下这三个控制点
				cardinalPoints->at(k * 3 + 2 + 0).set(p1l);
				cardinalPoints->at(k * 3 + 2 + 1).set(p1);
				cardinalPoints->at(k * 3 + 2 + 2).set(p1r);
			}

		}
		return cardinalPoints.release();
	}

	extern FEUTIL_EXPORT bool GetCenterPosition( osg::Vec3d start, osg::Vec3d end, osg::Vec3d &center, CRenderContext *pRenderContext )
	{
		// 获取两点距离的一半的值
		double dDis = osgEarth::GeoMath::distance(start, end, pRenderContext->GetMapSRS())/2.0;

		// 计算两点直线的方位角
		double dAzimuthAngle = osgEarth::GeoMath::bearing(
			osg::DegreesToRadians(start.y()),
			osg::DegreesToRadians(start.x()),
			osg::DegreesToRadians(end.y()),
			osg::DegreesToRadians(end.x()));

		// 计算中心点位置
		osg::Vec3d centerPos;
		osgEarth::GeoMath::destination(osg::DegreesToRadians(start.y()), osg::DegreesToRadians(start.x()), dAzimuthAngle, dDis, centerPos.y(), centerPos.x());
		center.x() = osg::RadiansToDegrees(centerPos.x());
		center.y() = osg::RadiansToDegrees(centerPos.y());

		return true;
	}

}
