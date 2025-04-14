#include "SQLHighlighter.h"

SQLHighlighter::SQLHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    setupHighlightingRules();
}

void SQLHighlighter::setupHighlightingRules() {
    // 设置关键字格式
    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);
    
    // SQL关键字列表
    QStringList keywordPatterns = {
        "\\bSELECT\\b", "\\bFROM\\b", "\\bWHERE\\b", "\\bAND\\b",
        "\\bOR\\b", "\\bINSERT\\b", "\\bINTO\\b", "\\bVALUES\\b",
        "\\bUPDATE\\b", "\\bSET\\b", "\\bDELETE\\b", "\\bCREATE\\b",
        "\\bTABLE\\b", "\\bDROP\\b", "\\bALTER\\b", "\\bINDEX\\b",
        "\\bGROUP\\b", "\\bBY\\b", "\\bHAVING\\b", "\\bORDER\\b",
        "\\bASC\\b", "\\bDESC\\b", "\\bLIMIT\\b", "\\bOFFSET\\b",
        "\\bJOIN\\b", "\\bINNER\\b", "\\bLEFT\\b", "\\bRIGHT\\b",
        "\\bON\\b", "\\bAS\\b", "\\bIN\\b", "\\bLIKE\\b", "\\bIS\\b",
        "\\bNULL\\b", "\\bNOT\\b", "\\bPRIMARY\\b", "\\bKEY\\b"
    };
    
    for (const QString& pattern : keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern, 
            QRegularExpression::CaseInsensitiveOption);
        rule.format = keywordFormat;
        highlightingRules.push_back(rule);
    }
    
    // 设置注释格式
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    HighlightingRule rule;
    rule.pattern = QRegularExpression("--[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.push_back(rule);
    
    // 设置字符串格式
    quotationFormat.setForeground(Qt::darkRed);
    rule.pattern = QRegularExpression("'[^']*'");
    rule.format = quotationFormat;
    highlightingRules.push_back(rule);
    
    // 设置数字格式
    numberFormat.setForeground(Qt::darkCyan);
    rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
    rule.format = numberFormat;
    highlightingRules.push_back(rule);
}

void SQLHighlighter::highlightBlock(const QString& text) {
    for (const HighlightingRule& rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = 
            rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), 
                     rule.format);
        }
    }
} 