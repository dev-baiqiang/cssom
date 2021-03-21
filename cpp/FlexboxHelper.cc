/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */


#include "FlexboxHelper.h"
#include "FlexLine.h"
#include "Item.h"
#include "FlexLayout.h"

#define NO_POSITION -1

void FlexboxHelper::calculateHorizontalFlexLines(FlexboxHelper::FlexLinesResult& result, int widthMeasureSpec,
                                                 int heightMeasureSpec) {
    calculateFlexLines(result, widthMeasureSpec, heightMeasureSpec, INT_MAX,
                       0, NO_POSITION, nullptr);

}

void FlexboxHelper::calculateVerticalFlexLines(FlexboxHelper::FlexLinesResult& result, int widthMeasureSpec,
                                               int heightMeasureSpec) {
    calculateFlexLines(result, heightMeasureSpec, widthMeasureSpec, INT_MAX,
                       0, NO_POSITION, nullptr);
}

static bool isLastFlexItem(int childIndex, int childCount,
                           FlexLine& flexLine) {
    return childIndex == childCount - 1 && flexLine.getItemCountNotGone() != 0;
}

void
FlexboxHelper::calculateFlexLines(FlexboxHelper::FlexLinesResult& result, int mainMeasureSpec, int crossMeasureSpec,
                                  int needsCalcAmount, int fromIndex, int toIndex, std::vector<FlexLine>* flexLines) {

    bool isMainHorizontal = mFlexContainer->isMainAxisDirectionHorizontal();

    int mainMode = Item::MeasureSpec::getMode(mainMeasureSpec);
    int mainSize = Item::MeasureSpec::getSize(mainMeasureSpec);

    int childState = 0;

    if (flexLines != nullptr) {
        result.mFlexLines.insert(result.mFlexLines.end(), flexLines->cbegin(), flexLines->cend());
    }

    bool reachedToIndex = toIndex == NO_POSITION;

    int mainPaddingStart = getPaddingStartMain(isMainHorizontal);
    int mainPaddingEnd = getPaddingEndMain(isMainHorizontal);
    int crossPaddingStart = getPaddingStartCross(isMainHorizontal);
    int crossPaddingEnd = getPaddingEndCross(isMainHorizontal);

    int largestSizeInCross = INT_MIN;

    // The amount of cross size calculated in this method call.
    int sumCrossSize = 0;

    // The index of the view in the flex line.
    int indexInFlexLine = 0;

    FlexLine flexLine;
    flexLine.mFirstIndex = fromIndex;
    flexLine.mMainSize = mainPaddingStart + mainPaddingEnd;

    int childCount = mFlexContainer->getFlexItemCount();
    for (int i = fromIndex; i < childCount; i++) {
        Item* flexItem = mFlexContainer->getFlexItemAt(i);

        if (flexItem == nullptr) {
            if (isLastFlexItem(i, childCount, flexLine)) {
                addFlexLine(result.mFlexLines, flexLine, i, sumCrossSize);
            }
            continue;
        } else if (flexItem->getVisibility() == Item::GONE) {
            flexLine.mGoneItemCount++;
            flexLine.mItemCount++;
            if (isLastFlexItem(i, childCount, flexLine)) {
                addFlexLine(result.mFlexLines, flexLine, i, sumCrossSize);
            }
            continue;
        }

        if (flexItem->getAlignSelf() == AlignItems::STRETCH) {
            flexLine.mIndicesAlignSelfStretch.emplace_back(i);
        }

        int childMainSize = getFlexItemSizeMain(flexItem, isMainHorizontal);

        if (flexItem->getFlexBasisPercent() != flexItem->FLEX_BASIS_PERCENT_DEFAULT
            && mainMode == Item::MeasureSpec::EXACTLY) {
            childMainSize = static_cast<int>(round(static_cast<float>(mainSize) * flexItem->getFlexBasisPercent()));
            // Use the dimension from the layout if the mainMode is not
            // MeasureSpec.EXACTLY even if any fraction value is set to
            // layout_flexBasisPercent.
        }

        int childMainMeasureSpec;
        int childCrossMeasureSpec;
        if (isMainHorizontal) {
            childMainMeasureSpec = mFlexContainer->getChildWidthMeasureSpec(mainMeasureSpec,
                                                                            mainPaddingStart + mainPaddingEnd +
                                                                            getFlexItemMarginStartMain(flexItem, true) +
                                                                            getFlexItemMarginEndMain(flexItem, true),
                                                                            childMainSize, flexItem->getWidthPercent());
            childCrossMeasureSpec = mFlexContainer->getChildHeightMeasureSpec(crossMeasureSpec,
                                                                              crossPaddingStart + crossPaddingEnd +
                                                                              getFlexItemMarginStartCross(flexItem,
                                                                                                          true) +
                                                                              getFlexItemMarginEndCross(flexItem, true)
                                                                              + sumCrossSize,
                                                                              getFlexItemSizeCross(flexItem, true),
                                                                              flexItem->getHeightPercent());
            flexItem->measure(childMainMeasureSpec, childCrossMeasureSpec);
            updateMeasureCache(i, childMainMeasureSpec, childCrossMeasureSpec, flexItem);
        } else {
            childCrossMeasureSpec = mFlexContainer->getChildWidthMeasureSpec(crossMeasureSpec,
                                                                             crossPaddingStart + crossPaddingEnd +
                                                                             getFlexItemMarginStartCross(flexItem,
                                                                                                         false) +
                                                                             getFlexItemMarginEndCross(flexItem,
                                                                                                       false) +
                                                                             sumCrossSize,
                                                                             getFlexItemSizeCross(flexItem, false),
                                                                             flexItem->getWidthPercent());
            childMainMeasureSpec = mFlexContainer->getChildHeightMeasureSpec(mainMeasureSpec,
                                                                             mainPaddingStart + mainPaddingEnd +
                                                                             getFlexItemMarginStartMain(flexItem,
                                                                                                        false) +
                                                                             getFlexItemMarginEndMain(flexItem, false),
                                                                             childMainSize,
                                                                             flexItem->getHeightPercent());
            flexItem->measure(childCrossMeasureSpec, childMainMeasureSpec);
            updateMeasureCache(i, childCrossMeasureSpec, childMainMeasureSpec, flexItem);
        }
        mFlexContainer->updateViewCache(i, flexItem);

        // Check the size constraint after the first measurement for the child
        // To prevent the child's width/height violate the size constraints imposed by the
        // {@link FlexItem#getMinWidth()}, {@link FlexItem#getMinHeight()},
        // {@link FlexItem#getMaxWidth()} and {@link FlexItem#getMaxHeight()} attributes.
        // E.g. When the child's layout_width is wrap_content the measured width may be
        // less than the min width after the first measurement.
        checkSizeConstraints(flexItem, i);

        childState = Item::combineMeasuredStates(childState, flexItem->getMeasuredState());

        if (isWrapRequired(flexItem, mainMode, mainSize, flexLine.mMainSize,
                           getViewMeasuredSizeMain(flexItem, isMainHorizontal)
                           + getFlexItemMarginStartMain(flexItem, isMainHorizontal) +
                           getFlexItemMarginEndMain(flexItem, isMainHorizontal),
                           i, indexInFlexLine, result.mFlexLines.size())) {
            if (flexLine.getItemCountNotGone() > 0) {
                addFlexLine(result.mFlexLines, flexLine, i > 0 ? i - 1 : 0, sumCrossSize);
                sumCrossSize += flexLine.mCrossSize;
            }

            if (isMainHorizontal) {
                if (flexItem->getHeight() == FlexLayout::LayoutParams::MATCH_PARENT) {
                    // This case takes care of the corner case where the cross size of the
                    // child is affected by the just added flex line.
                    // E.g. when the child's layout_height is set to match_parent, the height
                    // of that child needs to be determined taking the total cross size used
                    // so far into account. In that case, the height of the child needs to be
                    // measured again note that we don't need to judge if the wrapping occurs
                    // because it doesn't change the size along the main axis.
                    childCrossMeasureSpec = mFlexContainer->getChildHeightMeasureSpec(
                            crossMeasureSpec,
                            mFlexContainer->getPaddingTop() + mFlexContainer->getPaddingBottom()
                            + flexItem->getMarginTop()
                            + flexItem->getMarginBottom() + sumCrossSize,
                            flexItem->getHeight(), flexItem->getHeightPercent());
                    flexItem->measure(childMainMeasureSpec, childCrossMeasureSpec);
                    checkSizeConstraints(flexItem, i);
                }
            } else {
                if (flexItem->getWidth() == FlexLayout::LayoutParams::MATCH_PARENT) {
                    // This case takes care of the corner case where the cross size of the
                    // child is affected by the just added flex line.
                    // E.g. when the child's layout_width is set to match_parent, the width
                    // of that child needs to be determined taking the total cross size used
                    // so far into account. In that case, the width of the child needs to be
                    // measured again note that we don't need to judge if the wrapping occurs
                    // because it doesn't change the size along the main axis.
                    childCrossMeasureSpec = mFlexContainer->getChildWidthMeasureSpec(
                            crossMeasureSpec,
                            mFlexContainer->getPaddingLeft() + mFlexContainer->getPaddingRight()
                            + flexItem->getMarginLeft()
                            + flexItem->getMarginRight() + sumCrossSize,
                            flexItem->getWidth(), flexItem->getWidthPercent());
                    flexItem->measure(childCrossMeasureSpec, childMainMeasureSpec);
                    checkSizeConstraints(flexItem, i);
                }
            }

            flexLine = {};
            flexLine.mItemCount = 1;
            flexLine.mMainSize = mainPaddingStart + mainPaddingEnd;
            flexLine.mFirstIndex = i;
            indexInFlexLine = 0;
            largestSizeInCross = INT_MIN;
        } else {
            flexLine.mItemCount++;
            indexInFlexLine++;
        }
        flexLine.mAnyItemsHaveFlexGrow |= flexItem->getFlexGrow() != Item::FLEX_GROW_DEFAULT;
        flexLine.mAnyItemsHaveFlexShrink |= flexItem->getFlexShrink() != Item::FLEX_SHRINK_NOT_SET;

        flexLine.mMainSize += getViewMeasuredSizeMain(flexItem, isMainHorizontal)
                              + getFlexItemMarginStartMain(flexItem, isMainHorizontal) +
                              getFlexItemMarginEndMain(flexItem, isMainHorizontal);
        flexLine.mTotalFlexGrow += flexItem->getFlexGrow();
        flexLine.mTotalFlexShrink += flexItem->getFlexShrink();

        mFlexContainer->onNewFlexItemAdded(flexItem, i, indexInFlexLine, flexLine);

        largestSizeInCross = std::max(largestSizeInCross,
                                      getViewMeasuredSizeCross(flexItem, isMainHorizontal) +
                                      getFlexItemMarginStartCross(flexItem, isMainHorizontal) +
                                      getFlexItemMarginEndCross(flexItem, isMainHorizontal));
        // Temporarily set the cross axis length as the largest child in the flexLine
        // Expand along the cross axis depending on the mAlignContent property if needed
        // later
        flexLine.mCrossSize = std::max(flexLine.mCrossSize, largestSizeInCross);

        if (isMainHorizontal) {
            if (mFlexContainer->getFlexWrap() != FlexWrap::WRAP_REVERSE) {
                flexLine.mMaxBaseline = std::max(flexLine.mMaxBaseline,
                                                 flexItem->getBaseline() + flexItem->getMarginTop());
            } else {
                // if the flex wrap property is WRAP_REVERSE, calculate the
                // baseline as the distance from the cross end and the baseline
                // since the cross size calculation is based on the distance from the cross end
                flexLine.mMaxBaseline = std::max(flexLine.mMaxBaseline,
                                                 flexItem->getMeasuredHeight() - flexItem->getBaseline()
                                                 + flexItem->getMarginBottom());
            }
        }

        if (isLastFlexItem(i, childCount, flexLine)) {
            addFlexLine(result.mFlexLines, flexLine, i, sumCrossSize);
            sumCrossSize += flexLine.mCrossSize;
        }

        if (toIndex != NO_POSITION
            && !result.mFlexLines.empty()
            && result.mFlexLines.back().mLastIndex >= toIndex
            && i >= toIndex
            && !reachedToIndex) {
            // Calculated to include a flex line which includes the flex item having the
            // toIndex.
            // Let the sumCrossSize start from the negative value of the last flex line's
            // cross size because otherwise flex lines aren't calculated enough to fill the
            // visible area.
            sumCrossSize = -flexLine.getCrossSize();
            reachedToIndex = true;
        }
        if (sumCrossSize > needsCalcAmount && reachedToIndex) {
            // Stop the calculation if the sum of cross size calculated reached to the point
            // beyond the needsCalcAmount value to avoid unneeded calculation in a
            // RecyclerView.
            // To be precise, the decoration length may be added to the sumCrossSize,
            // but we omit adding the decoration length because even without the decorator
            // length, it's guaranteed that calculation is done at least beyond the
            // needsCalcAmount
            break;
        }
    }

    result.mChildState = childState;
}

