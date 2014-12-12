#ifndef QMLCOMPILATION_H
#define QMLCOMPILATION_H

#include "qmcfile.h"

#include <QUrl>
#include <QString>
#include <QSet>
#include <QList>

#include <private/qqmlimport_p.h>
#include <private/qqmltypeloader_p.h>
#include <private/qqmlirbuilder_p.h>

class QQmlCompiledData;
class QQmlTypeData;
class QQmlEngine;

#include "qmccompiler_global.h"

class QMCCOMPILERSHARED_EXPORT QmlCompilation
{
public:
    QmlCompilation(const QString &urlString, const QUrl& url, QQmlEngine *engine);
    virtual ~QmlCompilation();

    QString urlString;
    QString name;
    QUrl url;
    QUrl loadUrl;
    QString code;
    QQmlCompiledData *compiledData;
    bool checkData(QQmlError &error, int *sizeInBytes = NULL) const;
    int calculateSize() const;

    QV4::CompiledData::QmlUnit *qmlUnit;
    QV4::CompiledData::CompilationUnit *unit;

    QQmlEngine *engine;

    QList<QString> namespaces;

    QList<QmcUnitTypeReference> exportTypeRefs;
    QList<QmcUnitScriptReference> exportScriptRefs;

    QmcFileType type;

    QList<QmcUnitObjectIndexToId> objectIndexToIdRoot;
    QList<QmcUnitObjectIndexToIdComponent> objectIndexToIdComponent;
    QList<QmcUnitAlias> aliases;

    QList<QmcUnitCustomParser> customParsers;
    QVector<int> customParserBindings;
    QList<QmcUnitDeferredBinding> deferredBindings;

    QmlIR::Document* document;
    QQmlImports* importCache;
    QQmlImportDatabase* importDatabase;

    struct TypeReference {
        TypeReference() : type(0), majorVersion(0), minorVersion(0), typeData(0), needsCreation(true) {}

        QString name;
        QV4::CompiledData::Location location;
        bool composite;
        bool needsCreation;
        int majorVersion;
        int minorVersion;
        QQmlType *type;
        QmlCompilation *component;
        QString prefix; // used by CompositeSingleton types
        QQmlTypeData *typeData;
    };

    QHash<int, TypeReference> typeReferences;

    struct ScriptReference
    {
        QV4::CompiledData::Location location;
        QString qualifier;
        QmlCompilation *compilation;
    };

    QList<ScriptReference> scripts;

    QList<QmcUnitScriptReference> scriptrefs;

    QList<QVector<QmcUnitCodeRefLinkCall > > linkData;

    QList<QmcUnitExceptionReturnLabel> exceptionReturnLabels;
    QList<QVector<QmcUnitExceptionPropagationJump> > exceptionPropagationJumps;
    QList<TypeReference> m_compositeSingletons;

#if CPU(ARM_THUMB2)
    QList<QList<QmcUnitLinkRecord> > jumpsToLinkData;
    QList<QmcUnitUnlinkedCodeData> unlinkedCodeData;
#endif

    QHash<int, int> aliasIdToObjectIndex;
    QHash<int, QHash<int, int> > aliasIdToObjectIndexPerComponent;
    bool singleton;
};

#endif // QMLCOMPILATION_H
