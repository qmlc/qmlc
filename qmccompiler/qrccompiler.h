#ifndef QRCCOMPILER_H
#define QRCCOMPILER_H

class QrcCompiler
{

public:

    int compile(int argc, char **argv, const QString &projectBaseDir);

private:
    QQmlEngine *engine;

    int compileQml(const QString &inputFile, const QString &outputFile);
    int compileJs(const QString &inputFile, const QString &outputFile);

};

#endif
