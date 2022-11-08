var fs  = require('fs');
const { setMaxIdleHTTPParsers } = require('http');
const stringRandom = require('string-random');

let uppercase='ABCDEFGHIGKLMNOPQRSTUVWXYZ'
let same='A'



fs.rm("./pattern.txt", ()=> {
    console.log("删除文件成功")
})

fs.rm("./requests.txt", ()=> {
    console.log("删除文件成功")
})

for (i = 0; i < 10000; i++) {
    fs.writeFileSync("./pattern.txt", stringRandom(4, {numbers: false, letters: uppercase}) + '\n', {flag: 'a'}, (error)=> {
        if (error) {
            console.log("写入数据失败")
        }   
    });
}

for (i = 0; i < 300; i++) {
    fs.writeFileSync("./requests.txt", stringRandom(50, {numbers: false, letters: uppercase}) + '\n', {flag: 'a'}, (error)=> {
        if (error) {
            console.log("写入数据失败")
            console.log(error)
            
        }   
    });
}

console.log("写入数据成功")