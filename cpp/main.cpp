/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#include "FlexLayout.h"
#include "LinearLayout.h"
#include "FlowLayout.h"
#include <vector>
#include <iostream>

#define COUNT 4

void genView(Layout* layout, int index) {
    Item* item = new Item();

    // item->setFlexGrow(1);
    item->setWidth(500);
    item->setHeight(300);
    // item->setHeightPercent(0.25);
    layout->addItem(item);
    return;
    switch (index) {
        case 0:
            item->setAlignSelf(AlignSelf::FLEX_END);
            break;
        case 1:
            item->setFlexGrow(1);
            break;
        case 2:
            item->setFlexBasisPercent(0.5);
            break;
        case 3:
            item->setAlignSelf(AlignSelf::BASELINE);
            break;
        default:
            break;
    }
}

void measure(Layout* layout) {

    for (int i = 0; i < COUNT; i++) {
        genView(layout, i);
    }

    auto measureSpec = Item::MeasureSpec::makeMeasureSpec(10000, Item::MeasureSpec::AT_MOST);
    layout->measure(measureSpec, measureSpec);

    layout->layout(0, 0, layout->getMeasuredWidth(), layout->getMeasuredHeight());

    for (int i = 0; i < COUNT; i++) {
        std::cout << "w:" << layout->getChildAt(i)->getMeasuredWidth()
                  << ", h:" << layout->getChildAt(i)->getMeasuredHeight()
                  << ", l:" << layout->getChildAt(i)->getLeft()
                  << ", t:" << layout->getChildAt(i)->getTop() << std::endl;
    }
}

int main() {
    FlexLayout flexLayout;

    flexLayout.setFlexWrap(FlexWrap::WRAP);
    flexLayout.setMaxLine(100);
    flexLayout.setAlignItems(AlignItems::FLEX_START);
    // layout.setJustifyContent(JustifyContent::CENTER);

    FlowLayout flowLayout;
    // measure(&flowLayout);

    LinearLayout linearLayout;
    linearLayout.setOrientation(LinearLayout::HORIZONTAL);
    measure(&flowLayout);

}