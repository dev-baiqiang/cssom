const reduceCSSCalc = require('reduce-css-calc');

function isCSSCalcExpression(value) {
    return value.includes('calc(');
}

function isCSSVariableExpression(value) {
    return value.includes('var(--');
}

function isCSSVariable(property) {
    return /^--[^,\s]+?$/.test(property);
}

const unsetValue = {};

function _evaluateCSSVariableExpression(view, cssName, value) {
    if (typeof value !== 'string') {
        return value;
    }

    if (!isCSSVariableExpression(value)) {
        return value;
    }

    let output = value.trim();

    // Evaluate every (and nested) css-variables in the value.
    let lastValue;
    while (lastValue !== output) {
        lastValue = output;

        const idx = output.lastIndexOf('var(');
        if (idx === -1) {
            continue;
        }

        const endIdx = output.indexOf(')', idx);
        if (endIdx === -1) {
            continue;
        }

        const matched = output
            .substring(idx + 4, endIdx)
            .split(',')
            .map((v) => v.trim())
            .filter((v) => !!v);
        const cssVariableName = matched.shift();
        let cssVariableValue = view.style.getCSSVariable(cssVariableName);
        if (cssVariableValue === null && matched.length) {
            cssVariableValue = _evaluateCSSVariableExpression(view, cssName, matched.join(', ')).split(',')[0];
        }

        if (!cssVariableValue) {
            cssVariableValue = 'unset';
        }

        output = `${output.substring(0, idx)}${cssVariableValue}${output.substring(endIdx + 1)}`;
    }

    return output;
}

function _evaluateCSSCalcExpression(value) {
    if (typeof value !== 'string') {
        return value;
    }

    if (isCSSCalcExpression(value)) {
        return reduceCSSCalc(value);
    } else {
        return value;
    }
}

function evaluateCSSExpressions(view, property, value) {
    const newValue = _evaluateCSSVariableExpression(view, property, value);
    if (newValue === 'unset') {
        return unsetValue;
    }

    value = newValue;

    try {
        value = _evaluateCSSCalcExpression(value);
    } catch (e) {
        console.error(`Failed to evaluate css-calc for property [${property}] for expression [${value}] to ${view}. ${e.stack}`);

        return unsetValue;
    }

    return value;
}


class CSSState {
    constructor(viewRef) {
        this.viewRef = viewRef;
        this._onDynamicStateChangeHandler = () => this.updateDynamicState();
    }

    /**
     * Called when a change had occurred that may invalidate the statically matching selectors (class, id, ancestor selectors).
     * As a result, at some point in time, the selectors matched have to be requerried from the style scope and applied to the view.
     */
    onChange() {
        const view = this.viewRef.deref();
        if (view) {
            this.unsubscribeFromDynamicUpdates();
            this.updateMatch();
            this.subscribeForDynamicUpdates();
            this.updateDynamicState();
        } else {
            this._matchInvalid = true;
        }
    }

    isSelectorsLatestVersionApplied() {
        const view = this.viewRef.deref();
        if (!view) {
            console.log(`isSelectorsLatestVersionApplied returns default value "false" because "this.viewRef" cleared.`);
            return false;
        }
        return this.viewRef.deref()._styleScope.getSelectorsVersion() === this._appliedSelectorsVersion;
    }

    onLoaded() {
        if (this._matchInvalid) {
            this.updateMatch();
        }
        this.subscribeForDynamicUpdates();
        this.updateDynamicState();
    }

    onUnloaded() {
        this.unsubscribeFromDynamicUpdates();
    }

    updateMatch() {
        const view = this.viewRef.deref();
        if (view && view._styleScope) {
            this._match = view._styleScope.matchSelectors(view);
            this._appliedSelectorsVersion = view._styleScope.getSelectorsVersion();
        } else {
            this._match = CSSState.emptyMatch;
        }
        this._matchInvalid = false;
    }

    updateDynamicState() {
        const view = this.viewRef.deref();
        if (!view) {
            console.log(`updateDynamicState not executed to view because ".viewRef" is cleared`, Trace.categories.Style, Trace.messageType.warn);
            return;
        }
        const matchingSelectors = this._match.selectors.filter((sel) => (sel.dynamic ? sel.match(view) : true));
        this.setPropertyValues(matchingSelectors);
    }

