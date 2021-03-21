/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <climits>

struct FlexLine {

    int mLeft = INT_MAX;

    int mTop = INT_MAX;

    int mRight = INT_MIN;

    int mBottom = INT_MIN;

    /** @see #getMainSize() */
    int mMainSize = 0;

    /** @see #getCrossSize() */
    int mCrossSize = 0;

    /** @see #getItemCount() */
    int mItemCount = 0;

    /** Holds the count of the views whose visibilities are gone */
    int mGoneItemCount = 0;

    /** @see #getTotalFlexGrow() */
    float mTotalFlexGrow = 0;

    /** @see #getTotalFlexShrink() */
    float mTotalFlexShrink = 0;

    /**
     * The largest value of the individual child's baseline (obtained by View#getBaseline()
     * if the {@link FlexContainer#getAlignItems()} value is not {@link AlignItems#BASELINE}
     * or the flex direction is vertical, this value is not used.
     * If the alignment direction is from the bottom to top,
     * (e.g. flexWrap == WRAP_REVERSE and flexDirection == ROW)
     * store this value from the distance from the bottom of the view minus baseline.
     * (Calculated as view.getMeasuredHeight() - view.getBaseline - LayoutParams.bottomMargin)
     */
    int mMaxBaseline = 0;

    /**
     * The sum of the cross size used before this flex line.
     */
    int mSumCrossSizeBefore = 0;

    /**
     * Store the indices of the children views whose alignSelf property is stretch.
     * The stored indices are the absolute indices including all children in the Flexbox,
     * not the relative indices in this flex line.
     */
    std::vector<int> mIndicesAlignSelfStretch;

    int mFirstIndex = 0;

    int mLastIndex = 0;

    /**
     * Set to true if any {@link FlexItem}s in this line have {@link FlexItem#getFlexGrow()}
     * attributes set (have the value other than {@link FlexItem#FLEX_GROW_DEFAULT})
     */
    bool mAnyItemsHaveFlexGrow = false;

    /**
     * Set to true if any {@link FlexItem}s in this line have {@link FlexItem#getFlexShrink()}
     * attributes set (have the value other than {@link FlexItem#FLEX_SHRINK_NOT_SET})
     */
    bool mAnyItemsHaveFlexShrink = false;

    /**
     * @return the size of the flex line in pixels along the main axis of the flex container.
     */
    int getMainSize() const {
        return mMainSize;
    }

    /**
     * @return the size of the flex line in pixels along the cross axis of the flex container.
     */
    int getCrossSize() const {
        return mCrossSize;
    }

    /**
     * @return the count of the views contained in this flex line.
     */
    int getItemCount() const {
        return mItemCount;
    }

    /**
     * @return the count of the views whose visibilities are not gone in this flex line.
     */
    int getItemCountNotGone() const {
        return mItemCount - mGoneItemCount;
    }

    /**
     * @return the sum of the flexGrow properties of the children included in this flex line
     */
    float getTotalFlexGrow() const {
        return mTotalFlexGrow;
    }

    /**
     * @return the sum of the flexShrink properties of the children included in this flex line
     */
    float getTotalFlexShrink() const {
        return mTotalFlexShrink;
    }

    /**
     * @return the first view's index included in this flex line.
     */
    int getFirstIndex() const {
        return mFirstIndex;
    }
};

