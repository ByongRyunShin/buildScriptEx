#pragma once

#include <json/json.h>

class BuildOption
{
public:
	int docWidth;
	double scaleVal;
	bool autoInc;
	bool dynamicInc;
	bool autoScale;
	CString bridgeName;
	// 2017.12.13 kyh : 프로젝트 옵션에 프로젝트명 멤버 변수 추가
	CString projectName;
};

class CProjectBuilder
{
public:
	CProjectBuilder(void);
	~CProjectBuilder(void);

	bool Run(LPCTSTR prjLoc, LPCTSTR fileName);
	void SetBuildOption(Json::Value & jProp);
	
	bool LoadJsonFile(LPCTSTR strPath, Json::Value & jObj);

protected:
	void BuildFileTree(const Json::Value & value, LPCTSTR curPath);
	bool ReadSrcFile(LPCTSTR srcPath, string & retVal);
	bool WriteDstFile(LPCTSTR dstPath, string & data);
	BOOL CopySrcFile(LPCTSTR srcPath, LPCTSTR dstPath);

	void MakeIndexFile();
	void BuildLayMap();

	bool BuildClsFile(LPCTSTR srcPath, LPCTSTR dstPath);
	bool BuildLayFile(LPCTSTR srcPath, LPCTSTR dstPath);
	bool BuildStlFile(LPCTSTR srcPath, LPCTSTR dstPath);
	bool BuildRespFile();

	void AddAssetMap(LPCTSTR strName, LPCTSTR strDstPath);
	void ReplaceAbsToRel(LPCSTR strStart, LPCSTR strEnd, string & sData);

	void TestInclude();

	CString m_csProjectLoc;
	CString m_csProjectName;

	CString m_csDefCssInc;
	CString m_csUsrCssInc;
	CString m_csDefJsInc;
	CString m_csUsrJsInc;

	map<string, string> m_AssetMap;
	map<CString, CString> m_LayMap;
	list<pair<CString,CString>> m_ClsList;

	BuildOption m_BuildOption;
	Json::Value m_CompCountMap;
};
