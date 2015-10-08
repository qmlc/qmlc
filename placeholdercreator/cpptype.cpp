#include <QCoreApplication>
#include <stdio.h>
#include <QDebug>
#include <QFile>
#include <QCommandLineParser>
#include <QStringList>
#include <QTextStream>
#include "classparser.h"
#include "classparserinformation.h"


bool writeToFile(const QString &fileName, const QStringList &lines) {
    QFile file(fileName);
    file.remove();
    if (!file.open(QIODevice::ReadWrite)) {
        qWarning() << "Cannot open the file for writing " << fileName;
        return false;
    }
    QTextStream stream(&file);
    foreach (const QString &line, lines) {
        stream << line << "\n";
    }
    stream.flush();
    if (stream.status() == QTextStream::WriteFailed) {
        qWarning() << "Failed to write" << fileName;
        return false;
    }
    return true;
}

ClassParserInformation *getClassInfo(QString inputHeader, QString strClassName, ClassParser &parser)
{
    QByteArray ba = inputHeader.toLatin1();
    const char *strHeaderFile = ba.data();

    parser.parse(strHeaderFile);

    ClassParserInformation *retVal = NULL;
    for (int k=0; k < parser.getClassInformationCount(); ++k) {
        ClassParserInformation *pInformation = parser.getClassInformation(k);
        if (pInformation->m_strClassName == strClassName) {
            retVal = pInformation;
        }
    }
    return retVal;
}

bool addProperties(QStringList &header, const ClassParserInformation &info)
{
    foreach (const SClassParserQProperty &prop, info.m_listQProperty) {
        QString p = "Q_PROPERTY(";
        p += prop.m_strPropertyType + " " + prop.m_strPropertyName;
        // Check if this is default property.
        for (int k = 0; k < prop.m_listFunctionParamType.size(); ++k) {
            p += " " + prop.m_listFunctionParamType[k] + " " + prop.m_listFunctionParamName[k];
        }
        p += ")";
        header << p;
    }
    return true;
}

bool addFunctions(QStringList &header, QStringList &source,
    const QString &originalClassName, const QString &className,
    const ClassParserInformation &info)
{
    // Loop over all functions. Add to header and to source.
    // Keep order the same as in original.
    EClassParserMemberType previous = FUNCTION_TYPE_NONE;
    foreach (const SClassParserFunction &func, info.m_listParserFunction) {
        switch (func.m_pType) {
        case FUNCTION_TYPE_PRIVATE:
        case FUNCTION_TYPE_PROTECTED:
        case FUNCTION_TYPE_PRIVATE_SLOTS:
        case FUNCTION_TYPE_PROTECTED_SLOTS:
        case FUNCTION_TYPE_NONE:
            continue; // We do not need these at all.
        // Types below must be handled also below for "public:", "signals:" etc.
        case FUNCTION_TYPE_PUBLIC:
        case FUNCTION_TYPE_SIGNAL:
        case FUNCTION_TYPE_PUBLIC_SLOTS:
            break; // Needed types.
        default:
            qWarning() << "Unhandled function type, skipping:" << func.m_pType;
            continue;
        }
        if (func.m_strFunctionReturn == "emit")
            continue;
        QString decl = func.m_strFunctionInline;
        decl.replace("inline", "");
        QString ret = func.m_strFunctionReturn;
        QString funcName = func.m_strFunctionName;
        if (funcName == originalClassName)
            funcName = className; // Constructor, needs to be renamed.
        else if (funcName == "~" + originalClassName)
            funcName = "~" + className; // Destructor, needs to be renamed.
        decl += " " + ret + " " + funcName + "(";
        if (ret.startsWith("virtual"))
            ret.remove(0, strlen("virtual"));
        else if (ret.startsWith("explicit"))
            ret.remove(0, strlen("explicit"));
        QString def = ret + " " + className + "::" + funcName + "(";
        for (int k = 0; k < func.m_listFunctionParamType.size(); ++k) {
            QString type = func.m_listFunctionParamType[k];
            decl += type;
            def += type;
            decl += " " + func.m_listFunctionParamName[k];
            if (!func.m_listFunctionParamDefaultValue[k].isEmpty())
                decl += " = " + func.m_listFunctionParamDefaultValue[k];
            if (k < func.m_listFunctionParamType.size() - 1) {
                decl += ", ";
                def += ", ";
            }
        }
        QString end = func.m_strFunctionInlineEnd.trimmed();
        if (end.startsWith(")"))
            end.remove(0, 1);
        if (!end.isEmpty())
            end = " " + end;
        decl += ")" + end;
        if (func.m_bPureVirtualFunction) {
            decl += " = 0";
        }
        decl += ";";
        // Nothing is executed so we can get away with empty function body.
        def += ")" + end + "{}";
        if (previous != func.m_pType) {
            switch (func.m_pType) {
            case FUNCTION_TYPE_PUBLIC:
                header << "public:";
                break;
            case FUNCTION_TYPE_SIGNAL:
                header << "signals:";
                break;
            case FUNCTION_TYPE_PUBLIC_SLOTS:
                header << "public slots:";
                break;
            default:
                header << QString("//Unhandled function type: %1").arg(func.m_pType);
                qWarning() << "Unhandled function type:" << func.m_pType;
                break;
            }
            previous = func.m_pType;
        }
        header << decl.trimmed();
        if (!func.m_bPureVirtualFunction && func.m_pType != FUNCTION_TYPE_SIGNAL)
            source << def.trimmed();
    }
    return true;
}

