#include "StdAfx.h"
#include "ProjectBuilder.h"

CProjectBuilder::CProjectBuilder()
{
}


CProjectBuilder::~CProjectBuilder()
{
}

void CProjectBuilder::SetBuildOption(Json::Value & jProp)
{
	Json::Value general = jProp["general"];
	Json::Value build = jProp["build"];

	m_BuildOption.autoInc = build["autoInc"].asBool();
	m_BuildOption.dynamicInc = build["dynamicInc"].asBool();
	m_BuildOption.docWidth = general["docWidth"].asInt();
	m_BuildOption.autoScale = general["autoScale"].asBool();
	m_BuildOption.scaleVal = general["scaleVal"].asDouble();
	m_BuildOption.bridgeName = build["bridgeName"].asString().c_str();
	// 2017.12.13 kyh : 빌드시 생성되는 index.html 프로젝트 옵션 정보에
	// 프로젝트명 추가.
	m_BuildOption.projectName = build["projectName"].asString().c_str();
}

bool CProjectBuilder::LoadJsonFile(LPCTSTR strPath, Json::Value & jObj)
{
	char* buf = ReadFileToStrBuf(strPath);
	if(buf==NULL) return false;

	string jsonData(buf);
	delete [] buf;

	Json::Features features = Json::Features::all();
	Json::Reader reader(features);
	if(!reader.parse(jsonData, jObj)) 
	{
		//CString aa(reader.getFormatedErrorMessages().c_str());
		//AfxMessageBox(aa);
		return false;
	}

	return true;
}

bool CProjectBuilder::Run(LPCTSTR prjLoc, LPCTSTR fileName)
{
	m_csProjectLoc = prjLoc;
	
	Json::Value jProject, jProp, jParam;
	if(!LoadJsonFile(m_csProjectLoc+fileName, jProject) ||
		!LoadJsonFile(m_csProjectLoc+L"prop.inf", jProp) ||
		!LoadJsonFile(m_csProjectLoc+L"param.inf", jParam)) return false;

	SetBuildOption(jProp);

	m_CompCountMap = jParam["compCountMap"];
	m_csProjectName = jProject["projectName"].asCString();

	BuildFileTree(jProject["fileTree"], m_csProjectLoc);
	BuildLayMap();
	BuildRespFile();

	MakeIndexFile();

	return true;
}

