/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cmath>
#include <climits>
#include "FlexEnum.h"

class Item {
public:

    struct LayoutParams {
        /**
         * Special value for the height or width requested by a View.
         * FILL_PARENT means that the view wants to be as big as its parent,
         * minus the parent's padding, if any. This value is deprecated
         * starting in API Level 8 and replaced by {@link #MATCH_PARENT}.
         */
        static constexpr int FILL_PARENT = -1;

        /**
         * Special value for the height or width requested by a View.
         * MATCH_PARENT means that the view wants to be as big as its parent,
         * minus the parent's padding, if any. Introduced in API Level 8.
         */
        static constexpr int MATCH_PARENT = -1;

        /**
         * Special value for the height or width requested by a View.
         * WRAP_CONTENT means that the view wants to be just large enough to fit
         * its own internal content, taking its own padding into account.
         */
        static constexpr int WRAP_CONTENT = -2;

    };

    struct MeasureSpec {

        /**
         * Extracts the mode from the supplied measure specification.
         *
         * @param measureSpec the measure specification to extract the mode from
         * @return {@link android.view.View.MeasureSpec#UNSPECIFIED},
         *         {@link android.view.View.MeasureSpec#AT_MOST} or
         *         {@link android.view.View.MeasureSpec#EXACTLY}
         */
        static int getMode(int measureSpec) {
            //noinspection ResourceType
            return (measureSpec & MODE_MASK);
        }

        /**
         * Extracts the size from the supplied measure specification.
         *
         * @param measureSpec the measure specification to extract the size from
         * @return the size in pixels defined in the supplied measure specification
         */
        static int getSize(int measureSpec) {
            return (measureSpec & ~MODE_MASK);
        }

        static constexpr int MODE_SHIFT = 30;
        static constexpr int MODE_MASK = 0x3 << MODE_SHIFT;


        /**
         * Measure unspecified mode: The parent has not imposed any constraint
         * on the child. It can be whatever size it wants.
         */
        static constexpr int UNSPECIFIED = 0 << MODE_SHIFT;

        /**
         * Measure specification mode: The parent has determined an exact size
         * for the child. The child is going to be given those bounds regardless
         * of how big it wants to be.
         */
        static constexpr int EXACTLY = 1 << MODE_SHIFT;

        /**
         * Measure specification mode: The child can be as large as it wants up
         * to the specified size.
         */
        static constexpr int AT_MOST = 2 << MODE_SHIFT;


        static int makeMeasureSpec(int size, int mode) {
            return (size & ~MODE_MASK) | (mode & MODE_MASK);
        }


        /**
         * Like {@link #makeMeasureSpec(int, int)}, but any spec with a mode of UNSPECIFIED
         * will automatically get a size of 0. Older apps expect this.
         */
        static int makeSafeMeasureSpec(int size, int mode) {
            if (mode == UNSPECIFIED) {
                return 0;
            }
            return makeMeasureSpec(size, mode);
        }
    };

    /**
     * Bits of {@link #getMeasuredWidthAndState()} and
     * {@link #getMeasuredWidthAndState()} that provide the actual measured size.
     */
    static constexpr int MEASURED_SIZE_MASK = 0x00ffffff;

    /**
     * Bits of {@link #getMeasuredWidthAndState()} and
     * {@link #getMeasuredWidthAndState()} that provide the additional state bits.
     */
    static constexpr int MEASURED_STATE_MASK = 0xff000000;

    /**
     * Bit shift of {@link #MEASURED_STATE_MASK} to get to the height bits
     * for functions that combine both width and height into a single int,
     * such as {@link #getMeasuredState()} and the childState argument of
     * {@link #resolveSizeAndState(int, int, int)}.
     */
    static constexpr int MEASURED_HEIGHT_STATE_SHIFT = 16;


    /**
     * Bit of {@link #getMeasuredWidthAndState()} and
     * {@link #getMeasuredWidthAndState()} that indicates the measured size
     * is smaller that the space the view would like to have.
     */
    static constexpr int MEASURED_STATE_TOO_SMALL = 0x01000000;


    static constexpr int NOT_SET = -1;

    /** The default value for the order attribute */
    static constexpr int ORDER_DEFAULT = 1;

    /** The default value for the flex grow attribute */
    static constexpr float FLEX_GROW_DEFAULT = 0;

    /** The default value for the flex shrink attribute */
    static constexpr float FLEX_SHRINK_DEFAULT = 1;

    /** The value representing the flex shrink attribute is not set  */
    static constexpr float FLEX_SHRINK_NOT_SET = 0;

    /** The default value for the flex basis percent attribute */
    static constexpr float FLEX_BASIS_PERCENT_DEFAULT = -1;

    /** The maximum size of the max width and max height attributes */
    static constexpr int MAX_SIZE = INT_MAX & MEASURED_SIZE_MASK;

private:
    int mLeft = 0;
    int mRight = 0;
    int mTop = 0;
    int mBottom = 0;

    float mFlexGrow = FLEX_GROW_DEFAULT;
    float mFlexShrink = FLEX_SHRINK_DEFAULT;
    int mAlignSelf = AlignSelf::AUTO;
    float mFlexBasisPercent = FLEX_BASIS_PERCENT_DEFAULT;
    int mMinWidth = NOT_SET;
    int mMinHeight = NOT_SET;
    int mMaxWidth = MAX_SIZE;
    int mMaxHeight = MAX_SIZE;
    int mWidth = LayoutParams::WRAP_CONTENT;
    int mHeight = LayoutParams::WRAP_CONTENT;
    int mLeftMargin = 0;
    int mTopMargin = 0;
    int mRightMargin = 0;
    int mBottomMargin = 0;
    bool mWrapBefore = false;
    int mMeasuredWidth = 0;
    int mMeasuredHeight = 0;
    float mWidthPercent = 1;
    float mHeightPercent = 1;
    int mViewFlags = 0;
    float mWeight = 0;

protected:
    int mPaddingLeft = 0;
    int mPaddingRight = 0;
    int mPaddingTop = 0;
    int mPaddingBottom = 0;

public:
    inline int getWidth() const {
        return mWidth;
    }

