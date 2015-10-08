#ifndef CLASSPARSERFIND_H
#define CLASSPARSERFIND_H

#include <QObject>
#include "classparserinformation.h"

enum EMEMBER_TYPE {

    MEMBER_TYPE_FUNCTION,
    MEMBER_TYPE_DATATYPE,
    MEMBER_TYPE_NONE,
};

class ClassParserFind
{
public:
    ClassParserFind();

    /**
     * @brief isThisEqualToSpace, is this c equal to space or non text, can be \t, \n, \r also
     * @param c
     * @return, true is c is equal to non text
     */
    static bool isThisEqualToSpace(char c);
    static EClassParserMemberType isStartWithMemberType(const char *strText, unsigned int *rvPosition, bool bAddColon = true);
    static EClassParserClassMainType isStartWithClassType(const char *strText, unsigned int *rvPosition);

    static char findToNextNextValidNonSpaceChar(const char *strTextFromFind, unsigned int *rvPosition);
    static bool isCharInTheList(char c, const char *list);

    static bool skipThisLine(const char *strText, unsigned int iStartPosition, unsigned int *rvPosition);

    /**
     * @brief findToNextChar
     * @param strTextFromFind, find next char from this text
     * @param cFindThisChar, text string or single char to find
     * @param rvPosition, position, how many chars ahead
     * @return true if the char is found
     */
    static bool findToNextChar(const char *strTextFromFind, const char *cFindThisChar, unsigned int *rvPosition);
    static bool findToNextCharCleanButOnlyOneChar(const char *strTextFromFind, const char *cFindThisChar, unsigned int *rvPosition);

    static QString getStringBetween(const char *strHeader, unsigned int iStartIndex, unsigned int iEndIndex);

    static int parseAfterNextSpaceAndThenRemoveLineBreak(const char *strIn, char *strOut);
    static unsigned int parseToNextSpace(const char *strIn, char *strOut, const char *cAdditionalListEndChar);
    static unsigned int parseToNext(const char *strIn, char *strOut, const char *cAdditionalListEndChar);
    static unsigned int parseToNextWithBracketIndex0(const char *strIn, char *strOut, const char *cAdditionalListEndChar);
    static bool isNextFunctionParamStart(const char *strIn, unsigned int iEndIndex);
    static unsigned int parseDefine(const char *strIn, ClassParser *pClassParser);
    static unsigned int parseTemplate(const char *strIn, ClassParserInformation *pClassInformation);
    static unsigned int parseClassName(const char *strIn, ClassParserInformation *pClassInformation);
    static EMEMBER_TYPE getFunctionOrDatatypeEndPosition(const char *strIn, unsigned int *rvPositionAdd);
    static void getFunctionOrDataTypeBegin(const char *strHeader, unsigned int iStartIndex, EClassParserMemberType eFunctionType, ClassParserInformation *pClassInformation, unsigned int *rvPositionAdd);
    static void getFunctionWithMacroBegin(const char *strHeader, unsigned int iStartIndex, EClassParserMemberType eFunctionType, ClassParserInformation *pClassInformation, unsigned int *rvPositionAdd, const SClassParserMacroBeginForMember &sMacroBeginForMember);
    static void getDataTypeBetween(const char *strHeader, unsigned int iStartIndex, unsigned int iEndIndex, EClassParserMemberType eFunctionType, ClassParserInformation *pClassInformation, QString strPreDataType = "");
    static void getFunctionBetween(const char *strHeader, unsigned int iStartIndex, unsigned int iEndIndex, EClassParserMemberType eFunctionType, ClassParserInformation *pClassInformation, unsigned int *rvPositionAdd, QString strAdditionalInlineString);
    static bool isFunctionCTypeCallback(const char *strHeader, unsigned int iEndIndex);
    static void parseTextFromIn(const char *strIn, unsigned int iMax, char *strOut);
    static unsigned int parseToNextTextPosition(const char *strIn);
    static void getFunctionParamsFrom(const char *strIn, SClassParserFunction *pFunction);

    static void removeAllSpaceFromTheEnd( char *strInOut);
    static unsigned int parsePossibleFunctionFromThisLine(const char *strIn);

    /**
     * @brief getIncludeHeader, parse include file name from #include
     * @param strHeader, header text
     * @param rvPosition, position of the end of include
     * @param rvInPublicIncludeFolders, position of the end of include
     * @return include header
     */
    static QString getIncludeHeader(const char *strHeader, unsigned int *rvPosition, bool *rvInPublicIncludeFolders);

private:
    static void parseClassChildClass(const char *strIn, unsigned int iEndIndex, ClassParserInformation *pClassInformation);
    /**
     * @brief parseClassChildClassName, parse child class names and their member types
     * @param strIn, child class start here, can be public piip, or just piip, single string
     * @param iEndIndex, end index of the string
     * @param pClassInformation, class information to save
     */
    static void parseClassChildClassName(const char *strIn, unsigned int iEndIndex, ClassParserInformation *pClassInformation);
};

#endif // CLASSPARSERFIND_H
