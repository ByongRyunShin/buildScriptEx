const fs = require('fs');

const jsonFile = fs.readFileSync('./AssetPlusFund.prj', 'utf8');
//console.log(jsonFile);

const jsonData = JSON.parse(jsonFile);
//console.log(jsonData);

const fileTree = jsonData.fileTree;
const jArray = fileTree.children;
const size = jArray.length;
for(let i=0; i < size; i++){
    console.log(jArray[i]);
    //buildFileTree(jArray[i], csFolder);
}
/*
todos.forEach(todo => {
    console.log(todo);
});
*/
//잃어버린 변수 찾기
// stdafx.h를 잘 찾아보도록 하자
// g_strCurPath, HTML_LOC, 
//global.h
//CString g_strCurPath = L"";