FlexboxHelper::FlexboxHelper(FlexLayout* flexContainer) : mFlexContainer(flexContainer) {}

void FlexboxHelper::addFlexLine(std::vector<FlexLine>& flexLines, FlexLine& flexLine,
                                int viewIndex, int usedCrossSizeSoFar) {
    flexLine.mSumCrossSizeBefore = usedCrossSizeSoFar;
    mFlexContainer->onNewFlexLineAdded(flexLine);
    flexLine.mLastIndex = viewIndex;
    flexLines.emplace_back(flexLine);
}

void FlexboxHelper::updateMeasureCache(int index, int widthMeasureSpec, int heightMeasureSpec, Item* view) {
    // if (mMeasureSpecCache != null) {
    //     mMeasureSpecCache[index] = makeCombinedLong(
    //             widthMeasureSpec,
    //             heightMeasureSpec);
    // }
    // if (mMeasuredSizeCache != null) {
    //     mMeasuredSizeCache[index] = makeCombinedLong(
    //             view.getMeasuredWidth(),
    //             view.getMeasuredHeight());
    // }
}

void FlexboxHelper::checkSizeConstraints(Item* view, int index) {
    bool needsMeasure = false;
    int childWidth = view->getMeasuredWidth();
    int childHeight = view->getMeasuredHeight();

    if (childWidth < view->getMinWidth()) {
        needsMeasure = true;
        childWidth = view->getMinWidth();
    } else if (childWidth > view->getMaxWidth()) {
        needsMeasure = true;
        childWidth = view->getMaxWidth();
    }

    if (childHeight < view->getMinHeight()) {
        needsMeasure = true;
        childHeight = view->getMinHeight();
    } else if (childHeight > view->getMaxHeight()) {
        needsMeasure = true;
        childHeight = view->getMaxHeight();
    }
    if (needsMeasure) {
        int widthSpec = Item::MeasureSpec::makeMeasureSpec(childWidth, Item::MeasureSpec::EXACTLY);
        int heightSpec = Item::MeasureSpec::makeMeasureSpec(childHeight, Item::MeasureSpec::EXACTLY);
        view->measure(widthSpec, heightSpec);
        updateMeasureCache(index, widthSpec, heightSpec, view);
        mFlexContainer->updateViewCache(index, view);
    }
}

