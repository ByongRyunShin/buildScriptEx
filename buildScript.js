const { FILE, FORMERR } = require('dns');
const fs = require('fs');

const prjPath = './AssetPlusFund.prj';
const propPath = './prop.inf';
const paramPath = './param.inf';
global.jProject; //json data
global.jProp; //json data
global.jParam; //json data

global.m_CompCountMap; //json value

global.m_csProjectLoc; //string
global.m_csProjectName; //string

global.m_csDefCssInc; //string
global.m_csUsrCssInc; //string
global.m_csDefJsInc; //string
global.m_csUsrJsInc; //string

global.m_AssetMap; // map<string, string>
global.m_LayMap; // map<string, string>
global.m_ClsList; //list<pair<string, string>>

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

let REL_TYPE ={
    REL_NONE: 0,
    REL_PROJECT: 1,
    REL_AFC: 2,
    REL_FRM: 3,
    REL_LIB: 4,
    REL_THEME: 5
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

function buildFileTree(fileTree){
    let csName = fileTree.name;

    let type = parseInt(fileTree.type);

    switch(type){

        //root(project)
        case FILE_TYPE.TYPE_ROOT:
            let csName = 'bin';

        //folder
        case DeclareEnum.TYPE_FOLDER:
            if(csName == "Tempalte") break;
            
            let csFolder = "./"+csName+"/";

        break;
        
        //file
        default:
        {
            let scSrcPath, csFilePath = fileTree.path;

            let relType = parseInt(fileTree.relType);
            
           switch(relType){
               case REL_TYPE.REL_PROJECT:
                   break;
                case REL_TYPE.REL_AFC:
                    break;
                case REL_TYPE.REL_FRM:
                    break;
                case REL_TYPE.REL_LIB:
                    break;
                case REL_TYPE.REL_THEME:
                    break;
                default:
                    break;
            }

            switch(type)
            {
                case FILE_TYPE.TYPE_CLS:
                {

                }
                break;

                case FILE_TYPE.TYPE_LAY:
                {

                }
                break;
                
                case FILE_TYPE.TYPE_STL:
                {

                }
                break;

                case FILE_TYPE.TYPE_JS:
                {

                }
                break;

                case FILE_TYPE.TYPE_CSS:
                {

                }
                break;

                case FILE_TYPE.TYPE_PLUGIN:
                {

                }
                break;

                case FILE_TYPE.TYPE_ASSET:
                {

                }
                break;

                default:
                {

                }
                break;
            }
        }
        break;
    }
}

function copySrcFile(srcPath, dstPath){
	var isTrue;
	if(num == 1)
    	isTrue = true;
    else
    	isTrue = false;
    return Boolean(isTrue);
}

function addAssetMap(strName, strDstPath)
{
    let sName, sDstPath;
    let nLen, buf;

}

function makeIndexFile(){

}

function buildLayMap(){

}

function buildClsFile(srcPath, dstPath){
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

        isMark = (exmark=='\'' || exMark=='\"');

        if(isMark) exmark = sUrl.charAt(5);

        if(exMark=='#' || exMark=='.')
        {
            nStart += sUrl.length();
            continue;
        }

        sName = sUrl.substring()
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