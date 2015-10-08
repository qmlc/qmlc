#include <QDebug>
#include "classparserinformation.h"

ClassParserInformation::ClassParserInformation()
{
    m_iParentClassIndex = -1;
}


void ClassParserInformation::printLog() {

    qDebug() << "Type" << m_strClassType;
    qDebug() << "AdditionalType" << m_strClassAdditionalType;
    qDebug() << "Class" << m_strClassName;
    qDebug() << "TypedefString" << m_strTypedefString;
    qDebug() << "m_iParentClassIndex" << m_iParentClassIndex;

    int i;

    for (i=0;i<m_listParserFunction.size();i++) {
        qDebug() << "Function" << m_listParserFunction[i].m_pType <<
                    m_listParserFunction[i].m_strFunctionInline <<
                    m_listParserFunction[i].m_strFunctionReturn <<
                    m_listParserFunction[i].m_strFunctionName <<
                    m_listParserFunction[i].m_listFunctionParamType <<
                    m_listParserFunction[i].m_listFunctionParamName <<
                    m_listParserFunction[i].m_listFunctionParamDefaultValue <<
                    m_listParserFunction[i].m_strFunctionInlineEnd <<
                    m_listParserFunction[i].m_bPureVirtualFunction;
    }

    for (i=0;i<m_listDataType.size();i++) {
        qDebug() << "Data Type" << m_listDataType[i].m_strPreDataType << m_listDataType[i].m_strDataType <<
                    m_listDataType[i].m_strName;
    }

    for (i=0;i<m_listEnum.size();i++) {
        qDebug() << "Enum" << m_listEnum[i].m_strPropertyName <<
                    m_listEnum[i].m_strPropertyValue;
    }
    for (i=0;i<m_listPossibleMacroWithoutKnownledge.size();i++) {
        qDebug() << "MACRO" << m_listPossibleMacroWithoutKnownledge[i].m_strMacroLine;
    }

    qDebug() << "C-Type callback" << m_listCTypeFunctionCallback;

    for (i=0;i<m_listChildClass.size();i++) {
        qDebug() << "ChildClass" << m_listChildClass[i].m_pType << m_listChildClass[i].m_strName;
    }

    for (i=0;i<m_listQProperty.size();i++) {
        const SClassParserQProperty &p(m_listQProperty[i]);
        qDebug() << "Property" << /*p.m_pType <<*/ p.m_strPropertyName << p.m_strPropertyType;
        for (int k = 0; k < p.m_listFunctionParamType.size(); ++k) {
            qDebug() << "  " << p.m_listFunctionParamType[k] << p.m_listFunctionParamName[k];
        }
    }

}

