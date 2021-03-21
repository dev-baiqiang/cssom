/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */


#include "LinearLayout.h"

void LinearLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    if (mOrientation == VERTICAL) {
        measureVertical(widthMeasureSpec, heightMeasureSpec);
    } else {
        measureHorizontal(widthMeasureSpec, heightMeasureSpec);
    }
}

static int measureNullChild(int childIndex) {
    return 0;
}

static int getChildrenSkipCount(Item* child, int index) {
    return 0;
}

static int getNextLocationOffset(Item* child) {
    return 0;
}

void LinearLayout::measureVertical(int widthMeasureSpec, int heightMeasureSpec) {
    mTotalLength = 0;
    int maxWidth = 0;
    int childState = 0;
    int alternativeMaxWidth = 0;
    int weightedMaxWidth = 0;
    bool allFillParent = true;
    float totalWeight = 0;

    int count = getChildCount();

    int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    int heightMode = MeasureSpec::getMode(heightMeasureSpec);

    bool matchWidth = false;
    bool skippedMeasure = false;

    bool useLargestChild = mUseLargestChild;

    int largestChildHeight = INT_MIN;
    int consumedExcessSpace = 0;

    int nonSkippedChildCount = 0;

    // See how tall everyone is. Also remember max width.
    for (int i = 0; i < count; ++i) {
        Item* child = getChildAt(i);
        if (child == nullptr) {
            mTotalLength += measureNullChild(i);
            continue;
        }

        if (child->getVisibility() == Item::GONE) {
            i += getChildrenSkipCount(child, i);
            continue;
        }

        nonSkippedChildCount++;
        if (hasDividerBeforeChildAt(i)) {
            mTotalLength += mDividerHeight;
        }

        totalWeight += child->getWeight();

        bool useExcessSpace = child->getHeight() == 0 && child->getWeight() > 0;
        if (heightMode == MeasureSpec::EXACTLY && useExcessSpace) {
            // Optimization: don't bother measuring children who are only
            // laid out using excess space. These views will get measured
            // later if we have space to distribute.
            int totalLength = mTotalLength;
            mTotalLength = std::max(totalLength, totalLength + child->getMarginVertical());
            skippedMeasure = true;
        } else {
            if (useExcessSpace) {
                // The heightMode is either UNSPECIFIED or AT_MOST, and
                // this child is only laid out using excess space. Measure
                // using WRAP_CONTENT so that we can find out the view's
                // optimal height. We'll restore the original height of 0
                // after measurement.
                child->setHeight(LayoutParams::WRAP_CONTENT);
            }

            // Determine how big this child would like to be. If this or
            // previous children have given a weight, then we allow it to
            // use all available space (and we will shrink things later
            // if needed).
            int usedHeight = totalWeight == 0 ? mTotalLength : 0;
            measureChildBeforeLayout(child, i, widthMeasureSpec, 0,
                                     heightMeasureSpec, usedHeight);

            int childHeight = child->getMeasuredHeight();
            if (useExcessSpace) {
                // Restore the original height and record how much space
                // we've allocated to excess-only children so that we can
                // match the behavior of EXACTLY measurement.
                child->setHeight(0);
                consumedExcessSpace += childHeight;
            }

            int totalLength = mTotalLength;
            mTotalLength = std::max(totalLength, totalLength + childHeight + child->getMarginVertical() +
                                                 getNextLocationOffset(child));

            if (useLargestChild) {
                largestChildHeight = std::max(childHeight, largestChildHeight);
            }
        }

        bool matchWidthLocally = false;
        if (widthMode != MeasureSpec::EXACTLY && child->getWidth() == LayoutParams::MATCH_PARENT) {
            // The width of the linear layout will scale, and at least one
            // child said it wanted to match our width. Set a flag
            // indicating that we need to remeasure at least that view when
            // we know our width.
            matchWidth = true;
            matchWidthLocally = true;
        }

        int margin = child->getMarginHorizontal();
        int measuredWidth = child->getMeasuredWidth() + margin;
        maxWidth = std::max(maxWidth, measuredWidth);
        childState = combineMeasuredStates(childState, child->getMeasuredState());

        allFillParent = allFillParent && child->getWidth() == LayoutParams::MATCH_PARENT;
        if (child->getWeight() > 0) {
            /*
             * Widths of weighted Views are bogus if we end up
             * remeasuring, so keep them separate.
             */
            weightedMaxWidth = std::max(weightedMaxWidth,
                                        matchWidthLocally ? margin : measuredWidth);
        } else {
            alternativeMaxWidth = std::max(alternativeMaxWidth,
                                           matchWidthLocally ? margin : measuredWidth);
        }

        i += getChildrenSkipCount(child, i);
    }

    if (nonSkippedChildCount > 0 && hasDividerBeforeChildAt(count)) {
        mTotalLength += mDividerHeight;
    }

    if (useLargestChild &&
        (heightMode == MeasureSpec::AT_MOST || heightMode == MeasureSpec::UNSPECIFIED)) {
        mTotalLength = 0;

        for (int i = 0; i < count; ++i) {
            Item* child = getChildAt(i);
            if (child == nullptr) {
                mTotalLength += measureNullChild(i);
                continue;
            }

            if (child->getVisibility() == GONE) {
                i += getChildrenSkipCount(child, i);
                continue;
            }

            // Account for negative margins
            int totalLength = mTotalLength;
            mTotalLength = std::max(totalLength, totalLength + largestChildHeight +
                                                 child->getMarginVertical() + getNextLocationOffset(child));
        }
    }

    // Add in our padding
    mTotalLength += mPaddingTop + mPaddingBottom;

    int heightSize = mTotalLength;
    // Check against our minimum height
    heightSize = std::max(heightSize, getMinHeight());

    // Reconcile our calculated size with the heightMeasureSpec
    int heightSizeAndState = resolveSizeAndState(heightSize, heightMeasureSpec, 0);
    heightSize = heightSizeAndState & MEASURED_SIZE_MASK;
    // Either expand children with weight to take up available space or
    // shrink them if they extend beyond our current bounds. If we skipped
    // measurement on any children, we need to measure them now.
    int remainingExcess = heightSize - mTotalLength + consumedExcessSpace;
    if (skippedMeasure
        || ((remainingExcess != 0) && totalWeight > 0.0f)) {
        float remainingWeightSum = mWeightSum > 0.0f ? mWeightSum : totalWeight;

        mTotalLength = 0;

        for (int i = 0; i < count; ++i) {
            Item* child = getChildAt(i);
            if (child == nullptr || child->getVisibility() == Item::GONE) {
                continue;
            }

            float childWeight = child->getWeight();
            if (childWeight > 0) {
                int share = static_cast<int>(childWeight * remainingExcess / remainingWeightSum);
                remainingExcess -= share;
                remainingWeightSum -= childWeight;

                int childHeight;
                if (mUseLargestChild && heightMode != MeasureSpec::EXACTLY) {
                    childHeight = largestChildHeight;
                } else if (child->getHeight() == 0 && (heightMode == MeasureSpec::EXACTLY)) {
                    // This child needs to be laid out from scratch using
                    // only its share of excess space.
                    childHeight = share;
                } else {
                    // This child had some intrinsic height to which we
                    // need to add its share of excess space.
                    childHeight = child->getMeasuredHeight() + share;
                }

                int childHeightMeasureSpec = MeasureSpec::makeMeasureSpec(
                        std::max(0, childHeight), MeasureSpec::EXACTLY);
                int childWidthMeasureSpec = getChildMeasureSpec(widthMeasureSpec,
                                                                mPaddingLeft + mPaddingRight +
                                                                child->getMarginHorizontal(),
                                                                child->getWidth(), child->getWidthPercent());
                child->measure(childWidthMeasureSpec, childHeightMeasureSpec);

                // Child may now not fit in vertical dimension.
                childState = combineMeasuredStates(childState, child->getMeasuredState()
                                                               & (MEASURED_STATE_MASK >> MEASURED_HEIGHT_STATE_SHIFT));
            }

            int margin = child->getMarginHorizontal();
            int measuredWidth = child->getMeasuredWidth() + margin;
            maxWidth = std::max(maxWidth, measuredWidth);

            bool matchWidthLocally = widthMode != MeasureSpec::EXACTLY &&
                                     child->getWidth() == LayoutParams::MATCH_PARENT;

            alternativeMaxWidth = std::max(alternativeMaxWidth,
                                           matchWidthLocally ? margin : measuredWidth);

            allFillParent = allFillParent && child->getWidth() == LayoutParams::MATCH_PARENT;

            int totalLength = mTotalLength;
            mTotalLength = std::max(totalLength, totalLength + child->getMeasuredHeight() +
                                                 child->getMarginVertical() + getNextLocationOffset(child));
        }

        // Add in our padding
        mTotalLength += mPaddingTop + mPaddingBottom;
        // TODO: Should we recompute the heightSpec based on the new total length?
    } else {
        alternativeMaxWidth = std::max(alternativeMaxWidth,
                                       weightedMaxWidth);


        // We have no limit, so make all weighted views as tall as the largest child.
        // Children will have already been measured once.
        if (useLargestChild && heightMode != MeasureSpec::EXACTLY) {
            for (int i = 0; i < count; i++) {
                Item* child = getChildAt(i);
                if (child == nullptr || child->getVisibility() == Item::GONE) {
                    continue;
                }

                float childExtra = child->getWeight();
                if (childExtra > 0) {
                    child->measure(
                            MeasureSpec::makeMeasureSpec(child->getMeasuredWidth(),
                                                         MeasureSpec::EXACTLY),
                            MeasureSpec::makeMeasureSpec(largestChildHeight,
                                                         MeasureSpec::EXACTLY));
                }
            }
        }
    }

    if (!allFillParent && widthMode != MeasureSpec::EXACTLY) {
        maxWidth = alternativeMaxWidth;
    }

    maxWidth += mPaddingLeft + mPaddingRight;

    // Check against our minimum width
    maxWidth = std::max(maxWidth, getMinWidth());

    setMeasuredDimension(resolveSizeAndState(maxWidth, widthMeasureSpec, childState),
                         heightSizeAndState);

    if (matchWidth) {
        forceUniformWidth(count, heightMeasureSpec);
    }
}

