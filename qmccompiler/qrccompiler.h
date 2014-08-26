#ifndef QRCCOMPILER_H
#define QRCCOMPILER_H

class QrcCompiler
{

public:

    QrcCompiler();
    void exitOnCompileError(bool exit);
    int compile(int argc, char **argv, const QString &projectBaseDir,
            const QString &qrcFile);

private:
    QStringList qrcQmlFiles;
    QStringList qrcJsFiles;
    QQmlEngine *engine;
    bool exitOnError;

    int parseQrc(QString qrcFile);
    int compileQml(const QString &inputFile, const QString &outputFile);
    int compileJs(const QString &inputFile, const QString &outputFile);

};

#endif
