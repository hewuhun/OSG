#include <FeUtils/SystemInfo.h>

#ifdef WIN32
#include <FeUtils/StrUtil.h>

#include <atlbase.h>  
#include <winperf.h>

#include <iostream>
#include <algorithm>
#include <gl/gl.h>

namespace FeUtil
{
	CSystemInfo* CSystemInfo::m_pSystemInfo = NULL;
	//CSystemInfo::CDestoryInstance CSystemInfo::destroy;

	CSystemInfo::CSystemInfo() 
		:m_nDisplayCardType(E_Default)
	{  
// 		int nNum = 0;
// 		std::list<std::string> names;
// 		GetDisplayCardInfo(nNum, names);
// 
// 		int nAmdSize = 0, nNvdiaSize = 0;
// 		std::list<std::string>::iterator it = names.begin();
// 		
// 		while (it != names.end())
// 		{
// 			nAmdSize = (*it).find("amd");
// 			nNvdiaSize = (*it).find("nvdia");
// 
// 			it++;
// 		}
// 		if (nAmdSize > 0)
// 		{
// 			m_nDisplayCardType = E_AMD;
// 		}
// 
// 		if (nNvdiaSize > 0)
// 		{
// 			m_nDisplayCardType = E_NVIDIA;
// 		}
	}  

	CSystemInfo::~CSystemInfo()  
	{  

	}  

	CSystemInfo* CSystemInfo::Instance()
	{
		if(!m_pSystemInfo)
		{
			m_pSystemInfo = new CSystemInfo();
		}

		return m_pSystemInfo;
	}

	void CSystemInfo::GetDisplayCardInfo(int &dwNum,std::list<std::string>& names)  
	{  
		HKEY keyServ;  
		HKEY keyEnum;  
		HKEY key;  
		HKEY key2;  
		LONG lResult;//LONG型变量－保存函数返回值  

		//查询"SYSTEM\\CurrentControlSet\\Services"下的所有子键保存到keyServ  
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,TEXT("SYSTEM\\CurrentControlSet\\Services"),0,KEY_READ,&keyServ);  
		if (ERROR_SUCCESS != lResult)  
			return;  


