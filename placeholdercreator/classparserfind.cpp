#include <stdio.h>
#include <string.h>
#include <QDebug>
#include "classparser.h"
#include "classparserfind.h"
#include "classparserinformation.h"

ClassParserFind::ClassParserFind()
{
}

bool ClassParserFind::isThisEqualToSpace(char c) {

    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {

        return true;
    }

    return false;
}

EClassParserMemberType ClassParserFind::isStartWithMemberType(const char *strText, unsigned int *rvPosition, bool bAddColon) {

    if (bAddColon) {

        if (strncmp(strText, "public slots:", 13) == 0) {

            *rvPosition = strlen("public slots");
            return FUNCTION_TYPE_PUBLIC_SLOTS;
        }
        else if (strncmp(strText, "private slots:", 14) == 0) {

            *rvPosition = strlen("private slots");
            return FUNCTION_TYPE_PRIVATE_SLOTS;
        }
        else if (strncmp(strText, "protected slots:", strlen("protected slots:")) == 0) {

            *rvPosition = strlen("protected slots");
            return FUNCTION_TYPE_PROTECTED_SLOTS;
        }
        else if (strncmp(strText, "public:", 7) == 0) {

            *rvPosition = 6;
            return FUNCTION_TYPE_PUBLIC;
        }
        else if (strncmp(strText, "signals:", 8) == 0) {

            *rvPosition=strlen("signals");
            return FUNCTION_TYPE_SIGNAL;
        }
        else if (strncmp(strText, "protected:", 10) == 0) {

            *rvPosition=strlen("protected");
            return FUNCTION_TYPE_PROTECTED;
        }
        else if (strncmp(strText, "private:", 8) == 0) {

            *rvPosition=strlen("private");
            return FUNCTION_TYPE_PRIVATE;
        }
    }
    else {

        if (strncmp(strText, "public slots", 12) == 0) {

            *rvPosition = strlen("public slots");
            return FUNCTION_TYPE_PUBLIC_SLOTS;
        }
        else if (strncmp(strText, "private slots", 13) == 0) {

            *rvPosition = strlen("private slots");
            return FUNCTION_TYPE_PRIVATE_SLOTS;
        }
        else if (strncmp(strText, "protected slots", strlen("protected slots:")) == 0) {

            *rvPosition = strlen("protected slots");
            return FUNCTION_TYPE_PROTECTED_SLOTS;
        }
        else if (strncmp(strText, "public", 6) == 0) {

            *rvPosition = 6;
            return FUNCTION_TYPE_PUBLIC;
        }
        else if (strncmp(strText, "signals", 7) == 0) {

            *rvPosition=strlen("signals");
            return FUNCTION_TYPE_SIGNAL;
        }
        else if (strncmp(strText, "protected", 9) == 0) {

            *rvPosition=strlen("protected");
            return FUNCTION_TYPE_PROTECTED;
        }
        else if (strncmp(strText, "private", 7) == 0) {

            *rvPosition=strlen("private");
            return FUNCTION_TYPE_PRIVATE;
        }
    }


    return FUNCTION_TYPE_NONE;
}

EClassParserClassMainType ClassParserFind::isStartWithClassType(const char *strText, unsigned int *rvPosition) {

    if (strncmp(strText, "class ", 6) == 0 ||
        strncmp(strText, "class\t", 6) == 0 ||
        strncmp(strText, "class\n", 6) == 0 ||
        strncmp(strText, "class\r", 6) == 0) {

        *rvPosition = 5;
        return CLASS_MAIN_TYPE_CLASS;
    }
    else if (strncmp(strText, "struct ", 7) == 0 ||
        strncmp(strText, "struct\t", 7) == 0 ||
        strncmp(strText, "struct\n", 7) == 0 ||
        strncmp(strText, "struct\r", 7) == 0) {

        *rvPosition = 6;
        return CLASS_MAIN_TYPE_STRUCT;
    }
    else if (strncmp(strText, "enum ", 5) == 0 ||
        strncmp(strText, "enum\t", 5) == 0 ||
        strncmp(strText, "enum\n", 5) == 0 ||
        strncmp(strText, "enum\r", 5) == 0) {

        *rvPosition = 4;
        return CLASS_MAIN_TYPE_ENUM;
    }
    else if (strncmp(strText, "typedef ", 8) == 0 ||
        strncmp(strText, "typedef\t", 8) == 0 ||
        strncmp(strText, "typedef\n", 8) == 0 ||
        strncmp(strText, "typedef\r", 8) == 0) {

        *rvPosition = 7;
        return CLASS_MAIN_TYPE_TYPEDEF;
    }
    else if (strncmp(strText, "template ", 9) == 0 ||
        strncmp(strText, "template<", 9) == 0 ||
        strncmp(strText, "template\t", 9) == 0 ||
        strncmp(strText, "template\n", 9) == 0 ||
        strncmp(strText, "template\r", 9) == 0) {

        *rvPosition = 8;
        return CLASS_MAIN_TYPE_TEMPLATE;
    }
    else if (strncmp(strText, "template ", 9) == 0 ||
        strncmp(strText, "template{", 9) == 0 ||
        strncmp(strText, "template<", 9) == 0 ||
        strncmp(strText, "template\t", 9) == 0 ||
        strncmp(strText, "template\n", 9) == 0 ||
        strncmp(strText, "template\r", 9) == 0) {

        *rvPosition = 8;
        return CLASS_MAIN_TYPE_TEMPLATE;
    }
    else if (strncmp(strText, "union ",6) == 0 ||
        strncmp(strText, "union{", 6) == 0 ||
        strncmp(strText, "union\t", 6) == 0 ||
        strncmp(strText, "union\n", 6) == 0 ||
        strncmp(strText, "union\r", 6) == 0) {

        *rvPosition = 5;
        return CLASS_MAIN_TYPE_UNION;
    }

    return CLASS_MAIN_TYPE_NONE;
}

bool ClassParserFind::skipThisLine(const char *strText, unsigned int iStartPosition, unsigned int *rvPosition) {

    unsigned int iPosition;
    if (iStartPosition >= 1 && strText[iStartPosition] == '#') {

        if (strText[iStartPosition-1] == '\n' || strText[iStartPosition-1] == '\r') {

            if (findToNextCharCleanButOnlyOneChar(strText+iStartPosition, "\n\r", &iPosition)) {

                *rvPosition = iPosition+iStartPosition-1;
                return true;
            }
        }
    }
    else {

        if (iStartPosition == 0 && strText[iStartPosition] == '#') {

            if (findToNextCharCleanButOnlyOneChar(strText+iStartPosition, "\n\r", &iPosition)) {

                *rvPosition = iPosition+iStartPosition-1;
                return true;
            }
        }
    }

    return false;
}



