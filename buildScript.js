const { FILE, FORMERR } = require('dns');
const fs = require('fs');

const prjPath = './AssetPlusFund.prj';
const propPath = './prop.inf';
const paramPath = './param.inf';
global.jProject; //json data
global.jProp; //json data
global.jParam; //json data

global.m_CompCountMap; //json value

global.m_csProjectLoc = ''; //string
global.m_csProjectName = ''; //string

global.m_csDefCssInc= ''; //string
global.m_csUsrCssInc= ''; //string
global.m_csDefJsInc = ''; //string
global.m_csUsrJsInc = ''; //string

global.m_AssetMap = new Map(); // map<string, string>
global.m_LayMap = new Map(); // map<string, string>
global.m_ClsList = new Array(); //list<pair<string, string>>

//Enumeration
let FILE_TYPE = {
    TYPE_ROOT: 0,
    TYPE_FOLDER: 1,
    TYPE_CLS: 2,
    TYPE_LAY: 3,
    TYPE_STL: 4,
    TYPE_JS: 5,
    TYPE_CSS: 6,
    TYPE_PLUGIN: 7,
    TYPE_ASSET: 8
}

let REL_TYPE = {
    REL_NONE: 0,
    REL_PROJECT: 1,
    REL_AFC: 2,
    REL_FRM: 3,
    REL_LIB: 4,
    REL_THEME: 5
}

let m_BuildOption = {
    docWidth: 0,
    scaleVal: 0.1,
    autoInc: false,
    dynamicInc: false,
    autoScale: false,
    bridgeName: '',
    //프로젝트 옵션에 프로젝트명 멤버 변수 추가
    projectName: ''
}

//get commanline paramter 
//process.argv.slice(2);
//console.log(process.argv.slice(2).toString());

try{
    if(fs.existsSync(prjPath) && fs.existsSync(propPath) && fs.existsSync(paramPath)){
        
        jProject = JSON.parse(fs.readFileSync(prjPath, 'utf8'));
        jProp = JSON.parse(fs.readFileSync(propPath, 'utf8'));
        jParam = JSON.parse(fs.readFileSync(paramPath, 'utf8'));

        buildFileTree(jProject.fileTree);
        buildLayMap();
        buildRespFile();

        makeIndexFile();

        //const todos = jsonData.todos;
        //todos.forEach(todo => {
        //    console.log(todo);
        //});

        console.log("exist");
    }
}catch(err){
    console.error(err);
}

function setBuildOption(jProp) {
    const general = jProp.general;
    const build = jProp.build;

    m_BuildOption.autoInc = (build.autoInc === 'true');
    m_BuildOption.dynamicInc = (build.dynamicInc === 'true');
    m_BuildOption.docWidth = parseInt(general.docWidth);
    m_BuildOption.autoScale = (general.autoScale === 'true');
    m_BuildOption.scaleVal = parseFloat(general.scaleVal);
    m_BuildOption.bridgeName = build.bridgeName;
    //빌드 시 생성되는 index.html 프로젝트 옵션 정보에 프로젝트명 추가
    m_BuildOption.projectName = build.projectName;
}

