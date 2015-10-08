#include <QDebug>
#include <QFile>
#include "classparser.h"
#include "classparserfind.h"
#include "classparserinformation.h"

ClassParser::ClassParser(QObject *parent) :
    QObject(parent)
{
}

bool ClassParser::parseQProperty(QString strQPropertyLine, int iClassIndex) {

    if (strQPropertyLine.startsWith("Q_PROPERTY(")) {

        strQPropertyLine = strQPropertyLine.right(strQPropertyLine.length()-11);

        QByteArray ba = strQPropertyLine.toLatin1();
        const char *strQProperty = ba.data();

        unsigned int iCurrentPosition = 0;
        char *strOut = new char[ strlen(strQProperty) +1 ];

        int iStrOutIndex = 0;
        SClassParserQProperty s;
        bool bBreak = false;
        while (1) {

            iCurrentPosition += ClassParserFind::parseToNextSpace(strQProperty+iCurrentPosition, strOut, ",");

            if (strlen(strOut) == 0) {
                break;
            }

            if (strOut[strlen(strOut)-1] == ')') {

                strOut[strlen(strOut)-1] = '\0';
                bBreak = true;
            }

            if (iStrOutIndex == 0) {
                s.m_strPropertyType = strOut;
                iStrOutIndex = 1;
            }
            else if (iStrOutIndex == 1) {
                s.m_strPropertyName = strOut;
                iStrOutIndex = 2;
            }
            else if (iStrOutIndex == 2) {
                s.m_listFunctionParamType << strOut;
                iStrOutIndex = 3;
            }
            else if (iStrOutIndex == 3) {
                s.m_listFunctionParamName << strOut;
                iStrOutIndex = 2;
            }

            if (bBreak) {
                break;
            }
        }

        delete[]strOut;
        strOut = NULL;
        if (s.m_listFunctionParamName.length() != 0 &&
            s.m_listFunctionParamType.length() != 0) {

            m_listClassInformation[iClassIndex].m_listQProperty.insert(0, s);
            return true;
        }
    }

    return false;
}

void ClassParser::parseEnumLine(const char *strLine, int iClassIndex) {

    char *strTmp = new char[strlen(strLine)+1];
    unsigned int i = ClassParserFind::parseToNextSpace(strLine, strTmp, ",");

    SClassParserEnum s;

    ClassParserFind::removeAllSpaceFromTheEnd(strTmp);

    s.m_strPropertyName = strTmp;
    if (i < strlen(strLine)) {

        ClassParserFind::parseAfterNextSpaceAndThenRemoveLineBreak(strLine+i, strTmp);

        ClassParserFind::removeAllSpaceFromTheEnd(strTmp);
        s.m_strPropertyValue = strTmp;
    }

    if (s.m_strPropertyName.length() != 0) {

        m_listClassInformation[iClassIndex].m_listEnum.append(s);
    }

    delete[]strTmp;
    strTmp = NULL;
}