void FlexboxHelper::determineMainSize(int widthMeasureSpec, int heightMeasureSpec, int fromIndex) {
    ensureChildrenFrozen(mFlexContainer->getFlexItemCount());
    if (fromIndex >= mFlexContainer->getFlexItemCount()) {
        return;
    }
    int mainSize;
    int paddingAlongMainAxis;
    int flexDirection = mFlexContainer->getFlexDirection();
    switch (mFlexContainer->getFlexDirection()) {
        case FlexDirection::ROW: // Intentional fall through
        case FlexDirection::ROW_REVERSE: {
            int widthMode = Item::MeasureSpec::getMode(widthMeasureSpec);
            int widthSize = Item::MeasureSpec::getSize(widthMeasureSpec);
            int largestMainSize = mFlexContainer->getLargestMainSize();
            if (widthMode == Item::MeasureSpec::EXACTLY) {
                mainSize = widthSize;
            } else if (widthMode == Item::MeasureSpec::UNSPECIFIED) {
                mainSize = largestMainSize;
            } else {
                mainSize = largestMainSize > widthSize ? widthSize : largestMainSize;
            }
            paddingAlongMainAxis = mFlexContainer->getPaddingLeft()
                                   + mFlexContainer->getPaddingRight();
            break;
        }

        case FlexDirection::COLUMN: // Intentional fall through
        case FlexDirection::COLUMN_REVERSE: {
            int heightMode = Item::MeasureSpec::getMode(heightMeasureSpec);
            int heightSize = Item::MeasureSpec::getSize(heightMeasureSpec);
            if (heightMode == Item::MeasureSpec::EXACTLY) {
                mainSize = heightSize;
            } else {
                mainSize = mFlexContainer->getLargestMainSize();
            }
            paddingAlongMainAxis = mFlexContainer->getPaddingTop()
                                   + mFlexContainer->getPaddingBottom();
            break;
        }
        default:
            throw std::invalid_argument("Invalid flex direction: " + std::to_string(flexDirection));
    }

    int flexLineIndex = 0;
    // if (mIndexToFlexLine != null) {
    //     flexLineIndex = mIndexToFlexLine[fromIndex];
    // }
    std::vector<FlexLine>& flexLines = mFlexContainer->getFlexLinesInternal();
    for (int i = flexLineIndex, size = flexLines.size(); i < size; i++) {
        FlexLine& flexLine = flexLines[i];
        if (flexLine.mMainSize < mainSize && flexLine.mAnyItemsHaveFlexGrow) {
            expandFlexItems(widthMeasureSpec, heightMeasureSpec, flexLine,
                            mainSize, paddingAlongMainAxis, false);
        } else if (flexLine.mMainSize > mainSize && flexLine.mAnyItemsHaveFlexShrink) {
            shrinkFlexItems(widthMeasureSpec, heightMeasureSpec, flexLine,
                            mainSize, paddingAlongMainAxis, false);
        }
    }
}