void LinearLayout::measureHorizontal(int widthMeasureSpec, int heightMeasureSpec) {
    mTotalLength = 0;
    int maxHeight = 0;
    int childState = 0;
    int alternativeMaxHeight = 0;
    int weightedMaxHeight = 0;
    bool allFillParent = true;
    float totalWeight = 0;

    const int count = getChildCount();

    const int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);

    bool matchHeight = false;
    bool skippedMeasure = false;

    const bool useLargestChild = mUseLargestChild;
    const bool isExactly = widthMode == MeasureSpec::EXACTLY;

    int largestChildWidth = INT_MIN;
    int usedExcessSpace = 0;

    int nonSkippedChildCount = 0;

    // See how wide everyone is. Also remember max height.
    for (int i = 0; i < count; ++i) {
        auto child = getChildAt(i);
        if (child == nullptr) {
            mTotalLength += measureNullChild(i);
            continue;
        }

        if (child->getVisibility() == GONE) {
            i += getChildrenSkipCount(child, i);
            continue;
        }

        nonSkippedChildCount++;
        if (hasDividerBeforeChildAt(i)) {
            mTotalLength += mDividerWidth;
        }

        totalWeight += child->getWeight();

        const bool useExcessSpace = child->getWidth() == 0 && child->getWeight() > 0;
        if (widthMode == MeasureSpec::EXACTLY && useExcessSpace) {
            // Optimization: don't bother measuring children who are only
            // laid out using excess space. These views will get measured
            // later if we have space to distribute.
            if (isExactly) {
                mTotalLength += getMarginHorizontal();
            } else {
                const int totalLength = mTotalLength;
                mTotalLength = std::max(totalLength, totalLength + getMarginHorizontal());
            }


            skippedMeasure = true;
        } else {
            if (useExcessSpace) {
                // The widthMode is either UNSPECIFIED or AT_MOST, and
                // this child is only laid out using excess space. Measure
                // using WRAP_CONTENT so that we can find out the view's
                // optimal width. We'll restore the original width of 0
                // after measurement.
                setWidth(LayoutParams::WRAP_CONTENT);
            }

            // Determine how big this child would like to be. If this or
            // previous children have given a weight, then we allow it to
            // use all available space (and we will shrink things later
            // if needed).
            const int usedWidth = totalWeight == 0 ? mTotalLength : 0;
            measureChildBeforeLayout(child, i, widthMeasureSpec, usedWidth,
                                     heightMeasureSpec, 0);

            const int childWidth = child->getMeasuredWidth();
            if (useExcessSpace) {
                // Restore the original width and record how much space
                // we've allocated to excess-only children so that we can
                // match the behavior of EXACTLY measurement.
                child->setWidth(0);
                usedExcessSpace += childWidth;
            }

            if (isExactly) {
                mTotalLength += childWidth + child->getMarginHorizontal() + getNextLocationOffset(child);
            } else {
                const int totalLength = mTotalLength;
                mTotalLength = std::max(totalLength, totalLength + childWidth + child->getMarginHorizontal()
                                                     + getNextLocationOffset(child));
            }

            if (useLargestChild) {
                largestChildWidth = std::max(childWidth, largestChildWidth);
            }
        }

        bool matchHeightLocally = false;
        if (heightMode != MeasureSpec::EXACTLY && child->getHeight() == LayoutParams::MATCH_PARENT) {
            // The height of the linear layout will scale, and at least one
            // child said it wanted to match our height. Set a flag indicating that
            // we need to remeasure at least that view when we know our height.
            matchHeight = true;
            matchHeightLocally = true;
        }

        const int margin = child->getMarginVertical();
        const int childHeight = child->getMeasuredHeight() + margin;
        childState = combineMeasuredStates(childState, child->getMeasuredState());

        maxHeight = std::max(maxHeight, childHeight);

        allFillParent = allFillParent && child->getHeight() == LayoutParams::MATCH_PARENT;
        if (child->getWeight() > 0) {
            /*
             * Heights of weighted Views are bogus if we end up
             * remeasuring, so keep them separate.
             */
            weightedMaxHeight = std::max(weightedMaxHeight,
                                         matchHeightLocally ? margin : childHeight);
        } else {
            alternativeMaxHeight = std::max(alternativeMaxHeight,
                                            matchHeightLocally ? margin : childHeight);
        }

        i += getChildrenSkipCount(child, i);
    }

    if (nonSkippedChildCount > 0 && hasDividerBeforeChildAt(count)) {
        mTotalLength += mDividerWidth;
    }


    if (useLargestChild &&
        (widthMode == MeasureSpec::AT_MOST || widthMode == MeasureSpec::UNSPECIFIED)) {
        mTotalLength = 0;

        for (int i = 0; i < count; ++i) {
            Item* child = getChildAt(i);
            if (child == nullptr) {
                mTotalLength += measureNullChild(i);
                continue;
            }

            if (child->getVisibility() == GONE) {
                i += getChildrenSkipCount(child, i);
                continue;
            }

            if (isExactly) {
                mTotalLength += largestChildWidth + child->getMarginHorizontal() +
                                getNextLocationOffset(child);
            } else {
                const int totalLength = mTotalLength;
                mTotalLength = std::max(totalLength, totalLength + largestChildWidth +
                                                     child->getMarginHorizontal() + getNextLocationOffset(child));
            }
        }
    }

    // Add in our padding
    mTotalLength += mPaddingLeft + mPaddingRight;

    int widthSize = mTotalLength;

    // Check against our minimum width
    widthSize = std::max(widthSize, getMinWidth());

    // Reconcile our calculated size with the widthMeasureSpec
    int widthSizeAndState = resolveSizeAndState(widthSize, widthMeasureSpec, 0);
    widthSize = widthSizeAndState & MEASURED_SIZE_MASK;

    // Either expand children with weight to take up available space or
    // shrink them if they extend beyond our current bounds. If we skipped
    // measurement on any children, we need to measure them now.
    int remainingExcess = widthSize - mTotalLength + usedExcessSpace;
    if (skippedMeasure
        || ((remainingExcess != 0) && totalWeight > 0.0f)) {
        float remainingWeightSum = mWeightSum > 0.0f ? mWeightSum : totalWeight;

        maxHeight = -1;

        mTotalLength = 0;

        for (int i = 0; i < count; ++i) {
            Item* child = getChildAt(i);
            if (child == nullptr || child->getVisibility() == Item::GONE) {
                continue;
            }

            const float childWeight = child->getWeight();
            if (childWeight > 0) {
                const int share = (int) (childWeight * remainingExcess / remainingWeightSum);
                remainingExcess -= share;
                remainingWeightSum -= childWeight;

                int childWidth = 0;
                if (mUseLargestChild && widthMode != MeasureSpec::EXACTLY) {
                    childWidth = largestChildWidth;
                } else if (child->getWidth() == 0 && (widthMode == MeasureSpec::EXACTLY)) {
                    // This child needs to be laid out from scratch using
                    // only its share of excess space.
                    childWidth = share;
                } else {
                    // This child had some intrinsic width to which we
                    // need to add its share of excess space.
                    childWidth = child->getMeasuredWidth() + share;
                }

                const int childWidthMeasureSpec = MeasureSpec::makeMeasureSpec(
                        std::max(0, childWidth), MeasureSpec::EXACTLY);
                const int childHeightMeasureSpec = getChildMeasureSpec(heightMeasureSpec,
                                                                       mPaddingTop + mPaddingBottom +
                                                                       child->getMarginVertical(),
                                                                       child->getHeight(), child->getHeightPercent());
                child->measure(childWidthMeasureSpec, childHeightMeasureSpec);

                // Child may now not fit in horizontal dimension.
                childState = combineMeasuredStates(childState,
                                                   child->getMeasuredState() & MEASURED_STATE_MASK);
            }

            if (isExactly) {
                mTotalLength += child->getMeasuredWidth() + child->getMarginHorizontal() +
                                getNextLocationOffset(child);
            } else {
                const int totalLength = mTotalLength;
                mTotalLength = std::max(totalLength, totalLength + child->getMeasuredWidth() +
                                                     child->getMarginHorizontal() + getNextLocationOffset(child));
            }

            bool matchHeightLocally = heightMode != MeasureSpec::EXACTLY &&
                                      child->getHeight() == LayoutParams::MATCH_PARENT;

            const int margin = child->getMarginVertical();
            int childHeight = child->getMeasuredHeight() + margin;
            maxHeight = std::max(maxHeight, childHeight);
            alternativeMaxHeight = std::max(alternativeMaxHeight,
                                            matchHeightLocally ? margin : childHeight);

            allFillParent = allFillParent && child->getHeight() == LayoutParams::MATCH_PARENT;

        }

        // Add in our padding
        mTotalLength += mPaddingLeft + mPaddingRight;
        // TODO: Should we update widthSize with the new total length?

    } else {
        alternativeMaxHeight = std::max(alternativeMaxHeight, weightedMaxHeight);

        // We have no limit, so make all weighted views as wide as the largest child.
        // Children will have already been measured once.
        if (useLargestChild && widthMode != MeasureSpec::EXACTLY) {
            for (int i = 0; i < count; i++) {
                Item* child = getChildAt(i);
                if (child == nullptr || child->getVisibility() == Item::GONE) {
                    continue;
                }


                float childExtra = child->getWeight();
                if (childExtra > 0) {
                    child->measure(
                            MeasureSpec::makeMeasureSpec(largestChildWidth, MeasureSpec::EXACTLY),
                            MeasureSpec::makeMeasureSpec(child->getMeasuredHeight(),
                                                         MeasureSpec::EXACTLY));
                }
            }
        }
    }

    if (!allFillParent && heightMode != MeasureSpec::EXACTLY) {
        maxHeight = alternativeMaxHeight;
    }

    maxHeight += mPaddingTop + mPaddingBottom;

    // Check against our minimum height
    maxHeight = std::max(maxHeight, getMinHeight());

    setMeasuredDimension(widthSizeAndState | (childState & MEASURED_STATE_MASK),
                         resolveSizeAndState(maxHeight, heightMeasureSpec,
                                             (childState << MEASURED_HEIGHT_STATE_SHIFT)));

    if (matchHeight) {
        forceUniformHeight(count, widthMeasureSpec);
    }
}

