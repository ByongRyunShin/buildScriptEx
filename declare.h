#pragma once


//배포버전
//#define HTML_LOC		L"html\\"	//Debug 모드 시 사용
//#define _DECRYPT

//내부 개발 버전
#define HTML_LOC		L"..\\"

#define APP_CLASS		"Application"
//#define APP_CLASS		"AApplication"

#define DEFAULT_FILE	L"index.html"
#define PROJ_EXT		L"prj"
#define CSS_IMPORTANT	" !important;"

#define FULLSCREEN_FLAG	-10000
#define DO_CLOSE		(WM_USER+100)


enum REL_TYPE { REL_NONE=0, REL_PROJECT=1, REL_AFC=2, REL_FRM=3, REL_LIB=4, REL_THEME=5 };
enum FILE_TYPE { TYPE_ROOT=0, TYPE_FOLDER=1, TYPE_CLS=2, TYPE_LAY=3, TYPE_STL=4, TYPE_JS=5, TYPE_CSS=6, TYPE_PLUGIN=7, TYPE_ASSET=8 };

/***********************************************************
 * 2017.12.13 kyh
 * 컴파일시 생성되는 Index.html에 ProjectName 추가
 * IOTest 기능을 위한 자동 생성 스크립트 소스 추가
 ***********************************************************/
#define INDEX_FILE L"\
<!DOCTYPE html>\r\n\
<html>\r\n\
<head>\r\n\
	<meta charset=\"utf-8\">\r\n\
	<title>@title@</title>\r\n\
@default-css@\r\n\
@user-css@\r\n\
	<script>\r\n\
		var PROJECT_OPTION = \r\n\
		{\r\n\
			autoInc: @auto-inc@, dynamicInc: @dynamic-inc@, autoScale: @auto-scale@, docWidth: @doc-width@, scaleVal: @scale-val@,\r\n\
			bridgeName: '@bridge-name@',\r\n\
			projectName: '@project-name@'\r\n\
		};\r\n\
	</script>\r\n\
@default-js@\r\n\
@user-js@\r\n\
</head>\r\n\
<body>\r\n\
	<div id=\"page_navigator\">\r\n\
	</div>\r\n\
</body>\r\n\
</html>\r\n"

#define THE_APP "\r\n\
var theApp = null;\r\n\
\r\n\
$(document).ready(function()\r\n\
{\r\n\
	var urlParams = location.search.split(/[?&]/).slice(1).map(function(paramPair) {\r\n\
		return paramPair.split(/=(.+)?/).slice(0, 2);\r\n\
	}).reduce(function (obj, pairArray) {\r\n\
		obj[pairArray[0]] = pairArray[1];\r\n\
		return obj;\r\n\
	}, {});\r\n\
	if(urlParams.qryTest != 'true') {\r\n\
    	theApp = new @app-constructor@();\r\n\
		//theApp.projectOption = { autoInc: @auto-inc@, dynamicInc: @dynamic-inc@, autoScale: @auto-scale@, docWidth: @doc-width@, scaleVal: @scale-val@ };\r\n\
		theApp.onReady();\r\n\
	}\r\n\
});\r\n"