#include <QApplication>
#include <QMainWindow>
#include <QTextCodec>

#include <osg/Group>
#include <osg/BlendFunc>
#include <osg/BlendColor>
#include <osgDB/ReadFile>

#include <FeUtils/RenderContext.h>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/StrUtil.h>
#include <FeUtils/EnvironmentVariableReader.h>
#include <FeUtils/NodeUtils.h>
#include <FeUtils/MiniAnimationCtrl.h>
#include <FeUtils/FlightMotionAlgorithm.h>

#include <FeEarth/3DSceneWidget.h>
#include <FeLayers/LayerSys.h>

#include <FeManager/FreeMarkSys.h>
#include <FeExtNode/ExGridNode.h>
#include <FeExtNode/ExModelNode.h>

#include <FeEffects/WakeRibbonEffect.h>

#include <FreeMainWindow.h>



// 生成30条随机飞行路径，分为6组，每组对应5架飞机
const int nTeamNum = 6;
const int nModelPerTeam = 5;

/**  
  * @brief 获取模型路径
  * @note 该函数构建了30条模型运动路径，分为6组，每组5条
  * @param nTeam [in] 组号，0~5
  * @param nModel [in] 模型号, 0~4
  * @param vecOutput [out] 路径关键点
  * @return 无
*/
void GetModelPath(int nTeam, int nModel, std::vector<osg::Vec3d>& vecOutput)
{
	if(nTeam < 0 || nTeam >= nTeamNum || nModel < 0 || nModel >= nModelPerTeam)
	{
		return;
	}

	std::vector<osg::Vec3d> startPos;
	std::vector<osg::Vec3d> endPos;

	// 30架飞机的路径起始位置
	startPos.push_back(osg::Vec3d(120.736344,23.329826,10000));   
	startPos.push_back(osg::Vec3d(120.706344,23.329826,2000)); 
	startPos.push_back(osg::Vec3d(120.726344,23.359826,2000));
	startPos.push_back(osg::Vec3d(120.746344,23.329826,3000));
	startPos.push_back(osg::Vec3d(120.766344,23.359826,3000));

	startPos.push_back(osg::Vec3d(120.846344,23.369826,10000));
	startPos.push_back(osg::Vec3d(120.806344,23.359826,6000));
	startPos.push_back(osg::Vec3d(120.839344,23.389826,4000));
	startPos.push_back(osg::Vec3d(120.839344,23.329826,8000));
	startPos.push_back(osg::Vec3d(120.866344,23.359826,6000));

	startPos.push_back(osg::Vec3d(120.936344,23.329826,10000));   
	startPos.push_back(osg::Vec3d(120.906344,23.329826,6000));    
	startPos.push_back(osg::Vec3d(120.926344,23.329826,6000));
	startPos.push_back(osg::Vec3d(120.946344,23.329826,6000));    
	startPos.push_back(osg::Vec3d(120.966344,23.329826,6000));	

	startPos.push_back(osg::Vec3d(121.036344,23.389826,10000));
	startPos.push_back(osg::Vec3d(121.006344,23.419826,8000));
	startPos.push_back(osg::Vec3d(121.026344,23.389826,7000));
	startPos.push_back(osg::Vec3d(121.046344,23.359826,6000));
	startPos.push_back(osg::Vec3d(121.066344,23.329826,5000));

	startPos.push_back(osg::Vec3d(121.136344,23.389826,10000));
	startPos.push_back(osg::Vec3d(121.106344,23.419826,4000));
	startPos.push_back(osg::Vec3d(121.126344,23.389826,4000));
	startPos.push_back(osg::Vec3d(121.146344,23.359826,4000));
	startPos.push_back(osg::Vec3d(121.166344,23.329826,4000));

	startPos.push_back(osg::Vec3d(121.246344,23.389826,10000));
	startPos.push_back(osg::Vec3d(121.206344,23.359826,5000));
	startPos.push_back(osg::Vec3d(121.226344,23.389826,5000));
	startPos.push_back(osg::Vec3d(121.246344,23.389826,5000));
	startPos.push_back(osg::Vec3d(121.266344,23.359826,5000));


	endPos.push_back(osg::Vec3d(120.736344,24.199158,10000));
	endPos.push_back(osg::Vec3d(120.706344,24.189158,2000));
	endPos.push_back(osg::Vec3d(120.726344,24.219158,2000));
	endPos.push_back(osg::Vec3d(120.746344,24.189158,3000));
	endPos.push_back(osg::Vec3d(120.766344,24.219158,3000));

	endPos.push_back(osg::Vec3d(120.836344,24.219158,10000));
	endPos.push_back(osg::Vec3d(120.806344,24.189158,6000));
	endPos.push_back(osg::Vec3d(120.839344,24.219158,4000));
	endPos.push_back(osg::Vec3d(120.839344,24.159158,8000));
	endPos.push_back(osg::Vec3d(120.866344,24.189158,6000));

	endPos.push_back(osg::Vec3d(120.936344,24.219158,10000));
	endPos.push_back(osg::Vec3d(120.906344,24.219158,6000));
	endPos.push_back(osg::Vec3d(120.926344,24.219158,6000));
	endPos.push_back(osg::Vec3d(120.946344,24.219158,6000));
	endPos.push_back(osg::Vec3d(120.966344,24.219158,6000));

	endPos.push_back(osg::Vec3d(121.036344,24.219158,10000));
	endPos.push_back(osg::Vec3d(121.006344,24.219158,8000));
	endPos.push_back(osg::Vec3d(121.026344,24.189158,7000));
	endPos.push_back(osg::Vec3d(121.046344,24.159158,6000));
	endPos.push_back(osg::Vec3d(121.066344,24.129158,5000));

	endPos.push_back(osg::Vec3d(121.136344,24.219158,10000));
	endPos.push_back(osg::Vec3d(121.106344,24.219158,4000));
	endPos.push_back(osg::Vec3d(121.126344,24.189158,4000));
	endPos.push_back(osg::Vec3d(121.146344,24.159158,3000));
	endPos.push_back(osg::Vec3d(121.166344,24.129158,4000));

	endPos.push_back(osg::Vec3d(121.246344,24.159158,10000));
	endPos.push_back(osg::Vec3d(121.206344,24.159158,5000));
	endPos.push_back(osg::Vec3d(121.226344,24.189158,5000));
	endPos.push_back(osg::Vec3d(121.246344,24.189158,5000));
	endPos.push_back(osg::Vec3d(121.266344,24.159158,5000));
	
	osg::Vec3d mpos;
	osg::Vec3d vecMinPos = startPos[nTeam*nModelPerTeam+nModel];
	osg::Vec3d vecMaxPos = endPos[nTeam*nModelPerTeam+nModel];;

	switch(nTeam)
	{
	case 0:
		{
			if(nModel==0)
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==4)
					{
						fpos=mpos;
						break;
					}	
				}
				mpos.x() = fpos.x()+0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);
				fpos = mpos;
				mpos.x() = fpos.x()+0.02;
				mpos.y() = fpos.y();
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);
				vecMaxPos.x() = 121.468367;
				vecMaxPos.y() = mpos.y();
				vecMaxPos.z() = mpos.z();
				vecOutput.push_back(vecMaxPos);
                vecOutput.push_back(vecMinPos);
			}
			else
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==4)
					{
						fpos=mpos;
						break;
					}	
				}
				mpos.x() = fpos.x()+0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z()+2000;
				vecOutput.push_back(mpos);
				fpos = mpos;
				mpos.x() = fpos.x()+0.02;
				mpos.y() = fpos.y();
				mpos.z() = fpos.z()-1000;
				vecOutput.push_back(mpos);
				vecMaxPos.x() = 121.468367;
				vecMaxPos.y() = mpos.y();
				vecMaxPos.z() = mpos.z();
				vecOutput.push_back(vecMaxPos);
				vecOutput.push_back(vecMinPos);
			}
		}
		break;

	case 1:
		{
			if(nModel==0)
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==3)
					{
						fpos=mpos;
						break;
					}	
				}
				fpos = mpos;
				mpos.x() = fpos.x()+0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);
				vecMaxPos.x() = 121.368367;
				vecMaxPos.y() = mpos.y();
				vecMaxPos.z() = mpos.z();
				vecOutput.push_back(vecMaxPos);

				vecOutput.push_back(vecMinPos);
			}
			else
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==3)
					{
						fpos=mpos;
						break;
					}	
				}
				mpos.x() = fpos.x()+0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);
				fpos = mpos;
				for(int j=0;j<10;j++)
				{
					mpos.y() = fpos.y();
					mpos.x() = (j+1)*(121.368367-fpos.x())/11+fpos.x();
					mpos.z() = sinf(90*(j+1))*fpos.z()/2+6000;
					vecOutput.push_back(mpos);
				}
				vecOutput.push_back(vecMinPos);
			}
		}
		break;
	case 2:
		{
			if(nModel==0)
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==2)
					{
						fpos = mpos;
						break;
					}
				}
				mpos.x() = fpos.x()-0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);
				fpos = mpos;
				mpos.x() = fpos.x()-0.04;
				mpos.y() = fpos.y()-0.02;
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);
				fpos = mpos;
				vecMaxPos.x() = fpos.x()-0.1;
				vecMaxPos.y() = fpos.y()-0.04;
				vecMaxPos.z() = fpos.z();
				vecOutput.push_back(vecMaxPos);
				vecOutput.push_back(vecMinPos);
			}
			else
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==2)
					{
						fpos=mpos;
						break;
					}	
				}
				mpos.x() = fpos.x()-0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z()+2000;
				vecOutput.push_back(mpos);
				fpos = mpos;
				mpos.x() = fpos.x()-0.04;
				mpos.y() = fpos.y()-0.02;
				mpos.z() = fpos.z()-2000;
				vecOutput.push_back(mpos);
				fpos = mpos;
				vecMaxPos.x() = fpos.x()-0.2;
				vecMaxPos.y() = fpos.y()-0.02;
				vecMaxPos.z() = fpos.z();
				vecOutput.push_back(vecMaxPos);
				vecOutput.push_back(vecMinPos);
			}
		}
		break;

	case 3:
		{
			if(nModel==0)
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==4)
					{
						fpos=mpos;
						break;
					}	
				}
				mpos.x() = fpos.x()-0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);
				fpos = mpos;
				mpos.x() = fpos.x()-0.004;
				mpos.y() = vecMinPos.y()+0.004;
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);
				fpos = mpos;
				vecMaxPos.x() = 120.706344;
				vecMaxPos.y() = fpos.y();
				vecMaxPos.z() = fpos.z();
				vecOutput.push_back(vecMaxPos);
				vecOutput.push_back(vecMinPos);
			}
			else
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==4)
					{
						fpos=mpos;
						break;
					}	
				}
				mpos.x() = fpos.x()-0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);
				fpos = mpos;
				mpos.x() = fpos.x()-0.004;
				mpos.y() = vecMinPos.y()+0.004;
				mpos.z() = fpos.z()-1000;
				vecOutput.push_back(mpos);
				fpos = mpos;
				vecMaxPos.x() = 120.706344;
				vecMaxPos.y() = fpos.y();
				vecMaxPos.z() = fpos.z();
				vecOutput.push_back(vecMaxPos);
				vecOutput.push_back(vecMinPos);
			}
		}
		break;
	case 4:
		{
			if(nModel==0)
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==2)
					{
						fpos=mpos;
						break;
					}	
				}
				mpos.x() = fpos.x()-0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);
				vecMaxPos.x() = 120.706344;
				vecMaxPos.y() = mpos.y();
				vecMaxPos.z() = mpos.z();
				vecOutput.push_back(vecMaxPos);
				vecOutput.push_back(vecMinPos);
			}
			else
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==2)
					{
						fpos=mpos;
						break;
					}	
				}
				mpos.x() = fpos.x()-0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);
				fpos = mpos;
				mpos.x() = fpos.x()-(fpos.x()-120.706344)/2;
				mpos.y() = fpos.y();
				mpos.z() = fpos.z()+10000;
				vecOutput.push_back(mpos);
				fpos = mpos;
				mpos.x() = fpos.x()-(fpos.x()-120.706344)/2;
				mpos.y() = fpos.y();
				mpos.z() = fpos.z()-10000;
				vecOutput.push_back(mpos);
				vecOutput.push_back(vecMinPos);
			}
		}
		break;
	case 5:
		{
			if(nModel==0)
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==4)
					{
						fpos=mpos;
						break;
					}	
				}
				mpos.x() = fpos.x()+0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z();
				vecOutput.push_back(mpos);

				vecOutput.push_back(vecMinPos);
			}
			else
			{
				osg::Vec3 fpos = vecMinPos;
				osg::Vec3 lpos = vecMaxPos; 
				for(int j=0;j<5;j++)
				{
					mpos.x() = (j+1)*(lpos.x()-fpos.x())/6+fpos.x();
					mpos.y() = (j+1)*(lpos.y()-fpos.y())/6+fpos.y();
					mpos.z() = fpos.z();
					vecOutput.push_back(mpos);
					if(j==4)
					{
						fpos=mpos;
						break;
					}	
				}
				mpos.x() = fpos.x()+0.04;
				mpos.y() = fpos.y()+0.02;
				mpos.z() = fpos.z()+3000;
				vecOutput.push_back(mpos);
				vecOutput.push_back(vecMinPos);
			}
		}
		break;
	}
}
void GetModelPath(FeUtil::CRenderContext* pContext, int nTeam, int nModel, std::vector<FeUtil::SDriveData>& vecOutput)
{
	std::vector<osg::Vec3d> points;
	GetModelPath(nTeam, nModel,points);

	// 球面插值
	FeUtil::FlightInterpolationAndSphereSmooth(pContext, points, vecOutput, 300, 50, 4000, 50);

	for (int i = 0; i < vecOutput.size(); i++)
	{
		CMotionStateInfo& info = vecOutput.at(i);
		info.vecScale = osg::Vec3d(100.0, 100.0, 100.0);
	}
}