void CProjectBuilder::BuildFileTree(const Json::Value & fileTree, LPCTSTR curPath)
{
	Json::Value jArray;
	CString csName(fileTree["name"].asCString());

	int type = fileTree["type"].asInt();
	switch(type)
	{
		//root(project)
		case TYPE_ROOT:	
			csName = L"bin";
			//아래에서 폴더가 생성되도록 break 생략

		//folder
		case TYPE_FOLDER:	
		{
			if(csName.Compare(L"Template")==0) break;

			CString csFolder;
			csFolder.Format(L"%s%s\\", curPath, csName);
			CreateDirectory(csFolder, NULL);

			jArray = fileTree["children"];
			for(int i=0,size=jArray.size(); i<size; ++i)
				BuildFileTree(jArray[i], csFolder);
		}
		break;

		//file
		default:
		{
			CString csSrcPath, csFilePath(fileTree["path"].asCString());
			int relType = fileTree["relType"].asInt();

			switch(relType)
			{
				case REL_PROJECT: csSrcPath.Format(L"%s%s%s", m_csProjectLoc, csFilePath, csName); break;
				case REL_AFC: csSrcPath.Format(L"%s%sAFC\\%s%s", g_strCurPath, HTML_LOC, csFilePath, csName); break;
				case REL_FRM: csSrcPath.Format(L"%s%sframework\\%s%s.plg", g_strCurPath, HTML_LOC, csFilePath, csName); break;
				case REL_LIB: csSrcPath.Format(L"%s%slibrary\\%s%s", g_strCurPath, HTML_LOC, csFilePath, csName); break;
				case REL_THEME: csSrcPath.Format(L"%s%stheme\\%s%s", g_strCurPath, HTML_LOC, csFilePath, csName); break;
				default: csSrcPath.Format(L"%s%s", csFilePath, csName); break;
			}

			switch(type)
			{
				case TYPE_CLS: 
				{
					csName.Replace(L".cls", L".js");
					CString csDstPath(curPath+csName);

					BuildClsFile(csSrcPath, csDstPath);

					csDstPath.Replace(m_csProjectLoc+L"bin\\", L"");
					csDstPath.Replace(L"\\", L"/");

					// default library files
					/*******************************************************
					 * 2017.12.13 kyh
					 * IOTester 디렉토리내 cls파일 스크립트 Include 처리
					 *******************************************************/
					if(csDstPath.Find(L"AFC/")>-1 || csDstPath.Find(L"IOTester/")>-1 || csDstPath.Find(L"LIB/")>-1)
						m_csDefJsInc.Append(L"\t<script src=\"" + csDstPath + L"\"></script>\r\n");

					//Application 클래스도 무조건 인클루드
					else if(csName.Compare(m_csProjectName+L"App.js")==0) m_csUsrJsInc.Append(L"\t<script src=\"" + csDstPath + L"\"></script>\r\n");

					//자동 인클루드
					else if(m_BuildOption.autoInc) 
					{
						//레이아웃 동적 인클루드인 경우, 레이아웃 cls 파일은 추가하지 않기 위해 차후에 필터링을 수행한다.
						if(m_BuildOption.dynamicInc) m_ClsList.push_back(pair<CString,CString>(csSrcPath, csDstPath));
						else m_csUsrJsInc.Append(L"\t<script src=\"" + csDstPath + L"\"></script>\r\n");
					}
				}
				break;

				case TYPE_LAY: 
				{
					csName.Replace(L".lay", L".html");
					//BuildLayFile(csSrcPath, curPath+csName); 
					
					m_LayMap[csSrcPath] = (curPath+csName);
				}
				break;

				case TYPE_STL: 
				{
					csName.Replace(L".stl", L".css");
					CString csDstPath(curPath+csName);

					BuildStlFile(csSrcPath, csDstPath);

					csDstPath.Replace(m_csProjectLoc+L"bin\\", L"");
					csDstPath.Replace(L"\\", L"/");

					if(csDstPath.Find(L"Assets/")<0)
						m_csUsrCssInc.Append(L"\t<link rel=\"stylesheet\" href=\"" + csDstPath + L"\"/>\r\n");
				}
				break;

				case TYPE_JS:
				{
					CString csDstPath(curPath+csName);
					CopySrcFile(csSrcPath, csDstPath); 

					//상대 경로로 변경
					csDstPath.Replace(m_csProjectLoc+L"bin\\", L"");
					csDstPath.Replace(L"\\", L"/");

					/*******************************************************
					 * 2017.12.13 kyh
					 * IOTester 디렉토리내 cls파일 스크립트 Include 처리
					 *******************************************************/
					if(csDstPath.Find(L"AFC/")>-1 || csDstPath.Find(L"LIB/")>-1 || csDstPath.Find(L"IOTester/")>-1)
					{
						//사용한 컴포넌트 js만 인클루드
						//잠시 보류, data-base, data-class 상속관계 재설계후 진행할 것.
						//자신의 부모 js 파일도 인클루드해야 하므로
						/*
						if(csDstPath.Find(L"AFC/Component/")>-1) 
						{
							csName.Replace(L".js", L"");

							string compName = CW2A(csName);
							Json::Value jCount = m_CompCountMap[compName];

							if(!jCount.empty() && jCount.asInt()>0)
							{
								m_csDefJsInc.Append(L"\t<script src=\"" + csDstPath + L"\"></script>\r\n");
							}
						}
						else if(csDstPath.Find(L"AFC/Event/")>-1) 
						{
							csName.Replace(L"Event.js", L"");

							string compName = CW2A(csName);
							Json::Value jCount = m_CompCountMap[compName];

							if(!jCount.empty() && jCount.asInt()>0)
							{
								m_csDefJsInc.Append(L"\t<script src=\"" + csDstPath + L"\"></script>\r\n");
							}
						}
						else
						{
							m_csDefJsInc.Append(L"\t<script src=\"" + csDstPath + L"\"></script>\r\n");
						}
						*/

						m_csDefJsInc.Append(L"\t<script src=\"" + csDstPath + L"\"></script>\r\n");
					}
				}
				break;

				case TYPE_CSS:
				{
					CString csDstPath(curPath+csName);
					CopySrcFile(csSrcPath, csDstPath); 

					//상대 경로로 변경
					csDstPath.Replace(m_csProjectLoc+L"bin\\", L"");
					csDstPath.Replace(L"\\", L"/");

					/*******************************************************
					 * 2017.12.13 kyh
					 * IOTester 디렉토리내 css 파일 link 처리
					 *******************************************************/
					if(csDstPath.Find(L"AFC/")>-1 || csDstPath.Find(L"LIB/")>-1 || csDstPath.Find(L"Theme/")>-1 || csDstPath.Find(L"IOTester/")>-1)
						m_csDefCssInc.Append(L"\t<link rel=\"stylesheet\" href=\"" + csDstPath + L"\"/>\r\n");
				}
				break;

				case TYPE_PLUGIN:
				{
					CString csFolder;
					csFolder.Format(L"%s%s\\", curPath, csName);
					CreateDirectory(csFolder, NULL);

					Json::Value jObj;

					if(ParseJson(jObj, csSrcPath))
					{
						csSrcPath.Replace(csName+L".plg", L"");
						CString dstFile, csDstPath;

						const WCHAR* keys[] = { L"style", L"define", L"source" };  
						CString plgKey;
						Json::Value jArr;

						for(int h=0; h<3; h++)
						{
							plgKey = keys[h];
							jArr = jObj[CW2A(plgKey)];
							for(int i=0,size=jArr.size(); i<size; ++i)
							{
								dstFile = jArr[i].asString().c_str();
								csDstPath = csFolder + dstFile;
								CopySrcFile(csSrcPath + plgKey + L"\\" + dstFile, csDstPath);
							
								//상대 경로로 변경
								csDstPath.Replace(m_csProjectLoc+L"bin\\", L"");
								csDstPath.Replace(L"\\", L"/");

								if(plgKey.Compare(L"style")==0) m_csDefCssInc.Append(L"\t<link rel=\"stylesheet\" href=\"" + csDstPath + L"\"/>\r\n");
								else m_csDefJsInc.Append(L"\t<script src=\"" + csDstPath + L"\"></script>\r\n");
							}
						}
					}
				}
				break;

				case TYPE_ASSET:
				{
					CString csDstPath(curPath+csName);
					CopySrcFile(csSrcPath, csDstPath); 

					//상대 경로로 변경
					csDstPath.Replace(m_csProjectLoc+L"bin\\", L"");
					csDstPath.Replace(L"\\", L"/");

					AddAssetMap(csName, csDstPath);
				}
				break;

				default:
				{
					CopySrcFile(csSrcPath, curPath+csName); 
				}
				break;
			}
		}
		break;
	}
}

