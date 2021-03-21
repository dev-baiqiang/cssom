/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <vector>
#include "FlexLine.h"
#include "Item.h"

class FlexLayout;

class FlexboxHelper {

public:

    struct FlexLinesResult {

        std::vector<FlexLine> mFlexLines;

        int mChildState = 0;

        void reset() {
            mFlexLines.clear();
            mChildState = 0;
        }
    };

    explicit FlexboxHelper(FlexLayout* mFlexContainer);

    static constexpr int INITIAL_CAPACITY = 10;

    void calculateHorizontalFlexLines(FlexLinesResult& result, int widthMeasureSpec,
                                      int heightMeasureSpec);

    void calculateVerticalFlexLines(FlexLinesResult& result, int widthMeasureSpec,
                                    int heightMeasureSpec);

    void determineMainSize(int widthMeasureSpec, int heightMeasureSpec) {
        determineMainSize(widthMeasureSpec, heightMeasureSpec, 0);
    };

    void determineCrossSize(int widthMeasureSpec, int heightMeasureSpec,
                            int paddingAlongCrossAxis);

    void stretchViews() { stretchViews(0); }

    void stretchViews(int fromIndex);

    void layoutSingleChildHorizontal(Item* flexItem, FlexLine& flexLine, int left, int top, int right,
                                     int bottom);

    void layoutSingleChildVertical(Item* flexItem, FlexLine& flexLine, bool isRtl, int left, int top, int right,
                                   int bottom);

private:

    FlexLayout* mFlexContainer = nullptr;

    /**
     * Holds the 'frozen' state of children during measure. If a view is frozen it will no longer
     * expand or shrink regardless of flex grow/flex shrink attributes.
     */
    std::vector<bool> mChildrenFrozen;

    void calculateFlexLines(FlexLinesResult& result, int mainMeasureSpec,
                            int crossMeasureSpec, int needsCalcAmount, int fromIndex, int toIndex,
                            std::vector<FlexLine>* existingLines);

    /**
     * Returns the container's start padding in the main axis. Either start or top.
     *
     * @param isMainHorizontal is the main axis horizontal
     * @return the start padding in the main axis
     */
    int getPaddingStartMain(bool isMainHorizontal) const;

    /**
     * Returns the container's end padding in the main axis. Either end or bottom.
     *
     * @param isMainHorizontal is the main axis horizontal
     * @return the end padding in the main axis
     */
    int getPaddingEndMain(bool isMainHorizontal);

    /**
     * Returns the container's start padding in the cross axis. Either start or top.
     *
     * @param isMainHorizontal is the main axis horizontal.
     * @return the start padding in the cross axis
     */
    int getPaddingStartCross(bool isMainHorizontal);

    /**
     * Returns the container's end padding in the cross axis. Either end or bottom.
     *
     * @param isMainHorizontal is the main axis horizontal
     * @return the end padding in the cross axis
     */
    int getPaddingEndCross(bool isMainHorizontal);


    /**
     * Returns the view's measured size in the main axis. Either width or height.
     *
     * @param view             the view
     * @param isMainHorizontal is the main axis horizontal
     * @return the view's measured size in the main axis
     */
    static int getViewMeasuredSizeMain(Item* view, bool isMainHorizontal) {
        if (isMainHorizontal) {
            return view->getMeasuredWidth();
        }

        return view->getMeasuredHeight();
    }

    /**
     * Returns the view's measured size in the cross axis. Either width or height.
     *
     * @param view             the view
     * @param isMainHorizontal is the main axis horizontal
     * @return the view's measured size in the cross axis
     */
    static int getViewMeasuredSizeCross(Item* view, bool isMainHorizontal) {
        if (isMainHorizontal) {
            return view->getMeasuredHeight();
        }

        return view->getMeasuredWidth();
    }

    /**
     * Returns the flexItem's size in the main axis. Either width or height.
     *
     * @param flexItem         the flexItem
     * @param isMainHorizontal is the main axis horizontal
     * @return the flexItem's size in the main axis
     */
    static int getFlexItemSizeMain(Item* flexItem, bool isMainHorizontal) {
        if (isMainHorizontal) {
            return flexItem->getWidth();
        }

        return flexItem->getHeight();
    }