/**  
  * @brief 创建飞机模型
*/
void CreateModels(FeShell::CSystemManager* pMgr)
{
	// 获取标记系统
	FeManager::CFreeMarkSys* pMarkSys = dynamic_cast<FeManager::CFreeMarkSys*>(
		pMgr->GetSystemService()->GetModuleSys(FeManager::FREE_MARK_SYSTEM_CALL_DEFAULT_KEY));

	if(!pMarkSys)
	{
		pMarkSys = new FeManager::CFreeMarkSys();
		pMgr->GetSystemService()->AddAppModuleSys(pMarkSys);
		pMarkSys->Initialize(pMgr->GetRenderContext());
		pMarkSys->Start();
	}

	// 模型配置
	FeExtNode::CExModelNodeOption* optB737 = new FeExtNode::CExModelNodeOption;
	optB737->ModelPath() = FeFileReg->GetFullPath("model/B737.ive");
	FeExtNode::CExModelNodeOption* optL15 = new FeExtNode::CExModelNodeOption;
	optL15->ModelPath() = FeFileReg->GetFullPath("model/L15_1.ive");

	// 每组颜色
	osg::Vec4d vecColorTeams[nTeamNum];
	vecColorTeams[0] = osg::Vec4d(1,1,0,1);
	vecColorTeams[1] = osg::Vec4d(0,1,0,1);
	vecColorTeams[2] = osg::Vec4d(0,1,1,1);
	vecColorTeams[3] = osg::Vec4d(1,0,0,1);
	vecColorTeams[4] = osg::Vec4d(0.5,0,1,1);
	vecColorTeams[5] = osg::Vec4d(1,0.6,0.18,1);

	for(int nTeam = 0; nTeam < nTeamNum; nTeam++)
	{
		osg::Vec4d vecClr;
		FeExtNode::CExModelNode* pModelNode = NULL;

		for(int nModel = 0; nModel < nModelPerTeam; nModel++)
		{
			// 创建路径
			std::vector<FeUtil::SDriveData> modelPath;
			GetModelPath(pMgr->GetRenderContext(), nTeam, nModel, modelPath);
			
			if(0 == nModel)
			{
				// 每组的第一架飞机
				vecClr = osg::Vec4d(0.46,0.73,1,1);
				pModelNode = new FeExtNode::CExModelNode(pMgr->GetRenderContext(), optB737);
			}
			else
			{
				vecClr = vecColorTeams[nTeam];
				pModelNode = new FeExtNode::CExModelNode(pMgr->GetRenderContext(), optL15);
			}

			// 设置模型颜色
			FeUtil::ChangeNodeColor(pModelNode, vecClr);

			// 创建模型动画
			osg::ref_ptr<FeUtil::CMiniAnimationCtrl> miniAniCtrl = new FeUtil::CMiniAnimationCtrl(pMgr->GetRenderContext(), pModelNode);
			miniAniCtrl->CreateAnimationPath(modelPath);
			miniAniCtrl->SetAnimationLoopMode(FeUtil::AnimationPath::LOOP);
			miniAniCtrl->Start();

			// 创建尾迹效果
			FeEffect::CWakeRibbonEffect* m_rpWakeRibbon = new FeEffect::CWakeRibbonEffect();
			m_rpWakeRibbon->SetMatrixTransform(pModelNode->GetTransMT());
			m_rpWakeRibbon->SetColor(vecClr);
			m_rpWakeRibbon->CreateEffect();
			m_rpWakeRibbon->SetVisible(true);
			m_rpWakeRibbon->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);

			// 添加到场景
			pMgr->GetRenderContext()->GetRoot()->addChild(m_rpWakeRibbon);
			pMarkSys->AddMark(pModelNode);
		}
	}
}