function buildFileTree(fileTree){
    let jArray;
    let csName = fileTree.name;

    let type = parseInt(fileTree.type);

    switch(type){

        //root(project)
        case FILE_TYPE.TYPE_ROOT:
            csName = 'bin';

        //folder
        case FILE_TYPE.TYPE_FOLDER:
        {
            if(csName === 'Template') break;
            
            let csFolder = curPath+csName+'\\';
            if (!fs.existsSync(csFolder)) fs.mkdirSync(csFolder);

            jArray = fileTree.children;
            size=jArray.length;
            for(let i=0; i < size; i++){
                buildFileTree(jArray[i], csFolder);
            }
        }
        break;
        
        //file
        default:
        {
            let csSrcPath;
            const csFilePath = fileTree.path;
            const relType = parseInt(fileTree.relType);
            
           switch(relType){
               case REL_TYPE.REL_PROJECT:
                   csSrcPath = m_csProjectLoc+csFilePath+csName;
                   break;
                case REL_TYPE.REL_AFC:
                    csSrcPath = g_strCurPath + HTML_LOC + 'AFC\\' + csFilePath + csName;
                    break;
                case REL_TYPE.REL_FRM:
                    csSrcPath = g_strCurPath + HTML_LOC + 'framework\\' + csFilePath + csName + '.plg';
                    break;
                case REL_TYPE.REL_LIB:
                    csSrcPath = g_strCurPath + HTML_LOC + 'library\\' + csFilePath + csName;
                    break;
                case REL_TYPE.REL_THEME:
                    csSrcPath = g_strCurPath + HTML_LOC + 'theme\\' + csFilePath + csName;
                    break;
                default:
                    csSrcPath = csFilePath + csName;
                    break;
            }

            switch(type)
            {
                case FILE_TYPE.TYPE_CLS:
                {
                    sName = sName.replace(/.cls/gi, '.js') //대소구분없이 확장자 치환
                    const csDstPath = curPath + csName;

                    buildClsFile(csSrcPath, csDstPath);
                    
                    csDstPath = csDstPath.replace(m_csProjectLoc+'bin\\', '');
                    csDstPath = csDstPath.replace('\\', '/'); // 요거는 한번 검토 사항

                    //IOTester 디렉토리내 cls파일 스크립트 Include처리
                    if(csDstPath.search('AFC/') > -1 || csDstPath.search('IOTester/') > -1 || csDstPath.Find('LIB/') > -1)
                        m_csDefJsInc += '\t<script src=\"' + csDstPath + '\"></script>\r\n';
                    
                    //Application 클래스도 무조건 인클루드
                    else if(csName === m_csProjectName+'App.js') m_csUsrJsInc += '\t<script> src=\"' + csDstPath + '\"></script>\r\n';

                    //자동 인클루드
                    else if(m_BuildOption.autoInc)
                    {
                        //레이아웃 동적 인클루드인 경우, 레이아웃, cls파일은 추가하지 않기 위해 차후에 필터링을 수행한다.
                        if(m_BuildOption.dynamicInc) m_ClsList.push({first: csSrcPath, second: csDstPath});
                        else m_csUsrJsInc += '\t<script src=\"' + csDstPath + '\"></script>\r\n';
                    }
                }
                break;
                
                case FILE_TYPE.TYPE_LAY:
                {
                    csName = csName.replace(/.lay/gi, '.html'); //대소문자 관계없이 다 치환

                    m_LayMap.set(csSrcPath, curPath+csName);
                }
                break;
                
                case FILE_TYPE.TYPE_STL:
                {
                    csName = csName.replace(/.stl/gi, '.css'); //대소문자 관계없이 다 치환
                    let csDstPath = curPath + csName;

                    buildStlFile(csSrcPath, csDstPath);

                    csDstPath = csDstPath.replace(m_csProjectLoc+'bin\\', '');
                    csDstPath = csDstPath.replace(/\\/g, '/');

                    if(csDstPath.search('Assets/') < 0)
                        m_csUsrCssInc += '\t<link rel=\"stylesheet\" href=\"' + csDstPath + '\"/>\r\n';
                }
                break;

                case FILE_TYPE.TYPE_JS:
                {
                    let csDstPath = curPath + csName;
                    copySrcFile(scSrcPath, csDstPath);

                    //상대 경로로 변경
                    csDstPath = csDstPath.replace(m_csProjectLoc+'bin\\', '');
                    csDstPath = csDstPath.replace(/\\/g, '/');

                    if(csDstPath.search('AFC/') > -1 || csDstPath.search('LIB/') > -1 || csDstPath.search('IOTester/') > -1)
                    {
                        //주석 처리 된 부분 존재
                        m_csDefJsInc += '\t<script src=\"' + csDstPath + '\"></script>\r\n';
                    }
                }
                break;

                case FILE_TYPE.TYPE_CSS:
                {
                    let csDstPath = curPath+csName;
                    copySrcFile(csSrcPath, csDstPath);

                    //상대 경로로 변경
                    csDstPath = csDstPath.replace(m_csProjectLoc + 'bin\\', '');
                    csDstPath = csDstPath.replace(/\\/g, '/');

                    //IOTester 디렉토리내 css파일 link 처리
                    if(csDstPath.search('AFC/') > -1 || csDstPath.search('LIB/') > -1 || csDstPath.search('Theme/') > -1 || csDstPath.search('IOTester/') > -1)
                        m_csDefCssInc += '\t<link rel=\"stylesheet\" href=\"' + csDstPath + '\"/>\r\n';
                }
                break;

                case FILE_TYPE.TYPE_PLUGIN:
                {
                    let csFolder = curPath + csName + '\\';
                    if (!fs.existsSync(csFolder)) fs.mkdirSync(csFolder); //CreateDirectory

                    let jObj = JSON.parse(csSrcPath)

                    if()
                    {
                        csSrcPath = csSrcPath.replace(csName+'.plg', '');
                        let dstFile = '';
                        let csDstPath = '';

                        const keys = ['style', 'define', 'source'];
                        plgKey = '';
                        let jArr = {};
                        
                        for(let h = 0; h < 3; h++){
                            plgKey = keys[h];
                            jArr = jObj[plgKey];

                            let size = jArr.size();
                            for(let i = 0; i < size; i++){
                                dstFile = jArr[i];
                                csDstPath = csFolder + dstFile;
                                copySrcFile(csSrcPath + plgKey + '\\' + dstFile, csDstPath);

                                //상대경로로 변경
                                csDstPath = csDstPath.replace(m_csProjectLoc + 'bin\\', '');
                                csDstPath = csDstPath.replace(/\\/g, '/');

                                if(plgKey === 'style') m_csDefCssInc += '\t<link rel=\"stylesheet\" href=\"' + csDstPath + '\"/>\r\n';
                                else m_csDefJsInc += '\t<script src=\"' + csDstPath + '\"></script>\r\n';
                            }
                        }
                    }

                }
                break;

                case FILE_TYPE.TYPE_ASSET:
                {
                    let csDstPath = curPath + csName;
                    copySrcFile(csSrcPath, csDstPath);

                    //상대 경로로 변경
                    csDstPath = csDstPath.replace(m_csProjectLoc+'bin\\', '');
                    csDstPath = csDstPath.replace(/\\/g, '/');

                    addAssetMap(csName, csDstPath);
                }
                break;

                default:
                {
                    copySrcFile(csSrcPath, curPath+csName);
                }
                break;
            }
        }
        break;
    }
}