void
FlexboxHelper::expandFlexItems(int widthMeasureSpec, int heightMeasureSpec, FlexLine& flexLine, int maxMainSize,
                               int paddingAlongMainAxis, bool calledRecursively) {
    if (flexLine.mTotalFlexGrow <= 0 || maxMainSize < flexLine.mMainSize) {
        return;
    }
    int sizeBeforeExpand = flexLine.mMainSize;
    bool needsReexpand = false;
    float unitSpace = static_cast<float>(maxMainSize - flexLine.mMainSize) / flexLine.mTotalFlexGrow;
    flexLine.mMainSize = paddingAlongMainAxis;

    // Setting the cross size of the flex line as the temporal value since the cross size of
    // each flex item may be changed from the initial calculation
    // (in the measureHorizontal/measureVertical method) even this method is part of the main
    // size determination.
    // E.g. If a TextView's layout_width is set to 0dp, layout_height is set to wrap_content,
    // and layout_flexGrow is set to 1, the TextView is trying to expand to the vertical
    // direction to enclose its content (in the measureHorizontal method), but
    // the width will be expanded in this method. In that case, the height needs to be measured
    // again with the expanded width.
    int largestCrossSize = 0;
    if (!calledRecursively) {
        flexLine.mCrossSize = INT_MIN;
    }
    float accumulatedRoundError = 0;
    for (int i = 0; i < flexLine.mItemCount; i++) {
        int index = flexLine.mFirstIndex + i;
        Item* child = mFlexContainer->getFlexItemAt(index);
        Item* flexItem = child;
        if (child == nullptr || child->getVisibility() == Item::GONE) {
            continue;
        }
        int flexDirection = mFlexContainer->getFlexDirection();
        if (flexDirection == FlexDirection::ROW ||
            flexDirection == FlexDirection::ROW_REVERSE) {
            // The direction of the main axis is horizontal

            int childMeasuredWidth = child->getMeasuredWidth();
            // if (mMeasuredSizeCache != null) {
            // Retrieve the measured width from the cache because there
            // are some cases that the view is re-created from the last measure, thus
            // View#getMeasuredWidth returns 0.
            // E.g. if the flex container is FlexboxLayoutManager, the case happens
            // frequently
            // childMeasuredWidth = extractLowerInt(mMeasuredSizeCache[index]);
            // }
            int childMeasuredHeight = child->getMeasuredHeight();
            // if (mMeasuredSizeCache != null) {
            // Extract the measured height from the cache
            // childMeasuredHeight = extractHigherInt(mMeasuredSizeCache[index]);
            // }
            if (!mChildrenFrozen[index] && flexItem->getFlexGrow() > 0) {
                float rawCalculatedWidth = childMeasuredWidth
                                           + unitSpace * flexItem->getFlexGrow();
                if (i == flexLine.mItemCount - 1) {
                    rawCalculatedWidth += accumulatedRoundError;
                    accumulatedRoundError = 0;
                }
                int newWidth = static_cast<int>(round(rawCalculatedWidth));
                if (newWidth > flexItem->getMaxWidth()) {
                    // This means the child can't expand beyond the value of the mMaxWidth
                    // attribute.
                    // To adjust the flex line length to the size of maxMainSize, remaining
                    // positive free space needs to be re-distributed to other flex items
                    // (children views). In that case, invoke this method again with the same
                    // fromIndex.
                    needsReexpand = true;
                    newWidth = flexItem->getMaxWidth();
                    mChildrenFrozen[index] = true;
                    flexLine.mTotalFlexGrow -= flexItem->getFlexGrow();
                } else {
                    accumulatedRoundError += (rawCalculatedWidth - static_cast<float>(newWidth));
                    if (accumulatedRoundError > 1.0) {
                        newWidth += 1;
                        accumulatedRoundError -= 1.0;
                    } else if (accumulatedRoundError < -1.0) {
                        newWidth -= 1;
                        accumulatedRoundError += 1.0;
                    }
                }
                int childHeightMeasureSpec = getChildHeightMeasureSpecInternal(
                        heightMeasureSpec, flexItem, flexLine.mSumCrossSizeBefore);
                int childWidthMeasureSpec = Item::MeasureSpec::makeMeasureSpec(newWidth,
                                                                               Item::MeasureSpec::EXACTLY);
                child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
                childMeasuredWidth = child->getMeasuredWidth();
                childMeasuredHeight = child->getMeasuredHeight();
                updateMeasureCache(index, childWidthMeasureSpec, childHeightMeasureSpec,
                                   child);
                mFlexContainer->updateViewCache(index, child);
            }
            largestCrossSize = std::max(largestCrossSize, childMeasuredHeight
                                                          + flexItem->getMarginTop() + flexItem->getMarginBottom());
            flexLine.mMainSize += childMeasuredWidth + flexItem->getMarginLeft()
                                  + flexItem->getMarginRight();
        } else {
            // The direction of the main axis is vertical

            int childMeasuredHeight = child->getMeasuredHeight();
            // if (mMeasuredSizeCache != null) {
            //     // Retrieve the measured height from the cache because there
            //     // are some cases that the view is re-created from the last measure, thus
            //     // View#getMeasuredHeight returns 0.
            //     // E.g. if the flex container is FlexboxLayoutManager, that case happens
            //     // frequently
            //     childMeasuredHeight =
            //             extractHigherInt(mMeasuredSizeCache[index]);
            // }
            int childMeasuredWidth = child->getMeasuredWidth();
            // if (mMeasuredSizeCache != null) {
            //     // Extract the measured width from the cache
            //     childMeasuredWidth =
            //             extractLowerInt(mMeasuredSizeCache[index]);
            // }
            if (!mChildrenFrozen[index] && flexItem->getFlexGrow() > 0) {
                float rawCalculatedHeight = childMeasuredHeight
                                            + unitSpace * flexItem->getFlexGrow();
                if (i == flexLine.mItemCount - 1) {
                    rawCalculatedHeight += accumulatedRoundError;
                    accumulatedRoundError = 0;
                }
                int newHeight = static_cast<int>(round(rawCalculatedHeight));
                if (newHeight > flexItem->getMaxHeight()) {
                    // This means the child can't expand beyond the value of the mMaxHeight
                    // attribute.
                    // To adjust the flex line length to the size of maxMainSize, remaining
                    // positive free space needs to be re-distributed to other flex items
                    // (children views). In that case, invoke this method again with the same
                    // fromIndex.
                    needsReexpand = true;
                    newHeight = flexItem->getMaxHeight();
                    mChildrenFrozen[index] = true;
                    flexLine.mTotalFlexGrow -= flexItem->getFlexGrow();
                } else {
                    accumulatedRoundError += (rawCalculatedHeight - newHeight);
                    if (accumulatedRoundError > 1.0) {
                        newHeight += 1;
                        accumulatedRoundError -= 1.0;
                    } else if (accumulatedRoundError < -1.0) {
                        newHeight -= 1;
                        accumulatedRoundError += 1.0;
                    }
                }
                int childWidthMeasureSpec = getChildWidthMeasureSpecInternal(widthMeasureSpec,
                                                                             flexItem,
                                                                             flexLine.mSumCrossSizeBefore);
                int childHeightMeasureSpec = Item::MeasureSpec::makeMeasureSpec(newHeight,
                                                                                Item::MeasureSpec::EXACTLY);
                child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
                childMeasuredWidth = child->getMeasuredWidth();
                childMeasuredHeight = child->getMeasuredHeight();
                updateMeasureCache(index, childWidthMeasureSpec, childHeightMeasureSpec,
                                   child);
                mFlexContainer->updateViewCache(index, child);
            }
            largestCrossSize = std::max(largestCrossSize, childMeasuredWidth
                                                          + flexItem->getMarginLeft() + flexItem->getMarginRight());
            flexLine.mMainSize += childMeasuredHeight + flexItem->getMarginTop()
                                  + flexItem->getMarginBottom();
        }
        flexLine.mCrossSize = std::max(flexLine.mCrossSize, largestCrossSize);
    }

    if (needsReexpand && sizeBeforeExpand != flexLine.mMainSize) {
        // Re-invoke the method with the same flex line to distribute the positive free space
        // that wasn't fully distributed (because of maximum length constraint)
        expandFlexItems(widthMeasureSpec, heightMeasureSpec, flexLine, maxMainSize,
                        paddingAlongMainAxis, true);
    }
}


void FlexboxHelper::ensureChildrenFrozen(int size) {
    if (mChildrenFrozen.empty()) {
        mChildrenFrozen.resize(size < INITIAL_CAPACITY ? INITIAL_CAPACITY : size);
    } else if (mChildrenFrozen.size() < size) {
        auto newCapacity = mChildrenFrozen.size() * 2;
        mChildrenFrozen.resize(newCapacity >= size ? newCapacity : size);
    }
}

int FlexboxHelper::getPaddingStartMain(bool isMainHorizontal) const {
    if (isMainHorizontal) {
        return mFlexContainer->getPaddingStart();
    }

    return mFlexContainer->getPaddingTop();
}

int FlexboxHelper::getPaddingEndMain(bool isMainHorizontal) {
    if (isMainHorizontal) {
        return mFlexContainer->getPaddingEnd();
    }

    return mFlexContainer->getPaddingBottom();
}

int FlexboxHelper::getPaddingStartCross(bool isMainHorizontal) {
    if (isMainHorizontal) {
        return mFlexContainer->getPaddingTop();
    }

    return mFlexContainer->getPaddingStart();
}

int FlexboxHelper::getPaddingEndCross(bool isMainHorizontal) {
    if (isMainHorizontal) {
        return mFlexContainer->getPaddingBottom();
    }

    return mFlexContainer->getPaddingEnd();
}

int FlexboxHelper::getChildWidthMeasureSpecInternal(int widthMeasureSpec, Item* flexItem, int padding) {
    int childWidthMeasureSpec = mFlexContainer->getChildWidthMeasureSpec(widthMeasureSpec,
                                                                         mFlexContainer->getPaddingLeft() +
                                                                         mFlexContainer->getPaddingRight() +
                                                                         flexItem->getMarginLeft() +
                                                                         flexItem->getMarginRight() + padding,
                                                                         flexItem->getWidth(),
                                                                         flexItem->getWidthPercent());
    int childWidth = Item::MeasureSpec::getSize(childWidthMeasureSpec);
    if (childWidth > flexItem->getMaxWidth()) {
        childWidthMeasureSpec = Item::MeasureSpec::makeMeasureSpec(flexItem->getMaxWidth(),
                                                                   Item::MeasureSpec::getMode(
                                                                           childWidthMeasureSpec));
    } else if (childWidth < flexItem->getMinWidth()) {
        childWidthMeasureSpec = Item::MeasureSpec::makeMeasureSpec(flexItem->getMinWidth(),
                                                                   Item::MeasureSpec::getMode(
                                                                           childWidthMeasureSpec));
    }
    return childWidthMeasureSpec;
}