		//查询"SYSTEM\\CurrentControlSet\\Enum"下的所有子键保存到keyEnum  
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,TEXT("SYSTEM\\CurrentControlSet\\Enum"),0,KEY_READ,&keyEnum);  
		if (ERROR_SUCCESS != lResult)  
			return;  

		int i = 0,count = 0;  
		DWORD size = 0,type = 0;  
		for (;;++i)  
		{  
			//Sleep(5);  
			size = 512;  
			TCHAR name[512] = {0};//保存keyServ下各子项的字段名称  

			//逐个枚举keyServ下的各子项字段保存到name中  
			lResult = RegEnumKeyEx(keyServ,i,name,&size,NULL,NULL,NULL,NULL);  

			//要读取的子项不存在，即keyServ的子项全部遍历完时跳出循环  
			if(lResult == ERROR_NO_MORE_ITEMS)  
				break;  

			//打开keyServ的子项字段为name所标识的字段的值保存到key  
			lResult = RegOpenKeyEx(keyServ,name,0,KEY_READ,&key);  
			if (lResult != ERROR_SUCCESS)  
			{  
				RegCloseKey(keyServ);  
				return;  
			}  

			size = 512;  
			//查询key下的字段为Group的子键字段名保存到name  
			lResult = RegQueryValueEx(key,TEXT("Group"),0,&type,(LPBYTE)name,&size);  
			if(lResult == ERROR_FILE_NOT_FOUND)  
			{  
				//?键不存在  
				RegCloseKey(key);  
				continue;  
			};  

			//如果查询到的name不是Video则说明该键不是显卡驱动项  
			if(_tcscmp(TEXT("Video"),name)!=0)  
			{  
				RegCloseKey(key);  
				continue;     //返回for循环  
			};  

			//如果程序继续往下执行的话说明已经查到了有关显卡的信息，所以在下面的代码执行完之后要break第一个for循环，函数返回  
			lResult = RegOpenKeyEx(key,TEXT("Enum"),0,KEY_READ,&key2);  
			RegCloseKey(key);  
			key = key2;  
			size = sizeof(count);  
			lResult = RegQueryValueEx(key,TEXT("Count"),0,&type,(LPBYTE)&count,&size);//查询Count字段（显卡数目）  

			dwNum = count;//保存显卡数目  
			for(int j=0;j <count;++j)  
			{  
				TCHAR sz[512] = {0};  
				TCHAR name[64] = {0};  
				wsprintf(name,TEXT("%d"),j);  
				size = sizeof(sz);  
				lResult  = RegQueryValueEx(key,name,0,&type,(LPBYTE)sz,&size);  

				lResult = RegOpenKeyEx(keyEnum,sz,0,KEY_READ,&key2);  
				if (ERROR_SUCCESS)  
				{  
					RegCloseKey(keyEnum);  
					return;  
				}  

				size = sizeof(sz);  
				lResult = RegQueryValueEx(key2,TEXT("FriendlyName"),0,&type,(LPBYTE)sz,&size);  
				if(lResult == ERROR_FILE_NOT_FOUND)  
				{  
					size = sizeof(sz);  
					lResult = RegQueryValueEx(key2,TEXT("DeviceDesc"),0,&type,(LPBYTE)sz,&size);  
					names.push_back(sz);//保存显卡名称  
				};  
				RegCloseKey(key2);  
				key2 = NULL;  
			};  
			RegCloseKey(key);  
			key = NULL;  
			break;  
		}  

		////取得显卡名字
		//HKEY hCard;
		//HRESULT hr = RegOpenKeyA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\VIDEO", &hCard);
		//if(ERROR_SUCCESS == hr) {
		//	DWORD iSize = 255, iType = REG_DWORD, iMaxNum = 0;
		//	hr = RegQueryValueExA(hCard, "MaxObjectNumber", NULL, &iType, (LPBYTE)(&iMaxNum), &iSize);

		//	char szDefaultKey[64];
		//	sprintf(szDefaultKey, "\\Device\\Video%d", iMaxNum);
		//	iType = REG_SZ;
		//	iSize = 511;
		//	char szVedioMap[512];
		//	hr = RegQueryValueExA(hCard, szDefaultKey, NULL, &iType, (LPBYTE)szVedioMap, &iSize);
		//	//cout << "vedio-map: " << szVedioMap << endl;
		//	RegCloseKey(hCard);

		//	strcat(szVedioMap, "\\Settings");
		//	int len = strlen("\\Registry\\Machine\\");
		//	hr = RegOpenKeyExA(HKEY_LOCAL_MACHINE, szVedioMap + len, REG_OPTION_OPEN_LINK, KEY_QUERY_VALUE, &hCard);
		//	if(ERROR_SUCCESS == hr) {
		//		iSize = 255;
		//		iType = REG_SZ;
		//		char szVedioName[256];
		//		hr = RegQueryValueExA(hCard, "Device Description", NULL, &iType, (LPBYTE)szVedioName, &iSize);
		//		std::cout << "vedio-card name: \t\t" << szVedioName << endl;
		//	}
		//	RegCloseKey(hCard);
		//}
	}  

	CSystemInfo::EDisplayCardType CSystemInfo::GetDisplayCardType()
	{
		std::string strVender = (char*)glGetString(GL_VENDOR);
		transform(strVender.begin(), strVender.end(), strVender.begin(), tolower);

		if(std::string::npos != strVender.find("ati"))
		{
			m_nDisplayCardType = E_ATI;
		}
		else if(std::string::npos != strVender.find("nvidia"))
		{
			m_nDisplayCardType = E_NVIDIA;
		}
		else if(std::string::npos != strVender.find("intel"))
		{
			m_nDisplayCardType = E_INTEGRATED;
		}
		else
		{
			m_nDisplayCardType = E_Default;
		}
		
		return m_nDisplayCardType;
	}

}
#endif
