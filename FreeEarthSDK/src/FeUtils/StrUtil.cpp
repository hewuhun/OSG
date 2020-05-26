#include <string>
#include <sstream>
#include <algorithm>
#include <ctype.h>

#include <osgDB/ConvertUTF>

#include <FeUtils/StrUtil.h>


using namespace std;

string FeUtil::StringReplaceAll( string str, const string& old_value, const string& new_value )
{
    while(true)   
    {   
        string::size_type pos(0);   
        if((pos=str.find(old_value))!=string::npos)   
            str.replace(pos,old_value.length(),new_value);   
        else
            break;   
    }   
    return   str;
}

string FeUtil::StringReplace( string str, const string& old_value, const string& new_value )
{
    for(string::size_type pos(0); pos!=string::npos; pos+=new_value.length())   
    {   
        if((pos=str.find(old_value,pos))!=string::npos)   
            str.replace(pos,old_value.length(),new_value);   
        else   
            break;   
    }   
    return str;
}

void FeUtil::StringSplit( const std::string& s, const std::string& delim, std::vector< std::string >& ret )
{
    size_t last = 0;
    size_t index=s.find_first_of(delim,last);
    while (index!=std::string::npos)
    {
        ret.push_back(s.substr(last,index-last));
        last=index+1;
        index=s.find_first_of(delim,last);
    }
    if (index-last>0)
    {
        ret.push_back(s.substr(last,index-last));
    }
}

std::string FeUtil::StringUpper( std::string s )
{
    std::transform(s.begin(), s.end(), s.begin(),(int(*)(int))toupper);
    return s;
}

std::string FeUtil::StringLower( std::string s )
{
    std::transform(s.begin(), s.end(), s.begin(), (int(*)(int))tolower);
    return s;
}

string FeUtil::StringDirectoryFmt( string str )
{
    str = StringReplace(str, "\\", "/");
    if(str.at(str.size()-1) != '/')
    {
        str += "/";
    }

    return str;
}

extern FEUTIL_EXPORT std::string FeUtil::ToUTF( const std::string& str )
{
	return osgDB::convertStringFromCurrentCodePageToUTF8(str.c_str());
}

extern FEUTIL_EXPORT std::string FeUtil::UTFToCurCode( const std::string& str )
{
	return osgDB::convertStringFromUTF8toCurrentCodePage(str.c_str());
}

bool FeUtil::StringToBool(const string &str)
{
    std::string strBool = StringLower(str);
    if (strBool == "true")
    {
        return true;
    }
    else if (strBool == "false")
    {
        return false;
    }
    else
    {
        return false;
    }
}

string FeUtil::BoolToString(bool bState)
{
    std::string strBool = "false";
    if(bState)
    {
        strBool = "true";
    }

    return strBool;
}

double FeUtil::StringToDouble(const string &str)
{
    std::stringstream ss(str);
    double dData;
    ss>>dData;
    return dData;
}

string FeUtil::DoubleToString(double dData)
{
    std::stringstream ss;
    ss<<dData;
    return ss.str();
}

int FeUtil::StringToInt(const string &str)
{
    std::stringstream ss(str);
    int dData;
    ss>>dData;
    return dData;
}

string FeUtil::IntToString(int nData)
{
    std::stringstream ss;
    ss<<nData;
    return ss.str();
}