int FlexboxHelper::getChildHeightMeasureSpecInternal(int heightMeasureSpec, Item* flexItem, int padding) {
    int childHeightMeasureSpec = mFlexContainer->getChildHeightMeasureSpec(heightMeasureSpec,
                                                                           mFlexContainer->getPaddingTop() +
                                                                           mFlexContainer->getPaddingBottom()
                                                                           + flexItem->getMarginTop() +
                                                                           flexItem->getMarginBottom() + padding,
                                                                           flexItem->getHeight(),
                                                                           flexItem->getHeightPercent());
    int childHeight = Item::MeasureSpec::getSize(childHeightMeasureSpec);
    if (childHeight > flexItem->getMaxHeight()) {
        childHeightMeasureSpec = Item::MeasureSpec::makeMeasureSpec(flexItem->getMaxHeight(),
                                                                    Item::MeasureSpec::getMode(
                                                                            childHeightMeasureSpec));
    } else if (childHeight < flexItem->getMinHeight()) {
        childHeightMeasureSpec = Item::MeasureSpec::makeMeasureSpec(flexItem->getMinHeight(),
                                                                    Item::MeasureSpec::getMode(
                                                                            childHeightMeasureSpec));
    }
    return childHeightMeasureSpec;
}

void
FlexboxHelper::shrinkFlexItems(int widthMeasureSpec, int heightMeasureSpec, FlexLine& flexLine, int maxMainSize,
                               int paddingAlongMainAxis, bool calledRecursively) {
    int sizeBeforeShrink = flexLine.mMainSize;
    if (flexLine.mTotalFlexShrink <= 0 || maxMainSize > flexLine.mMainSize) {
        return;
    }
    bool needsReshrink = false;
    float unitShrink = static_cast<float>(flexLine.mMainSize - maxMainSize) / flexLine.mTotalFlexShrink;
    float accumulatedRoundError = 0;
    flexLine.mMainSize = paddingAlongMainAxis;

    // Setting the cross size of the flex line as the temporal value since the cross size of
    // each flex item may be changed from the initial calculation
    // (in the measureHorizontal/measureVertical method) even this method is part of the main
    // size determination.
    // E.g. If a TextView's layout_width is set to 0dp, layout_height is set to wrap_content,
    // and layout_flexGrow is set to 1, the TextView is trying to expand to the vertical
    // direction to enclose its content (in the measureHorizontal method), but
    // the width will be expanded in this method. In that case, the height needs to be measured
    // again with the expanded width.
    int largestCrossSize = 0;
    if (!calledRecursively) {
        flexLine.mCrossSize = INT_MIN;
    }
    for (int i = 0; i < flexLine.mItemCount; i++) {
        int index = flexLine.mFirstIndex + i;
        Item* flexItem = mFlexContainer->getFlexItemAt(index);
        if (flexItem == nullptr || flexItem->getVisibility() == Item::GONE) {
            continue;
        }
        int flexDirection = mFlexContainer->getFlexDirection();
        if (flexDirection == FlexDirection::ROW || flexDirection == FlexDirection::ROW_REVERSE) {
            // The direction of main axis is horizontal

            int childMeasuredWidth = flexItem->getMeasuredWidth();
            // if (mMeasuredSizeCache != null) {
            // Retrieve the measured width from the cache because there
            // are some cases that the view is re-created from the last measure, thus
            // View#getMeasuredWidth returns 0.
            // E.g. if the flex container is FlexboxLayoutManager, the case happens
            // frequently
            // childMeasuredWidth = extractLowerInt(mMeasuredSizeCache[index]);
            // }
            int childMeasuredHeight = flexItem->getMeasuredHeight();
            // if (mMeasuredSizeCache != null) {
            // Extract the measured height from the cache
            // childMeasuredHeight = extractHigherInt(mMeasuredSizeCache[index]);
            // }
            if (!mChildrenFrozen[index] && flexItem->getFlexShrink() > 0) {
                float rawCalculatedWidth = childMeasuredWidth
                                           - unitShrink * flexItem->getFlexShrink();
                if (i == flexLine.mItemCount - 1) {
                    rawCalculatedWidth += accumulatedRoundError;
                    accumulatedRoundError = 0;
                }
                int newWidth = static_cast<int>(round(rawCalculatedWidth));
                if (newWidth < flexItem->getMinWidth()) {
                    // This means the child doesn't have enough space to distribute the negative
                    // free space. To adjust the flex line length down to the maxMainSize,
                    // remaining
                    // negative free space needs to be re-distributed to other flex items
                    // (children views). In that case, invoke this method again with the same
                    // fromIndex.
                    needsReshrink = true;
                    newWidth = flexItem->getMinWidth();
                    mChildrenFrozen[index] = true;
                    flexLine.mTotalFlexShrink -= flexItem->getFlexShrink();
                } else {
                    accumulatedRoundError += (rawCalculatedWidth - newWidth);
                    if (accumulatedRoundError > 1.0) {
                        newWidth += 1;
                        accumulatedRoundError -= 1;
                    } else if (accumulatedRoundError < -1.0) {
                        newWidth -= 1;
                        accumulatedRoundError += 1;
                    }
                }
                int childHeightMeasureSpec = getChildHeightMeasureSpecInternal(
                        heightMeasureSpec, flexItem, flexLine.mSumCrossSizeBefore);
                int childWidthMeasureSpec =
                        Item::MeasureSpec::makeMeasureSpec(newWidth, Item::MeasureSpec::EXACTLY);
                flexItem->measure(childWidthMeasureSpec, childHeightMeasureSpec);

                childMeasuredWidth = flexItem->getMeasuredWidth();
                childMeasuredHeight = flexItem->getMeasuredHeight();
                updateMeasureCache(index, childWidthMeasureSpec, childHeightMeasureSpec,
                                   flexItem);
                mFlexContainer->updateViewCache(index, flexItem);
            }
            largestCrossSize = std::max(largestCrossSize, childMeasuredHeight +
                                                          flexItem->getMarginTop() + flexItem->getMarginBottom());
            flexLine.mMainSize += childMeasuredWidth + flexItem->getMarginLeft()
                                  + flexItem->getMarginRight();
        } else {
            // The direction of main axis is vertical

            int childMeasuredHeight = flexItem->getMeasuredHeight();
            // if (mMeasuredSizeCache != null) {
            // Retrieve the measured height from the cache because there
            // are some cases that the view is re-created from the last measure, thus
            // View#getMeasuredHeight returns 0.
            // E.g. if the flex container is FlexboxLayoutManager, that case happens
            // frequently
            // childMeasuredHeight =
            //         extractHigherInt(mMeasuredSizeCache[index]);
            // }
            int childMeasuredWidth = flexItem->getMeasuredWidth();
            // if (mMeasuredSizeCache != null) {
            // Extract the measured width from the cache
            // childMeasuredWidth =
            //         extractLowerInt(mMeasuredSizeCache[index]);
            // }
            if (!mChildrenFrozen[index] && flexItem->getFlexShrink() > 0) {
                float rawCalculatedHeight = childMeasuredHeight
                                            - unitShrink * flexItem->getFlexShrink();
                if (i == flexLine.mItemCount - 1) {
                    rawCalculatedHeight += accumulatedRoundError;
                    accumulatedRoundError = 0;
                }
                int newHeight = static_cast<int>(round(rawCalculatedHeight));
                if (newHeight < flexItem->getMinHeight()) {
                    // Need to invoke this method again like the case flex direction is vertical
                    needsReshrink = true;
                    newHeight = flexItem->getMinHeight();
                    mChildrenFrozen[index] = true;
                    flexLine.mTotalFlexShrink -= flexItem->getFlexShrink();
                } else {
                    accumulatedRoundError += (rawCalculatedHeight - newHeight);
                    if (accumulatedRoundError > 1.0) {
                        newHeight += 1;
                        accumulatedRoundError -= 1;
                    } else if (accumulatedRoundError < -1.0) {
                        newHeight -= 1;
                        accumulatedRoundError += 1;
                    }
                }
                int childWidthMeasureSpec = getChildWidthMeasureSpecInternal(widthMeasureSpec,
                                                                             flexItem,
                                                                             flexLine.mSumCrossSizeBefore);
                int childHeightMeasureSpec =
                        Item::MeasureSpec::makeMeasureSpec(newHeight, Item::MeasureSpec::EXACTLY);
                flexItem->measure(childWidthMeasureSpec, childHeightMeasureSpec);

                childMeasuredWidth = flexItem->getMeasuredWidth();
                childMeasuredHeight = flexItem->getMeasuredHeight();
                updateMeasureCache(index, childWidthMeasureSpec, childHeightMeasureSpec,
                                   flexItem);
                mFlexContainer->updateViewCache(index, flexItem);
            }
            largestCrossSize = std::max(largestCrossSize, childMeasuredWidth +
                                                          flexItem->getMarginLeft() + flexItem->getMarginRight());
            flexLine.mMainSize += childMeasuredHeight + flexItem->getMarginTop()
                                  + flexItem->getMarginBottom();
        }
        flexLine.mCrossSize = std::max(flexLine.mCrossSize, largestCrossSize);
    }

    if (needsReshrink && sizeBeforeShrink != flexLine.mMainSize) {
        // Re-invoke the method with the same fromIndex to distribute the negative free space
        // that wasn't fully distributed (because some views length were not enough)
        shrinkFlexItems(widthMeasureSpec, heightMeasureSpec, flexLine,
                        maxMainSize, paddingAlongMainAxis, true);
    }

}

