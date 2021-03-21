/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#include "Layout.h"

int Layout::getChildMeasureSpec(int spec, int padding, int childDimension, float percent) {
    int specMode = MeasureSpec::getMode(spec);
    int specSize = MeasureSpec::getSize(spec);

    int size = std::max(0, specSize - padding);

    int resultSize = 0;
    int resultMode = 0;

    switch (specMode) {
        // Parent has imposed an exact size on us
        case MeasureSpec::EXACTLY:
            if (childDimension >= 0) {
                resultSize = childDimension;
                resultMode = MeasureSpec::EXACTLY;
            } else if (childDimension == LayoutParams::MATCH_PARENT) {
                // Child wants to be our size. So be it.
                resultSize = std::min(static_cast<int>(specSize * percent), size);
                resultMode = MeasureSpec::EXACTLY;
            } else if (childDimension == LayoutParams::WRAP_CONTENT) {
                // Child wants to determine its own size. It can't be
                // bigger than us.
                resultSize = size;
                resultMode = MeasureSpec::AT_MOST;
            }
            break;

            // Parent has imposed a maximum size on us
        case MeasureSpec::AT_MOST:
            if (childDimension >= 0) {
                // Child wants a specific size... so be it
                resultSize = childDimension;
                resultMode = MeasureSpec::EXACTLY;
            } else if (childDimension == LayoutParams::MATCH_PARENT) {

                // Child wants to be our size, but our size is not fixed.
                // Constrain child to not be bigger than us.
                resultSize = std::min(static_cast<int>(specSize * percent), size);
                resultMode = MeasureSpec::AT_MOST;
            } else if (childDimension == LayoutParams::WRAP_CONTENT) {
                // Child wants to determine its own size. It can't be
                // bigger than us.
                resultSize = size;
                resultMode = MeasureSpec::AT_MOST;
            }
            break;

            // Parent asked to see how big we want to be
        case MeasureSpec::UNSPECIFIED:
            if (childDimension >= 0) {
                // Child wants a specific size... let him have it
                resultSize = childDimension;
                resultMode = MeasureSpec::EXACTLY;
            } else if (childDimension == LayoutParams::MATCH_PARENT) {
                // Child wants to be our size... find out how big it should
                // be
                resultSize = std::min(static_cast<int>(specSize * percent), size);
                resultMode = MeasureSpec::UNSPECIFIED;
            } else if (childDimension == LayoutParams::WRAP_CONTENT) {
                // Child wants to determine its own size.... find out how
                // big it should be
                resultSize = size;
                resultMode = MeasureSpec::UNSPECIFIED;
            }
            break;
    }
    // noinspection ResourceType
    return MeasureSpec::makeMeasureSpec(resultSize, resultMode);
}

void Layout::measureChild(Item* child, int parentWidthMeasureSpec, int parentHeightMeasureSpec) {

    int childWidthMeasureSpec = getChildMeasureSpec(parentWidthMeasureSpec,
                                                    mPaddingLeft + mPaddingRight, child->getWidth(),
                                                    child->getWidthPercent());
    int childHeightMeasureSpec = getChildMeasureSpec(parentHeightMeasureSpec,
                                                     mPaddingTop + mPaddingBottom, child->getHeight(),
                                                     child->getHeightPercent());

    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}

void Layout::measureChildren(int widthMeasureSpec, int heightMeasureSpec) {
    for (auto& child : mChildren) {
        if (child->getVisibility() != GONE) {
            measureChild(child, widthMeasureSpec, heightMeasureSpec);
        }
    }
}

void Layout::measureChildWithMargins(Item* child, int parentWidthMeasureSpec, int widthUsed,
                                     int parentHeightMeasureSpec, int heightUsed) {

    int childWidthMeasureSpec = getChildMeasureSpec(parentWidthMeasureSpec,
                                                    mPaddingLeft + mPaddingRight + child->getMarginHorizontal()
                                                    + widthUsed, child->getWidth(), child->getWidthPercent());
    int childHeightMeasureSpec = getChildMeasureSpec(parentHeightMeasureSpec,
                                                     mPaddingTop + mPaddingBottom + child->getMarginVertical()
                                                     + heightUsed, child->getHeight(), child->getHeightPercent());

    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}
