#ifndef SQLHIGHLIGHTER_H
#define SQLHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class SQLHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
    
private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    std::vector<HighlightingRule> highlightingRules;
    
    QTextCharFormat keywordFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat numberFormat;
    
    void setupHighlightingRules();
    
protected:
    void highlightBlock(const QString& text) override;
    
public:
    SQLHighlighter(QTextDocument* parent = nullptr);
};

#endif 