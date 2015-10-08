#ifndef CLASSPARSER_H
#define CLASSPARSER_H

#include <QObject>
#include <QStringList>
#include "classparserinformation.h"

struct ClassParserIncludeHeaderInformation {

    /**
     * @brief m_bInPublicIncludeFolders
     * true == for example #include <myheader.h>
     * false == for example #include "myheader.h"
     */
    bool m_bInPublicIncludeFolders;
    QString m_strIncludeHeader;
    QString m_strIncludeHeaderWithFullFilePath;
};

struct SClassParserDefine {

    bool m_bMacro;
    QString m_strDefineIn;
    QString m_strDefineOutput;
};

/**
 * @brief The SClassParserMacroBeginForMember struct
 * simple macro QString that can be before the function (or data type), such as
 * Q_REVISION
 * if (m_bRequiresBrackets == true), then the Q_REVISION must have brackets
 * for example:
 * m_strMacroBegin == "Q_REVISION"
 * m_bRequiresBrackets == true
 * -> means actually, Q_REVISION(X)
 * m_strMacroBegin == "Q_REVISION"
 * m_bRequiresBrackets == false
 * -> means actually, Q_REVISION   without any macro functionality
 */
struct SClassParserMacroBeginForMember {

    QString m_strMacroBegin;
    bool m_bRequiresBrackets;
};

class ClassParser : public QObject
{
    Q_OBJECT
public:
    explicit ClassParser(QObject *parent = 0);
    bool parse(const char *strHeaderFullFilePath);
    bool parseHeader(const char *strHeader, const char *strHeaderFullFilePath);
    unsigned int parseClass(const char *strHeader, int iClassIndex);
    unsigned int parseEnum(const char *strHeader, int iClassIndex);
    unsigned int parseTypedef(const char *strHeader, int iClassIndex);

    void setPreDefines(QStringList in, QStringList out);
    void addDefine(SClassParserDefine pDef);
    void setListPublicIncludePaths(QStringList listPublicIncludePaths);

    void add(SClassParserMacroBeginForMember macroBegin);

    int getClassInformationCount();
    ClassParserInformation *getClassInformation(int iIndex);

    int getIncludeHeaderInformationCount();
    ClassParserIncludeHeaderInformation *getIncludeHeaderInformation(int iIndex);
signals:

public slots:

private:
    QList<ClassParserInformation>m_listClassInformation;
    QList<ClassParserIncludeHeaderInformation> m_listIncludeHeader;
    QList<SClassParserDefine>m_listDefine;
    QList<SClassParserMacroBeginForMember>m_listMacroBeginForMember;

    QString getStringBetween(const char *strHeader, unsigned int iStartIndex, unsigned int iEndIndex);
    bool parseQProperty(QString strQPropertyLine, int iClassIndex);
    bool isNextFunctionParamStart(const char *strIn, unsigned int iEndIndex);
    void parseEnumLine(const char *strLine, int iClassIndex);

    bool isStartWithMacroBeginForFunction(const char *strHeader, int *iMacroIndex);

    bool isIncludeHeaderParsed(ClassParserIncludeHeaderInformation *pIncludeHeader);
    QString findFullFilePathForHeaderInformation(ClassParserIncludeHeaderInformation *pIncludeHeader, const char *strHeaderFullFilePath);

    void setChildClassPointer();

    QStringList m_listPreDefineIn;
    QStringList m_listPreDefineOut;
    QStringList m_listPublicIncludePaths;
};

#endif // CLASSPARSER_H
