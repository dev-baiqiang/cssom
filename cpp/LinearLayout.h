/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */


#pragma once

#include "Layout.h"

class LinearLayout : public Layout {
public:
    static constexpr int HORIZONTAL = 0;
    static constexpr int VERTICAL = 1;

    /**
     * Don't show any dividers.
     */
    static constexpr int SHOW_DIVIDER_NONE = 0;
    /**
     * Show a divider at the beginning of the group.
     */
    static constexpr int SHOW_DIVIDER_BEGINNING = 1;
    /**
     * Show dividers between each item in the group.
     */
    static constexpr int SHOW_DIVIDER_MIDDLE = 2;
    /**
     * Show a divider at the end of the group.
     */
    static constexpr int SHOW_DIVIDER_END = 4;


    void setOrientation(int orientation) {
        if (mOrientation != orientation) {
            mOrientation = orientation;
        }
    }

protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;


    /**
     * <p>Measure the child according to the parent's measure specs. This
     * method should be overridden by subclasses to force the sizing of
     * children. This method is called by {@link #measureVertical(int, int)} and
     * {@link #measureHorizontal(int, int)}.</p>
     *
     * @param child the child to measure
     * @param childIndex the index of the child in this view
     * @param widthMeasureSpec horizontal space requirements as imposed by the parent
     * @param totalWidth extra space that has been used up by the parent horizontally
     * @param heightMeasureSpec vertical space requirements as imposed by the parent
     * @param totalHeight extra space that has been used up by the parent vertically
     */
    void measureChildBeforeLayout(Item* child, int childIndex,
                                  int widthMeasureSpec, int totalWidth, int heightMeasureSpec,
                                  int totalHeight);

private:
    int mOrientation = 0;
    int mTotalLength = 0;
    float mWeightSum = 0;
    bool mUseLargestChild = false;
    int mShowDividers = 0;
    int mDividerWidth = 0;
    int mDividerHeight = 0;

    void measureVertical(int widthMeasureSpec, int heightMeasureSpec);

    void measureHorizontal(int widthMeasureSpec, int heightMeasureSpec);

    bool hasDividerBeforeChildAt(int childIndex);

    void forceUniformHeight(int count, int widthMeasureSpec);

    void forceUniformWidth(int count, int heightMeasureSpec);

    bool allViewsAreGoneBefore(int childIndex);
};
