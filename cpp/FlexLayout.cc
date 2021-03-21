/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#include "FlexLayout.h"

FlexLayout::FlexLayout() : mFlexboxHelper(this) {
}

void FlexLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    switch (mFlexDirection) {
        case FlexDirection::ROW: // Intentional fall through
        case FlexDirection::ROW_REVERSE:
            measureHorizontal(widthMeasureSpec, heightMeasureSpec);
            break;
        case FlexDirection::COLUMN: // Intentional fall through
        case FlexDirection::COLUMN_REVERSE:
            measureVertical(widthMeasureSpec, heightMeasureSpec);
            break;
        default:
            throw std::invalid_argument(
                    "Invalid value for the flex direction is set: " + std::to_string(mFlexDirection));
    }
}

void FlexLayout::measureHorizontal(int widthMeasureSpec, int heightMeasureSpec) {
    mFlexLines.clear();

    mFlexLinesResult.reset();
    mFlexboxHelper.calculateHorizontalFlexLines(mFlexLinesResult, widthMeasureSpec,
                                                heightMeasureSpec);
    mFlexLines = mFlexLinesResult.mFlexLines;

    mFlexboxHelper.determineMainSize(widthMeasureSpec, heightMeasureSpec);

    mFlexboxHelper.determineCrossSize(widthMeasureSpec, heightMeasureSpec,
                                      getPaddingTop() + getPaddingBottom());
    // Now cross size for each flex line is determined.
    // Expand the views if alignItems (or mAlignSelf in each child view) is set to stretch
    mFlexboxHelper.stretchViews();
    setMeasuredDimensionForFlex(mFlexDirection, widthMeasureSpec, heightMeasureSpec,
                                mFlexLinesResult.mChildState);
}

void FlexLayout::measureVertical(int widthMeasureSpec, int heightMeasureSpec) {
    mFlexLines.clear();
    mFlexLinesResult.reset();
    mFlexboxHelper.calculateVerticalFlexLines(mFlexLinesResult, widthMeasureSpec,
                                              heightMeasureSpec);
    mFlexLines = mFlexLinesResult.mFlexLines;

    mFlexboxHelper.determineMainSize(widthMeasureSpec, heightMeasureSpec);
    mFlexboxHelper.determineCrossSize(widthMeasureSpec, heightMeasureSpec,
                                      getPaddingLeft() + getPaddingRight());
    // Now cross size for each flex line is determined.
    // Expand the views if alignItems (or mAlignSelf in each child view) is set to stretch
    mFlexboxHelper.stretchViews();
    setMeasuredDimensionForFlex(mFlexDirection, widthMeasureSpec, heightMeasureSpec,
                                mFlexLinesResult.mChildState);
}

int FlexLayout::getSumOfCrossSize() {
    int sum = 0;
    for (const auto& flexLine : mFlexLines) {
        sum += flexLine.mCrossSize;
    }
    return sum;
}

int FlexLayout::getLargestMainSize() {
    int largestSize = INT_MIN;
    for (const auto& flexLine : mFlexLines) {
        largestSize = std::max(largestSize, flexLine.mMainSize);
    }
    return largestSize;
}

void FlexLayout::onLayout(bool changed, int left, int top, int right, int bottom) {
    bool isRtl = false;
    switch (mFlexDirection) {
        case FlexDirection::ROW:
            layoutHorizontal(isRtl, left, top, right, bottom);
            break;
        case FlexDirection::ROW_REVERSE:
            layoutHorizontal(isRtl, left, top, right, bottom);
            break;
        case FlexDirection::COLUMN:
            if (mFlexWrap == FlexWrap::WRAP_REVERSE) {
                isRtl = !isRtl;
            }
            layoutVertical(isRtl, false, left, top, right, bottom);
            break;
        case FlexDirection::COLUMN_REVERSE:
            if (mFlexWrap == FlexWrap::WRAP_REVERSE) {
                isRtl = !isRtl;
            }
            layoutVertical(isRtl, true, left, top, right, bottom);
            break;
        default:
            throw std::invalid_argument("Invalid flex direction is set: " + std::to_string(mFlexDirection));
    }
}

