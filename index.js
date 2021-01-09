const {onCSSChanged} = require('./style-scope');
const {View} = require('./View');

onCSSChanged({
    cssText: ` View[xxx]{background:red;  --main-bg-color: pink; } .main .p+view{background: var(--main-bg-color);} `
});

let v = new View();
v.id = "main";
v.xxx = '';
v.className = 'main';

let child = new View();
child.className = 'p';
v.addChild(child);

let child2 = new View();
child2.className = 'pp';
v.addChild(child2);

v._onCssStateChange();

console.log(v.children[0]);