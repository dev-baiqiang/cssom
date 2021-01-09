const EventEmitter = require('events').EventEmitter;
const {CSSStyle} = require('./css-style');
const {StyleScope} = require('./style-scope');
const {CSSState} = require('./css-state');

class View extends EventEmitter {
    constructor() {
        super();
        this._cssStyle = new CSSStyle(new WeakRef(this));
        this._styleScope = new StyleScope();
        this._cssState = new CSSState(new WeakRef(this));
        this.cssClasses = new Set();
        this.cssPseudoClasses = new Set();
        this.children = [];
    }

    eachChildView(callback) {
        for (let i = 0, length = this.children.length; i < length; i++) {
            const retVal = callback(this.children[i]);
            if (retVal === false) {
                break;
            }
        }
    }

    set style(str) {
        this._cssStyle = str;
    }

    get style() {
        return this._cssStyle;
    }

    get cssType() {
        if (!this._cssType) {
            this._cssType = this.typeName.toLowerCase();
        }
        return this._cssType;
    }

    set cssType(type) {
        this._cssType = type.toLowerCase();
    }

    get typeName() {
        return this.constructor.name;
    }

    addEventListener(...args) {
        return super.on(args);
    }

    removeEventListener(...args) {
        return super.off(args);
    }

    _onCssStateChange() {
        this._cssState.onChange();
        this.eachChildView((child) => {
            child._cssState.onChange();
            return true;
        });
    }

    set className(newValue) {
        const cssClasses = this.cssClasses;
        cssClasses.clear();
        if (typeof newValue === 'string' && newValue !== '') {
            newValue.split(' ').forEach((c) => cssClasses.add(c));
        }
        this._onCssStateChange();
    }

    addChild(v) {
        if (v.parent === this) return;
        this.children.push(v);
        v.parent = this;
    }

    getChildIndex(v) {
        return this.children.indexOf(v);
    }

    getChildAt(index) {
        return this.children[index];
    }

}

exports.View = View;