void FlexLayout::layoutHorizontal(bool isRtl, int left, int top, int right, int bottom) {
    int paddingLeft = getPaddingLeft();
    int paddingRight = getPaddingRight();
    // Use float to reduce the round error that may happen in when justifyContent ==
    // SPACE_BETWEEN or SPACE_AROUND
    float childLeft;

    int height = bottom - top;
    int width = right - left;
    // childBottom is used if the mFlexWrap is WRAP_REVERSE otherwise
    // childTop is used to align the vertical position of the children views.
    int childBottom = height - getPaddingBottom();
    int childTop = getPaddingTop();

    // Used only for RTL layout
    // Use float to reduce the round error that may happen in when justifyContent ==
    // SPACE_BETWEEN or SPACE_AROUND
    float childRight;
    for (int i = 0, size = mFlexLines.size(); i < size; i++) {
        FlexLine& flexLine = mFlexLines[i];
        float spaceBetweenItem = 0;
        switch (mJustifyContent) {
            case JustifyContent::FLEX_START:
                childLeft = paddingLeft;
                childRight = width - paddingRight;
                break;
            case JustifyContent::FLEX_END:
                childLeft = width - flexLine.mMainSize + paddingRight;
                childRight = flexLine.mMainSize - paddingLeft;
                break;
            case JustifyContent::CENTER:
                childLeft = paddingLeft + static_cast<float>(width - flexLine.mMainSize) / 2;
                childRight = width - paddingRight - static_cast<float>(width - flexLine.mMainSize) / 2;
                break;
            case JustifyContent::SPACE_AROUND: {
                int visibleCount = flexLine.getItemCountNotGone();
                if (visibleCount != 0) {
                    spaceBetweenItem = static_cast<float>(width - flexLine.mMainSize)
                                       / static_cast<float>( visibleCount);
                }
                childLeft = paddingLeft + spaceBetweenItem / 2;
                childRight = width - paddingRight - spaceBetweenItem / 2;
                break;
            }
            case JustifyContent::SPACE_BETWEEN: {
                childLeft = paddingLeft;
                int visibleCount = flexLine.getItemCountNotGone();
                float denominator = visibleCount != 1 ? visibleCount - 1 : 1;
                spaceBetweenItem = static_cast<float>(width - flexLine.mMainSize) / denominator;
                childRight = width - paddingRight;
                break;
            }
            case JustifyContent::SPACE_EVENLY: {
                int visibleCount = flexLine.getItemCountNotGone();
                if (visibleCount != 0) {
                    spaceBetweenItem = static_cast<float>(width - flexLine.mMainSize)
                                       / static_cast<float>(visibleCount + 1);
                }
                childLeft = paddingLeft + spaceBetweenItem;
                childRight = width - paddingRight - spaceBetweenItem;
                break;
            }
            default:
                throw std::invalid_argument(
                        "Invalid justifyContent is set: " + std::to_string(mJustifyContent));
        }
        spaceBetweenItem = std::max(spaceBetweenItem, 0.f);

        for (int j = 0; j < flexLine.mItemCount; j++) {
            int index = flexLine.mFirstIndex + j;
            Item* child = getFlexItemAt(index);
            if (child == nullptr || child->getVisibility() == Item::GONE) {
                continue;
            }
            childLeft += child->getMarginLeft();
            childRight -= child->getMarginRight();

            if (mFlexWrap == FlexWrap::WRAP_REVERSE) {
                if (isRtl) {
                    mFlexboxHelper.layoutSingleChildHorizontal(child, flexLine,
                                                               round(childRight) - child->getMeasuredWidth(),
                                                               childBottom - child->getMeasuredHeight(),
                                                               round(childRight),
                                                               childBottom);
                } else {
                    mFlexboxHelper.layoutSingleChildHorizontal(child, flexLine,
                                                               round(childLeft),
                                                               childBottom - child->getMeasuredHeight(),
                                                               round(childLeft) + child->getMeasuredWidth(),
                                                               childBottom);
                }
            } else {
                if (isRtl) {
                    mFlexboxHelper.layoutSingleChildHorizontal(child, flexLine,
                                                               round(childRight) - child->getMeasuredWidth(),
                                                               childTop, round(childRight),
                                                               childTop + child->getMeasuredHeight());
                } else {
                    mFlexboxHelper.layoutSingleChildHorizontal(child, flexLine,
                                                               round(childLeft), childTop,
                                                               round(childLeft) + child->getMeasuredWidth(),
                                                               childTop + child->getMeasuredHeight());
                }
            }
            childLeft += child->getMeasuredWidth() + spaceBetweenItem + child->getMarginRight();
            childRight -= child->getMeasuredWidth() + spaceBetweenItem + child->getMarginLeft();

            flexLine.mLeft = std::min(flexLine.mLeft, child->getLeft() - child->getMarginLeft());
            flexLine.mTop = std::min(flexLine.mTop, child->getTop() - child->getMarginTop());
            flexLine.mRight = std::max(flexLine.mRight, child->getRight() + child->getMarginRight());
            flexLine.mBottom = std::max(flexLine.mBottom, child->getBottom() + child->getMarginBottom());

        }
        childTop += flexLine.mCrossSize;
        childBottom -= flexLine.mCrossSize;
    }
}