void FlexboxHelper::determineCrossSize(int widthMeasureSpec, int heightMeasureSpec, int paddingAlongCrossAxis) {
    // The MeasureSpec mode along the cross axis
    int mode;
    // The MeasureSpec size along the cross axis
    int size;
    int flexDirection = mFlexContainer->getFlexDirection();
    switch (flexDirection) {
        case FlexDirection::ROW: // Intentional fall through
        case FlexDirection::ROW_REVERSE:
            mode = Item::MeasureSpec::getMode(heightMeasureSpec);
            size = Item::MeasureSpec::getSize(heightMeasureSpec);
            break;
        case FlexDirection::COLUMN: // Intentional fall through
        case FlexDirection::COLUMN_REVERSE:
            mode = Item::MeasureSpec::getMode(widthMeasureSpec);
            size = Item::MeasureSpec::getSize(widthMeasureSpec);
            break;
        default:
            throw std::invalid_argument("Invalid flex direction: " + std::to_string(flexDirection));
    }
    std::vector<FlexLine>& flexLines = mFlexContainer->getFlexLinesInternal();
    if (mode == Item::MeasureSpec::EXACTLY) {
        int totalCrossSize = mFlexContainer->getSumOfCrossSize() + paddingAlongCrossAxis;
        if (flexLines.size() == 1) {
            flexLines[0].mCrossSize = size - paddingAlongCrossAxis;
            // alignContent property is valid only if the Flexbox has at least two lines
        } else if (flexLines.size() >= 2) {
            switch (mFlexContainer->getAlignContent()) {
                case AlignContent::STRETCH: {
                    if (totalCrossSize >= size) {
                        break;
                    }
                    float freeSpaceUnit = static_cast<float>(size - totalCrossSize) / flexLines.size();
                    float accumulatedError = 0;
                    for (int i = 0, flexLinesSize = flexLines.size(); i < flexLinesSize; i++) {
                        FlexLine flexLine = flexLines[i];
                        float newCrossSizeAsFloat = flexLine.mCrossSize + freeSpaceUnit;
                        if (i == flexLines.size() - 1) {
                            newCrossSizeAsFloat += accumulatedError;
                            accumulatedError = 0;
                        }
                        int newCrossSize = static_cast<int>(round(newCrossSizeAsFloat));
                        accumulatedError += (newCrossSizeAsFloat - newCrossSize);
                        if (accumulatedError > 1) {
                            newCrossSize += 1;
                            accumulatedError -= 1;
                        } else if (accumulatedError < -1) {
                            newCrossSize -= 1;
                            accumulatedError += 1;
                        }
                        flexLine.mCrossSize = newCrossSize;
                    }
                    break;
                }
                case AlignContent::SPACE_AROUND: {
                    if (totalCrossSize >= size) {
                        // If the size of the content is larger than the flex container, the
                        // Flex lines should be aligned center like ALIGN_CONTENT_CENTER
                        mFlexContainer->setFlexLines(
                                constructFlexLinesForAlignContentCenter(flexLines, size,
                                                                        totalCrossSize));
                        break;
                    }
                    // The value of free space along the cross axis which needs to be put on top
                    // and below the bottom of each flex line.
                    int spaceTopAndBottom = size - totalCrossSize;
                    // The number of spaces along the cross axis
                    int numberOfSpaces = flexLines.size() * 2;
                    spaceTopAndBottom = spaceTopAndBottom / numberOfSpaces;
                    std::vector<FlexLine> newFlexLines;
                    FlexLine dummySpaceFlexLine;
                    dummySpaceFlexLine.mCrossSize = spaceTopAndBottom;
                    for (FlexLine flexLine : flexLines) {
                        newFlexLines.emplace_back(dummySpaceFlexLine);
                        newFlexLines.emplace_back(flexLine);
                        newFlexLines.emplace_back(dummySpaceFlexLine);
                    }
                    mFlexContainer->setFlexLines(newFlexLines);
                    break;
                }
                case AlignContent::SPACE_BETWEEN: {
                    if (totalCrossSize >= size) {
                        break;
                    }
                    // The value of free space along the cross axis between each flex line.
                    float spaceBetweenFlexLine = size - totalCrossSize;
                    int numberOfSpaces = flexLines.size() - 1;
                    spaceBetweenFlexLine = spaceBetweenFlexLine / (float) numberOfSpaces;
                    float accumulatedError = 0;

                    std::vector<FlexLine> newFlexLines;
                    for (int i = 0, flexLineSize = flexLines.size(); i < flexLineSize; i++) {
                        FlexLine flexLine = flexLines[i];
                        newFlexLines.emplace_back(flexLine);

                        if (i != flexLines.size() - 1) {
                            FlexLine dummySpaceFlexLine;
                            if (i == flexLines.size() - 2) {
                                // The last dummy space block in the flex container.
                                // Adjust the cross size by the accumulated error.
                                dummySpaceFlexLine.mCrossSize = static_cast<int>(round(
                                        spaceBetweenFlexLine + accumulatedError));
                                accumulatedError = 0;
                            } else {
                                dummySpaceFlexLine.mCrossSize = static_cast<int>(round(spaceBetweenFlexLine));
                            }
                            accumulatedError += (spaceBetweenFlexLine
                                                 - dummySpaceFlexLine.mCrossSize);
                            if (accumulatedError > 1) {
                                dummySpaceFlexLine.mCrossSize += 1;
                                accumulatedError -= 1;
                            } else if (accumulatedError < -1) {
                                dummySpaceFlexLine.mCrossSize -= 1;
                                accumulatedError += 1;
                            }
                            newFlexLines.emplace_back(dummySpaceFlexLine);
                        }
                    }
                    mFlexContainer->setFlexLines(newFlexLines);
                    break;
                }
                case AlignContent::CENTER: {
                    mFlexContainer->setFlexLines(
                            constructFlexLinesForAlignContentCenter(flexLines, size,
                                                                    totalCrossSize));
                    break;
                }
                case AlignContent::FLEX_END: {
                    int spaceTop = size - totalCrossSize;
                    FlexLine dummySpaceFlexLine;
                    dummySpaceFlexLine.mCrossSize = spaceTop;
                    flexLines.insert(flexLines.begin(), dummySpaceFlexLine);
                    break;
                }
                case AlignContent::FLEX_START:
                    // No op. Just to cover the available switch statement options
                    break;
            }
        }
    }

}

