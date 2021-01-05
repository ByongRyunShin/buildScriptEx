const fs = require('fs')

const path = './AssetPlusFund.prj'

try{
    if(fs.existsSync(path)){
        console.log("exist");
    }
}catch(err){
    console.error(err)
}