unsigned int ClassParser::parseEnum(const char *strHeader, int iClassIndex) {

    unsigned int i;
    unsigned int iCount = strlen(strHeader);
    unsigned int iSquareBracketsOfClassIndex = 0;
    unsigned int iPosition;
    EParseAction eAction = PARSE_ACTION_NOTHING;
    EParseActionEnum eActionEnum = PARSE_ACTION_ENUM_NOTHING;
    char *strLine = new char[strlen(strHeader)+1];
    int iLineIndex = 0;

    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (strncmp(strHeader+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;

                    if (ClassParserFind::findToNextChar(strHeader+i+1, "*/", &iPosition)) {

                        i+=iPosition+2;
                        eAction = PARSE_ACTION_NOTHING;
                    }
                }
                else if (strncmp(strHeader+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                    if (ClassParserFind::findToNextChar(strHeader+i+1, "\n", &iPosition)) {

                        i+=iPosition+1;
                        eAction = PARSE_ACTION_NOTHING;
                    }
                }
                else {

                    if (strHeader[i] == '{') {

                        iSquareBracketsOfClassIndex++;
                        eActionEnum = PARSE_ACTION_ENUM_PARSE_THE_LINE;
                        strLine[0] = '\0';
                        iLineIndex = 0;
                    }
                    else if (strHeader[i] == '}') {

                        iSquareBracketsOfClassIndex--;
                        if (iSquareBracketsOfClassIndex == 0) {
                            if (strlen(strLine) > 0) {

                                parseEnumLine(strLine, iClassIndex);
                                strLine[0] = '\0';
                                iLineIndex = 0;
                            }

                            if (ClassParserFind::findToNextChar(strHeader+i, ";", &iPosition)) {

                                i+=iPosition;
                                delete[]strLine;
                                strLine = NULL;
                                return i;
                            }
                        }
                    }
                    else if (eActionEnum == PARSE_ACTION_ENUM_NOTHING) {

                        i += ClassParserFind::parseToNextSpace(strHeader+i, strLine, ",{");
                        m_listClassInformation[iClassIndex].m_strClassName = strLine;
                        strLine[0] = '\0';
                        iLineIndex = 0;
                        eActionEnum = PARSE_ACTION_ENUM_GOT_NAME;
                        if (strHeader[i] == '{') {

                            iSquareBracketsOfClassIndex++;
                            eActionEnum = PARSE_ACTION_ENUM_PARSE_THE_LINE;
                        }
                    }
                    else if (eActionEnum == PARSE_ACTION_ENUM_PARSE_THE_LINE) {

                        if (strHeader[i] == ',') {

                            parseEnumLine(strLine, iClassIndex);
                            strLine[0] = '\0';
                            iLineIndex = 0;
                        }
                        else {
                            strLine[iLineIndex++] = strHeader[i];
                            strLine[iLineIndex] = '\0';
                        }
                    }
                }
            break;

            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strHeader+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strHeader+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
        }
    }

    delete[]strLine;
    strLine = NULL;
    return i;
}

unsigned int ClassParser::parseTypedef(const char *strHeader, int iClassIndex) {

    unsigned int i;
    unsigned int iCount = strlen(strHeader);
    unsigned int iSquareBracketsOfClassIndex = 0;
    unsigned int iPosition;
    EParseAction eAction = PARSE_ACTION_NOTHING;
    EParseActionTypedef eActionEnum = PARSE_ACTION_TYPEDEF_NOTHING;
    char *strLine = new char[strlen(strHeader)+1];
    int iLineIndex = 0;

    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (strncmp(strHeader+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;

                    if (ClassParserFind::findToNextChar(strHeader+i+1, "*/", &iPosition)) {

                        i+=iPosition+2;
                        eAction = PARSE_ACTION_NOTHING;
                    }
                }
                else if (strncmp(strHeader+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                    if (ClassParserFind::findToNextChar(strHeader+i+1, "\n", &iPosition)) {

                        i+=iPosition+1;
                        eAction = PARSE_ACTION_NOTHING;
                    }
                }
                else {

                    if (eActionEnum == PARSE_ACTION_TYPEDEF_NOTHING) {

                        i += ClassParserFind::parseToNextSpace(strHeader+i, strLine, ",{");
                        m_listClassInformation[iClassIndex].m_strClassAdditionalType = strLine;
                        strLine[0] = '\0';
                        iLineIndex = 0;
                        eActionEnum = PARSE_ACTION_TYPEDEF_GOT_ADDITIONAL_TYPE;
                    }
                    else if (eActionEnum == PARSE_ACTION_TYPEDEF_GOT_ADDITIONAL_TYPE) {

                        if (strHeader[i] == '{') {

                            iSquareBracketsOfClassIndex++;

                            strLine[iLineIndex++] = strHeader[i];
                            strLine[iLineIndex] = '\0';
                        }
                        else if (strHeader[i] == '}') {

                            iSquareBracketsOfClassIndex--;

                            strLine[iLineIndex++] = strHeader[i];
                            strLine[iLineIndex] = '\0';
                        }
                        else if (strHeader[i] == ';' && iSquareBracketsOfClassIndex == 0) {

                            m_listClassInformation[iClassIndex].m_strTypedefString = strLine;
                            delete[]strLine;
                            strLine = NULL;
                            return i;
                        }
                        else {
                            strLine[iLineIndex++] = strHeader[i];
                            strLine[iLineIndex] = '\0';
                        }
                    }
                }
            break;

            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strHeader+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strHeader+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
        }
    }

    delete[]strLine;
    strLine = NULL;
    return i;
}