std::vector<FlexLine> FlexboxHelper::constructFlexLinesForAlignContentCenter(std::vector<FlexLine> flexLines,
                                                                             int size, int totalCrossSize) {
    int spaceAboveAndBottom = size - totalCrossSize;
    spaceAboveAndBottom = spaceAboveAndBottom / 2;
    std::vector<FlexLine> newFlexLines;
    FlexLine dummySpaceFlexLine;
    dummySpaceFlexLine.mCrossSize = spaceAboveAndBottom;
    for (int i = 0, flexLineSize = flexLines.size(); i < flexLineSize; i++) {
        if (i == 0) {
            newFlexLines.emplace_back(dummySpaceFlexLine);
        }
        FlexLine flexLine = flexLines[i];
        newFlexLines.emplace_back(flexLine);
        if (i == flexLines.size() - 1) {
            newFlexLines.emplace_back(dummySpaceFlexLine);
        }
    }
    return newFlexLines;
}

void FlexboxHelper::stretchViews(int fromIndex) {
    if (fromIndex >= mFlexContainer->getFlexItemCount()) {
        return;
    }
    int flexDirection = mFlexContainer->getFlexDirection();
    if (mFlexContainer->getAlignItems() == AlignItems::STRETCH) {
        int flexLineIndex = 0;

        auto flexLines = mFlexContainer->getFlexLinesInternal();
        for (int i = flexLineIndex, size = flexLines.size(); i < size; i++) {
            FlexLine flexLine = flexLines[i];
            for (int j = 0, itemCount = flexLine.mItemCount; j < itemCount; j++) {
                int viewIndex = flexLine.mFirstIndex + j;
                if (j >= mFlexContainer->getFlexItemCount()) {
                    continue;
                }
                auto flexItem = mFlexContainer->getFlexItemAt(viewIndex);
                if (flexItem == nullptr || flexItem->getVisibility() == Item::GONE) {
                    continue;
                }
                if (flexItem->getAlignSelf() != AlignSelf::AUTO &&
                    flexItem->getAlignSelf() != AlignItems::STRETCH) {
                    continue;
                }
                switch (flexDirection) {
                    case FlexDirection::ROW: // Intentional fall through
                    case FlexDirection::ROW_REVERSE:
                        stretchViewVertically(flexItem, flexLine.mCrossSize, viewIndex);
                        break;
                    case FlexDirection::COLUMN:
                    case FlexDirection::COLUMN_REVERSE:
                        stretchViewHorizontally(flexItem, flexLine.mCrossSize, viewIndex);
                        break;
                    default:
                        throw std::invalid_argument(
                                "Invalid flex direction: " + std::to_string(flexDirection));
                }
            }
        }
    } else {
        for (const FlexLine& flexLine : mFlexContainer->getFlexLinesInternal()) {
            for (auto index : flexLine.mIndicesAlignSelfStretch) {
                auto view = mFlexContainer->getFlexItemAt(index);
                switch (flexDirection) {
                    case FlexDirection::ROW: // Intentional fall through
                    case FlexDirection::ROW_REVERSE:
                        stretchViewVertically(view, flexLine.mCrossSize, index);
                        break;
                    case FlexDirection::COLUMN:
                    case FlexDirection::COLUMN_REVERSE:
                        stretchViewHorizontally(view, flexLine.mCrossSize, index);
                        break;
                    default:
                        throw std::invalid_argument(
                                "Invalid flex direction: " + std::to_string(flexDirection));
                }
            }
        }
    }
}

void FlexboxHelper::stretchViewVertically(Item* flexItem, int crossSize, int index) {
    int newHeight = crossSize - flexItem->getMarginTop() - flexItem->getMarginBottom();
    newHeight = std::max(newHeight, flexItem->getMinHeight());
    newHeight = std::min(newHeight, flexItem->getMaxHeight());
    int childWidthSpec;
    int measuredWidth;
    // if (mMeasuredSizeCache != null) {
    // Retrieve the measured height from the cache because there
    // are some cases that the view is re-created from the last measure, thus
    // View#getMeasuredHeight returns 0.
    // E.g. if the flex container is FlexboxLayoutManager, that case happens
    // frequently
    // measuredWidth = extractLowerInt(mMeasuredSizeCache[index]);
    // } else {
    measuredWidth = flexItem->getMeasuredWidth();
    // }
    childWidthSpec = Item::MeasureSpec::makeMeasureSpec(measuredWidth,
                                                        Item::MeasureSpec::EXACTLY);

    int childHeightSpec = Item::MeasureSpec::makeMeasureSpec(newHeight, Item::MeasureSpec::EXACTLY);
    flexItem->measure(childWidthSpec, childHeightSpec);

    updateMeasureCache(index, childWidthSpec, childHeightSpec, flexItem);
    mFlexContainer->updateViewCache(index, flexItem);
}