bool CProjectBuilder::ReadSrcFile(LPCTSTR srcPath, string & retVal)
{
	char* buf = ReadFileToStrBuf(srcPath);

	if(buf==NULL) return false;
	else
	{
		retVal = buf;
		delete [] buf;
	}

	return true;
}

bool CProjectBuilder::WriteDstFile(LPCTSTR dstPath, string & data)
{
	HANDLE hFile = CreateFile(dstPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile==INVALID_HANDLE_VALUE) return false;
	else
	{
		CFile file(hFile);
		file.Write(data.c_str(),  data.size());
		file.Close();
	}

	return true;
}

BOOL CProjectBuilder::CopySrcFile(LPCTSTR srcPath, LPCTSTR dstPath)
{
	return ::CopyFile(srcPath, dstPath, FALSE);
}

void CProjectBuilder::AddAssetMap(LPCTSTR strName, LPCTSTR strDstPath)
{
	string sName, sDstPath;
	int nLen; char* buf;

	nLen = WCtoMB(strName, CP_UTF8, buf);
	sName = buf;
	delete [] buf;

	nLen = WCtoMB(strDstPath, CP_UTF8, buf);
	sDstPath = buf;
	delete [] buf;

	m_AssetMap[sName] = sDstPath;
}

void CProjectBuilder::MakeIndexFile()
{
	list<pair<CString,CString>>::iterator it = m_ClsList.begin();
	list<pair<CString,CString>>::iterator end = m_ClsList.end();

	CString sTemp;
	for(; it!=end; it++)
	{
		it->first.Replace(L".cls", L".lay");
		
		sTemp = m_LayMap[it->first];

		//레이아웃과 연결된 cls 파일이 아닌 경우만 인클루드 한다.
		if(sTemp.IsEmpty()) m_csUsrJsInc.Append(L"\t<script src=\"" + it->second + L"\"></script>\r\n");
	}
	

	CString indexFile = INDEX_FILE;

	indexFile.Replace(L"@title@", m_csProjectName);
	indexFile.Replace(L"@default-css@", m_csDefCssInc);
	indexFile.Replace(L"@user-css@", m_csUsrCssInc);

	// 프로젝트 옵션 
	// 2017.12.13 kyh : ProjectName 옵션에 추가.
	indexFile.Replace(L"@auto-inc@", m_BuildOption.autoInc ? L"true" : L"false");
	indexFile.Replace(L"@dynamic-inc@", m_BuildOption.dynamicInc ? L"true" : L"false");
	indexFile.Replace(L"@auto-scale@", m_BuildOption.autoScale ? L"true" : L"false");
	sTemp.Format(L"%d", m_BuildOption.docWidth);
	indexFile.Replace(L"@doc-width@", sTemp);
	sTemp.Format(L"%f", m_BuildOption.scaleVal);
	indexFile.Replace(L"@scale-val@", sTemp);
	indexFile.Replace(L"@bridge-name@", m_BuildOption.bridgeName);
	indexFile.Replace(L"@project-name@", m_BuildOption.projectName);
	//----
	
	indexFile.Replace(L"@default-js@", m_csDefJsInc);
	indexFile.Replace(L"@user-js@", m_csUsrJsInc);

	WriteFileToUTF(m_csProjectLoc+L"bin\\index.html", indexFile);
}