/**  
  * @brief 创建地面网格
*/
void CreateFeatureGrid(FeShell::CSystemManager* pMgr)
{
	FeManager::CFreeMarkSys* pMarkSys = dynamic_cast<FeManager::CFreeMarkSys*>(
		pMgr->GetSystemService()->GetModuleSys(FeManager::FREE_MARK_SYSTEM_CALL_DEFAULT_KEY));

	if(!pMarkSys)
	{
		pMarkSys = new FeManager::CFreeMarkSys();
		pMgr->GetSystemService()->AddAppModuleSys(pMarkSys);
		pMarkSys->Initialize(pMgr->GetRenderContext());
		pMarkSys->Start();
	}

	osg::ref_ptr<FeExtNode::CExGridNodeOption> opt = new FeExtNode::CExGridNodeOption;
    opt->lineColor() = osg::Vec4d(1.0, 1.0, 1.0, 0.2);
    opt->lineWidth() = 2;
    opt->GridRegion() = osg::Vec4d(120.606344, 121.568367, 23.329826, 24.219158);
    opt->Row() = 100;
    opt->Col() = 100;

	pMarkSys->AddMark(new FeExtNode::CExGridNode(pMgr->GetRenderContext(), opt));
}


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QTextCodec *codec = NULL;
#ifdef WIN32
	codec = QTextCodec::codecForName("GBK");
