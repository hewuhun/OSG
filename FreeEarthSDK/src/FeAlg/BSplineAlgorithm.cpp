#include <FeAlg/BSplineAlgorithm.h>
#include <string.h>
namespace FeAlg
{
	CBSplineAlgorithm::CBSplineAlgorithm(void)
	{
	}

	CBSplineAlgorithm::~CBSplineAlgorithm(void)
	{
	}

	CBSplineAlgorithm::CBSplineAlgorithm(std::vector<osg::Vec3d>& pt)
	{
		if (pt.empty())
		{
			return;
		}
		m_vecInterpolatioNode.clear();
		m_n = pt.size()-1;
		if (pt.size() < 4)
		{
			m_k = pt.size()-1;
		}
		else
		{
			m_k = 3;
		}
		m_vecControlPoint.resize(pt.size());
		memcpy(&m_vecControlPoint[0], &pt[0], pt.size() * sizeof(osg::Vec3d));
		m_vecU.resize(m_n+m_k+2);
	}

	void CBSplineAlgorithm::Computeinterpolation(void)
	{   
		ComputeNode();

		int m;
		int n;
		float interpolationnumber;
		std::vector<osg::Vec3d> relevantPoint;
		std::vector<float> relevantU;
		relevantPoint.resize(m_k+1);//相关控制顶点
		relevantU.resize(2*m_k);//相关节点矢量
		//相关控制顶点拷贝
		float u = 0;
		int i = m_k;//m表示u值所在的节点,则控制顶点为m_controlpoint[i-k]\m_controlpoint[i]
		float a;
		for (interpolationnumber=0.0; interpolationnumber<300.0; interpolationnumber++)
		{
			n = 0;
			m = 0;
			u = interpolationnumber/300;
			if (u > m_vecU[i+1])
			{
				i++;
			}
			for (m=i-m_k; m<=i; m++)
			{   
				relevantPoint[n] = m_vecControlPoint[m];
				n++;
			}
			//相关节点矢量拷贝
			n = 0;
			for (m=i-m_k+1; m<=i+m_k; m++)
			{   
				relevantU[n] = m_vecU[m];
				n++;
			}

			//开始插值计
			for (n=1; n<relevantPoint.size(); n++)
			{
				for (m=0; m<relevantPoint.size()-n; m++)
				{
					a = relevantU[m_k+m] - relevantU[m+n-1];
					if (fabs(a) < 1e-5)
					{
						a = 0.0;
					}
					else
					{
						a = (u - relevantU[m+n-1])/a;
					}
					relevantPoint[m] = relevantPoint[m]*(1-a) + relevantPoint[m+1]*a;
				}
			}
			m_vecInterpolatioNode.push_back(relevantPoint[0]);
		}
	}
	//计算节点矢量
	void CBSplineAlgorithm::ComputeNode(void)
	{
		int i;
		float u=0;

		for (i=0;i<=m_k;i++)
		{
			m_vecU[i] = 0;
		}

		for (i=1; i<=m_k+1; i++)
		{
			m_vecU[i+m_n] = 1;
		}

		Hartley();
	}
	void CBSplineAlgorithm::Hartley(void)
	{   
		int i,j;
		float u = 0;
		float sum = 0;
		float s = 0;
		std::vector<float> length(m_n);
		std::vector<float> ll;
		ll.push_back(0.0);
		osg::Vec3d pq;
		for(i=0; i<m_n; i++)
		{
			pq = m_vecControlPoint[i+1] - m_vecControlPoint[i];
			length[i] = ComputerLen(pq);
		}
		for (i=0; i<=length.size()-m_k; i++)
		{   
			s = 0;
			for(j=0; j<m_k; j++)
			{
				s += length[i+j];
			}
			sum += s;
			ll.push_back(s);
		}
		for(i=1; i<ll.size(); i++)
		{   
			ll[i] = ll[i-1]+ll[i];
			m_vecU[i+m_k] = ll[i]/sum;
		}
	}

	float CBSplineAlgorithm::ComputerLen(osg::Vec3d &p)
	{
		float pTemp = p.x() * p.x();
		pTemp += p.y() * p.y();
		pTemp += p.z() * p.z();

		return sqrt(pTemp);
	}
}