void CProjectBuilder::BuildLayMap()
{
	map<CString, CString>::iterator it = m_LayMap.begin();
	map<CString, CString>::iterator end = m_LayMap.end();

	for(; it!=end; it++)
		BuildLayFile(it->first, it->second); 
}

/*
@include "LIB/iScroll.js"			-> document.write('<script src="LIB/iScroll.js"></script>');

@class TestView()					-> function TestView()
{
	@super();						-> AView.call(this);

	//TODO:edit here

}
@extends AView;						-> afc.extendsClass(TestView, AView);


@function TestView.init(context)	-> TestView.prototype.init = function(context)
{
	@super.init(context);			-> AView.prototype.init.call(this, context);


	//TODO:edit here

};
*/

bool CProjectBuilder::BuildClsFile(LPCTSTR srcPath, LPCTSTR dstPath)
{
	string sFileData;
	ReadSrcFile(srcPath, sFileData);

	size_t offset = 0, tmp;
	string keyword, strTemp, className, baseClass = "";

	while(TRUE)
	{
		offset = sFileData.find("@", offset);
		if(offset==string::npos) break;

		tmp = sFileData.find_first_of(" \t.(", offset);
		if(tmp==string::npos) return false;

		keyword = sFileData.substr(offset, tmp-offset);

		//@include <LIB/iScroll.js>			-> document.write('<script src="LIB/iScroll.js"></script>');
		if(keyword=="@include")
		{
			tmp = sFileData.find("<", tmp);
			if(tmp==string::npos) return false;

			sFileData.replace(offset, tmp-offset+1, "document.write('<script src=\""); //@include -> document.write('<script src=
			offset += strlen("document.write('<script src=\"");

			offset = sFileData.find(">", offset+1);
			if(offset==string::npos) return false;

			sFileData.replace(offset, 1, "\"></script>');");
			offset += strlen("\"></script>');");
		}

		//@class TestView()					-> function TestView()
		else if(keyword=="@class")
		{
			sFileData.replace(offset, strlen("@class"), "function");
			offset += strlen("function");

			tmp = sFileData.find("(", offset);

			className = sFileData.substr(offset, tmp-offset);
			_trim(className);

			offset = tmp;
		}

		else if(keyword=="@super")
		{
			//@super.init(context);			-> AView.prototype.init.call(this, context);
			if(sFileData[tmp]=='.')
			{
				strTemp = baseClass+".prototype";
				sFileData.replace(offset, tmp-offset, strTemp);
				offset += strTemp.size();

				offset = sFileData.find("(", offset);

				sFileData.replace(offset, 1, ".call(this, ");	// ( -> .call(this,  ==  @super.init( -> @super.init.call(this,
				offset += strlen(".call(this, ");

				tmp = sFileData.find_first_not_of(" \t", offset);
				//파라미터가 없으면 콤마 제거
				if(sFileData[tmp]==')') sFileData.erase(offset-2, 2);
			}

			//@super();						-> AView.call(this);
			else
			{
				//공백이 있을 수 있으므로 다시 찾는다. -> @super ();
				tmp = sFileData.find("(", tmp);
				tmp++;

				strTemp = "@base-class@.call(this, ";
				sFileData.replace(offset, tmp-offset, strTemp);
				offset += strTemp.size();

				tmp = sFileData.find_first_not_of(" \t", offset);
				//파라미터가 없으면 콤마 제거
				if(sFileData[tmp]==')') sFileData.erase(offset-2, 2);
			}
		}

		//@extends AView;						-> afc.extendsClass(TestView, AView);
		else if(keyword=="@extends")
		{
			size_t end = sFileData.find(";", tmp);

			baseClass = sFileData.substr(tmp, end-tmp);
			_trim(baseClass);

			strTemp = "afc.extendsClass("+className+", "+baseClass+")";
			sFileData.replace(offset, end-offset, strTemp);
			offset += strTemp.size();
		}

		//@function TestView.init(context)	-> TestView.prototype.init = function(context)
		else if(keyword=="@function")
		{
			tmp = sFileData.find_first_not_of(" \t", tmp);

			sFileData.erase(offset, tmp-offset);	//@function TestView.init(context) -> TestView.init(context)

			offset = sFileData.find_first_of(":.*", offset);	//:.* 모두 가능
			sFileData.replace(offset, 1, ".prototype.");	//TestView:init(context) -> TestView.prototype.init(context)
			offset += strlen(".prototype.");

			offset = sFileData.find("(", offset);
			sFileData.replace(offset, 1, " = function(");

			offset += strlen(" = function(");
		}

		else offset = tmp;
	}

	if(!baseClass.empty()) 
	{
		_replace(sFileData, "@base-class@", baseClass.c_str());

		if(baseClass==APP_CLASS) 
		{
			strTemp = THE_APP;

			//size_t inx = _replace(strTemp, "@app-constructor@", className.c_str());
			_replace(strTemp, "@app-constructor@", className.c_str());

			sFileData.append(strTemp);
		}
	}
	
	WriteDstFile(dstPath, sFileData);

	return true;

}