#else
	codec = QTextCodec::codecForName("UTF-8");
#endif 
	if (codec)
	{
#ifdef QT4_VERSION
		QTextCodec::setCodecForLocale(codec);
 		QTextCodec::setCodecForCStrings(codec);
 		QTextCodec::setCodecForTr(codec);
#endif
	}

	CFreeMainWindow* pWidget = new CFreeMainWindow();
	pWidget->SetTitle(QString::fromLocal8Bit("兵棋推演效果演示"));

	FeUtil::CEnvironmentVariableReader reader;
	FeUtil::CFileRegInitAgent fileReg;
	fileReg.InitDataPath(reader.DoRead("FREE_EARTH_FILE_PATH"));

	FeEarth::C3DSceneWidget* p3DScenePanel = new FeEarth::C3DSceneWidget("");
	p3DScenePanel->SetEarthPath(FeFileReg->GetFullPath("earth/FreeEarth_flat_Wargaming.earth"));

	if(!p3DScenePanel->Initialize())
	{
		return -1;
	}

	pWidget->GetSceneLayout()->addWidget(p3DScenePanel);

	FeShell::CSystemManager* pSystemManager = p3DScenePanel->GetSystemManager();
	if (pSystemManager)
	{
		FeUtil::CRenderContext *pContext = pSystemManager->GetRenderContext();
		if (NULL == pContext)
		{
			return 0;
		}

		//开启法线光照特效
		FeLayers::CLayerSysOptions opt;
		FeLayers::CLayerSys* pLayerSys = new FeLayers::CLayerSys(opt);
		pLayerSys->Initialize(pContext);
		pSystemManager->GetSystemService()->AddAppModuleSys(pLayerSys);
		pLayerSys->RegistryEffectLayer();
		FeLayers::IFeLayerEffect* pEffectLayer = pLayerSys->GetEffectLayer(FeUtil::ToUTF("法线光照特效"));
		if (pEffectLayer)
		{
			pEffectLayer->SetVisible(true);
		}

		//开启光照等效果
		pSystemManager->GetSystemService()->GetEnvironmentSys()->GetSkyNode()->Show();
		pSystemManager->GetSystemService()->GetEnvironmentSys()->GetSkyNode()->SetSunVisible(true);
		pSystemManager->GetSystemService()->GetEnvironmentSys()->GetSkyNode()->SetDateTime(osgEarth::DateTime(2016, 12, 12, 8));
		pSystemManager->GetSystemService()->GetEnvironmentSys()->GetSkyNode()->SetAmbient(osg::Vec4d(0.8, 0.8, 0.8, 0.8));
		
		// 创建网格和模型
		CreateFeatureGrid(pSystemManager);
		CreateModels(pSystemManager);
		
		CFreeLableWidget* pLabelWidget = new CFreeLableWidget(pWidget);
		pLabelWidget->SetTitleText(QString::fromLocal8Bit("功能说明"));
		pLabelWidget->SetContentText(QString::fromLocal8Bit(
			"    兵棋推演效果通过对三维场景中的数字"
			"高程数据进行解析，类似晕眩图效果的实现"
			"了按高度区间进行不同颜色渲染，并辅以公"
			"司特有的法线光照技术，使该效果展现的更"
			"加的清晰、亮丽。\n"
			"    该效果可用于作战部署、态势推演、军"
			"事训练等系统中，通过地图上颜色可以清晰"
			"的了解地形的走势，为兵力部署、航路规划"
			"等提供了依据。\n"
			));
		pWidget->AddControlWidget(pLabelWidget);
		
		pWidget->AddControlItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

		pWidget->ShowDialogNormal();
		pWidget->ShowDialogMaxisize();

		pSystemManager->GetSystemService()->Locate(
			FeUtil::CFreeViewPoint(120.5012733308, 23.9842753057, 9940.3957404327, 126.9789496231, -9.0714030130, 9940.3957404327, 3),
			true);

		int nFlag = a.exec();

		return nFlag;
	}

	return 0;
}