    /**
     * Returns the flexItem's size in the cross axis. Either width or height.
     *
     * @param flexItem         the flexItem
     * @param isMainHorizontal is the main axis horizontal
     * @return the flexItem's size in the cross axis
     */
    static int getFlexItemSizeCross(Item* flexItem, bool isMainHorizontal) {
        if (isMainHorizontal) {
            return flexItem->getHeight();
        }

        return flexItem->getWidth();
    }

    /**
     * Returns the flexItem's start margin in the main axis. Either start or top.
     * For the backward compatibility for API level < 17, the horizontal margin is returned using
     * {@link FlexItem#getMarginLeft} (ViewGroup.MarginLayoutParams#getMarginStart isn't available
     * in API level < 17). Thus this method needs to be used with {@link #getFlexItemMarginEndMain}
     * not to misuse the margin in RTL.
     *
     *
     * @param flexItem         the flexItem
     * @param isMainHorizontal is the main axis horizontal
     * @return the flexItem's start margin in the main axis
     */
    static int getFlexItemMarginStartMain(Item* flexItem, bool isMainHorizontal) {
        if (isMainHorizontal) {
            return flexItem->getMarginLeft();
        }

        return flexItem->getMarginTop();
    }

    /**
     * Returns the flexItem's end margin in the main axis. Either end or bottom.
     * For the backward compatibility for API level < 17, the horizontal margin is returned using
     * {@link FlexItem#getMarginRight} (ViewGroup.MarginLayoutParams#getMarginEnd isn't available
     * in API level < 17). Thus this method needs to be used with
     * {@link #getFlexItemMarginStartMain} not to misuse the margin in RTL.
     *
     * @param flexItem         the flexItem
     * @param isMainHorizontal is the main axis horizontal
     * @return the flexItem's end margin in the main axis
     */
    static int getFlexItemMarginEndMain(Item* flexItem, bool isMainHorizontal) {
        if (isMainHorizontal) {
            return flexItem->getMarginRight();
        }

        return flexItem->getMarginBottom();
    }

    /**
     * Returns the flexItem's start margin in the cross axis. Either start or top.
     *
     * @param flexItem         the flexItem
     * @param isMainHorizontal is the main axis horizontal
     * @return the flexItem's start margin in the cross axis
     */
    static int getFlexItemMarginStartCross(Item* flexItem, bool isMainHorizontal) {
        if (isMainHorizontal) {
            return flexItem->getMarginTop();
        }

        return flexItem->getMarginLeft();
    }

    /**
     * Returns the flexItem's end margin in the cross axis. Either end or bottom.
     *
     * @param flexItem         the flexItem
     * @param isMainHorizontal is the main axis horizontal
     * @return the flexItem's end margin in the cross axis
     */
    static int getFlexItemMarginEndCross(Item* flexItem, bool isMainHorizontal) {
        if (isMainHorizontal) {
            return flexItem->getMarginBottom();
        }

        return flexItem->getMarginRight();
    }

    /**
     * Determine if a wrap is required (add a new flex line).
     *
     * @param view          the view being judged if the wrap required
     * @param mode          the width or height mode along the main axis direction
     * @param maxSize       the max size along the main axis direction
     * @param currentLength the accumulated current length
     * @param childLength   the length of a child view which is to be collected to the flex line
     * @param flexItem      the LayoutParams for the view being determined whether a new flex line
     *                      is needed
     * @param index         the index of the view being added within the entire flex container
     * @param indexInFlexLine the index of the view being added within the current flex line
     * @param flexLinesSize the number of the existing flexlines size
     * @return {@code true} if a wrap is required, {@code false} otherwise
     * @see FlexContainer#getFlexWrap()
     * @see FlexContainer#setFlexWrap(int)
     */
    bool isWrapRequired(Item* flexItem, int mode, int maxSize, int currentLength,
                        int childLength, int index, int indexInFlexLine, int flexLinesSize);