void CProjectBuilder::TestInclude()
{
	/*
	string s;
	const char* find;
	size_t startInx = 0;
	size_t inx = startInx;

	while(TRUE)
	{
		startInx = s.find(find, inx);
		if(startInx==string::npos) break;

		s.replace(startInx, strlen(find), replace);
		inx = startInx + strlen(replace);
	}

	return inx;
	*/
}

bool CProjectBuilder::BuildLayFile(LPCTSTR srcPath, LPCTSTR dstPath)
{
	string sFileData;
	ReadSrcFile(srcPath, sFileData);

	ReplaceAbsToRel("url(", ")", sFileData);
	ReplaceAbsToRel("src=\"", "\"", sFileData);

	WriteDstFile(dstPath, sFileData);

	return true;
}

/***************************************************************************************************
{
	styleName: "button_style",
	items : 
	{
		btn_normal: ".btn_normal { background-color: #f0f0f0; border:1px solid black; }",
		btn_touch: ".btn_touch { background-color: #ffffff; border:1px solid white;  }"
	}
}
***************************************************************************************************/
bool CProjectBuilder::BuildStlFile(LPCTSTR srcPath, LPCTSTR dstPath)
{
	string sTemp;
	ReadSrcFile(srcPath, sTemp);

	Json::Features features = Json::Features::all();
	Json::Value clsInfo;
	Json::Reader reader(features);
	if(!reader.parse(sTemp, clsInfo)) return false;

	string sFileData;
	Json::Value jObj, items;

	sFileData.append("\r\n");

	//스타일 아이템 목록을 얻어온다.
	items = clsInfo["items"];

	Json::ValueIterator it = items.begin();
	Json::ValueIterator end = items.end();
	for(; it!=end; it++)
	{
		sTemp = (*it).asString();
		_trim(sTemp);
		_replace(sTemp, ";", CSS_IMPORTANT);

		sFileData.append(sTemp+"\r\n\r\n");
	}

	WriteDstFile(dstPath, sFileData);

	return true;
}

