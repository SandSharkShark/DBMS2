#include "BatchProcessDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

BatchProcessDialog::BatchProcessDialog(DatabaseManager& manager, QWidget* parent)
    : QDialog(parent)
    , dbManager(manager)
    , isProcessing(false) {
    setupUi();
}

void BatchProcessDialog::setupUi() {
    setWindowTitle("SQL批处理");
    resize(600, 400);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // SQL文件选择
    QHBoxLayout* sqlFileLayout = new QHBoxLayout;
    sqlFileEdit = new QLineEdit(this);
    QPushButton* browseSqlBtn = new QPushButton("浏览", this);
    sqlFileLayout->addWidget(new QLabel("SQL文件:"));
    sqlFileLayout->addWidget(sqlFileEdit);
    sqlFileLayout->addWidget(browseSqlBtn);
    
    // 输出文件选择
    QHBoxLayout* outputFileLayout = new QHBoxLayout;
    outputFileEdit = new QLineEdit(this);
    QPushButton* browseOutputBtn = new QPushButton("浏览", this);
    outputFileLayout->addWidget(new QLabel("输出文件:"));
    outputFileLayout->addWidget(outputFileEdit);
    outputFileLayout->addWidget(browseOutputBtn);
    
    // 日志视图
    logView = new QTextEdit(this);
    logView->setReadOnly(true);
    
    // 进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    
    // 状态标签
    statusLabel = new QLabel("就绪", this);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    startBtn = new QPushButton("开始处理", this);
    stopBtn = new QPushButton("停止", this);
    stopBtn->setEnabled(false);
    buttonLayout->addStretch();
    buttonLayout->addWidget(startBtn);
    buttonLayout->addWidget(stopBtn);
    
    // 添加到主布局
    mainLayout->addLayout(sqlFileLayout);
    mainLayout->addLayout(outputFileLayout);
    mainLayout->addWidget(logView);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(browseSqlBtn, &QPushButton::clicked, this, &BatchProcessDialog::browseSqlFile);
    connect(browseOutputBtn, &QPushButton::clicked, this, &BatchProcessDialog::browseOutputFile);
    connect(startBtn, &QPushButton::clicked, this, &BatchProcessDialog::startProcessing);
    connect(stopBtn, &QPushButton::clicked, this, &BatchProcessDialog::stopProcessing);
}

void BatchProcessDialog::browseSqlFile() {
    QString filename = QFileDialog::getOpenFileName(this,
        "选择SQL文件", "", "SQL文件 (*.sql);;所有文件 (*)");
    if (!filename.isEmpty()) {
        sqlFileEdit->setText(filename);
    }
}

void BatchProcessDialog::browseOutputFile() {
    QString filename = QFileDialog::getSaveFileName(this,
        "选择输出文件", "", "日志文件 (*.log);;所有文件 (*)");
    if (!filename.isEmpty()) {
        outputFileEdit->setText(filename);
    }
}

void BatchProcessDialog::startProcessing() {
    QString sqlFile = sqlFileEdit->text();
    if (sqlFile.isEmpty()) {
        QMessageBox::warning(this, "错误", "请选择SQL文件");
        return;
    }
    
    isProcessing = true;
    startBtn->setEnabled(false);
    stopBtn->setEnabled(true);
    progressBar->setValue(0);
    logView->clear();
    
    log("开始处理...");
    processFile(sqlFile);
}

void BatchProcessDialog::stopProcessing() {
    isProcessing = false;
    startBtn->setEnabled(true);
    stopBtn->setEnabled(false);
    log("处理已停止");
}

void BatchProcessDialog::processFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        log("无法打开文件: " + filename, true);
        return;
    }
    
    QTextStream in(&file);
    QString sqlBuffer;
    int lineCount = 0;
    int totalLines = 0;
    
    // 计算总行数
    while (!in.atEnd()) {
        in.readLine();
        totalLines++;
    }
    
    // 重置文件指针
    in.seek(0);
    
    // 打开输出文件
    QFile outputFile(outputFileEdit->text());
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        log("无法打开输出文件", true);
        return;
    }
    QTextStream out(&outputFile);
    
    // 写入处理开始时间
    out << "\n=== 批处理开始 " << QDateTime::currentDateTime().toString() << " ===\n\n";
    
    while (!in.atEnd() && isProcessing) {
        QString line = in.readLine();
        lineCount++;
        
        // 更新进度
        updateProgress(lineCount, totalLines);
        
        // 忽略注释和空行
        if (line.trimmed().startsWith("--") || line.trimmed().isEmpty()) {
            continue;
        }
        
        sqlBuffer += line + "\n";
        
        // 如果遇到分号，执行SQL语句
        if (line.trimmed().endsWith(";")) {
            try {
                processSqlStatement(sqlBuffer);
                out << "成功: " << sqlBuffer;
            } catch (const std::exception& e) {
                QString error = QString("错误: %1\nSQL: %2").arg(e.what()).arg(sqlBuffer);
                log(error, true);
                out << error << "\n";
            }
            sqlBuffer.clear();
        }
    }
    
    // 写入处理结束时间
    out << "\n=== 批处理结束 " << QDateTime::currentDateTime().toString() << " ===\n";
    
    file.close();
    outputFile.close();
    
    if (isProcessing) {
        log("处理完成");
    }
    
    isProcessing = false;
    startBtn->setEnabled(true);
    stopBtn->setEnabled(false);
}

void BatchProcessDialog::processSqlStatement(const QString& sql) {
    try {
        auto query = sqlParser.parse(sql.toStdString());
        
        if (query.type == "SELECT") {
            auto results = dbManager.executeSelect(query);
            log(QString("查询返回 %1 行").arg(results.size()));
        } else {
            if (dbManager.executeNonQuery(query)) {
                log("执行成功: " + sql);
            } else {
                throw std::runtime_error("执行失败");
            }
        }
    } catch (const std::exception& e) {
        log(QString("错误: %1").arg(e.what()), true);
        throw;
    }
}

void BatchProcessDialog::log(const QString& message, bool isError) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString formattedMessage = QString("[%1] %2").arg(timestamp).arg(message);
    
    QTextCharFormat format;
    if (isError) {
        format.setForeground(Qt::red);
    }
    
    QTextCursor cursor = logView->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(formattedMessage + "\n");
    logView->setTextCursor(cursor);
    
    statusLabel->setText(message);
}

void BatchProcessDialog::updateProgress(int current, int total) {
    int percentage = (current * 100) / total;
    progressBar->setValue(percentage);
    statusLabel->setText(QString("处理中... %1/%2").arg(current).arg(total));
} 