class CSSStyle {
    constructor(ref) {
        this.unscopedCSSVariables = new Map();
        this.scopedCSSVariables = new Map();
        this.viewRef = ref;
    }

    setScopedCSSVariable(varName, value) {
        this.scopedCSSVariables.set(varName, value);
    }

    setUnscopedCSSVariable(varName, value) {
        this.unscopedCSSVariables.set(varName, value);
    }

    getCSSVariable(varName) {
        const view = this.view;
        if (!view) {
            return null;
        }
        if (this.unscopedCSSVariables.has(varName)) {
            return this.unscopedCSSVariables.get(varName);
        }
        if (this.scopedCSSVariables.has(varName)) {
            return this.scopedCSSVariables.get(varName);
        }
        if (!view.parent || !view.parent.style) {
            return null;
        }
        return view.parent.style.getCSSVariable(varName);
    }

    resetScopedCSSVariables() {
        this.scopedCSSVariables.clear();
    }

    resetUnscopedCSSVariables() {
        this.unscopedCSSVariables.clear();
    }

    toString() {
        const view = this.viewRef.deref();
        if (!view) {
            return '';
        }
        return `${view}.style`;
    }

    get view() {
        if (this.viewRef) {
            return this.viewRef.deref();
        }
        return undefined;
    }
}

CSSStyle.prototype.Property = class {
};


exports.CSSStyle = CSSStyle;