/***************************************************************************************************
{
	"styleName": "responsive",
	"items":
	{
		"MainPage":	//class name
		{
			"orientation_landscape": //media condition
			{
				"MainPage--KospiView": //Component id
				{
					"width": "30%", //css value
					"height": "50%"
				}
			},
			
			"orientation_portrait": {}		
		},
		...
	}
}
***************************************************************************************************/
bool CProjectBuilder::BuildRespFile()
{
	//m_csDefCssInc.Append(L"\t<link rel=\"stylesheet\" href=\"resp.css\"/>\r\n");

	CString srcPath = m_csProjectLoc + L"resp.inf";
	CString dstPath = m_csProjectLoc + L"bin\\resp.css";

	string sTemp;
	ReadSrcFile(srcPath, sTemp);

	Json::Features features = Json::Features::all();
	Json::Value clsInfo;
	Json::Reader reader(features);
	if(!reader.parse(sTemp, clsInfo)) return false;


	string sFileData, left, right, itemKey;
	size_t inx = 0;
	Json::Value items, cssObj;

	//스타일 아이템 목록을 얻어온다.
	items = clsInfo["items"];

	for(Json::ValueIterator it1=items.begin(); it1!=items.end(); it1++)	//it1 -> MainPage
	{
		itemKey = it1.key().asString();

		sFileData.append("\r\n/* "+itemKey+" */\r\n");

		for(Json::ValueIterator it2=(*it1).begin(); it2!=(*it1).end(); it2++)	//it2 -> orientation_landscape
		{
			//condObj = (*it2);

			sTemp = it2.key().asString();
			inx = sTemp.find("_", 0);

			left = sTemp.substr(0, inx);
			right = sTemp.substr(inx+1);

			sFileData.append("@media screen and ("+left+":"+right+")\r\n{\r\n");
			
			for(Json::ValueIterator it3=(*it2).begin(); it3!=(*it2).end(); it3++)
			{
				sFileData.append("\t."+itemKey+" #"+it3.key().asString()+"\r\n\t{\r\n");

				for(Json::ValueIterator it4=(*it3).begin(); it4!=(*it3).end(); it4++)
				{
					sFileData.append("\t\t"+it4.key().asString()+":"+(*it4).asString()+" !important;\r\n");
				}
				sFileData.append("\t}\r\n");
			}

			sFileData.append("}\r\n");
		}
	}

	if(!sFileData.empty()) m_csDefCssInc.Append(L"\t<link rel=\"stylesheet\" href=\"resp.css\"/>\r\n");

	WriteDstFile(dstPath, sFileData);

	return true;
}


void CProjectBuilder::ReplaceAbsToRel(LPCSTR strStart, LPCSTR strEnd, string & sData)
{
	size_t nStart = 0, nEnd;
	string sUrl, sName, sReplace;
	char exMark;
	bool isMark;

	while(true)
	{
		nStart = sData.find(strStart, nStart);

		if(nStart==string::npos) break;

		nEnd = sData.find(strEnd, nStart+strlen(strStart));

		//extract sUrl -> url('~/~/~.png' or url(~/~/~.png
		sUrl = sData.substr(nStart, nEnd-nStart);

		exMark = sUrl.at(4);
		isMark = (exMark=='\'' || exMark=='\"');

		if(isMark) exMark = sUrl.at(5);

		//ex) svg value or relative path to skip
		if(exMark=='#' || exMark=='.')
		{
			nStart += sUrl.size();
			continue;
		}

		//extract file name -> imgName.png or imgName.png'
		sName = sUrl.substr(sUrl.find_last_of("/")+1);

		if(isMark) sName.erase(sName.length()-1, 1);

		sReplace = "url('" + m_AssetMap[sName] + "'";
		sData.replace(nStart, sUrl.size(), sReplace);

		nStart += sReplace.size();
	}

}
