/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "Layout.h"

/**
 * Horizontally lay out children until the row is filled and then moved to the next line. Call
 * {@link FlowLayout#setSingleLine(boolean)} to disable reflow and lay all children out in one line.
 *
 */
class FlowLayout : public Layout {
private:

    int mLineSpacing = 0;
    int mItemSpacing = 0;
    bool mSingleLine = false;
    int mRowCount = 0;

protected:

    int getLineSpacing() const {
        return mLineSpacing;
    }

    void setLineSpacing(int lineSpacing) {
        mLineSpacing = lineSpacing;
    }

    int getItemSpacing() const {
        return mItemSpacing;
    }

    void setItemSpacing(int itemSpacing) {
        mItemSpacing = itemSpacing;
    }

    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;

    void onLayout(bool sizeChanged, int left, int top, int right, int bottom) override;

public:

    /** Returns whether this chip group is single line or reflowed multiline. */
    bool isSingleLine() const {
        return mSingleLine;
    }

    /** Sets whether this chip group is single line, or reflowed multiline. */
    void setSingleLine(bool singleLine) {
        mSingleLine = singleLine;
    }

    int getRowIndex(Item* item);
};