void FlexLayout::layoutVertical(bool isRtl, bool fromBottomToTop, int left, int top, int right, int bottom) {
    int paddingTop = getPaddingTop();
    int paddingBottom = getPaddingBottom();

    int paddingRight = getPaddingRight();
    int childLeft = getPaddingLeft();

    int width = right - left;
    int height = bottom - top;
    // childRight is used if the mFlexWrap is WRAP_REVERSE otherwise
    // childLeft is used to align the horizontal position of the children views.
    int childRight = width - paddingRight;

    // Use float to reduce the round error that may happen in when justifyContent ==
    // SPACE_BETWEEN or SPACE_AROUND
    float childTop;

    // Used only for if the direction is from bottom to top
    float childBottom;

    for (int i = 0, size = mFlexLines.size(); i < size; i++) {
        FlexLine& flexLine = mFlexLines[i];
        float spaceBetweenItem = 0;
        switch (mJustifyContent) {
            case JustifyContent::FLEX_START:
                childTop = paddingTop;
                childBottom = height - paddingBottom;
                break;
            case JustifyContent::FLEX_END:
                childTop = height - flexLine.mMainSize + paddingBottom;
                childBottom = flexLine.mMainSize - paddingTop;
                break;
            case JustifyContent::CENTER:
                childTop = paddingTop + static_cast<float>(height - flexLine.mMainSize) / 2;
                childBottom = height - paddingBottom - static_cast<float>(height - flexLine.mMainSize) / 2;
                break;
            case JustifyContent::SPACE_AROUND: {
                int visibleCount = flexLine.getItemCountNotGone();
                if (visibleCount != 0) {
                    spaceBetweenItem = static_cast<float>(height - flexLine.mMainSize) / visibleCount;
                }
                childTop = paddingTop + spaceBetweenItem / 2;
                childBottom = height - paddingBottom - spaceBetweenItem / 2;
                break;
            }
            case JustifyContent::SPACE_BETWEEN: {
                childTop = paddingTop;
                int visibleCount = flexLine.getItemCountNotGone();
                float denominator = visibleCount != 1 ? visibleCount - 1 : 1;
                spaceBetweenItem = (height - flexLine.mMainSize) / denominator;
                childBottom = height - paddingBottom;
                break;
            }
            case JustifyContent::SPACE_EVENLY: {
                int visibleCount = flexLine.getItemCountNotGone();
                if (visibleCount != 0) {
                    spaceBetweenItem = static_cast<float>(height - flexLine.mMainSize) / (visibleCount + 1);
                }
                childTop = paddingTop + spaceBetweenItem;
                childBottom = height - paddingBottom - spaceBetweenItem;
                break;
            }
            default:
                throw std::invalid_argument(
                        "Invalid justifyContent is set: " + std::to_string(mJustifyContent));
        }
        spaceBetweenItem = std::max(spaceBetweenItem, 0.f);

        for (int j = 0; j < flexLine.mItemCount; j++) {
            int index = flexLine.mFirstIndex + j;
            Item* child = getFlexItemAt(index);
            if (child == nullptr || child->getVisibility() == Item::GONE) {
                continue;
            }
            childTop += child->getMarginTop();
            childBottom -= child->getMarginBottom();

            if (isRtl) {
                if (fromBottomToTop) {
                    mFlexboxHelper.layoutSingleChildVertical(child, flexLine, true,
                                                             childRight - child->getMeasuredWidth(),
                                                             round(childBottom) - child->getMeasuredHeight(),
                                                             childRight,
                                                             round(childBottom));
                } else {
                    mFlexboxHelper.layoutSingleChildVertical(child, flexLine, true,
                                                             childRight - child->getMeasuredWidth(),
                                                             round(childTop),
                                                             childRight,
                                                             round(childTop) + child->getMeasuredHeight());
                }
            } else {
                if (fromBottomToTop) {
                    mFlexboxHelper.layoutSingleChildVertical(child, flexLine, false,
                                                             childLeft,
                                                             round(childBottom) - child->getMeasuredHeight(),
                                                             childLeft + child->getMeasuredWidth(),
                                                             round(childBottom));
                } else {
                    mFlexboxHelper.layoutSingleChildVertical(child, flexLine, false,
                                                             childLeft, round(childTop),
                                                             childLeft + child->getMeasuredWidth(),
                                                             round(childTop) + child->getMeasuredHeight());
                }
            }
            childTop += child->getMeasuredHeight() + spaceBetweenItem + child->getMarginBottom();
            childBottom -= child->getMeasuredHeight() + spaceBetweenItem + child->getMarginTop();

            flexLine.mLeft = std::min(flexLine.mLeft, child->getLeft() - child->getMarginLeft());
            flexLine.mTop = std::min(flexLine.mTop, child->getTop() - child->getMarginTop());
            flexLine.mRight = std::max(flexLine.mRight, child->getRight() + child->getMarginRight());
            flexLine.mBottom = std::max(flexLine.mBottom, child->getBottom() + child->getMarginBottom());
        }
        childLeft += flexLine.mCrossSize;
        childRight -= flexLine.mCrossSize;
    }
}