bool generateFileContents(QStringList &header, QStringList &source,
    const QString &originalClassName, const QString &className,
    const QStringList &dependencyIncludes, const QStringList &buildIncludes,
    const QString &headerName, const ClassParserInformation &info)
{
    // Add header and source file contents to strings.
    QString upper = className.toUpper() + "_H";
    header << "#if !defined(" + upper + ")";
    header << "#define " + upper;
    // Add necessary includes.
    // Trick so that qmake finds dependences but files are not included.
    header << "#if !defined(_QMCTYPELIBBUILD)";
    foreach (const QString &inc, dependencyIncludes)
        header << "#include \"" + inc + "\"";
    // Takes care of class name appearing in parameters and return types.
    header << "#else";
    header << "#define " + originalClassName + " " + className;
    header << "#endif";
    // Some of these could be in <> but as long as it works...
    foreach (const QString &inc, buildIncludes)
        header << "#include \"" + inc + "\"";
    source << "#include \"" + headerName + "\"";
    // Add all parent classes.
    QString parents;
    if (!info.m_listChildClass.isEmpty())
        parents = " : ";
    EClassParserMemberType previous = FUNCTION_TYPE_NONE;
    if (info.m_strClassType == "class")
        previous = FUNCTION_TYPE_PRIVATE;
    else if (info.m_strClassType == "struct")
        previous = FUNCTION_TYPE_PUBLIC;
    else {
        qWarning() << "Do not know how to handle:" << info.m_strClassType;
        return false;
    }
    // In plain "class Foo : Bar" what is the type listed as?
    for (int k = 0; k < info.m_listChildClass.size(); ++k) {
        const SClassParserChildClass &parent = info.m_listChildClass[k];
        if (previous != parent.m_pType) {
            switch (parent.m_pType) {
            case FUNCTION_TYPE_PUBLIC:
                parents += "public ";
                break;
            case FUNCTION_TYPE_PRIVATE:
                parents += "private ";
                break;
            case FUNCTION_TYPE_PROTECTED:
                parents += "protected ";
                break;
            default:
                // What is the type for Bar in "public Foo, Bar"?
                qWarning() << "Unexpected class inheritance type:" << parent.m_pType;
                continue;
            }
            previous = parent.m_pType;
        } else if (k && k < info.m_listChildClass.size() - 1) {
            // Uses same inheritance as previous (or default if first).
            parents += ", ";
        }
        parents += parent.m_strName;
        if (k != info.m_listChildClass.size() - 1)
            parents += ", ";
    }
    header << info.m_strClassType + " " + className + parents + " {";
    if (!addProperties(header, info))
        return false;
    // Anything seemingly recognized as macros is probably needed.
    // This includes Q_CLASSINFO, for example.
    foreach (const SClassParserPossibleMacroWithoutKnownledge& m, info.m_listPossibleMacroWithoutKnownledge) {
        header << m.m_strMacroLine;
    }
    if (!addFunctions(header, source, originalClassName, className, info))
        return false;
    header << "};";
    header << "#endif";
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser cmdline;
    cmdline.setApplicationDescription("C++ type placeholder generator.");
    cmdline.addHelpOption();

    QCommandLineOption inputHeader("i",
        QCoreApplication::translate("main", "Input header file name."),
        QCoreApplication::translate("main", "file name"));
    cmdline.addOption(inputHeader);

    QCommandLineOption dependencyInclude("d",
        QCoreApplication::translate("main", "Header included for depencencies."),
        QCoreApplication::translate("main", "file name"));
    cmdline.addOption(dependencyInclude);

    QCommandLineOption buildInclude("b",
        QCoreApplication::translate("main", "Header included for build."),
        QCoreApplication::translate("main", "file name[,file,...]"));
    cmdline.addOption(buildInclude);

    QCommandLineOption className("c",
        QCoreApplication::translate("main", "Input class name."),
        QCoreApplication::translate("main", "class name"));
    cmdline.addOption(className);

    QCommandLineOption outputSource("source",
        QCoreApplication::translate("main", "Output source file name."),
        QCoreApplication::translate("main", "file name"));
    cmdline.addOption(outputSource);

    QCommandLineOption outputHeader("header",
        QCoreApplication::translate("main", "Output header file name."),
        QCoreApplication::translate("main", "file name"));
    cmdline.addOption(outputHeader);

    QCommandLineOption outputClass("class",
        QCoreApplication::translate("main", "Output class name."),
        QCoreApplication::translate("main", "class name"));
    cmdline.addOption(outputClass);

    cmdline.process(a.arguments());

    bool requiredGiven = true;
    if (!cmdline.isSet(inputHeader)) {
        requiredGiven = false;
        qWarning() << "Input header name must be given.";
    }
    if (!cmdline.isSet(className)) {
        requiredGiven = false;
        qWarning() << "Input class name must be given.";
    }
    if (!cmdline.isSet(outputSource)) {
        requiredGiven = false;
        qWarning() << "Output source file name must be given.";
    }
    if (!cmdline.isSet(outputHeader)) {
        requiredGiven = false;
        qWarning() << "Output header file name must be given.";
    }
    if (!cmdline.isSet(outputClass)) {
        requiredGiven = false;
        qWarning() << "Output class name must be given.";
    }
    if (!requiredGiven)
        return 5;

    QStringList listIn;
    QStringList listOut;

    listIn << "Q_OBJECT";
    listOut << "Q_OBJECT;";
    listIn << "Q_SLOTS";
    listOut << "slots";
    listIn << "Q_SIGNALS";
    listOut << "signals";

    ClassParser parser;
    parser.setPreDefines(listIn, listOut);

    SClassParserMacroBeginForMember sMacroBegin;
    sMacroBegin.m_bRequiresBrackets = true;
    sMacroBegin.m_strMacroBegin = "Q_REVISION";
    parser.add(sMacroBegin);

    QString input = cmdline.value(inputHeader);
    QString inputClassName(cmdline.value(className));
    ClassParserInformation *info = getClassInfo(input, inputClassName, parser);
    if (!info) {
        qWarning() << "No" << inputClassName << "info found in" << input;
        return 3;
    }
    QStringList buildIncludeList, dependencyIncludeList;
    if (cmdline.isSet(buildInclude))
        buildIncludeList = cmdline.value(buildInclude).split(",", QString::SkipEmptyParts);
    if (cmdline.isSet(dependencyInclude))
        dependencyIncludeList = cmdline.value(dependencyInclude).split(",", QString::SkipEmptyParts);
    QStringList header, source;
    QString headerName = cmdline.value(outputHeader);
    QString sourceName = cmdline.value(outputSource);
    QString outputClassName = cmdline.value(outputClass);
    if (!generateFileContents(header, source, inputClassName, outputClassName,
        dependencyIncludeList, buildIncludeList, headerName, *info))
    {
        qWarning() << "Output files not generated.";
        return 4;
    }
    writeToFile(sourceName, source);
    writeToFile(headerName, header);
    return 0;
}
