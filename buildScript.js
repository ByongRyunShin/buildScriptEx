const fs = require('fs');

const prjPath = './AssetPlusFund.prj';
const propPath = './prop.inf';
const paramPath = './param.inf';
global.jProject;
global.jProp;
global.jParam;

global.m_CompCountMap;
global.m_csProjectName;

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

    }
}

function buildLayMap(){

}

function buildRespFile(){

}

function makeIndexFile(){

}