    void addFlexLine(std::vector<FlexLine>& flexLines, FlexLine& flexLine, int viewIndex,
                     int usedCrossSizeSoFar);

    void updateMeasureCache(int index, int widthMeasureSpec, int heightMeasureSpec, Item* view);


    /**
     * Checks if the view's width/height don't violate the minimum/maximum size constraints imposed
     * by the {@link FlexItem#getMinWidth()}, {@link FlexItem#getMinHeight()},
     * {@link FlexItem#getMaxWidth()} and {@link FlexItem#getMaxHeight()} attributes.
     *
     * @param view  the view to be checked
     * @param index index of the view
     */
    void checkSizeConstraints(Item* view, int index);


    /**
     * Determine the main size by expanding (shrinking if negative remaining free space is given)
     * an individual child in each flex line if any children's mFlexGrow (or mFlexShrink if
     * remaining
     * space is negative) properties are set to non-zero.
     *
     * @param widthMeasureSpec  horizontal space requirements as imposed by the parent
     * @param heightMeasureSpec vertical space requirements as imposed by the parent
     * @see FlexContainer#setFlexDirection(int)
     * @see FlexContainer#getFlexDirection()
     */
    void determineMainSize(int widthMeasureSpec, int heightMeasureSpec, int fromIndex);


    /**
     * Expand the flex items along the main axis based on the individual mFlexGrow attribute.
     *
     * @param widthMeasureSpec     the horizontal space requirements as imposed by the parent
     * @param heightMeasureSpec    the vertical space requirements as imposed by the parent
     * @param flexLine             the flex line to which flex items belong
     * @param maxMainSize          the maximum main size. Expanded main size will be this size
     * @param paddingAlongMainAxis the padding value along the main axis
     * @param calledRecursively    true if this method is called recursively, false otherwise
     * @see FlexContainer#getFlexDirection()
     * @see FlexContainer#setFlexDirection(int)
     * @see FlexItem#getFlexGrow()
     */
    void expandFlexItems(int widthMeasureSpec, int heightMeasureSpec, FlexLine& flexLine,
                         int maxMainSize, int paddingAlongMainAxis, bool calledRecursively);


    /**
     * Shrink the flex items along the main axis based on the individual mFlexShrink attribute.
     *
     * @param widthMeasureSpec     the horizontal space requirements as imposed by the parent
     * @param heightMeasureSpec    the vertical space requirements as imposed by the parent
     * @param flexLine             the flex line to which flex items belong
     * @param maxMainSize          the maximum main size. Shrank main size will be this size
     * @param paddingAlongMainAxis the padding value along the main axis
     * @param calledRecursively    true if this method is called recursively, false otherwise
     * @see FlexContainer#getFlexDirection()
     * @see FlexContainer#setFlexDirection(int)
     * @see FlexItem#getFlexShrink()
     */
    void shrinkFlexItems(int widthMeasureSpec, int heightMeasureSpec, FlexLine& flexLine,
                         int maxMainSize, int paddingAlongMainAxis, bool calledRecursively);

    void ensureChildrenFrozen(int size);


    int getChildWidthMeasureSpecInternal(int widthMeasureSpec, Item* flexItem,
                                         int padding);

    int getChildHeightMeasureSpecInternal(int heightMeasureSpec, Item* flexItem,
                                          int padding);

    std::vector<FlexLine> constructFlexLinesForAlignContentCenter(std::vector<FlexLine> flexLines,
                                                                  int size, int totalCrossSize);

    /**
    * Expand the view vertically to the size of the crossSize (considering the view margins)
    *
    * @param view      the View to be stretched
    * @param crossSize the cross size
    * @param index     the index of the view
    */
    void stretchViewVertically(Item* view, int crossSize, int index);


    /**
     * Expand the view horizontally to the size of the crossSize (considering the view margins)
     *
     * @param view      the View to be stretched
     * @param crossSize the cross size
     * @param index     the index of the view
     */
    void stretchViewHorizontally(Item* view, int crossSize, int index);
};
