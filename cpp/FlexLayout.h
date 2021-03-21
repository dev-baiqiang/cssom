/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <vector>
#include <string>
#include "Item.h"
#include "FlexboxHelper.h"
#include "FlexLine.h"
#include "FlexEnum.h"
#include "Layout.h"

class FlexLayout : public Layout {
public:
    FlexLayout();

protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;

    void onLayout(bool changed, int left, int top, int right, int bottom) override;

private:
    /**
     * The current value of the {@link FlexDirection}, the default value is {@link
     * FlexDirection#ROW}.
     *
     * @see FlexDirection
     */
    int mFlexDirection = FlexDirection::ROW;

    /**
     * The current value of the {@link FlexWrap}, the default value is {@link FlexWrap#NOWRAP}.
     *
     * @see FlexWrap
     */
    int mFlexWrap = FlexWrap::NOWRAP;

    /**
     * The current value of the {@link JustifyContent}, the default value is
     * {@link JustifyContent#FLEX_START}.
     *
     * @see JustifyContent
     */
    int mJustifyContent = JustifyContent::FLEX_START;

    /**
     * The current value of the {@link AlignItems}, the default value is
     * {@link AlignItems#FLEX_START}.
     *
     * @see AlignItems
     */
    int mAlignItems = AlignItems::FLEX_START;

    /**
     * The current value of the {@link AlignContent}, the default value is
     * {@link AlignContent#FLEX_START}.
     *
     * @see AlignContent
     */
    int mAlignContent = AlignContent::FLEX_START;

    /**
     * The current value of the maxLine attribute, which specifies the maximum number of flex lines.
     */
    int mMaxLine = NOT_SET;

    FlexboxHelper mFlexboxHelper;

    std::vector<FlexLine> mFlexLines;

    /**
     * Used for receiving the calculation of the flex results to avoid creating a new instance
     * every time flex lines are calculated.
     */
    FlexboxHelper::FlexLinesResult mFlexLinesResult;

public:

    static constexpr int NOT_SET = -1;

    /**
     * @return the number of flex items contained in the flex container.
     */
    int getFlexItemCount() { return getChildCount(); }

    /**
     * Returns a flex item as a View at the given index.
     *
     * @param index the index
     * @return the view at the index
     */
    Item* getFlexItemAt(int index) { return getChildAt(index); }


    /**
     * @return the flex direction attribute of the flex container.
     * @see FlexDirection
     */
    int getFlexDirection() const { return mFlexDirection; }

    /**
     * Sets the given flex direction attribute to the flex container.
     *
     * @param flexDirection the flex direction value
     * @see FlexDirection
     */
    void setFlexDirection(int flexDirection) { mFlexDirection = flexDirection; }

    /**
     * @return the flex wrap attribute of the flex container.
     * @see FlexWrap
     */
    int getFlexWrap() const { return mFlexWrap; }

    /**
     * Sets the given flex wrap attribute to the flex container.
     *
     * @param flexWrap the flex wrap value
     * @see FlexWrap
     */
    void setFlexWrap(int flexWrap) { mFlexWrap = flexWrap; }

    /**
     * @return the justify content attribute of the flex container.
     * @see JustifyContent
     */
    int getJustifyContent() const { return mJustifyContent; }

    /**
     * Sets the given justify content attribute to the flex container.
     *
     * @param justifyContent the justify content value
     * @see JustifyContent
     */
    void setJustifyContent(int justifyContent) { mJustifyContent = justifyContent; }

    /**
     * @return the align content attribute of the flex container.
     * @see AlignContent
     */
    int getAlignContent() const { return mAlignContent; }

    /**
     * Sets the given align content attribute to the flex container.
     *
     * @param alignContent the align content value
     */
    void setAlignContent(int alignContent) { mAlignContent = alignContent; }

    /**
     * @return the align items attribute of the flex container.
     * @see AlignItems
     */
    int getAlignItems() const { return mAlignItems; }

    /**
     * Sets the given align items attribute to the flex container.
     *
     * @param alignItems the align items value
     * @see AlignItems
     */
    void setAlignItems(int alignItems) { mAlignItems = alignItems; }

    /**
     * @return the flex lines composing this flex container. The overridden method should return a
     * copy of the original list excluding a dummy flex line (flex line that doesn't have any flex
     * items in it but used for the alignment along the cross axis) so that any changes of the
     * returned list are not reflected to the original list.
     */
    std::vector<FlexLine> getFlexLines() { return mFlexLines; }

    /**
     * Returns true if the main axis is horizontal, false otherwise.
     *
     * @return true if the main axis is horizontal, false otherwise
     */
    bool isMainAxisDirectionHorizontal() const {
        return mFlexDirection == FlexDirection::ROW || mFlexDirection == FlexDirection::ROW_REVERSE;
    }

    /**
     * @return the start padding of this view depending on its resolved layout direction.
     */
    int getPaddingStart() const { return mFlexDirection == FlexDirection::ROW_REVERSE ? mPaddingRight : mPaddingLeft; }

    /**
     * @return the end padding of this view depending on its resolved layout direction.
     */
    int getPaddingEnd() const { return mFlexDirection == FlexDirection::ROW_REVERSE ? mPaddingLeft : mPaddingRight; }

    /**
     * @return the largest main size of all flex lines including decorator lengths.
     */
    int getLargestMainSize();

    /**
     * @return the sum of the cross sizes of all flex lines including decorator lengths.
     */
    int getSumOfCrossSize();

    /**
     * Callback when a new flex item is added to the current container
     *
     * @param view            the view as a flex item which is added
     * @param index           the absolute index of the flex item added
     * @param indexInFlexLine the relative index of the flex item added within the flex line
     * @param flexLine        the flex line where the new flex item is added
     */
    void onNewFlexItemAdded(Item* view, int index, int indexInFlexLine, FlexLine flexLine) {}

    /**
     * Callback when a new flex line is added to the current container
     *
     * @param flexLine the new added flex line
     */
    void onNewFlexLineAdded(FlexLine flexLine) {}

    /**
     * Sets the list of the flex lines that compose the flex container to the one received as an
     * argument.
     *
     * @param flexLines the list of flex lines
     */
    void setFlexLines(std::vector<FlexLine> flexLines) { mFlexLines = flexLines; }

    /**
     * @return the current value of the maximum number of flex lines. If not set, {@link #NOT_SET}
     * is returned.
     */
    int getMaxLine() const { return mMaxLine; }

    /**
     *
     * @param maxLine the int value, which specifies the maximum number of flex lines
     */
    void setMaxLine(int maxLine) { mMaxLine = maxLine; }

    /**
     * @return the list of the flex lines including dummy flex lines (flex line that doesn't have
     * any flex items in it but used for the alignment along the cross axis), which aren't included
     * in the {@link FlexContainer#getFlexLines()}.
     */
    std::vector<FlexLine>& getFlexLinesInternal() { return mFlexLines; }

    /**
     * Update the view cache in the flex container.
     *
     * @param position the position of the view to be updated
     * @param view     the view instance
     */
    void updateViewCache(int position, Item* view) {}

    void measureHorizontal(int widthMeasureSpec, int heightMeasureSpec);

    void measureVertical(int widthMeasureSpec, int heightMeasureSpec);

    void setMeasuredDimensionForFlex(int flexDirection, int widthMeasureSpec,
                                     int heightMeasureSpec, int childState);

    void layoutHorizontal(bool isRtl, int left, int top, int right, int bottom);

    void layoutVertical(bool isRtl, bool fromBottomToTop, int left, int top,
                        int right, int bottom);
};
