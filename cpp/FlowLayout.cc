/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#include "FlowLayout.h"

static int getMeasuredDimension(int size, int mode, int childrenEdge) {
    switch (mode) {
        case FlowLayout::MeasureSpec::EXACTLY:
            return size;
        case FlowLayout::MeasureSpec::AT_MOST:
            return std::min(childrenEdge, size);
        default: // UNSPECIFIED:
            return childrenEdge;
    }
}

void FlowLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    int width = MeasureSpec::getSize(widthMeasureSpec);
    int widthMode = MeasureSpec::getMode(widthMeasureSpec);

    int height = MeasureSpec::getSize(heightMeasureSpec);
    int heightMode = MeasureSpec::getMode(heightMeasureSpec);

    int maxWidth =
            widthMode == MeasureSpec::AT_MOST || widthMode == MeasureSpec::EXACTLY
            ? width
            : INT_MAX;

    int childLeft = getPaddingLeft();
    int childTop = getPaddingTop();
    int childBottom = childTop;
    int childRight = childLeft;
    int maxChildRight = 0;
    int maxRight = maxWidth - getPaddingRight();
    for (int i = 0; i < getChildCount(); i++) {
        Item* child = getChildAt(i);

        if (child->getVisibility() == Item::GONE) {
            continue;
        }
        measureChild(child, widthMeasureSpec, heightMeasureSpec);

        int leftMargin = child->getMarginLeft();
        int rightMargin = child->getMarginRight();

        childRight = childLeft + leftMargin + child->getMeasuredWidth();

        // If the current child's right bound exce->ds Flowlayout's max right bound and flowlayout is
        // not confined to a single line, move this child to the next line and reset its left bound to
        // flowlayout's left bound.
        if (childRight > maxRight && !isSingleLine()) {
            childLeft = getPaddingLeft();
            childTop = childBottom + mLineSpacing;
        }

        childRight = childLeft + leftMargin + child->getMeasuredWidth();
        childBottom = childTop + child->getMeasuredHeight();

        // Updates Flowlayout's max right bound if current child's right bound exceeds it.
        if (childRight > maxChildRight) {
            maxChildRight = childRight;
        }

        childLeft += (leftMargin + rightMargin + child->getMeasuredWidth()) + mItemSpacing;

        // For all preceding children, the child's right margin is taken into account in the next
        // child's left bound (childLeft). However, childLeft is ignored after the last child so the
        // last child's right margin needs to be explicitly added to Flowlayout's max right bound.
        if (i == (getChildCount() - 1)) {
            maxChildRight += rightMargin;
        }
    }

    maxChildRight += getPaddingRight();
    childBottom += getPaddingBottom();

    int Width = getMeasuredDimension(width, widthMode, maxChildRight);
    int Height = getMeasuredDimension(height, heightMode, childBottom);
    setMeasuredDimension(Width, Height);
}

void FlowLayout::onLayout(bool sizeChanged, int left, int top, int right, int bottom) {
    if (getChildCount() == 0) {
        // Do not re-layout when there are no children.
        mRowCount = 0;
        return;
    }
    mRowCount = 1;

    bool isRtl = false;
    int paddingStart = isRtl ? getPaddingRight() : getPaddingLeft();
    int paddingEnd = isRtl ? getPaddingLeft() : getPaddingRight();
    int childStart = paddingStart;
    int childTop = getPaddingTop();
    int childBottom = childTop;
    int childEnd;

    int maxChildEnd = right - left - paddingEnd;

    for (int i = 0; i < getChildCount(); i++) {
        Item* child = getChildAt(i);

        if (child->getVisibility() == Item::GONE) {
            continue;
        }

        int startMargin = child->getMarginLeft();
        int endMargin = child->getMarginRight();

        childEnd = childStart + startMargin + child->getMeasuredWidth();

        if (!mSingleLine && (childEnd > maxChildEnd)) {
            childStart = paddingStart;
            childTop = childBottom + mLineSpacing;
            mRowCount++;
        }

        childEnd = childStart + startMargin + child->getMeasuredWidth();
        childBottom = childTop + child->getMeasuredHeight();

        if (isRtl) {
            child->layout(
                    maxChildEnd - childEnd, childTop, maxChildEnd - childStart - startMargin, childBottom);
        } else {
            child->layout(childStart + startMargin, childTop, childEnd, childBottom);
        }

        childStart += (startMargin + endMargin + child->getMeasuredWidth()) + mItemSpacing;
    }
}

int FlowLayout::getRowIndex(Item* item) {
    return 0;
}