void FlexLayout::setMeasuredDimensionForFlex(int flexDirection, int widthMeasureSpec, int heightMeasureSpec,
                                             int childState) {
    int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    int heightSize = MeasureSpec::getSize(heightMeasureSpec);
    int calculatedMaxHeight;
    int calculatedMaxWidth;
    switch (flexDirection) {
        case FlexDirection::ROW: // Intentional fall through
        case FlexDirection::ROW_REVERSE:
            calculatedMaxHeight = getSumOfCrossSize() + getPaddingTop()
                                  + getPaddingBottom();
            calculatedMaxWidth = getLargestMainSize();
            break;
        case FlexDirection::COLUMN: // Intentional fall through
        case FlexDirection::COLUMN_REVERSE:
            calculatedMaxHeight = getLargestMainSize();
            calculatedMaxWidth = getSumOfCrossSize() + getPaddingLeft() + getPaddingRight();
            break;
        default:
            throw std::invalid_argument("Invalid flex direction: " + std::to_string(flexDirection));
    }

    int widthSizeAndState;
    switch (widthMode) {
        case MeasureSpec::EXACTLY:
            if (widthSize < calculatedMaxWidth) {
                childState = Item::combineMeasuredStates(childState, Item::MEASURED_STATE_TOO_SMALL);
            }
            widthSizeAndState = Item::resolveSizeAndState(widthSize, widthMeasureSpec,
                                                          childState);
            break;
        case MeasureSpec::AT_MOST: {
            if (widthSize < calculatedMaxWidth) {
                childState = Item::combineMeasuredStates(childState, Item::MEASURED_STATE_TOO_SMALL);
            } else {
                widthSize = calculatedMaxWidth;
            }
            widthSizeAndState = Item::resolveSizeAndState(widthSize, widthMeasureSpec,
                                                          childState);
            break;
        }
        case MeasureSpec::UNSPECIFIED: {
            widthSizeAndState = Item::resolveSizeAndState(calculatedMaxWidth, widthMeasureSpec, childState);
            break;
        }
        default:
            throw std::invalid_argument("Unknown width mode is set: " + std::to_string(widthMode));
    }
    int heightSizeAndState;
    switch (heightMode) {
        case MeasureSpec::EXACTLY:
            if (heightSize < calculatedMaxHeight) {
                childState = Item::combineMeasuredStates(childState,
                                                         Item::MEASURED_STATE_TOO_SMALL
                                                                 >> Item::MEASURED_HEIGHT_STATE_SHIFT);
            }
            heightSizeAndState = Item::resolveSizeAndState(heightSize, heightMeasureSpec,
                                                           childState);
            break;
        case MeasureSpec::AT_MOST: {
            if (heightSize < calculatedMaxHeight) {
                childState = Item::combineMeasuredStates(childState,
                                                         Item::MEASURED_STATE_TOO_SMALL
                                                                 >> Item::MEASURED_HEIGHT_STATE_SHIFT);
            } else {
                heightSize = calculatedMaxHeight;
            }
            heightSizeAndState = Item::resolveSizeAndState(heightSize, heightMeasureSpec,
                                                           childState);
            break;
        }
        case MeasureSpec::UNSPECIFIED: {
            heightSizeAndState = Item::resolveSizeAndState(calculatedMaxHeight,
                                                           heightMeasureSpec, childState);
            break;
        }
        default:
            throw std::invalid_argument("Unknown height mode is set: " + std::to_string(heightMode));
    }
    setMeasuredDimension(widthSizeAndState, heightSizeAndState);
}