bool LinearLayout::hasDividerBeforeChildAt(int childIndex) {

    if (childIndex == getChildCount()) {
        // Check whether the end divider should draw.
        return (mShowDividers & SHOW_DIVIDER_END) != 0;
    }
    bool all = allViewsAreGoneBefore(childIndex);
    if (all) {
        // This is the first view that's not gone, check if beginning divider is enabled.
        return (mShowDividers & SHOW_DIVIDER_BEGINNING) != 0;
    } else {
        return (mShowDividers & SHOW_DIVIDER_MIDDLE) != 0;
    }
}

void LinearLayout::measureChildBeforeLayout(Item* child, int childIndex, int widthMeasureSpec, int totalWidth,
                                            int heightMeasureSpec, int totalHeight) {
    measureChildWithMargins(child, widthMeasureSpec, totalWidth,
                            heightMeasureSpec, totalHeight);
}

void LinearLayout::forceUniformHeight(int count, int widthMeasureSpec) {
    // Pretend that the linear layout has an exact size. This is the measured height of
    // ourselves. The measured height should be the max height of the children, changed
    // to accommodate the heightMeasureSpec from the parent
    int uniformMeasureSpec = MeasureSpec::makeMeasureSpec(getMeasuredHeight(),
                                                          MeasureSpec::EXACTLY);
    for (int i = 0; i < count; ++i) {
        Item* child = getChildAt(i);
        if (child != nullptr && child->getVisibility() != GONE) {

            if (child->getHeight() == LayoutParams::MATCH_PARENT) {
                // Temporarily force children to reuse their old measured width
                // FIXME: this may not be right for something like wrapping text?
                int oldWidth = child->getWidth();
                child->setWidth(child->getMeasuredWidth());
                // Remeasure with new dimensions
                measureChildWithMargins(child, widthMeasureSpec, 0, uniformMeasureSpec, 0);
                child->setWidth(oldWidth);

            }
        }
    }
}

void LinearLayout::forceUniformWidth(int count, int heightMeasureSpec) {
    // Pretend that the linear layout has an exact size.
    int uniformMeasureSpec = MeasureSpec::makeMeasureSpec(getMeasuredWidth(),
                                                          MeasureSpec::EXACTLY);
    for (int i = 0; i < count; ++i) {
        Item* child = getChildAt(i);
        if (child != nullptr && child->getVisibility() != GONE) {

            if (child->getWidth() == LayoutParams::MATCH_PARENT) {
                // Temporarily force children to reuse their old measured height
                // FIXME: this may not be right for something like wrapping text?
                int oldHeight = child->getHeight();
                child->setHeight(child->getMeasuredHeight());

                // Remeasue with new dimensions
                measureChildWithMargins(child, uniformMeasureSpec, 0, heightMeasureSpec, 0);
                child->setHeight(oldHeight);
            }
        }
    }
}

bool LinearLayout::allViewsAreGoneBefore(int childIndex) {
    for (int i = childIndex - 1; i >= 0; i--) {
        Item* child = getChildAt(i);
        if (child != nullptr && child->getVisibility() != GONE) {
            return false;
        }
    }
    return true;
}