    /**
     * Calculate the difference between the previously applied property values,
     * and the new set of property values that have to be applied for the provided selectors.
     * Apply the values and ensure each property setter is called at most once to avoid excessive change notifications.
     * @param matchingSelectors
     */
    setPropertyValues(matchingSelectors) {
        const view = this.viewRef.deref();
        if (!view) {
            console.error(`${matchingSelectors} not set to view's property because ".viewRef" is cleared`);
            return;
        }
        const newPropertyValues = new view.style.Property();
        matchingSelectors.forEach((selector) => selector.ruleset.declarations.forEach((declaration) => (newPropertyValues[declaration.property] = declaration.value)));
        const oldProperties = this._appliedPropertyValues;

        let isCssExpressionInUse = false;
        // Update values for the scope's css-variables
        view.style.resetScopedCSSVariables();

        for (const property in newPropertyValues) {
            const value = newPropertyValues[property];
            if (isCSSVariable(property)) {
                view.style.setScopedCSSVariable(property, value);

                delete newPropertyValues[property];
                continue;
            }

            isCssExpressionInUse = isCssExpressionInUse || isCSSVariableExpression(value) || isCSSCalcExpression(value);
        }

        if (isCssExpressionInUse) {
            // Evalute css-expressions to get the latest values.
            for (const property in newPropertyValues) {
                const value = evaluateCSSExpressions(view, property, newPropertyValues[property]);
                if (value === unsetValue) {
                    delete newPropertyValues[property];
                    continue;
                }

                newPropertyValues[property] = value;
            }
        }
        // Property values are fully updated, freeze the object to be used for next update.
        Object.freeze(newPropertyValues);
        // Unset removed values
        for (const property in oldProperties) {
            if (!(property in newPropertyValues)) {
                if (property in view.style) {
                    view.style[`css:${property}`] = unsetValue;
                } else {
                    // TRICKY: How do we unset local value?
                }
            }
        }
        // Set new values to the style
        for (const property in newPropertyValues) {
            if (oldProperties && property in oldProperties && oldProperties[property] === newPropertyValues[property]) {
                // Skip unchanged values
                continue;
            }
            const value = newPropertyValues[property];
            try {
                if (property in view.style) {
                    view.style[`css:${property}`] = value;
                } else {
                    const camelCasedProperty = property.replace(/-([a-z])/g, function (g) {
                        return g[1].toUpperCase();
                    });
                    view[camelCasedProperty] = value;
                }
            } catch (e) {
                console.log(`Failed to apply property [${property}] with value [${value}] to ${view}. ${e.stack}`, Trace.categories.Error, Trace.messageType.error);
            }
        }
        this._appliedPropertyValues = newPropertyValues;
    }

    subscribeForDynamicUpdates() {
        const changeMap = this._match.changeMap;
        changeMap.forEach((changes, view) => {
            if (changes.attributes) {
                changes.attributes.forEach((attribute) => {
                    view.addEventListener(attribute + 'Change', this._onDynamicStateChangeHandler);
                });
            }
            if (changes.pseudoClasses) {
                changes.pseudoClasses.forEach((pseudoClass) => {
                    let eventName = ':' + pseudoClass;
                    view.addEventListener(':' + pseudoClass, this._onDynamicStateChangeHandler);
                    if (view[eventName]) {
                        view[eventName](+1);
                    }
                });
            }
        });
        this._appliedChangeMap = changeMap;
    }

    unsubscribeFromDynamicUpdates() {
        this._appliedChangeMap.forEach((changes, view) => {
            if (changes.attributes) {
                changes.attributes.forEach((attribute) => {
                    view.removeEventListener(attribute + 'Change', this._onDynamicStateChangeHandler);
                });
            }
            if (changes.pseudoClasses) {
                changes.pseudoClasses.forEach((pseudoClass) => {
                    let eventName = ':' + pseudoClass;
                    view.removeEventListener(eventName, this._onDynamicStateChangeHandler);
                    if (view[eventName]) {
                        view[eventName](-1);
                    }
                });
            }
        });
        this._appliedChangeMap = CSSState.emptyChangeMap;
    }

    toString() {
        const view = this.viewRef.deref();
        if (!view) {
            console.log(`toString() of CSSState cannot execute correctly because ".viewRef" is cleared`, Trace.categories.Animation, Trace.messageType.warn);
            return '';
        }
        return `${view}._cssState`;
    }
}

CSSState.emptyChangeMap = Object.freeze(new Map());
CSSState.emptyPropertyBag = Object.freeze({});
CSSState.emptyAnimationArray = Object.freeze([]);
CSSState.emptyMatch = {
    selectors: [],
    changeMap: new Map(),
    addAttribute: () => {
    },
    addPseudoClass: () => {
    },
    properties: null,
};

CSSState.prototype._appliedChangeMap = CSSState.emptyChangeMap;
CSSState.prototype._appliedPropertyValues = CSSState.emptyPropertyBag;
CSSState.prototype._matchInvalid = true;

exports.CSSState = CSSState;

