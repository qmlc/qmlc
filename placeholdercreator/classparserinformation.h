#ifndef CLASSPARSERINFORMATION_H
#define CLASSPARSERINFORMATION_H

#include <QObject>
#include <QStringList>

enum EClassParserMemberType {
    FUNCTION_TYPE_PUBLIC,
    FUNCTION_TYPE_PRIVATE,
    FUNCTION_TYPE_PROTECTED,
    FUNCTION_TYPE_SIGNAL,
    FUNCTION_TYPE_PUBLIC_SLOTS,
    FUNCTION_TYPE_PRIVATE_SLOTS,
    FUNCTION_TYPE_PROTECTED_SLOTS,
    FUNCTION_TYPE_NONE,
};

enum EClassParserClassMainType {
    CLASS_MAIN_TYPE_CLASS,
    CLASS_MAIN_TYPE_STRUCT,
    CLASS_MAIN_TYPE_ENUM,
    CLASS_MAIN_TYPE_TYPEDEF,
    CLASS_MAIN_TYPE_TEMPLATE,
    CLASS_MAIN_TYPE_UNION,
    CLASS_MAIN_TYPE_NONE,
};

enum EParseAction {
    PARSE_ACTION_NOTHING,
    PARSE_ACTION_COMMENTS_OUT_MULTI_LINE,
    PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE,
};

enum EParseActionEnum {

    PARSE_ACTION_ENUM_NOTHING,
    PARSE_ACTION_ENUM_GOT_NAME,
    PARSE_ACTION_ENUM_PARSE_THE_LINE,
};


enum EParseActionTypedef {

    PARSE_ACTION_TYPEDEF_NOTHING,
    PARSE_ACTION_TYPEDEF_GOT_ADDITIONAL_TYPE,
    PARSE_ACTION_TYPEDEF_GOT_NAME,
    PARSE_ACTION_TYPEDEF_PARSE_THE_LINE,
};


enum EParseActionSecondary {
    PARSE_ACTION_SECONDARY_NOTHING,
    PARSE_ACTION_SECONDARY_IN_CLASS_BEGIN,
    PARSE_ACTION_SECONDARY_IN_GOT_CLASS_NAME,
    PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING,
    PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FIRST_HAND_INFO,
    PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_OR_DATATYPE,
    PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_BRACETS_IN,
    PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_BRACETS_OUT,
    PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_INLINE,
};

struct SClassParserFunction {

    EClassParserMemberType m_pType;

    QString m_strFunctionInline;
    QString m_strFunctionName;
    QString m_strFunctionReturn;
    QString m_strFunctionInlineEnd; // for example, can be const

    QStringList m_listFunctionParamName;
    QStringList m_listFunctionParamType;
    QStringList m_listFunctionParamDefaultValue;

    bool m_bPureVirtualFunction;
};

struct SClassParserChildClass {

    EClassParserMemberType m_pType;
    QString m_strName;
};


struct SClassParserPossibleMacroWithoutKnownledge {

    EClassParserMemberType m_pType;
    QString m_strMacroLine;
};

struct SClassParserDataType {

    EClassParserMemberType m_pType;
    QString m_strPreDataType;
    QString m_strDataType;
    QString m_strName;
};

struct SClassParserQProperty {

    EClassParserMemberType m_pType;

    QString m_strPropertyName;
    QString m_strPropertyType;

    QStringList m_listFunctionParamName;
    QStringList m_listFunctionParamType;
};

struct SClassParserEnum {

    QString m_strPropertyName;
    QString m_strPropertyValue;
};

class ClassParserInformation
{
public:
    ClassParserInformation();

    QString m_strFromHeaderFile;
    QString m_strClassName;
    QString m_strClassType; // class or struct, enum or typedef, can be also "class presentation";
    QString m_strClassAdditionalType; // on typedef, can be enum, or QMCLOADERSHARED_EXPORT before class's name
    QList<SClassParserFunction>m_listParserFunction;
    QList<SClassParserPossibleMacroWithoutKnownledge>m_listPossibleMacroWithoutKnownledge;
    QList<SClassParserDataType>m_listDataType;
    QList<SClassParserQProperty>m_listQProperty;
    QList<SClassParserEnum>m_listEnum;
    QList<SClassParserChildClass>m_listChildClass;
    QStringList m_listCTypeFunctionCallback;
    QString m_strTypedefString;
    int m_iParentClassIndex;

    /**
     * @brief m_listToChildClass
     * just pointer to child class (if it's previously parsed)
     */
    QList<ClassParserInformation *>m_listToChildClass;

    void printLog();
};

#endif // CLASSPARSERINFORMATION_H