function copySrcFile(srcPath, dstPath){
    fs.copyFile(srcPath, dstPath, (err) => {
        if (err) throw err;
      });
}

//와이드 문자 정상 인식 확인 필요
function addAssetMap(strName, strDstPath)
{
    m_AssetMap.set(strName, strDstPath);
}

function makeIndexFile(){
    let sTemp = '';

    m_ClsList.forEach(it => {
        it.first = it.first.replace(/.cls/gi, '.lay');
        sTemp = m_LayMap.get(it.first);

        if(sTemp === '') m_csUsrJsInc += '\t<script src=\"' + it.second + '\"><script>\r\n';
    });

    let indexFile = INDEX_FILE; // -> 이기 머꼬 찾아라

    indexFile = indexFile.replace(/@title@/g, m_csProjectName);
    indexFile = indexFile.replace(/@default-css@/g, m_csDefCssInc);
    indexFile = indexFile.replace(/@user-css@/g, m_csUsrCssInc);

    //프로젝트 옵션
    indexFile = indexFile.replace(/@auto-inc@/g, m_BuildOption.autoInc ? 'true' : 'false');
    indexFile = indexFile.replace(/@dynamic-inc@/g, m_BuildOption.dynamicInc ? 'true' : 'false');
    indexFile = indexFile.replace(/@auto-scale@/g, m_BuildOption.autoScale ? 'true' : 'false');
    sTemp = String(m_BuildOption.docWidth);
    indexFile = indexFile.replace(/@doc-width@/g, sTemp);
    sTemp = String(m_BuildOption.scaleVal);
    indexFile = indexFile.replace(/@scale-val@/g, sTemp);
    indexFile = indexFile.replace(/@bridge-name@/g, m_BuildOption.bridgeName);
    indexFile = indexFile.replace(/@project-name@/g, m_BuildOption.projectName);

    indexFile = indexFile.replace(/@default-js@/g, m_csDefJsInc);
    indexFile = indexFile.replace(/@user-js@/g, m_csUsrJsInc);

    fs.writeFile(m_csProjectLoc + 'bin\\index.html', indexFile, 'utf8', function(error){
        console.log('write end');
    });
}