void FlexboxHelper::stretchViewHorizontally(Item* flexItem, int crossSize, int index) {
    int newWidth = crossSize - flexItem->getMarginLeft() - flexItem->getMarginRight();
    newWidth = std::max(newWidth, flexItem->getMinWidth());
    newWidth = std::min(newWidth, flexItem->getMaxWidth());
    int childHeightSpec;
    int measuredHeight;
    // if (mMeasuredSizeCache != null) {
    // Retrieve the measured height from the cache because there
    // are some cases that the view is re-created from the last measure, thus
    // View#getMeasuredHeight returns 0.
    // E.g. if the flex container is FlexboxLayoutManager, that case happens
    // frequently
    // measuredHeight = extractHigherInt(mMeasuredSizeCache[index]);
    // } else {
    measuredHeight = flexItem->getMeasuredHeight();
    // }
    childHeightSpec = Item::MeasureSpec::makeMeasureSpec(measuredHeight,
                                                         Item::MeasureSpec::EXACTLY);
    int childWidthSpec = Item::MeasureSpec::makeMeasureSpec(newWidth, Item::MeasureSpec::EXACTLY);
    flexItem->measure(childWidthSpec, childHeightSpec);

    updateMeasureCache(index, childWidthSpec, childHeightSpec, flexItem);
    mFlexContainer->updateViewCache(index, flexItem);
}

void
FlexboxHelper::layoutSingleChildHorizontal(Item* flexItem, FlexLine& flexLine, int left, int top, int right,
                                           int bottom) {
    int alignItems = mFlexContainer->getAlignItems();
    if (flexItem->getAlignSelf() != AlignSelf::AUTO) {
        // Expecting the values for alignItems and mAlignSelf match except for ALIGN_SELF_AUTO.
        // Assigning the mAlignSelf value as alignItems should work.
        alignItems = flexItem->getAlignSelf();
    }
    int crossSize = flexLine.mCrossSize;
    switch (alignItems) {
        case AlignItems::FLEX_START: // Intentional fall through
        case AlignItems::STRETCH:
            if (mFlexContainer->getFlexWrap() != FlexWrap::WRAP_REVERSE) {
                flexItem->layout(left, top + flexItem->getMarginTop(), right,
                                 bottom + flexItem->getMarginTop());
            } else {
                flexItem->layout(left, top - flexItem->getMarginBottom(), right,
                                 bottom - flexItem->getMarginBottom());
            }
            break;
        case AlignItems::BASELINE:
            if (mFlexContainer->getFlexWrap() != FlexWrap::WRAP_REVERSE) {
                int marginTop = flexLine.mMaxBaseline - flexItem->getBaseline();
                marginTop = std::max(marginTop, flexItem->getMarginTop());
                flexItem->layout(left, top + marginTop, right, bottom + marginTop);
            } else {
                int marginBottom = flexLine.mMaxBaseline - flexItem->getMeasuredHeight() + flexItem->getBaseline();
                marginBottom = std::max(marginBottom, flexItem->getMarginBottom());
                flexItem->layout(left, top - marginBottom, right, bottom - marginBottom);
            }
            break;
        case AlignItems::FLEX_END:
            if (mFlexContainer->getFlexWrap() != FlexWrap::WRAP_REVERSE) {
                flexItem->layout(left,
                                 top + crossSize - flexItem->getMeasuredHeight() - flexItem->getMarginBottom(),
                                 right, top + crossSize - flexItem->getMarginBottom());
            } else {
                // If the flexWrap == WRAP_REVERSE, the direction of the
                // flexEnd is flipped (from top to bottom).
                flexItem->layout(left,
                                 top - crossSize + flexItem->getMeasuredHeight() + flexItem->getMarginTop(),
                                 right,
                                 bottom - crossSize + flexItem->getMeasuredHeight() + flexItem->getMarginTop());
            }
            break;
        case AlignItems::CENTER:
            int topFromCrossAxis = (crossSize - flexItem->getMeasuredHeight()
                                    + flexItem->getMarginTop() - flexItem->getMarginBottom()) / 2;
            if (mFlexContainer->getFlexWrap() != FlexWrap::WRAP_REVERSE) {
                flexItem->layout(left, top + topFromCrossAxis,
                                 right, top + topFromCrossAxis + flexItem->getMeasuredHeight());
            } else {
                flexItem->layout(left, top - topFromCrossAxis,
                                 right, top - topFromCrossAxis + flexItem->getMeasuredHeight());
            }
            break;
    }
}

void FlexboxHelper::layoutSingleChildVertical(Item* flexItem, FlexLine& flexLine, bool isRtl, int left, int top,
                                              int right, int bottom) {
    int alignItems = mFlexContainer->getAlignItems();
    if (flexItem->getAlignSelf() != AlignSelf::AUTO) {
        // Expecting the values for alignItems and mAlignSelf match except for ALIGN_SELF_AUTO.
        // Assigning the mAlignSelf value as alignItems should work.
        alignItems = flexItem->getAlignSelf();
    }
    int crossSize = flexLine.mCrossSize;
    switch (alignItems) {
        case AlignItems::FLEX_START: // Intentional fall through
        case AlignItems::STRETCH: // Intentional fall through
        case AlignItems::BASELINE:
            if (!isRtl) {
                flexItem->layout(left + flexItem->getMarginLeft(), top,
                                 right + flexItem->getMarginLeft(), bottom);
            } else {
                flexItem->layout(left - flexItem->getMarginRight(), top,
                                 right - flexItem->getMarginRight(), bottom);
            }
            break;
        case AlignItems::FLEX_END:
            if (!isRtl) {
                flexItem->layout(
                        left + crossSize - flexItem->getMeasuredWidth() - flexItem->getMarginRight(),
                        top,
                        right + crossSize - flexItem->getMeasuredWidth() - flexItem->getMarginRight(),
                        bottom);
            } else {
                // If the flexWrap == WRAP_REVERSE, the direction of the
                // flexEnd is flipped (from left to right).
                flexItem->layout(
                        left - crossSize + flexItem->getMeasuredWidth() + flexItem->getMarginLeft(),
                        top,
                        right - crossSize + flexItem->getMeasuredWidth() + flexItem->getMarginLeft(),
                        bottom);
            }
            break;
        case AlignItems::CENTER:
            int leftFromCrossAxis = (crossSize - flexItem->getMeasuredWidth()
                                     + flexItem->getMarginLeft()
                                     - flexItem->getMarginRight()) / 2;
            if (!isRtl) {
                flexItem->layout(left + leftFromCrossAxis, top, right + leftFromCrossAxis, bottom);
            } else {
                flexItem->layout(left - leftFromCrossAxis, top, right - leftFromCrossAxis, bottom);
            }
            break;
    }
}

bool FlexboxHelper::isWrapRequired(Item* flexItem, int mode, int maxSize, int currentLength, int childLength,
                                   int index, int indexInFlexLine, int flexLinesSize) {
    if (mFlexContainer->getFlexWrap() == FlexWrap::NOWRAP) {
        return false;
    }
    if (flexItem->isWrapBefore()) {
        return true;
    }
    if (mode == Item::MeasureSpec::UNSPECIFIED) {
        return false;
    }
    int maxLine = mFlexContainer->getMaxLine();
    // Judge the condition by adding 1 to the current flexLinesSize because the flex line
    // being computed isn't added to the flexLinesSize.
    if (maxLine != FlexLayout::NOT_SET && maxLine <= flexLinesSize + 1) {
        return false;
    }
    return maxSize < currentLength + childLength;
}
