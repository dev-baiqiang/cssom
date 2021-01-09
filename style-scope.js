const {SelectorsMap} = require('./css-selector');
const {CSSSource} = require('./css-source');

let applicationCSSSelectorVersion = 0;
let applicationCSSSelectors = [];
let applicationSelectors = [];
let applicationAdditionalSelectors = [];

function mergeCSSSelectors() {
    applicationCSSSelectors = applicationSelectors.slice();
    applicationCSSSelectors.push.apply(applicationCSSSelectors, applicationAdditionalSelectors);
    applicationCSSSelectorVersion++;
}

function onCSSChanged(args) {
    const parsed = CSSSource.fromSource(args.cssText, args.cssFile).selectors;
    if (parsed) {
        applicationAdditionalSelectors.push.apply(applicationAdditionalSelectors, parsed);
        mergeCSSSelectors();
    }
}

exports.onCSSChanged = onCSSChanged;

class StyleScope {
    constructor() {
        this._css = '';
        this._localCSSSelectors = [];
        this._localCSSSelectorVersion = 0;
        this._localCSSSelectorsAppliedVersion = 0;
        this._applicationCSSSelectorsAppliedVersion = 0;
    }

    get css() {
        return this._css;
    }

    set css(value) {
        this.setCSS(value);
    }

    addCSS(cssString, cssFileName) {
        this.appendCSS(cssString, cssFileName);
    }

    addCSSFile(cssFileName) {
        this.appendCSS(null, cssFileName);
    }

    setCSS(cssString, cssFileName) {
        this._css = cssString;
        const cssFile = CSSSource.fromSource(cssString, cssFileName);
        this._localCSSSelectors = cssFile.selectors;
        this._localCSSSelectorVersion++;
        this.ensureSelectors();
    }

    appendCSS(cssString, cssFileName) {
        if (!cssString && !cssFileName) {
            return;
        }
        let parsedCSSSelectors = CSSSource.fromSource(cssString, cssFileName);
        this._css = this._css + parsedCSSSelectors.source;
        this._localCSSSelectors.push.apply(this._localCSSSelectors, parsedCSSSelectors.selectors);
        this._localCSSSelectorVersion++;
        this.ensureSelectors();
    }

    ensureSelectors() {
        if (!this.isApplicationCSSSelectorsLatestVersionApplied() || !this.isLocalCSSSelectorsLatestVersionApplied() || !this._mergedCSSSelectors) {
            this._createSelectors();
        }
        return this.getSelectorsVersion();
    }

    _increaseApplicationCSSSelectorVersion() {
        applicationCSSSelectorVersion++;
    }

    isApplicationCSSSelectorsLatestVersionApplied() {
        return this._applicationCSSSelectorsAppliedVersion === applicationCSSSelectorVersion;
    }

    isLocalCSSSelectorsLatestVersionApplied() {
        return this._localCSSSelectorsAppliedVersion === this._localCSSSelectorVersion;
    }

    _createSelectors() {
        let toMerge = [];
        toMerge.push(applicationCSSSelectors);
        this._applicationCSSSelectorsAppliedVersion = applicationCSSSelectorVersion;
        toMerge.push(this._localCSSSelectors);
        this._localCSSSelectorsAppliedVersion = this._localCSSSelectorVersion;
        if (toMerge.length > 0) {
            this._mergedCSSSelectors = toMerge.reduce((merged, next) => merged.concat(next || []), []);
            this._selectors = new SelectorsMap(this._mergedCSSSelectors);
        }
    }

    matchSelectors(view) {
        this.ensureSelectors();
        return this._selectors.query(view);
    }

    query(node) {
        this.ensureSelectors();
        return this._selectors.query(node).selectors;
    }

    getSelectorsVersion() {
        // The counters can only go up. So we can return just appVersion + localVersion
        // The 100000 * appVersion is just for easier debugging
        return 100000 * this._applicationCSSSelectorsAppliedVersion + this._localCSSSelectorsAppliedVersion;
    }
}

exports.StyleScope = StyleScope;