bool ClassParserFind::findToNextChar(const char *strTextFromFind, const char *cFindThisChar, unsigned int *rvPosition) {

    const char *text = strstr(strTextFromFind, cFindThisChar);
    if (text == NULL) {
        return false;
    }

    *rvPosition = strlen(strTextFromFind) - strlen(text);

    return true;
}

bool ClassParserFind::findToNextCharCleanButOnlyOneChar(const char *strTextFromFind, const char *cFindThisChar, unsigned int *rvPosition) {

    unsigned int i, i2;
    unsigned int iCount = strlen(strTextFromFind);
    unsigned int iFindThisCharCount = strlen(cFindThisChar);
    EParseAction eAction = PARSE_ACTION_NOTHING;
    bool bInTheQuotes = false;

    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (bInTheQuotes == false && strncmp(strTextFromFind+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && strncmp(strTextFromFind+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                }
                else {

                    if (bInTheQuotes == false) {

                        if (strTextFromFind[i] == '\"') {

                            bInTheQuotes = true;
                        }
                    }
                    else {

                        if (i == 0 && strTextFromFind[i] == '\"') {

                            bInTheQuotes = false;
                        }
                        else if (i != 0 && strTextFromFind[i-1] != '\\' && strTextFromFind[i] == '\"') {

                            bInTheQuotes = false;
                        }
                    }

                    if (bInTheQuotes == false) {

                        for (i2=0;i2<iFindThisCharCount;i2++) {

                            if (strTextFromFind[i] == cFindThisChar[i2]) {

                                *rvPosition = i;
                                return true;
                            }
                        }
                    }
                }
                break;
           case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strTextFromFind+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strTextFromFind+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            default: break;
        }
    }

    return false;
}

char ClassParserFind::findToNextNextValidNonSpaceChar(const char *strTextFromFind, unsigned int *rvPosition) {

    unsigned int i;
    unsigned int iCount = strlen(strTextFromFind);
    EParseAction eAction = PARSE_ACTION_NOTHING;
    bool bInTheQuotes = false;

    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (bInTheQuotes == false && strncmp(strTextFromFind+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && strncmp(strTextFromFind+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                }
                else {

                    if (bInTheQuotes == false) {

                        if (strTextFromFind[i] == '\"') {

                            bInTheQuotes = true;
                        }
                    }
                    else {

                        if (i == 0 && strTextFromFind[i] == '\"') {

                            bInTheQuotes = false;
                        }
                        else if (i != 0 && strTextFromFind[i-1] != '\\' && strTextFromFind[i] == '\"') {

                            bInTheQuotes = false;
                        }
                    }

                    if (bInTheQuotes == false) {

                        if (!isThisEqualToSpace(strTextFromFind[i])) {

                            *rvPosition = i;
                            return strTextFromFind[i];
                        }
                    }
                }
                break;
           case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strTextFromFind+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strTextFromFind+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            default: break;
        }
    }

    return '\0';
}


QString ClassParserFind::getStringBetween(const char *strHeader, unsigned int iStartIndex, unsigned int iEndIndex) {

    QString rv = "";

    char *strTmp = new char[iEndIndex-iStartIndex+1];
    unsigned int i;

    for (i=0;i<iEndIndex-iStartIndex;i++) {

        strTmp[i] = strHeader[iStartIndex+i];
    }
    strTmp[i] = '\0';

    rv = strTmp;

    delete[]strTmp;

    return rv;
}

unsigned int ClassParserFind::parseToNextTextPosition(const char *strIn) {

    unsigned int i;
    unsigned int iCount = strlen(strIn);

    for (i=0;i<iCount;i++) {

        if (strIn[i] != ' ' && strIn[i] != '\t' &&
            strIn[i] != '\n' && strIn[i] != '\r' &&
            strIn[i] != ',') {

            return i;
        }
    }

    return 0;
}

void ClassParserFind::removeAllSpaceFromTheEnd( char *strInOut) {

    unsigned int iCount;
    while (1) {

        iCount = strlen(strInOut);
        if (iCount == 0) {

            break;
        }

        if (strInOut[iCount-1] != ' ') {
            break;
        }

        strInOut[iCount-1] = '\0';
    }
}


