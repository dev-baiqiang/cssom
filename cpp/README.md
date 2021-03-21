# FlexLayout 实现

Flexible Box 模型，通常被称为 flexbox，是一种一维的布局模型。它给 flexbox 的子元素之间提供了强大的空间分布和对齐能力

本文参考 facebook/yoga 和 google/flexlayout 给出一种 flex 的布局实现过程

### Flex 前提

对于 Flex 容器来说，和它相关的布局参数及默认行为如下：

* 元素排列为一行(flex-direction 属性的初始值是 row)
* 元素从主轴的起始线开始(justify-content 初始值 start)
* 元素不会在主维度方向拉伸，但是可以缩小(即容器大等于子元素大小时候不采取措施，但是小于的时候会根据子元素 flex 参数缩小)
* 元素被拉伸来填充交叉轴大小(align-items 为 stretch)
* flex-wrap 属性为 nowrap，即单行

对于子元素来说主要的是三个

* flex-grow，若被赋值为一个正整数，flex 元素会以 flex-basis 为基础，沿主轴方向增长尺寸，默认为 1，可以按比例分配空间
* flex-shrink，是处理flex元素收缩的问题，其默认值为 1，在所有因子相加之后计算比率来进行空间收缩
* flex-basis，默认为 auto，即参照元素的 width 和 height 属性进行分配，也可以指定大小

关于 shrink 计算过程可能比较复杂，举例如下：

```html
<style>
    #content {
        display: flex;
        width: 500px;
    }

    #content div {
        flex-basis: 120px;
    }

    .box {
        flex-shrink: 1;
    }

    .box1 {
        flex-shrink: 2;
    }
</style>
<div id="content">
    <div class="box" style="background-color:red;">A</div>
    <div class="box" style="background-color:lightblue;">B</div>
    <div class="box" style="background-color:yellow;">C</div>
    <div class="box1" style="background-color:brown;">D</div>
    <div class="box1" style="background-color:lightgreen;">E</div>
</div>
```

上面 A/B/C 定义 flex-shrink 为 1，D/E 定义了 flex-shrink 为 2，即整体比例为 1:1:1:2:2

可以看到父容器大小 500px，子项 120px，子项相加之后 600px，超出父容器 100px。那么超出的 100px 需要被 A/B/C/D/E 分摊，通过收缩因子，所以加权综合可得 100+100+100+100x2+100x2=700

计算 A/B/C/D/E 将被分摊的：

* A/B/C: (100x1/700)x100，即约等于14px
* D/E: (100x2/700)x100，即约等于28px

最后 A/B/C/D/E 的实际宽度分别为：120-14=106px, 120-14=106px, 120-14=106px, 120-28=92px,120-28=92px

对于对齐方面主要属性如下

* justify-content - 控制主轴上所有 flex 项目的对齐，start/center/end 等
* align-items - 控制交叉轴上所有 flex 项目的对齐，start/center/stretch/end 等
* align-self - 控制交叉轴上的单个 flex 项目的对齐，start/center/end 等
* align-content - 控制"多条主轴"的 flex 项目在交叉轴的对齐，start/center/end 等


### 确定 FlexLine

首先是需要明确下一些布局的参数和关键的信息，



