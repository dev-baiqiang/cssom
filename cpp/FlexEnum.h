/*
 * Copyright 2021 BaiQiang
 *
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#pragma once

/** This attribute controls the alignment along the cross axis. */
struct AlignItems {

    /** Flex item's edge is placed on the cross start line. */
    static constexpr int FLEX_START = 0;

    /** Flex item's edge is placed on the cross end line. */
    static constexpr int FLEX_END = 1;

    /** Flex item's edge is centered along the cross axis. */
    static constexpr int CENTER = 2;

    /** Flex items are aligned based on their text's baselines. */
    static constexpr int BASELINE = 3;

    /** Flex items are stretched to fill the flex line's cross size. */
    static constexpr int STRETCH = 4;
};

/**
 * This attribute controls the alignment along the cross axis.
 * The alignment in the same direction can be determined by the {@link AlignItems} attribute in the
 * parent, but if this is set to other than {@link AlignSelf#AUTO},
 * the cross axis alignment is overridden for this child.
 */
struct AlignSelf {

    /**
     * The default value for the AlignSelf attribute, which means use the inherit
     * the {@link AlignItems} attribute from its parent.
     */
    static constexpr int AUTO = -1;

    /** This item's edge is placed on the cross start line. */
    static constexpr int FLEX_START = AlignItems::FLEX_START;

    /** This item's edge is placed on the cross end line. */
    static constexpr int FLEX_END = AlignItems::FLEX_END;

    /** This item's edge is centered along the cross axis. */
    static constexpr int CENTER = AlignItems::CENTER;

    /** This items is aligned based on their text's baselines. */
    static constexpr int BASELINE = AlignItems::BASELINE;

    /** This item is stretched to fill the flex line's cross size. */
    static constexpr int STRETCH = AlignItems::STRETCH;
};

/**
 * The direction children items are placed inside the flex container, it determines the
 * direction of the main axis (and the cross axis, perpendicular to the main axis).
 */
enum FlexDirection {

    /**
     * Main axis direction -> horizontal. Main start to
     * main end -> Left to right (in LTR languages).
     * Cross start to cross end -> Top to bottom
     */
    ROW = 0,

    /**
     * Main axis direction -> horizontal. Main start
     * to main end -> Right to left (in LTR languages). Cross start to cross end ->
     * Top to bottom.
     */
    ROW_REVERSE = 1,

    /**
     * Main axis direction -> vertical. Main start
     * to main end -> Top to bottom. Cross start to cross end ->
     * Left to right (In LTR languages).
     */
    COLUMN = 2,

    /**
     * Main axis direction -> vertical. Main start
     * to main end -> Bottom to top. Cross start to cross end -> Left to right
     * (In LTR languages)
     */
    COLUMN_REVERSE = 3
};

/**
 * This attribute controls whether the flex container is single-line or multi-line, and the
 * direction of the cross axis.
 */
struct FlexWrap {

    /** The flex container is single-line. */
    static constexpr int NOWRAP = 0;

    /** The flex container is multi-line. */
    static constexpr int WRAP = 1;

    /**
     * The flex container is multi-line. The direction of the
     * cross axis is opposed to the direction as the {@link #WRAP}
     */
    static constexpr int WRAP_REVERSE = 2;
};

/** This attribute controls the alignment of the flex lines in the flex container. */
struct AlignContent {

    /** Flex lines are packed to the start of the flex container. */
    static constexpr int FLEX_START = 0;

    /** Flex lines are packed to the end of the flex container. */
    static constexpr int FLEX_END = 1;

    /** Flex lines are centered in the flex container. */
    static constexpr int CENTER = 2;

    /**
     * Flex lines are evenly distributed in the flex container. The first flex line is
     * placed at the start of the flex container, the last flex line is placed at the
     * end of the flex container.
     */
    static constexpr int SPACE_BETWEEN = 3;

    /**
     * Flex lines are evenly distributed in the flex container with the same amount of spaces
     * between the flex lines.
     */
    static constexpr int SPACE_AROUND = 4;

    /**
     * Flex lines are stretched to fill the remaining space along the cross axis.
     */
    static constexpr int STRETCH = 5;
};

/** This attribute controls the alignment along the main axis. */
struct JustifyContent {

    /** Flex items are packed toward the start line. */
    static constexpr int FLEX_START = 0;

    /** Flex items are packed toward the end line. */
    static constexpr int FLEX_END = 1;

    /** Flex items are centered along the flex line where the flex items belong. */
    static constexpr int CENTER = 2;

    /**
     * Flex items are evenly distributed along the flex line, first flex item is on the
     * start line, the last flex item is on the end line.
     */
    static constexpr int SPACE_BETWEEN = 3;

    /**
     * Flex items are evenly distributed along the flex line with the same amount of spaces between
     * the flex lines.
     */
    static constexpr int SPACE_AROUND = 4;

    /**
     * Flex items are evenly distributed along the flex line. The difference between
     * {@link #SPACE_AROUND} is that all the spaces between items should be the same as the
     * space before the first item and after the last item.
     * See
     * <a href="https://developer.mozilla.org/en-US/docs/Web/CSS/justify-content">the document on MDN</a>
     * for more details.
     */
    static constexpr int SPACE_EVENLY = 5;
};