int ClassParserFind::parseAfterNextSpaceAndThenRemoveLineBreak(const char *strIn, char *strOut) {

    unsigned int i;
    unsigned int iCount = strlen(strIn);
    strOut[0] = '\0';

    int iStepIndex = 0, iOutIndex = 0;
    EParseAction eAction = PARSE_ACTION_NOTHING;
    bool bInTheQuotes = false;

    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (bInTheQuotes == false && strncmp(strIn+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && strncmp(strIn+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && iStepIndex == 0 &&
                         (isThisEqualToSpace(strIn[i]) ||
                         strIn[i] == ',')) {

                    if (iStepIndex == 1) {

                        return i;
                    }
                }
                else if (bInTheQuotes == false && iStepIndex == 1 &&
                         isThisEqualToSpace(strIn[i])) {

                    if (iOutIndex != 0 && strOut[iOutIndex-1] != ' ') {

                        strOut[iOutIndex++] = ' ';
                        strOut[iOutIndex] = '\0';
                    }
                }
                else {
                    if (bInTheQuotes == false) {

                        if (strIn[i] == '\"') {

                            bInTheQuotes = true;
                        }
                    }
                    else {

                        if (i == 0 && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                        else if (i != 0 && strIn[i-1] != '\\' && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                    }

                    if (iStepIndex == 0) {

                        iStepIndex = 1;
                    }

                    strOut[iOutIndex++] = strIn[i];
                    strOut[iOutIndex] = '\0';
                }
                break;

            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strIn+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strIn+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            default: break;
        }

    }

    return iCount;
}

bool ClassParserFind::isCharInTheList(char c, const char *list) {

    unsigned int i, iCount = strlen(list);

    for (i=0;i<iCount;i++) {

        if (list[i] == c) {

            return true;
        }

    }

    return false;
}

unsigned int ClassParserFind::parseToNext(const char *strIn, char *strOut, const char *cAdditionalListEndChar) {

    unsigned int i;
    unsigned int iCount = strlen(strIn);
    strOut[0] = '\0';

    int iStepIndex = 0, iOutIndex = 0;
    EParseAction eAction = PARSE_ACTION_NOTHING;
    bool bInTheQuotes = false;

    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (bInTheQuotes == false && strncmp(strIn+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && strncmp(strIn+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                }
                else if (bInTheQuotes == false &&
                         isCharInTheList(strIn[i], cAdditionalListEndChar) ) {

                    if (iStepIndex == 1) {

                        return i;
                    }
                }
                else {

                    if (bInTheQuotes == false) {

                        if (strIn[i] == '\"') {

                            bInTheQuotes = true;
                        }
                    }
                    else {

                        if (i == 0 && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                        else if (i != 0 && strIn[i-1] != '\\' && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                    }

                    if (bInTheQuotes == false && iStepIndex == 0) {

                        iStepIndex = 1;
                    }

                    strOut[iOutIndex++] = strIn[i];
                    strOut[iOutIndex] = '\0';
                }
                break;

            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strIn+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strIn+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            default: break;
        }

    }

    return iCount;
}

unsigned int ClassParserFind::parseToNextWithBracketIndex0(const char *strIn, char *strOut, const char *cAdditionalListEndChar) {

    unsigned int i;
    unsigned int iCount = strlen(strIn);
    strOut[0] = '\0';

    int iStepIndex = 0, iOutIndex = 0;
    EParseAction eAction = PARSE_ACTION_NOTHING;
    bool bInTheQuotes = false;
    int iBrackedIndex = 0;

    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (bInTheQuotes == false && strncmp(strIn+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && strncmp(strIn+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                }
                else if (bInTheQuotes == false &&
                         isCharInTheList(strIn[i], cAdditionalListEndChar) ) {

                    if (iStepIndex == 1 && iBrackedIndex == 0) {

                        return i;
                    }

                    if (iBrackedIndex != 0) {

                        strOut[iOutIndex++] = strIn[i];
                        strOut[iOutIndex] = '\0';
                    }

                    if (strIn[i] == '(') {
                        iBrackedIndex++;
                    }

                    if (strIn[i] == ')') {
                        iBrackedIndex--;
                    }
                }
                else {

                    if (bInTheQuotes == false) {

                        if (strIn[i] == '(') {
                            iBrackedIndex++;
                        }
                        if (strIn[i] == ')') {
                            iBrackedIndex--;
                        }

                        if (strIn[i] == '\"') {

                            bInTheQuotes = true;
                        }
                    }
                    else {

                        if (i == 0 && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                        else if (i != 0 && strIn[i-1] != '\\' && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                    }

                    if (bInTheQuotes == false && iStepIndex == 0) {

                        iStepIndex = 1;
                    }

                    strOut[iOutIndex++] = strIn[i];
                    strOut[iOutIndex] = '\0';
                }
                break;

            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strIn+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strIn+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            default: break;
        }

    }

    return iCount;
}

unsigned int ClassParserFind::parseToNextSpace(const char *strIn, char *strOut, const char *cAdditionalListEndChar) {

    unsigned int i;
    unsigned int iCount = strlen(strIn);
    strOut[0] = '\0';

    int iStepIndex = 0, iOutIndex = 0;
    EParseAction eAction = PARSE_ACTION_NOTHING;
    bool bInTheQuotes = false;

    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (bInTheQuotes == false && strncmp(strIn+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && strncmp(strIn+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && (isThisEqualToSpace(strIn[i]) ||
                         isCharInTheList(strIn[i], cAdditionalListEndChar)) ) {

                    if (iStepIndex == 1) {

                        return i;
                    }
                }
                else {

                    if (bInTheQuotes == false) {

                        if (strIn[i] == '\"') {

                            bInTheQuotes = true;
                        }
                    }
                    else {

                        if (i == 0 && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                        else if (i != 0 && strIn[i-1] != '\\' && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                    }

                    if (bInTheQuotes == false && iStepIndex == 0) {

                        iStepIndex = 1;
                    }

                    strOut[iOutIndex++] = strIn[i];
                    strOut[iOutIndex] = '\0';
                }
                break;

            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strIn+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strIn+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            default: break;
        }

    }

    return iCount;
}

void ClassParserFind::parseTextFromIn(const char *strIn, unsigned int iMax, char *strOut) {

    unsigned int i;
    unsigned int iCount = strlen(strIn);
    strOut[0] = '\0';
    int iOutIndex = 0;
    EParseAction eAction = PARSE_ACTION_NOTHING;
    bool bInTheQuotes = false;

    for (i=0;i<iCount && i<iMax;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (bInTheQuotes == false && strncmp(strIn+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && strncmp(strIn+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && isThisEqualToSpace(strIn[i])) {

                    if (iOutIndex != 0) {

                        if (strOut[iOutIndex-1] != ' ') {

                            strOut[iOutIndex++] = ' ';
                            strOut[iOutIndex] = '\0';
                        }
                    }
                }
                else {

                    if (bInTheQuotes == false) {

                        if (strIn[i] == '\"') {

                            bInTheQuotes = true;
                        }
                    }
                    else {

                        if (i == 0 && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                        else if (i != 0 && strIn[i-1] != '\\' && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                    }

                    strOut[iOutIndex++] = strIn[i];
                    strOut[iOutIndex] = '\0';
                }
                break;

            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strIn+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strIn+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            default: break;
        }
    }
}

void ClassParserFind::parseClassChildClassName(const char *strIn,
                                           unsigned int iEndIndex,
                                           ClassParserInformation *pClassInformation) {

    if (iEndIndex == 0) {

        return;
    }

    // remove first character that is not requird
    if (isThisEqualToSpace(strIn[0])  || strIn[0] == ',') {
        parseClassChildClassName(strIn+1, iEndIndex-1, pClassInformation);
        return;
    }

    while (1) {

        if (iEndIndex == 0) {

            return;
        }

        if (isThisEqualToSpace(strIn[iEndIndex-1]) || strIn[iEndIndex-1] == ',') {

            iEndIndex--;
        }
        else {
            break;
        }
    }

    unsigned int i, rvPosition;

    for (i=0;i<iEndIndex;i++) {

        if (isThisEqualToSpace(strIn[i])) {

            SClassParserChildClass s;
            s.m_pType = isStartWithMemberType(strIn, &rvPosition, false);
            s.m_strName = strIn+i+1;
            s.m_strName.resize(iEndIndex-i-1);
            pClassInformation->m_listChildClass.append(s);
            return;
        }
    }


    if (iEndIndex > 0) {
        SClassParserChildClass s;

        if (pClassInformation->m_listChildClass.size() > 0) {

            s.m_pType = pClassInformation->m_listChildClass.at(pClassInformation->m_listChildClass.size()-1).m_pType;
            s.m_strName = strIn;
            s.m_strName.resize(iEndIndex);
            pClassInformation->m_listChildClass.append(s);
        }
        else {

            s.m_pType = FUNCTION_TYPE_PRIVATE;
            s.m_strName = strIn;
            s.m_strName.resize(iEndIndex);
            pClassInformation->m_listChildClass.append(s);
        }
    }
}


void ClassParserFind::parseClassChildClass(const char *strIn,
                                           unsigned int iEndIndex,
                                           ClassParserInformation *pClassInformation) {

    unsigned int i, iCount;
    char *strOut = new char[iEndIndex+1];
    parseTextFromIn(strIn, iEndIndex, strOut);
    iCount = strlen(strOut);
    unsigned int iStartIndex = 0;

    for (i=0;i<iCount;i++) {

        if (strOut[i] == ',') {

            parseClassChildClassName(strOut+iStartIndex, i-iStartIndex, pClassInformation);
            iStartIndex = i+1;
        }
    }

    if (iStartIndex != iCount) {

        parseClassChildClassName(strOut+iStartIndex, iCount-iStartIndex, pClassInformation);
    }

    delete[]strOut;
    strOut = NULL;

}

unsigned int ClassParserFind::parseTemplate(const char *strIn, ClassParserInformation *pClassInformation) {

    unsigned int iPositionEnd;
    unsigned int iCurrentPosition;
    unsigned int iCount = strlen(strIn);

    if (findToNextCharCleanButOnlyOneChar(strIn, ";{", &iPositionEnd)) {

        if (strIn[iPositionEnd] == ';') {

            pClassInformation->m_strClassName = strIn;
            pClassInformation->m_strClassName.resize(iPositionEnd);
            return iPositionEnd;
        }
        else {

            int iSquareBracketIndex = 1;
            iCurrentPosition = iPositionEnd+1;
            while (1) {

                if (findToNextCharCleanButOnlyOneChar(strIn+iCurrentPosition, "{}", &iPositionEnd)) {

                    if (strIn[iCurrentPosition+iPositionEnd] == '}') {
                        iSquareBracketIndex--;
                        if (iSquareBracketIndex == 0) {

                            pClassInformation->m_strClassName = strIn;
                            pClassInformation->m_strClassName.resize(iCurrentPosition+iPositionEnd);
                            return iCurrentPosition+iPositionEnd;
                        }
                    }
                    else {
                        iSquareBracketIndex++;
                    }
                    iCurrentPosition += iPositionEnd+1;
                }
                else {
                    break;
                }
            }
        }
    }

    return iCount;
}


unsigned int ClassParserFind::parseDefine(const char *strIn, ClassParser *pClassParser) {

    unsigned int iPositionEnd;
    unsigned int iCount = strlen(strIn);
    unsigned int iCurrentPosition;
    QString strTmp = "";
    SClassParserDefine pDefine;
    QString strDefineIn;
    if (findToNextCharCleanButOnlyOneChar(strIn, "\n\r", &iPositionEnd)) {

        strDefineIn = strIn;
        strDefineIn.resize(iPositionEnd);
        iCurrentPosition = iPositionEnd;

        while (1) {

            // define range goes to next line
            if (strIn[iCurrentPosition-1] == '\\') {

                strDefineIn.resize(strDefineIn.length()-1);
                if (findToNextCharCleanButOnlyOneChar(strIn+iCurrentPosition+1, "\n\r", &iPositionEnd)) {

                    strTmp = strIn+iCurrentPosition+1;
                    strTmp.resize(iPositionEnd);
                    strDefineIn += strTmp;
                    iCurrentPosition += iPositionEnd+1;
                }
                else {

                    strTmp = strIn+iCurrentPosition+1;
                    strDefineIn += strTmp;
                    break;
                }
            }
            else {

                break;
            }
        }

        while (1) {

            if (strDefineIn.length() > 0 &&
                isThisEqualToSpace(strDefineIn.at(0).toLatin1())) {

                strDefineIn = strDefineIn.right(strDefineIn.length()-1);
            }
            else {

                break;
            }
        }

        pDefine.m_strDefineIn = strDefineIn;
        pDefine.m_bMacro = false;
        int i;
        int iBracketsIndex = 0;
        for (i=0;i<strDefineIn.size();i++) {

            if (strDefineIn.at(i).toLatin1() == '(') {
                pDefine.m_bMacro = true;
                iBracketsIndex++;
            }
            else if (iBracketsIndex > 0 && strDefineIn.at(i).toLatin1() == ')') {
                iBracketsIndex--;
            }
            else if (iBracketsIndex == 0 && isThisEqualToSpace(strDefineIn.at(i).toLatin1())) {

                pDefine.m_strDefineIn.resize(i);
                pDefine.m_strDefineOutput = strDefineIn.right(strDefineIn.length()-i);
                break;
            }
        }

        while (1) {

            if (pDefine.m_strDefineOutput.length() > 0 &&
                isThisEqualToSpace(pDefine.m_strDefineOutput.at(0).toLatin1())) {

                pDefine.m_strDefineOutput = pDefine.m_strDefineOutput.right(pDefine.m_strDefineOutput.length()-1);
            }
            else {

                break;
            }
        }

        pClassParser->addDefine(pDefine);
        return iCurrentPosition;
    }

    return iCount;
}

unsigned int ClassParserFind::parsePossibleFunctionFromThisLine(const char *strIn) {

    unsigned int iPositionEnd;
    unsigned int iCurrentPosition;

    if (!isThisEqualToSpace(strIn[0])) {

        if (findToNextCharCleanButOnlyOneChar(strIn, ";{(", &iPositionEnd)) {

            if (strIn[iPositionEnd] == ';') {
                // data type, but ends here
                return iPositionEnd;
            }
            else {

                if (strIn[iPositionEnd] == '(') {

                    iCurrentPosition = iPositionEnd;
                    if (findToNextCharCleanButOnlyOneChar(strIn+iCurrentPosition, ";{", &iPositionEnd)) {

                        if (strIn[iPositionEnd] == ';') {

                            // data type, but ends here
                            return iCurrentPosition+iPositionEnd;
                        }

                        int iSquareBracketIndex = 1;
                        iCurrentPosition += iPositionEnd+1;

                        while (1) {

                            if (findToNextCharCleanButOnlyOneChar(strIn+iCurrentPosition, "{}", &iPositionEnd)) {

                                if (strIn[iCurrentPosition+iPositionEnd] == '}') {
                                    iSquareBracketIndex--;
                                    if (iSquareBracketIndex == 0) {

                                        return iCurrentPosition+iPositionEnd;
                                    }
                                }
                                else {
                                    iSquareBracketIndex++;
                                }
                                iCurrentPosition += iPositionEnd+1;
                            }
                            else {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }



    return 0;
}

unsigned int ClassParserFind::parseClassName(const char *strIn, ClassParserInformation *pClassInformation) {

    unsigned int iCount = strlen(strIn);
    unsigned int iPositionEnd;
    int iTextPositionBegin;
    int iTextPositionBeginPrevious=0;
    char *strOut = NULL;

    if (findToNextCharCleanButOnlyOneChar(strIn, ":;{", &iPositionEnd)) {

        strOut = new char[iCount+1];
        if (strIn[iPositionEnd] == ';') {

            parseToNextSpace(strIn, strOut, ",:;{");

            pClassInformation->m_strClassType += " presentation";
            pClassInformation->m_strClassName = strOut;
        }
        else {

            while (1) {

                iTextPositionBegin = parseToNextSpace(strIn+iTextPositionBeginPrevious, strOut, ",:;{");

                if (iTextPositionBeginPrevious == iTextPositionBegin) {

                    break;
                }
                else if ((unsigned int)(iTextPositionBegin + iTextPositionBeginPrevious) <= iPositionEnd) {

                    if (pClassInformation->m_strClassName.length() == 0) {

                        pClassInformation->m_strClassName = strOut;
                    }
                    else {

                        if (pClassInformation->m_strClassAdditionalType.length() != 0) {
                            pClassInformation->m_strClassAdditionalType += " ";
                        }
                        pClassInformation->m_strClassAdditionalType += pClassInformation->m_strClassName;
                        pClassInformation->m_strClassName = strOut;
                    }
               }
               else {

                    break;
               }

                iTextPositionBeginPrevious += iTextPositionBegin;
            }
        }

        unsigned int rvPositionSquareBracket;
        unsigned int rvPositionHalfDot;

        if (findToNextCharCleanButOnlyOneChar(strIn, "{", &rvPositionSquareBracket) &&
            findToNextCharCleanButOnlyOneChar(strIn, ":", &rvPositionHalfDot) &&
            rvPositionHalfDot < rvPositionSquareBracket) {

            parseClassChildClass(strIn+rvPositionHalfDot+1, rvPositionSquareBracket-rvPositionHalfDot-1, pClassInformation);
        }

        delete []strOut;
        strOut = NULL;
        return iPositionEnd;
    }


    return iCount;
}


bool ClassParserFind::isNextFunctionParamStart(const char *strIn, unsigned int iEndIndex) {

    unsigned int i;
    EParseAction eAction = PARSE_ACTION_NOTHING;
    bool bInTheQuotes = false;
    for (i=0;i<iEndIndex;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (bInTheQuotes == false && strncmp(strIn+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && strncmp(strIn+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && isThisEqualToSpace(strIn[i])) {

                }
                else if (bInTheQuotes == false && strIn[i] == '(') {

                    return true;
                }
                else {
                    if (bInTheQuotes == false) {

                        if (strIn[i] == '\"') {

                            bInTheQuotes = true;
                        }
                    }
                    else {

                        if (i == 0 && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                        else if (i != 0 && strIn[i-1] != '\\' && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                    }

                    if (bInTheQuotes == false) {

                        return false;
                    }
                }
                break;

            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strIn+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strIn+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            default: break;
        }
    }

    return false;
}

void ClassParserFind::getFunctionParamsFrom(const char *strIn,
                                     SClassParserFunction *pFunction) {
    unsigned int i;
    unsigned int iCount = strlen(strIn);
    bool bParsingDefaultValue = false;
    EParseAction eAction = PARSE_ACTION_NOTHING;
    QStringList listString;
    QString strString = "";
    bool bInTheQuotes = false;
    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (bInTheQuotes == false && strncmp(strIn+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && strncmp(strIn+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && (isThisEqualToSpace(strIn[i]))) {

                    if (bParsingDefaultValue == false) {

                        if (strString.length() != 0) {

                            listString.append(strString);
                            strString = "";
                        }

                    }
                    else {

                        if (strString.length() != 0) {

                            strString += QString("%1").arg(strIn[i]);
                        }
                    }
                }
                else if (bInTheQuotes == false && strIn[i] == '=') {

                    bParsingDefaultValue = true;
                    if (strString.length() != 0) {

                        listString.append(strString);
                        strString = "";
                    }
                } else {

                    if (bInTheQuotes == false) {

                        if (strIn[i] == '\"') {

                            bInTheQuotes = true;
                        }
                    }
                    else {

                        if (i == 0 && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                        else if (i != 0 && strIn[i-1] != '\\' && strIn[i] == '\"') {

                            bInTheQuotes = false;
                        }
                    }
                    strString += QString("%1").arg(strIn[i]);
                }
                break;

            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strIn+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strIn+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            default: break;
        }
    }


    // check if there is any * or & alone
    int i2;
    bool bFound;
    while (1) {

        bFound = false;
        for (i2=0;i2<listString.size();i2++) {

            if (listString[i2] == "*" || listString[i2] == "&" ) {

                bFound = true;
                listString[i2-1] += listString[i2];
                listString.removeAt(i2);
                break;
            }
        }

        if (bFound == false) {
            break;
        }
    }

    if (bParsingDefaultValue) {

        pFunction->m_listFunctionParamDefaultValue.append(strString);
        strString = "";
    }
    else {
        pFunction->m_listFunctionParamDefaultValue.append("");

        if (strString.length() != 0) {

            listString.append(strString);
        }
    }

    // fix: QList<SClass>list
    if (listString.size() == 1) {

        if (listString[0].indexOf("<") != -1 && listString[0].indexOf(">") != -1 &&
            listString[0].indexOf("<") < listString[0].indexOf(">") ) {

            QString strTmp = listString[0].right( listString[0].length() - listString[0].indexOf(">") - 1);
            listString[0].resize( listString[0].indexOf(">") + 1 );
            listString.append(strTmp);
        }
    }

    // fix possible: const QList<SClass>list -> const QList<SClass> list
    if (listString.size() == 2) {

        if (listString[0].indexOf("<") == -1 && listString[0].indexOf(">") == -1 &&
            listString[1].indexOf("<") != -1 && listString[1].indexOf(">") != -1 &&
            listString[1].indexOf("<") < listString[1].indexOf(">") ) {

            QString strTmp = listString[1].right( listString[1].length() - listString[1].indexOf(">") - 1);
            listString[1].resize( listString[1].indexOf(">") + 1 );
            listString.append(strTmp);
        }
    }

    if (listString.size() == 1) {

        pFunction->m_listFunctionParamType.append( listString[0] );
        pFunction->m_listFunctionParamName.append("");
    }
    else if (listString.size() == 2) {

        pFunction->m_listFunctionParamType.append(listString[0]);
        pFunction->m_listFunctionParamName.append(listString[1]);
    }
    else if (listString.size() == 3) {

        pFunction->m_listFunctionParamType.append(listString[0] + " " + listString[1]);
        pFunction->m_listFunctionParamName.append(listString[2]);
    }
    else if (listString.size() > 0){

        strString = "";
        for (i2=0;i2<listString.size() - 2;i2++) {
            if (!strString.isEmpty()) {
                strString += " ";
            }
            strString += listString[i2];
        }
        pFunction->m_listFunctionParamType.append( strString + " " + listString[  listString.size() - 2  ] );
        pFunction->m_listFunctionParamName.append( listString[ listString.size() - 1 ] );
    }

    while (1) {

        // if it's char *piip -> change to char* piip (can be also **)
        // if it's char &piip -> change to char& piip
        if (pFunction->m_listFunctionParamName.at(pFunction->m_listFunctionParamName.size()-1).length() > 0 &&
            (pFunction->m_listFunctionParamName.at(pFunction->m_listFunctionParamName.size()-1).at(0) == '*' ||
            pFunction->m_listFunctionParamName.at(pFunction->m_listFunctionParamName.size()-1).at(0) == '&')) {

            strString = QString("%1%2").arg(pFunction->m_listFunctionParamType[ pFunction->m_listFunctionParamName.size()-1 ]).arg(pFunction->m_listFunctionParamName.at(pFunction->m_listFunctionParamName.size()-1).at(0));
            pFunction->m_listFunctionParamType[ pFunction->m_listFunctionParamType.size()-1 ] = strString;

            strString = pFunction->m_listFunctionParamName[ pFunction->m_listFunctionParamName.size()-1 ];
            strString = strString.right(strString.length()-1);
            pFunction->m_listFunctionParamName[ pFunction->m_listFunctionParamName.size()-1 ] = strString;
        }
        else {
            break;
        }
    }
}

bool ClassParserFind::isFunctionCTypeCallback(const char *strHeader,
                                     unsigned int iEndIndex) {

    unsigned int rvPosition;
    if (findToNextCharCleanButOnlyOneChar(strHeader, "{", &rvPosition) && rvPosition < iEndIndex ) {

        return false;

    }
    else {
        // check if the function is actually c-language type callback
        char c = findToNextNextValidNonSpaceChar(strHeader+iEndIndex, &rvPosition);

        if (c == ';') {

            unsigned int i;
            unsigned int iCount = strlen(strHeader);
            EParseAction eAction = PARSE_ACTION_NOTHING;
            bool bInTheQuotes = false;
            int iBracketsIndex = 0;
            int iBracketsInOutCount = 0;

            if (iCount > iEndIndex) {
                iCount = iEndIndex;
            }

            for (i=0;i<iCount;i++) {

                switch (eAction) {

                    case PARSE_ACTION_NOTHING:
                        if (bInTheQuotes == false && strncmp(strHeader+i, "/*", 2) == 0) {
                            eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                            i++;
                        }
                        else if (bInTheQuotes == false && strncmp(strHeader+i, "//", 2) == 0) {
                            eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                            i++;
                        }
                        else if (bInTheQuotes == false && strHeader[i] == '(') {

                            iBracketsIndex++;
                        }
                        else if (bInTheQuotes == false && iBracketsIndex != 0 && strHeader[i] == ')') {

                            iBracketsIndex--;
                            if (iBracketsIndex == 0) {

                                iBracketsInOutCount++;
                            }
                        }
                        else {

                            if (bInTheQuotes == false) {

                                if (strHeader[i] == '\"') {

                                    bInTheQuotes = true;
                                }
                            }
                            else {

                                if (i == 0 && strHeader[i] == '\"') {

                                    bInTheQuotes = false;
                                }
                                else if (i != 0 && strHeader[i-1] != '\\' && strHeader[i] == '\"') {

                                    bInTheQuotes = false;
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
                    default: break;
                }
            }

            // it's c-type function callback
            if (iBracketsInOutCount == 2) {

                return true;
            }
        }
    }

    return false;
}

void ClassParserFind::getDataTypeBetween(const char *strHeader, unsigned int iStartIndex,
                                         unsigned int iEndIndex, EClassParserMemberType eFunctionType,
                                         ClassParserInformation *pClassInformation, QString strPreDataType) {

    SClassParserDataType s;
    unsigned int i;
    int iStepIndex = 0;
    char *strOut = new char[iEndIndex-iStartIndex+1];
    unsigned int iCount;
    s.m_pType = eFunctionType;

    ClassParserFind::parseTextFromIn(strHeader+iStartIndex, iEndIndex-iStartIndex, strOut);

    iCount = strlen(strOut);

    for (i=0;i<iCount;i++) {

        if (ClassParserFind::isThisEqualToSpace(strOut[i])) {

            iStepIndex++;
        }
        else {

            if (iStepIndex == 0) {

                s.m_strDataType += QString("%1").arg(strOut[i]);
            }
            else if (iStepIndex == 1 || iStepIndex == 2 || iStepIndex == 3) {

                if (iStepIndex == 2) {

                    s.m_strDataType += " " + s.m_strName;
                    s.m_strName = "";
                    iStepIndex = 3;
                }

                s.m_strName += QString("%1").arg(strOut[i]);
            }
        }
    }

    if (s.m_strName.length() == 0 && iStepIndex == 0 && s.m_strDataType.length() != 0) {

        if (s.m_strDataType.indexOf("<") != -1 && s.m_strDataType.indexOf(">") != -1 &&
            s.m_strDataType.indexOf("<") < s.m_strDataType.indexOf(">") ) {

            s.m_strName = s.m_strDataType.right( s.m_strDataType.length() - s.m_strDataType.indexOf(">") - 1);
            s.m_strDataType.resize( s.m_strDataType.indexOf(">") + 1 );
        }
    }

    if (s.m_strName.indexOf("<") != -1 && s.m_strName.indexOf(">") != -1 &&
        s.m_strName.indexOf("<") < s.m_strName.indexOf(">") ) {

        QString str = s.m_strName;
        s.m_strName = s.m_strName.right( s.m_strName.length() - s.m_strName.indexOf(">") - 1);
        str.resize( str.indexOf(">") + 1 );
        s.m_strDataType += " " + str;
    }



    if (s.m_strName.length() == 0) {
        SClassParserPossibleMacroWithoutKnownledge s1;
        s1.m_pType = eFunctionType;
        s1.m_strMacroLine = s.m_strDataType;
        pClassInformation->m_listPossibleMacroWithoutKnownledge.append( s1 );
    }
    else {

        s.m_strPreDataType = strPreDataType;
        pClassInformation->m_listDataType.append(s);
    }

    delete []strOut;
    strOut = NULL;
}


EMEMBER_TYPE ClassParserFind::getFunctionOrDatatypeEndPosition(const char *strIn, unsigned int *rvPositionAdd) {


    unsigned int i;
    unsigned int iCount = strlen(strIn);
    EParseAction eAction = PARSE_ACTION_NOTHING;
    bool bBracketsFound = false;
    int iBracketsIndex = 0;
    int iSquareBracketsIndex = 0;
    bool bInTheQuotes = false;

    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (bInTheQuotes == false && strncmp(strIn+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;
                }
                else if (bInTheQuotes == false && strncmp(strIn+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                }
                else if (bInTheQuotes == false) {

                    if (strIn[i] == '\"') {

                        bInTheQuotes = true;
                    }
                    else if (strIn[i] == '(') {

                        iBracketsIndex++;
                        bBracketsFound = true;
                    }
                    else if (strIn[i] == ')') {

                        iBracketsIndex--;
                        if (iBracketsIndex <= -1) { // this should not be possible, fail parsing

                            return MEMBER_TYPE_NONE;
                        }
                    }
                    else if (strIn[i] == '{') {

                        iSquareBracketsIndex++;
                    }
                    else if (strIn[i] == '}') {

                        iSquareBracketsIndex--;
                        if (iSquareBracketsIndex <= -1) { // this should not be possible, fail parsing

                            return MEMBER_TYPE_NONE;
                        }

                        if (bBracketsFound == true &&
                            iSquareBracketsIndex == 0 &&
                            iBracketsIndex == 0) {

                            *rvPositionAdd = i+1;
                            return MEMBER_TYPE_FUNCTION;
                        }
                    }
                    else if (strIn[i] == ';') {

                        if (bBracketsFound == true &&
                            iSquareBracketsIndex == 0 &&
                            iBracketsIndex == 0) {

                            *rvPositionAdd = i;
                            return MEMBER_TYPE_FUNCTION;
                        }
                        else if (bBracketsFound == false &&
                                 iSquareBracketsIndex == 0 &&
                                 iBracketsIndex == 0) {

                            *rvPositionAdd = i;
                            return MEMBER_TYPE_DATATYPE;
                        }
                    }
                }
                else {

                    if (i == 0 && strIn[i] == '\"') {

                        bInTheQuotes = false;
                    }
                    else if (i != 0 && strIn[i-1] != '\\' && strIn[i] == '\"') {

                        bInTheQuotes = false;
                    }
                }
                break;

            case PARSE_ACTION_COMMENTS_OUT_MULTI_LINE:
                if (strncmp(strIn+i, "*/", 2) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            case PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE:
                if (strncmp(strIn+i, "\n", 1) == 0) {
                    eAction = PARSE_ACTION_NOTHING;
                    i++;
                }
                break;
            default: break;
        }
    }

    return MEMBER_TYPE_NONE;
}

void ClassParserFind::getFunctionWithMacroBegin(const char *strHeader,
                                           unsigned int iStartIndex,
                                           EClassParserMemberType eFunctionType,
                                           ClassParserInformation *pClassInformation,
                                           unsigned int *rvPositionAdd, const SClassParserMacroBeginForMember &sMacroBeginForMember) {

    unsigned int iRvPosition;
    unsigned int iRvPositionAdd;
    unsigned int iRvPositionEnd;
    EMEMBER_TYPE type;

    *rvPositionAdd = 0;

    if (sMacroBeginForMember.m_bRequiresBrackets) {

        if (findToNextCharCleanButOnlyOneChar(strHeader+iStartIndex, ")", &iRvPosition)) {

            QString strQRevision = QString("%1").arg(strHeader+iStartIndex);
            strQRevision.resize(iRvPosition+1);
            type = getFunctionOrDatatypeEndPosition(strHeader+iStartIndex+iRvPosition+1, &iRvPositionEnd);

            if (type == MEMBER_TYPE_FUNCTION) {

                getFunctionBetween(strHeader,
                                   iStartIndex+iRvPosition+1,
                                   iStartIndex+iRvPosition+1+iRvPositionEnd,
                                   eFunctionType,
                                   pClassInformation,
                                   &iRvPositionAdd, strQRevision);

                *rvPositionAdd = iRvPosition + iRvPositionEnd + iRvPositionAdd;
            }
            else if (type == MEMBER_TYPE_DATATYPE) {

                getDataTypeBetween(strHeader,
                                   iStartIndex+iRvPosition+1,
                                   iStartIndex+iRvPosition+1+iRvPositionEnd, eFunctionType,
                                   pClassInformation, strQRevision);

                *rvPositionAdd = iRvPosition + iRvPositionEnd;
            }
        }
    }
    else {
        iRvPosition = (unsigned int)sMacroBeginForMember.m_strMacroBegin.length();

        QString strQRevision = QString("%1").arg(strHeader+iStartIndex);
        strQRevision.resize(iRvPosition+1);

        type = getFunctionOrDatatypeEndPosition(strHeader+iStartIndex+iRvPosition+1, &iRvPositionEnd);

        if (type == MEMBER_TYPE_FUNCTION) {

            getFunctionBetween(strHeader,
                               iStartIndex+iRvPosition+1,
                               iStartIndex+iRvPosition+1+iRvPositionEnd,
                               eFunctionType,
                               pClassInformation,
                               &iRvPositionAdd, strQRevision);

            *rvPositionAdd = iRvPosition + iRvPositionEnd + iRvPositionAdd;
        }
        else if (type == MEMBER_TYPE_DATATYPE) {

            getDataTypeBetween(strHeader,
                               iStartIndex+iRvPosition+1,
                               iStartIndex+iRvPosition+1+iRvPositionEnd, eFunctionType,
                               pClassInformation, strQRevision);

            *rvPositionAdd = iRvPosition + iRvPositionEnd;
        }
    }
}

void ClassParserFind::getFunctionOrDataTypeBegin(const char *strHeader,
                                           unsigned int iStartIndex,
                                           EClassParserMemberType eFunctionType,
                                           ClassParserInformation *pClassInformation,
                                           unsigned int *rvPositionAdd) {

    unsigned int iRvPosition;
    unsigned int iRvPositionAdd;
    unsigned int iRvPositionEnd;
    EMEMBER_TYPE type;

    *rvPositionAdd = 0;


    iRvPosition = 0;

    type = getFunctionOrDatatypeEndPosition(strHeader+iStartIndex+iRvPosition+1, &iRvPositionEnd);

    if (type == MEMBER_TYPE_FUNCTION) {

        getFunctionBetween(strHeader,
                           iStartIndex+iRvPosition+1,
                           iStartIndex+iRvPosition+1+iRvPositionEnd,
                           eFunctionType,
                           pClassInformation,
                           &iRvPositionAdd, "");

        *rvPositionAdd = iRvPosition + iRvPositionEnd + iRvPositionAdd;
    }
    else if (type == MEMBER_TYPE_DATATYPE) {

        getDataTypeBetween(strHeader,
                           iStartIndex+iRvPosition+1,
                           iStartIndex+iRvPosition+1+iRvPositionEnd, eFunctionType,
                           pClassInformation, "");

        *rvPositionAdd = iRvPosition + iRvPositionEnd;
    }
}


void ClassParserFind::getFunctionBetween(const char *strHeader,
                                     unsigned int iStartIndex,
                                     unsigned int iEndIndex,
                                     EClassParserMemberType eFunctionType,
                                     ClassParserInformation *pClassInformation,
                                     unsigned int *rvPositionAdd,
                                     QString strAdditionalInlineString) {

    SClassParserFunction s;
    unsigned int iCurrentPosition = iStartIndex;
    unsigned int rvPosition;
    char *strOut = new char[strlen(strHeader) - iStartIndex + 2 ];
    QStringList listPreFunction;
    unsigned int rvPositionParam;
    unsigned int rvPositionParam2;
    s.m_bPureVirtualFunction = false;

    strcpy(strOut, strHeader+iStartIndex);
    strOut[iEndIndex-iStartIndex] = '\0';

    if (isFunctionCTypeCallback(strHeader+iStartIndex, iEndIndex-iStartIndex)) {

        pClassInformation->m_listCTypeFunctionCallback.append(strOut);
        delete[]strOut;
        strOut = NULL;
        return;
    }

    *rvPositionAdd = 0;
    if (strHeader[iEndIndex] == '=') {

        if (findToNextNextValidNonSpaceChar(strHeader+iEndIndex+1, &rvPosition) == '0') {

            rvPositionParam = rvPosition+2;
            if (findToNextNextValidNonSpaceChar(strHeader+iEndIndex+rvPosition+2, &rvPosition) == ';') {

                s.m_bPureVirtualFunction = true;
                *rvPositionAdd = rvPositionParam + rvPosition;
            }
        }
    }

    if (findToNextCharCleanButOnlyOneChar(strHeader+iCurrentPosition, "(", &rvPosition)) {

        while (1) {

            if (isNextFunctionParamStart(strHeader+iCurrentPosition, iEndIndex-iCurrentPosition)) {
                break;
            }
            if (iCurrentPosition > iEndIndex) {
                break;
            }

            iCurrentPosition += parseToNextSpace(strHeader+iCurrentPosition, strOut, "(");


            if (strlen(strOut) == 0) {
                break;
            }

            listPreFunction.append(strOut);
        }
    }

    if (findToNextCharCleanButOnlyOneChar(strHeader+iCurrentPosition, "(", &rvPositionParam)) {

        iCurrentPosition += rvPositionParam + 1;

        if (iCurrentPosition < iEndIndex) {

            while (1) {
                if (strHeader[iCurrentPosition] == ')') {
                    break;
                }

                iCurrentPosition += parseToNextWithBracketIndex0(strHeader+iCurrentPosition, strOut, ",)");

                if (strHeader[iCurrentPosition] == ')') {
                    getFunctionParamsFrom(strOut, &s);
                    break;
                }

                getFunctionParamsFrom(strOut, &s);
            }
        }
    }

    if (findToNextNextValidNonSpaceChar(strHeader+iCurrentPosition+1, &rvPosition) == 'c') {

        if (strncmp(strHeader+iCurrentPosition+1+rvPosition, "const", strlen("const")) == 0) {

            s.m_strFunctionInlineEnd = "const";
        }
    }

    if (findToNextCharCleanButOnlyOneChar(strHeader+iCurrentPosition, "{", &rvPositionParam))  {

        if (iCurrentPosition+rvPositionParam <= iEndIndex) {

            parseTextFromIn(strHeader+iCurrentPosition, rvPositionParam, strOut);

            if (strlen(strOut) != 0) {

                s.m_strFunctionInlineEnd = strOut;
            }
        }
        else if (findToNextCharCleanButOnlyOneChar(strHeader+iCurrentPosition, ";", &rvPositionParam2))  {

            if (iCurrentPosition+rvPositionParam2 <= iEndIndex) {

                parseTextFromIn(strHeader+iCurrentPosition, rvPositionParam2, strOut);

                if (strlen(strOut) != 0) {

                    s.m_strFunctionInlineEnd = strOut;
                }
            }
        }
    }

    if (listPreFunction.size() == 1) {

        s.m_strFunctionName = listPreFunction.at(0);
    }
    else if (listPreFunction.size() == 2) {

        s.m_strFunctionReturn = listPreFunction.at(0);
        s.m_strFunctionName = listPreFunction.at(1);
    }
    else if (listPreFunction.size() > 2) {

        s.m_strFunctionInline = listPreFunction.at(0);
        s.m_strFunctionReturn = listPreFunction.at(1);
        s.m_strFunctionName = listPreFunction.at(2);
    }
    // Needs to be fixed at higher level not to call this function in this case.
    if (s.m_strFunctionReturn == "emit")
        return;

    while (1) {

        if (s.m_strFunctionName.startsWith("*")) {

            s.m_strFunctionReturn += "*";
            s.m_strFunctionName = s.m_strFunctionName.right(s.m_strFunctionName.length()-1);
        }
        else {
            break;
        }
    }


    delete[]strOut;
    strOut = NULL;

    s.m_pType = eFunctionType;

    // to prevent duplicates by accident, perhaps by #ifdef
    bool bFoundAlready=false;
    int i;

    for (i=0;i<pClassInformation->m_listParserFunction.size();i++) {

        if (pClassInformation->m_listParserFunction.at(i).m_strFunctionName == s.m_strFunctionName &&
            pClassInformation->m_listParserFunction.at(i).m_listFunctionParamType == s.m_listFunctionParamType) {

            bFoundAlready = true;
        }
    }

    if (strAdditionalInlineString.isEmpty() == false) {

        if (s.m_strFunctionInline.isEmpty()) {

            s.m_strFunctionInline = strAdditionalInlineString;
        }
        else {

            s.m_strFunctionInline = strAdditionalInlineString + " " + s.m_strFunctionInline;
        }
    }

    if (bFoundAlready == false && s.m_strFunctionName.isEmpty() == false) {

        pClassInformation->m_listParserFunction.append(s);
    }
}

QString ClassParserFind::getIncludeHeader(const char *strHeader, unsigned int *rvPosition, bool *rvInPublicIncludeFolders) {

    unsigned int i;
    unsigned int iCount = strlen(strHeader);
    int iStepStatus = 0;
    unsigned int iStepStartI = 0;
    QString rv;
    EParseAction eAction = PARSE_ACTION_NOTHING;

    for (i=0;i<iCount;i++) {

        switch (eAction) {

            case PARSE_ACTION_NOTHING:
                if (strncmp(strHeader+i, "/*", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_MULTI_LINE;
                    i++;
                }
                else if (strncmp(strHeader+i, "//", 2) == 0) {
                    eAction = PARSE_ACTION_COMMENTS_OUT_SINGLE_LINE;
                    i++;
                }
                else if (strHeader[i] == '\n' || strHeader[i] == '\r') {

                    *rvPosition = i;
                    return "";
                }
                else if (iStepStatus == 0 && strHeader[i] == '<') {
                    iStepStatus = 1;
                    iStepStartI = i+1;
                }
                else if (iStepStatus == 1 && strHeader[i] == '>') {
                    *rvPosition = i;
                    rv = strHeader+iStepStartI;
                    rv.resize(i-iStepStartI);
                    *rvInPublicIncludeFolders = true;
                    return rv;
                }
                else if (iStepStatus == 0 && strHeader[i] == '\"') {
                    iStepStatus = 2;
                    iStepStartI = i+1;
                }
                else if (iStepStatus == 2 && strHeader[i] == '\"') {
                    *rvPosition = i;
                    rv = strHeader+iStepStartI;
                    rv.resize(i-iStepStartI);
                    *rvInPublicIncludeFolders = false;
                    return rv;
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
            default: break;
        }
    }

    *rvPosition = iCount;
    return "";
}

