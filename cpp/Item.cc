/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#include "Item.h"

void Item::measure(int widthMeasureSpec, int heightMeasureSpec) {
    onMeasure(widthMeasureSpec, heightMeasureSpec);
}

int Item::getMeasuredState() {
    return (mMeasuredWidth & MEASURED_STATE_MASK)
           | ((mMeasuredHeight >> MEASURED_HEIGHT_STATE_SHIFT)
              & (MEASURED_STATE_MASK >> MEASURED_HEIGHT_STATE_SHIFT));
}

int Item::getBaseline() {
    return 0;
}

int Item::getDefaultSize(int size, int measureSpec) {
    int result = size;
    int specMode = MeasureSpec::getMode(measureSpec);
    int specSize = MeasureSpec::getSize(measureSpec);

    switch (specMode) {
        case MeasureSpec::UNSPECIFIED:
            result = size;
            break;
        case MeasureSpec::AT_MOST:
        case MeasureSpec::EXACTLY:
        default:
            result = specSize;
            break;
    }
    return result;
}

void Item::setMeasuredDimension(int measuredWidth, int measuredHeight) {
    mMeasuredWidth = measuredWidth;
    mMeasuredHeight = measuredHeight;
}

void Item::layout(int l, int t, int r, int b) {
    bool changed = setFrame(l, t, r, b);
    if (changed) onLayout(changed, l, t, r, b);
}

bool Item::setFrame(int left, int top, int right, int bottom) {
    bool changed = false;

    if (mLeft != left || mRight != right || mTop != top || mBottom != bottom) {
        changed = true;

        int oldWidth = mRight - mLeft;
        int oldHeight = mBottom - mTop;
        int newWidth = right - left;
        int newHeight = bottom - top;
        bool sizeChanged = (newWidth != oldWidth) || (newHeight != oldHeight);

        mLeft = left;
        mTop = top;
        mRight = right;
        mBottom = bottom;

    }
    return changed;
}

int Item::resolveSizeAndState(int size, int measureSpec, int childMeasuredState) {
    int specMode = MeasureSpec::getMode(measureSpec);
    int specSize = MeasureSpec::getSize(measureSpec);
    int result;
    switch (specMode) {
        case MeasureSpec::AT_MOST:
            if (specSize < size) {
                result = specSize | MEASURED_STATE_TOO_SMALL;
            } else {
                result = size;
            }
            break;
        case MeasureSpec::EXACTLY:
            result = specSize;
            break;
        case MeasureSpec::UNSPECIFIED:
        default:
            result = size;
    }
    return result | (childMeasuredState & MEASURED_STATE_MASK);
}

void Item::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    setMeasuredDimension(getDefaultSize(mMinWidth, widthMeasureSpec),
                         getDefaultSize(mMinHeight, heightMeasureSpec));
}
