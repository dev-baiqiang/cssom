/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <vector>
#include "Item.h"

class Layout : public Item {
private:
    std::vector<Item*> mChildren;

protected:
    /**
     * Ask one of the children of this item to measure itself, taking into
     * account both the MeasureSpec requirements for this item and its padding.
     * The heavy lifting is done in getChildMeasureSpec.
     *
     * @param child The child to measure
     * @param parentWidthMeasureSpec The width requirements for this item
     * @param parentHeightMeasureSpec The height requirements for this item
     */
    void measureChild(Item* child, int parentWidthMeasureSpec,
                      int parentHeightMeasureSpec);

    /**
     * Ask all of the children of this item to measure themselves, taking into
     * account both the MeasureSpec requirements for this item and its padding.
     * We skip children that are in the GONE state The heavy lifting is done in
     * getChildMeasureSpec.
     *
     * @param widthMeasureSpec The width requirements for this item
     * @param heightMeasureSpec The height requirements for this item
     */
    void measureChildren(int widthMeasureSpec, int heightMeasureSpec);

    /**
     * Ask one of the children of this view to measure itself, taking into
     * account both the MeasureSpec requirements for this view and its padding
     * and margins. The child must have MarginLayoutParams The heavy lifting is
     * done in getChildMeasureSpec.
     *
     * @param child The child to measure
     * @param parentWidthMeasureSpec The width requirements for this view
     * @param widthUsed Extra space that has been used up by the parent
     *        horizontally (possibly by other children of the parent)
     * @param parentHeightMeasureSpec The height requirements for this view
     * @param heightUsed Extra space that has been used up by the parent
     *        vertically (possibly by other children of the parent)
     */
    void measureChildWithMargins(Item* child,
                                           int parentWidthMeasureSpec, int widthUsed,
                                           int parentHeightMeasureSpec, int heightUsed);

public:

    /**
     * Adds the item to the container.
     *
     * @param item the item to be added
     */
    void addItem(Item* item) { mChildren.emplace_back(item); }

    /**
     * Adds the item to the specified index of the container.
     *
     * @param item  the item to be added
     * @param index the index for the item to be added
     */
    void addItem(Item* item, int index) {
        auto iter = mChildren.begin();
        mChildren.insert(iter + index, item);
    }

    /**
     * Removes all the items contained in the container.
     */
    void removeAllItems() {
        mChildren.clear();
    }

    /**
     * Removes the item at the specified index.
     *
     * @param index the index from which the item is removed.
     */
    void removeItemAt(int index) {
        auto iter = mChildren.begin();
        mChildren.erase(iter);
    }

    /**
     * Returns the position in the group of the specified child item.
     *
     * @param child the item for which to get the position
     * @return a positive integer representing the position of the item in the
     *         group, or -1 if the item does not exist in the group
     */
    int indexOfChild(Item* child) {
        for (int i = 0; i < mChildren.size(); i++) {
            if (mChildren[i] == child) {
                return i;
            }
        }
        return -1;
    }

    /**
     * Returns the number of children in the layout.
     *
     * @return a positive integer representing the number of children in
     *         the layout
     */
    int getChildCount() {
        return mChildren.size();
    }

    /**
     * Returns the item at the specified position in the layout.
     *
     * @param index the position at which to get the item from
     * @return the item at the specified position or null if the position
     *         does not exist within the group
     */
    Item* getChildAt(int index) {
        return mChildren[index];
    }

    static int getChildMeasureSpec(int spec, int padding, int childDimension, float percent);

    /**
     * Returns the child measure spec for its width.
     *
     * @param widthSpec      the measure spec for the width imposed by the parent
     * @param padding        the padding along the width for the parent
     * @param childDimension the value of the child dimension
     * @param percent        the percent value of the child dimension
     */
    static int getChildWidthMeasureSpec(int widthSpec, int padding, int childDimension, float percent) {
        return getChildMeasureSpec(widthSpec, padding, childDimension, percent);
    }

    /**
     * Returns the child measure spec for its height.
     *
     * @param heightSpec     the measure spec for the height imposed by the parent
     * @param padding        the padding along the height for the parent
     * @param childDimension the value of the child dimension
     * @param percent        the percent value of the child dimension
     */
    static int getChildHeightMeasureSpec(int heightSpec, int padding, int childDimension, float percent) {
        return getChildMeasureSpec(heightSpec, padding, childDimension, percent);
    }

    /**
     * @return the top padding of the flex container.
     */
    int getPaddingTop() const { return mPaddingTop; }

    /**
     * @return the left padding of the flex container.
     */
    int getPaddingLeft() const { return mPaddingLeft; }

    /**
     * @return the right padding of the flex container.
     */
    int getPaddingRight() const { return mPaddingRight; }

    /**
     * @return the bottom padding of the flex container.
     */
    int getPaddingBottom() const { return mPaddingBottom; }
};