function buildLayMap(){
    m_LayMap.forEach(it => {
        buildLayFile(it.first, it.second); // 수정 요
    });
}

function buildClsFile(srcPath, dstPath){
    let sFileData = fs.readFileSync(srcPath, 'utf-8');
    
    let offset = 0;
    let tmp = 0;
    let keyword = '';
    let strTemp = '';
    let className = '';
    let baseClass = '';

    while(true){
        offset = sFileData.indexOf("@", offset);
        if(offset == -1) break;

        tmp = sFileData.indexOf(' \t.(', offset);
        if(tmp == -1) return false;

        keyword = sFileData.substring(offset, tmp);

        //@include <LIB/iScroll.js>  -> document.write('<script src="LIB/iScroll.js"></script>');
        if(keyword === '@include')
        {
            tmp = sFileData.indexOf('<', tmp);
            if(tmp == -1) return false;

            sFileData = sFileData.replace(offset, tmp)
            sFileData = sFileData.substring(0, offset) + 'document.write(\'<script src=\"' + sFileData.substring(nStart+sUrl.length, sData.length);
        }
    }
	var isTrue;
	if(num == 1)
    	isTrue = true;
    else
    	isTrue = false;
    return Boolean(isTrue);
}

function buildLayFile(srcPath, dstPath){
    let sFileData = fs.readFileSync(srcPath, 'utf-8');

    replaceAbsToRel('url(', ')', sFileData);
    replaceAbsToRel('src=\'', '\'', sFileData);

    fs.writeFile(dstPath, sFileData, 'utf8', function(error){
        console.log('write end');
    });
}

function buildStlFile(srcPath, dstPath){
    let sTemp = fs.readFileSync(srcPath, 'utf-8');

    let clsInfo = JSON.parse(sTemp);

    let sFileData='\r\n';
    let items;

    items = clsInfo.items;
    items.forEach(item => {
        sTemp = item.trim();
        sTemp = sTemp.replace(';', CSS_IMPORTANT);

        sFileData+=(sTemp+'\r\n');
    });

    fs.writeFile(dstPath, sFileData);
}

function buildRespFile(){
    let srcpath = "./resp.inf";
    let dstPath = "./bin/resp.css";

    let sTemp = fs.readFileSync(srcPath, 'utf-8');

    let clsInfo = JSON.parse(sTemp);

    let sFileData, left, right, itemKey;
    let inx = 0;
    let items = clsInfo.items;
    for(let key in items){
        let item = items[key];
        itemKey = key.toString();

        sFileData+="\r\n/* "+itemKey+" */r/n";

        for(let key2 in items){}
        //Pause
    }

}

function replaceAbsToRel(strStart, strEnd, sData){
    let nStart = 0, nEnd;
    let sUrl, sName, sReplace;
    let exMark;
    let isMark;

    while(true)
    {
        nStart = sData.indexOf(strStart, nStart);

        if(nStart == -1) break;

        nEnd = sData.indexof(strEnd, nStart+strStart.length);

        sUrl = sData.substring(nStart, nEnd);

        exMark = sUrl.charAt(4);

        isMark = (exmark==='\'' || exMark==='\"');

        if(isMark) exmark = sUrl.charAt(5);

        if(exMark==='#' || exMark==='.')
        {
            nStart += sUrl.length();
            continue;
        }

        //extract file name -> imgName.png or imgName.png'
        sName = sUrl.substring(sUrl.lastIndexOf('/')+1);

        if(isMark) sName = sUrl.substring(0, sName.length-1);

        sReplace = 'url(\''+m_AssetMap[sName] + '\'';
        sData = sData.substring(0, nStart) + sReplace + sData.substring(nStart+sUrl.length, sData.length);

        nStart += sReplace.length;
    }
}

function isTrue(num){
	var isTrue;
	if(num == 1)
    	isTrue = true;
    else
    	isTrue = false;
    return Boolean(isTrue);
}