bool ClassParser::isStartWithMacroBeginForFunction(const char *strHeader, int *iMacroIndex) {

    int i;
    QString strTmp;
    for (i=0;i<m_listMacroBeginForMember.size();i++) {

        strTmp = m_listMacroBeginForMember[i].m_strMacroBegin;
        if (m_listMacroBeginForMember[i].m_bRequiresBrackets) {

            strTmp += "(";
        }

        QByteArray ba = strTmp.toLatin1();
        const char *strCmp = ba.data();

        if (strncmp(strHeader, strCmp, strTmp.length()) == 0) {

            *iMacroIndex = i;
            return true;
        }
    }

    return false;
}

unsigned int ClassParser::parseClass(const char *strHeader, int iClassIndex) {

    unsigned int i;
    unsigned int iCount = strlen(strHeader);
    unsigned int iPreviousImportantActionIndex = 0;
    int iSquareBracketsIndex = 0;
    int iSquareBracketsOfClassIndex = 0;
    int iBracketsIndex = 0;
    unsigned int iPosition;
    unsigned int iPositionAdd;
    int iMacroIndex;
    bool bInTheQuotes = false;
    EParseAction eAction = PARSE_ACTION_NOTHING;
    EParseActionSecondary eSecondaryAction = PARSE_ACTION_SECONDARY_IN_CLASS_BEGIN;
    EClassParserMemberType eFunctionType = FUNCTION_TYPE_PRIVATE;
    EClassParserMemberType eMemberType;

    for (i=0;i<iCount;i++) {
        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (bInTheQuotes == false && strncmp(strHeader+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;

                    if (ClassParserFind::findToNextChar(strHeader+i+1, "*/", &iPosition)) {

                        i+=iPosition+2;
                        eAction = PARSE_ACTION_NOTHING;
                    }
                }
                else if (bInTheQuotes == false && strncmp(strHeader+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                    if (ClassParserFind::findToNextChar(strHeader+i+1, "\n", &iPosition)) {

                        i+=iPosition+1;
                        eAction = PARSE_ACTION_NOTHING;
                    }
                }
                else {
                    if (eSecondaryAction != PARSE_ACTION_SECONDARY_NOTHING &&
                        eSecondaryAction != PARSE_ACTION_SECONDARY_IN_CLASS_BEGIN &&
                        eSecondaryAction != PARSE_ACTION_SECONDARY_IN_GOT_CLASS_NAME)
                    {

                        if (strHeader[i] == '{') {

                            iSquareBracketsOfClassIndex++;
                        }
                        else if (strHeader[i] == '}') {

                            iSquareBracketsOfClassIndex--;
                            if (iSquareBracketsOfClassIndex == 0) {
                                eSecondaryAction = PARSE_ACTION_SECONDARY_NOTHING;
                                if (ClassParserFind::findToNextChar(strHeader+i, ";", &iPosition)) {

                                    i+=iPosition;
                                    return i;
                                }
                            }
                        }
                    }

                    if (bInTheQuotes == false) {

                        if (strHeader[i] == '\"') {

                            bInTheQuotes = true;
                            continue;
                        }
                    }
                    else {

                        if (i == 0 && strHeader[i] == '\"') {

                            bInTheQuotes = false;
                            continue;
                        }
                        else if (i != 0 && strHeader[i-1] != '\\' && strHeader[i] == '\"') {

                            bInTheQuotes = false;
                            continue;
                        }
                    }

                    if (bInTheQuotes == false) {

                        switch (eSecondaryAction) {

                            case PARSE_ACTION_SECONDARY_NOTHING:
                                break;

                            case PARSE_ACTION_SECONDARY_IN_CLASS_BEGIN:

                                eSecondaryAction = PARSE_ACTION_SECONDARY_IN_GOT_CLASS_NAME;
                                iPreviousImportantActionIndex = i;

                                i += ClassParserFind::parseClassName(strHeader+i, &m_listClassInformation[iClassIndex]);
                                if (strHeader[i] == ';') {
                                    eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                    return i;
                                }
                                else {
                                    if (strHeader[i] == '{') {

                                        eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                        eFunctionType = FUNCTION_TYPE_PRIVATE;
                                        iSquareBracketsOfClassIndex++;
                                    }
                                }
                                break;

                            case PARSE_ACTION_SECONDARY_IN_GOT_CLASS_NAME:

                                if (strHeader[i] == '{') {

                                    eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                    eFunctionType = FUNCTION_TYPE_PRIVATE;
                                    iSquareBracketsOfClassIndex++;
                                }
                                break;

                            case PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING:
                            case PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FIRST_HAND_INFO:
                            case PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_BRACETS_OUT:

                                if ( !ClassParserFind::isThisEqualToSpace(strHeader[i]) &&
                                     iSquareBracketsIndex == 0 && iBracketsIndex == 0 &&
                                     strHeader[i] != '(' && strHeader[i] != '{' &&
                                     strHeader[i] != ';' &&
                                     strHeader[i] != ')' && strHeader[i] != '}' && strHeader[i] != '=') {

                                    if (eSecondaryAction == PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_BRACETS_OUT) {

                                        if (strncmp(strHeader+i, "const", 5) == 0) {

                                            i+=4;
                                            continue;
                                        }
                                        else if (iSquareBracketsIndex == 0 && iBracketsIndex == 0) {

                                            SClassParserPossibleMacroWithoutKnownledge s;
                                            s.m_pType = eFunctionType;
                                            s.m_strMacroLine = ClassParserFind::getStringBetween(strHeader, iPreviousImportantActionIndex, i) ;
                                            m_listClassInformation[iClassIndex].m_listPossibleMacroWithoutKnownledge.append( s );
                                        }
                                    }

                                    eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                    EClassParserClassMainType eClassType = ClassParserFind::isStartWithClassType(strHeader+i, &iPosition);

                                    if (eClassType == CLASS_MAIN_TYPE_CLASS) {

                                        ClassParserInformation s;
                                        s.m_strClassType = "class";
                                        s.m_iParentClassIndex = iClassIndex;
                                        s.m_strFromHeaderFile = m_listClassInformation[iClassIndex].m_strFromHeaderFile;
                                        m_listClassInformation.append(s);
                                        i += iPosition;
                                        i += parseClass(strHeader+i, m_listClassInformation.size()-1);
                                    }
                                    else if (eClassType == CLASS_MAIN_TYPE_STRUCT) {

                                        ClassParserInformation s;
                                        s.m_strClassType = "struct";
                                        s.m_iParentClassIndex = iClassIndex;
                                        s.m_strFromHeaderFile = m_listClassInformation[iClassIndex].m_strFromHeaderFile;
                                        m_listClassInformation.append(s);
                                        i += iPosition;
                                        i += parseClass(strHeader+i, m_listClassInformation.size()-1);
                                    }
                                    else if (eClassType == CLASS_MAIN_TYPE_ENUM) {

                                        ClassParserInformation s;
                                        s.m_strClassType = "enum";
                                        s.m_iParentClassIndex = iClassIndex;
                                        s.m_strFromHeaderFile = m_listClassInformation[iClassIndex].m_strFromHeaderFile;
                                        m_listClassInformation.append(s);
                                        i += iPosition;
                                        i += parseEnum(strHeader+i, m_listClassInformation.size()-1);
                                    }
                                    else if (eClassType == CLASS_MAIN_TYPE_TYPEDEF) {

                                        ClassParserInformation s;
                                        s.m_strClassType = "typedef";
                                        s.m_iParentClassIndex = iClassIndex;
                                        s.m_strFromHeaderFile = m_listClassInformation[iClassIndex].m_strFromHeaderFile;
                                        m_listClassInformation.append(s);
                                        i += iPosition;
                                        i += parseTypedef(strHeader+i, m_listClassInformation.size()-1);
                                    }
                                    else if (eClassType == CLASS_MAIN_TYPE_TEMPLATE) {

                                        ClassParserInformation s;
                                        s.m_strClassType = "template";
                                        s.m_iParentClassIndex = iClassIndex;
                                        s.m_strFromHeaderFile = m_listClassInformation[iClassIndex].m_strFromHeaderFile;
                                        i += iPosition;
                                        i += ClassParserFind::parseTemplate(strHeader+i, &s);
                                        m_listClassInformation.append(s);
                                    }
                                    else if (eClassType == CLASS_MAIN_TYPE_UNION) {

                                        ClassParserInformation s;
                                        s.m_strClassType = "union";
                                        s.m_iParentClassIndex = iClassIndex;
                                        s.m_strFromHeaderFile = m_listClassInformation[iClassIndex].m_strFromHeaderFile;
                                        i += iPosition;
                                        i += ClassParserFind::parseTemplate(strHeader+i, &s);
                                        m_listClassInformation.append(s);
                                    }
                                    else if (ClassParserFind::skipThisLine(strHeader, i, &iPosition) ) {

                                        i=iPosition;
                                    }
                                    else if (isStartWithMacroBeginForFunction(strHeader+i, &iMacroIndex)) {

                                        ClassParserFind::getFunctionWithMacroBegin(strHeader, i, eFunctionType, &m_listClassInformation[iClassIndex], &iPosition, m_listMacroBeginForMember[iMacroIndex]);
                                        i += iPosition;
                                    }
                                    else {

                                        eMemberType = ClassParserFind::isStartWithMemberType(strHeader+i, &iPosition);
                                        if (eMemberType != FUNCTION_TYPE_NONE) {

                                            i+=iPosition;
                                            eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FIRST_HAND_INFO;
                                            eFunctionType = eMemberType;
                                        }
                                        else {

                                            eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_OR_DATATYPE;
                                            iPreviousImportantActionIndex = i;
                                        }
                                    }
                                }
                                else if (eSecondaryAction == PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_BRACETS_OUT) {

                                    if (strHeader[i] == '{' ) {

                                        iSquareBracketsIndex = 1;
                                        eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_INLINE;
                                    }
                                    else if (strHeader[i] == ';' ) {

                                        // This call creates function from
                                        // emit foo(); in inline function.
                                        // Also Q_OBJECT handled incorrectly.
                                        ClassParserFind::getFunctionBetween(strHeader, iPreviousImportantActionIndex, i, eFunctionType, &m_listClassInformation[iClassIndex], &iPositionAdd, "");
                                        i+=iPositionAdd;
                                        eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                    }
                                    else if (strHeader[i] == '=' && iBracketsIndex == 0 && iSquareBracketsIndex == 0) {

                                        ClassParserFind::getFunctionBetween(strHeader, iPreviousImportantActionIndex, i, eFunctionType, &m_listClassInformation[iClassIndex], &iPositionAdd, "");
                                        i+=iPositionAdd;
                                        eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                    }
                                }
                                break;
                            case PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_OR_DATATYPE:
                                if (strHeader[i] == ';') {

                                    if (iSquareBracketsIndex == 0 && iBracketsIndex == 0) {

                                        ClassParserFind::getDataTypeBetween(strHeader, iPreviousImportantActionIndex, i, eFunctionType, &m_listClassInformation[iClassIndex]);
                                    }
                                    else {


                                       ClassParserFind::getFunctionBetween(strHeader, iPreviousImportantActionIndex, i, eFunctionType, &m_listClassInformation[iClassIndex], &iPositionAdd, "");
                                       i+=iPositionAdd;
                                    }
                                    eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                }
                                else if (strHeader[i] == '(' ) {

                                    iBracketsIndex++;
                                    eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_BRACETS_IN;
                                }
                                break;
                            case PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_BRACETS_IN:
                                if (strHeader[i] == ';' ) {

                                    ClassParserFind::getFunctionBetween(strHeader, iPreviousImportantActionIndex, i, eFunctionType, &m_listClassInformation[iClassIndex], &iPositionAdd, "");
                                    i+=iPositionAdd;
                                    eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                }
                                else if (strHeader[i] == '(' ) {

                                    iBracketsIndex++;
                                }
                                else if (strHeader[i] == ')' ) {

                                    iBracketsIndex--;
                                    if (iBracketsIndex == 0) {

                                        char c = ClassParserFind::findToNextNextValidNonSpaceChar(strHeader+i+1, &iPosition);

                                        if (c == '(') { // function? still continues

                                            i += iPosition;
                                            iBracketsIndex++;
                                        }
                                        else {

                                            eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_BRACETS_OUT;
                                        }
                                    }
                                }
                                else if (strHeader[i] == '=' && iBracketsIndex == 0 && iSquareBracketsIndex == 0) {

                                    ClassParserFind::getFunctionBetween(strHeader, iPreviousImportantActionIndex, i, eFunctionType, &m_listClassInformation[iClassIndex], &iPositionAdd, "");
                                    i+=iPositionAdd;
                                    eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                }
                                break;

                            case PARSE_ACTION_SECONDARY_IN_THE_CLASS_GETTING_FUNCTION_INLINE:

                                if (strHeader[i] == '{' ) {

                                    iSquareBracketsIndex++;
                                }
                                else if (strHeader[i] == '}' ) {

                                    iSquareBracketsIndex--;
                                    if (iSquareBracketsIndex==0) {

                                        ClassParserFind::getFunctionBetween(strHeader, iPreviousImportantActionIndex, i, eFunctionType, &m_listClassInformation[iClassIndex], &iPositionAdd, "");
                                        i+=iPositionAdd;
                                        eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                    }

                                }
                                else if (strHeader[i] == ';' ) {

                                    if (iSquareBracketsIndex==0) {

                                        ClassParserFind::getFunctionBetween(strHeader, iPreviousImportantActionIndex, i, eFunctionType, &m_listClassInformation[iClassIndex], &iPositionAdd, "");
                                        i+=iPositionAdd;
                                        eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                    }
                                }
                                else if (strHeader[i] == '=') {

                                    if (iSquareBracketsIndex == 0) {

                                        ClassParserFind::getFunctionBetween(strHeader, iPreviousImportantActionIndex, i, eFunctionType, &m_listClassInformation[iClassIndex], &iPositionAdd, "");
                                        i+=iPositionAdd;
                                        eSecondaryAction = PARSE_ACTION_SECONDARY_IN_THE_CLASS_NOTHING;
                                    }
                                }
                                break;
                        }
                    }
                }

                break;
            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strHeader+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strHeader+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
        }
    }

    return i;
}

void ClassParser::setListPublicIncludePaths(QStringList listPublicIncludePaths) {

    m_listPublicIncludePaths = listPublicIncludePaths;
}



QString ClassParser::findFullFilePathForHeaderInformation(ClassParserIncludeHeaderInformation *pIncludeHeader, const char *strHeaderFullFilePath) {

    unsigned int i;
    unsigned int iCount = strlen(strHeaderFullFilePath);
    unsigned int iLastSeparator = iCount;
    QString strFilePath;
    QString strNewFuleFilePath;

    for (i=0;i<iCount;i++) {

        if (strHeaderFullFilePath[i] == '/' || strHeaderFullFilePath[i] == '\\') {

            iLastSeparator = i;
        }
    }

    if (iLastSeparator != iCount) {

        strFilePath = strHeaderFullFilePath;
        strFilePath.resize(iLastSeparator+1);

        if (pIncludeHeader->m_bInPublicIncludeFolders) {

            for (int ii=0;ii<m_listPublicIncludePaths.size();ii++) {

                strNewFuleFilePath = m_listPublicIncludePaths[ii] + "/" + pIncludeHeader->m_strIncludeHeader;
                QFile file(strNewFuleFilePath);
                if (file.exists()) {

                    return strNewFuleFilePath;
                }
            }

        }
        else {

            strFilePath += pIncludeHeader->m_strIncludeHeader;

            QFile file(strFilePath);
            if (file.exists()) {

                return strFilePath;
            }
        }
    }

    return "";
}


bool ClassParser::isIncludeHeaderParsed(ClassParserIncludeHeaderInformation *pIncludeHeader) {

    int i;

    for (i=0;i<m_listIncludeHeader.size();i++) {

        if (m_listIncludeHeader[i].m_strIncludeHeaderWithFullFilePath == pIncludeHeader->m_strIncludeHeaderWithFullFilePath) {

            return true;
        }
    }

    return false;
}


bool ClassParser::parseHeader(const char *strHeader, const char *strHeaderFullFilePath)
{
    unsigned int i;
    unsigned int iCount = strlen(strHeader);
    int iSquareBracketsOfClassIndex = 0;
    unsigned int iPosition;
    bool bIsParsed;

    EParseAction eAction = PARSE_ACTION_NOTHING;
    EParseActionSecondary eSecondaryAction = PARSE_ACTION_SECONDARY_NOTHING;

    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (strncmp(strHeader+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;

                    if (ClassParserFind::findToNextChar(strHeader+i+1, "*/", &iPosition)) {

                        i+=iPosition+2;
                        eAction = PARSE_ACTION_NOTHING;
                    }
                }
                else if (strncmp(strHeader+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                    if (ClassParserFind::findToNextChar(strHeader+i+1, "\n", &iPosition)) {

                        i+=iPosition+1;
                        eAction = PARSE_ACTION_NOTHING;
                    }
                }
                else {
                    if (eSecondaryAction != PARSE_ACTION_SECONDARY_NOTHING &&
                        eSecondaryAction != PARSE_ACTION_SECONDARY_IN_CLASS_BEGIN &&
                        eSecondaryAction != PARSE_ACTION_SECONDARY_IN_GOT_CLASS_NAME)
                    {

                        if (strHeader[i] == '{') {

                            iSquareBracketsOfClassIndex++;
                        }
                        else if (strHeader[i] == '}') {

                            iSquareBracketsOfClassIndex--;
                            if (iSquareBracketsOfClassIndex == 0) {
                                eSecondaryAction = PARSE_ACTION_SECONDARY_NOTHING;
                                if (ClassParserFind::findToNextChar(strHeader+i, ";", &iPosition)) {

                                    i+=iPosition;
                                }
                            }
                        }
                    }

                    switch (eSecondaryAction) {

                        case PARSE_ACTION_SECONDARY_NOTHING:
                            if (strncmp(strHeader+i, "class", 5) == 0) {
                                ClassParserInformation s;
                                s.m_strClassType = "class";
                                s.m_strFromHeaderFile = strHeaderFullFilePath;
                                m_listClassInformation.append(s);
                                i += 5;
                                i += parseClass(strHeader+i, m_listClassInformation.size()-1);
                            }
                            else if (strncmp(strHeader+i, "struct", 6) == 0) {
                                ClassParserInformation s;
                                s.m_strClassType = "struct";
                                s.m_strFromHeaderFile = strHeaderFullFilePath;
                                m_listClassInformation.append(s);
                                i += 6;
                                i += parseClass(strHeader+i, m_listClassInformation.size()-1);
                            }
                            else if (strncmp(strHeader+i, "enum", 4) == 0) {
                                ClassParserInformation s;
                                s.m_strClassType = "enum";
                                s.m_strFromHeaderFile = strHeaderFullFilePath;
                                m_listClassInformation.append(s);
                                i += 4;
                                i += parseEnum(strHeader+i, m_listClassInformation.size()-1);
                            }
                            else if (strncmp(strHeader+i, "typedef", 7) == 0) {
                                ClassParserInformation s;
                                s.m_strClassType = "typedef";
                                s.m_strFromHeaderFile = strHeaderFullFilePath;
                                m_listClassInformation.append(s);
                                i += 7;
                                i += parseTypedef(strHeader+i, m_listClassInformation.size()-1);
                            }
                            else if (strncmp(strHeader+i, "template", 8) == 0) {
                                ClassParserInformation s;
                                s.m_strClassType = "template";
                                s.m_strFromHeaderFile = strHeaderFullFilePath;
                                i += 8;
                                i += ClassParserFind::parseTemplate(strHeader+i, &s);
                                m_listClassInformation.append(s);
                            }
                            else if (strncmp(strHeader+i, "union", 5) == 0) {
                                ClassParserInformation s;
                                s.m_strClassType = "union";
                                s.m_strFromHeaderFile = strHeaderFullFilePath;
                                i += 5;
                                i += ClassParserFind::parseTemplate(strHeader+i, &s);
                                m_listClassInformation.append(s);
                            }
                            else if (strncmp(strHeader+i, "#define", 7) == 0) {
                                i += 7;
                                i += ClassParserFind::parseDefine(strHeader+i, this);
                            }
                            else if (strncmp(strHeader+i, "#include", 8) == 0) {

                                bool bInPublicIncludeFolders;
                                QString strInclude = ClassParserFind::getIncludeHeader(strHeader+i+8, &iPosition, &bInPublicIncludeFolders);
                                if (strInclude.length() != 0) {

                                    i+=8+iPosition;
                                    ClassParserIncludeHeaderInformation s;
                                    s.m_bInPublicIncludeFolders = bInPublicIncludeFolders;
                                    s.m_strIncludeHeader = strInclude;
                                    s.m_strIncludeHeaderWithFullFilePath = findFullFilePathForHeaderInformation(&s, strHeaderFullFilePath);
                                    bIsParsed = isIncludeHeaderParsed(&s);
                                    m_listIncludeHeader.append(s);

                                    if (bIsParsed == false && s.m_strIncludeHeaderWithFullFilePath.length() != 0) {

                                        QByteArray ba = s.m_strIncludeHeaderWithFullFilePath.toLatin1();
                                        parse(ba.data());
                                    }
                                }
                            }
                            else if (ClassParserFind::skipThisLine(strHeader, i, &iPosition) ) {

                                i=iPosition;
                            }
                            else  {

                                i+= ClassParserFind::parsePossibleFunctionFromThisLine(strHeader+i);
                            }

                            break;

                        default:
                            break;
                    }
                }

                break;
            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strHeader+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strHeader+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
        }
    }

    return false;
}

void ClassParser::setPreDefines(QStringList in, QStringList out) {
    m_listPreDefineIn = in;
    m_listPreDefineOut = out;
}

void ClassParser::addDefine(SClassParserDefine pDef) {

    m_listDefine.append(pDef);
}


bool ClassParser::parse(const char *strHeaderFullFilePath) {

    FILE *fp = fopen(strHeaderFullFilePath, "r");

    if (fp) {

        char buf[1024];
        QString strFileBuf = "";
        while (1) {

            if (fgets(buf, 1024, fp) != NULL) {

                strFileBuf += buf;
            }
            else {
                break;
            }
        }

        fclose(fp);

        int i;
        for (i=0;i<m_listPreDefineIn.size();i++) {
            int iReplaceAllIndex = 0;
            while (1) {

                iReplaceAllIndex = strFileBuf.indexOf(m_listPreDefineIn[i], iReplaceAllIndex);

                if (iReplaceAllIndex == -1) {
                    break;
                }

                strFileBuf = strFileBuf.replace(iReplaceAllIndex, m_listPreDefineIn[i].length(), m_listPreDefineOut[i]);
                iReplaceAllIndex += m_listPreDefineOut[i].length();
            }
        }

        QByteArray ba = strFileBuf.toLatin1();
        const char *strHeader = ba.data();

        parseHeader(strHeader, strHeaderFullFilePath);

        int iClassIndex;
        for (iClassIndex=0;iClassIndex<m_listClassInformation.size();iClassIndex++) {

            for (i=m_listClassInformation[iClassIndex].m_listPossibleMacroWithoutKnownledge.size()-1;i>=0;i--) {

                if (parseQProperty(m_listClassInformation[iClassIndex].m_listPossibleMacroWithoutKnownledge[i].m_strMacroLine, iClassIndex)) {

                    m_listClassInformation[iClassIndex].m_listPossibleMacroWithoutKnownledge.removeAt(i);
                }
            }
        }

        setChildClassPointer();
        return true;

    }

    return false;
}

void ClassParser::setChildClassPointer() {

    int iChildClassName;
    int iClassIndex;
    int iClassIndex2;
    QString strName;
    bool bFound;
    for (iClassIndex=0;iClassIndex<getClassInformationCount();iClassIndex++) {

        ClassParserInformation *pClassInformation = getClassInformation(iClassIndex);

        for (iChildClassName=0;iChildClassName<pClassInformation->m_listChildClass.size();iChildClassName++) {

            strName = pClassInformation->m_listChildClass[iChildClassName].m_strName;

            bFound = false;
            // check that it's not already there
            for (iClassIndex2=0;iClassIndex2<pClassInformation->m_listToChildClass.size();iClassIndex2++) {

                if (pClassInformation->m_listToChildClass[iClassIndex2]->m_strClassName == strName) {

                    bFound = true;
                    break;
                }
            }

            if (bFound == false) {
                for (iClassIndex2=0;iClassIndex2<getClassInformationCount();iClassIndex2++) {

                    if (iClassIndex != iClassIndex2) {

                        ClassParserInformation *pClassInformation2 = getClassInformation(iClassIndex2);

                        if (pClassInformation2->m_strClassType == "class" &&
                            pClassInformation2->m_strClassName == strName) {

                            pClassInformation->m_listToChildClass.append(pClassInformation2);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void ClassParser::add(SClassParserMacroBeginForMember macroBegin) {

    m_listMacroBeginForMember.append(macroBegin);
}

int ClassParser::getClassInformationCount() {

    return m_listClassInformation.size();
}

ClassParserInformation *ClassParser::getClassInformation(int iIndex) {

    if (iIndex >= m_listClassInformation.size()) {

        return NULL;
    }
    return &m_listClassInformation[iIndex];
}

int ClassParser::getIncludeHeaderInformationCount() {

    return m_listIncludeHeader.size();
}

ClassParserIncludeHeaderInformation *ClassParser::getIncludeHeaderInformation(int iIndex) {

    if (iIndex >= m_listIncludeHeader.size()) {

        return NULL;
    }
    return &m_listIncludeHeader[iIndex];
}

