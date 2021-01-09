const {cssTreeParse} = require('./css-parser');
const {fromAstNodes} = require('./css-selector');

class CSSSource {
    constructor(_ast, _url, _file, _source) {
        this._ast = _ast;
        this._url = _url;
        this._file = _file;
        this._source = _source;
        this._selectors = [];
        this.parse();
    }

    static fromSource(source, url) {
        return new CSSSource(undefined, url, undefined, source);
    }

    get selectors() {
        return this._selectors;
    }

    get ast() {
        return this._ast;
    }

    get source() {
        return this._source;
    }

    parse() {
        try {
            if (!this._ast) {
                // [object Object] check guards against empty app.css file
                if (this._source && this.source !== '[object Object]') {
                    this.parseCSSAst();
                }
            }
            if (this._ast) {
                this.createSelectors();
            } else {
                this._selectors = [];
            }
        } catch (e) {
            console.error('Css styling failed: ' + e);
            this._selectors = [];
        }
    }

    parseCSSAst() {
        if (this._source) {
            this._ast = cssTreeParse(this._source, this._file);
        }
    }

    createSelectors() {
        if (this._ast) {
            this._selectors = [...this.createSelectorsFromSyntaxTree()];
        }
    }

    createSelectorsFromSyntaxTree() {
        const nodes = this._ast.stylesheet.rules;
        return fromAstNodes(nodes);
    }

    toString() {
        return this._file || this._url || '(in-memory)';
    }
}

exports.CSSSource = CSSSource;