    inline void setWidth(int width) {
        mWidth = width;
    }

    inline int getHeight() const {
        return mHeight;
    }

    inline void setHeight(int height) {
        mHeight = height;
    }

    inline float getFlexGrow() const {
        return mFlexGrow;
    }

    inline void setFlexGrow(float flexGrow) {
        mFlexGrow = flexGrow;
    }

    inline float getFlexShrink() const {
        return mFlexShrink;
    }

    inline void setFlexShrink(float flexShrink) {
        mFlexShrink = flexShrink;
    }

    inline int getAlignSelf() const {
        return mAlignSelf;
    }

    inline void setAlignSelf(int alignSelf) {
        mAlignSelf = alignSelf;
    }

    inline int getMinWidth() const {
        return mMinWidth;
    }

    inline void setMinWidth(int minWidth) {
        mMinWidth = minWidth;
    }

    inline int getMinHeight() const {
        return mMinHeight;
    }

    inline void setMinHeight(int minHeight) {
        mMinHeight = minHeight;
    }

    inline int getMaxWidth() const {
        return mMaxWidth;
    }

    inline void setMaxWidth(int maxWidth) {
        mMaxWidth = maxWidth;
    }

    inline int getMaxHeight() const {
        return mMaxHeight;
    }

    inline void setMaxHeight(int maxHeight) {
        mMaxHeight = maxHeight;
    }

    inline bool isWrapBefore() const {
        return mWrapBefore;
    }

    inline void setWrapBefore(bool wrapBefore) {
        mWrapBefore = wrapBefore;
    }

    inline float getFlexBasisPercent() const {
        return mFlexBasisPercent;
    }

    inline void setFlexBasisPercent(float flexBasisPercent) {
        mFlexBasisPercent = flexBasisPercent;
    }

    inline int getMarginLeft() const {
        return mLeftMargin;
    }

    inline int getMarginTop() const {
        return mTopMargin;
    }

    inline int getMarginRight() const {
        return mRightMargin;
    }

    inline int getMarginBottom() const {
        return mBottomMargin;
    }

    inline int getMarginHorizontal() const {
        return mRight + mLeft;
    }

    inline int getMarginVertical() const {
        return mTop + mBottom;
    }

    inline int getLeft() const {
        return mLeft;
    }

    inline int getRight() const {
        return mRight;
    }

    inline int getTop() const {
        return mTop;
    }

    inline int getBottom() const {
        return mBottom;
    }

    inline void setWidthPercent(float percent) {
        mWidthPercent = percent;
    }

    inline float getWidthPercent() const {
        return mWidthPercent;
    };

    inline void setHeightPercent(float percent) {
        mHeightPercent = percent;
    }

    inline float getHeightPercent() const {
        return mHeightPercent;
    };

    inline float getWeight() const {
        return mWeight;
    }

    inline void setWeight(float weight) {
        mWeight = weight;
    }

protected:
    virtual void onMeasure(int widthMeasureSpec, int heightMeasureSpec);

    virtual void onLayout(bool changed, int left, int top, int right, int bottom) {}

public:

    enum class Visibility {
        VISIBLE,
        INVISIBLE,
        GONE
    };

    /**
     * This view is visible.
     * Use with {@link #setVisibility} and <a href="#attr_android:visibility">{@code
     * android:visibility}.
     */
    static constexpr int VISIBLE = 0x00000000;

    /**
     * This view is invisible, but it still takes up space for layout purposes.
     * Use with {@link #setVisibility} and <a href="#attr_android:visibility">{@code
     * android:visibility}.
     */
    static constexpr int INVISIBLE = 0x00000004;

    /**
     * This view is invisible, and it doesn't take any space for layout
     * purposes. Use with {@link #setVisibility} and <a href="#attr_android:visibility">{@code
     * android:visibility}.
     */
    static constexpr int GONE = 0x00000008;

    /**
     * Mask for use with setFlags indicating bits used for visibility.
     * {@hide}
     */
    static constexpr int VISIBILITY_MASK = 0x0000000C;


    int getVisibility() const { return mViewFlags & VISIBILITY_MASK; }

    void measure(int widthMeasureSpec, int heightMeasureSpec);

    int getMeasuredWidth() const { return mMeasuredWidth & MEASURED_SIZE_MASK; }

    int getMeasuredHeight() const { return mMeasuredHeight & MEASURED_SIZE_MASK; }

    /**
     * Merge two states as returned by {@link #getMeasuredState()}.
     * @param curState The current state as returned from a view or the result
     * of combining multiple views.
     * @param newState The new view state to combine.
     * @return Returns a new integer reflecting the combination of the two
     * states.
     */
    static int combineMeasuredStates(int curState, int newState) {
        return curState | newState;
    }

    static int resolveSizeAndState(int size, int measureSpec, int childMeasuredState);

    static int getDefaultSize(int size, int measureSpec);

    int getMeasuredState();

    int getBaseline();

    void setMeasuredDimension(int measuredWidth, int measuredHeight);

    void layout(int l, int t, int r, int b);

    bool setFrame(int left, int top, int right, int bottom);
};
