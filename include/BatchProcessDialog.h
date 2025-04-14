#ifndef BATCHPROCESSDIALOG_H
#define BATCHPROCESSDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include "DatabaseManager.h"
#include "SQLParser.h"

class BatchProcessDialog : public QDialog {
    Q_OBJECT
    
public:
    BatchProcessDialog(DatabaseManager& dbManager, QWidget* parent = nullptr);
    
private slots:
    void browseSqlFile();
    void browseOutputFile();
    void startProcessing();
    void stopProcessing();
    
private:
    DatabaseManager& dbManager;
    SQLParser::SQLParser sqlParser;
    
    QLineEdit* sqlFileEdit;
    QLineEdit* outputFileEdit;
    QTextEdit* logView;
    QProgressBar* progressBar;
    QPushButton* startBtn;
    QPushButton* stopBtn;
    QLabel* statusLabel;
    
    bool isProcessing;
    
    void setupUi();
    void processFile(const QString& filename);
    void processSqlStatement(const QString& sql);
    void log(const QString& message, bool isError = false);
    void updateProgress(int current, int total);
};

#endif 