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
    QmlCompilation(const QString &urlString, const QUrl& url);
    virtual ~QmlCompilation();

    QString urlString;
    QString name;
    QUrl url;
    QString code;
    QQmlCompiledData *compiledData;
    bool checkData(int *sizeInBytes = NULL) const;
    int calculateSize() const;

    QV4::CompiledData::QmlUnit *qmlUnit;
    QV4::CompiledData::CompilationUnit *unit;

    QQmlEngine *engine;

    QList<QString> namespaces;

    QList<QmcUnitTypeReference> exportTypeRefs;

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
        QString name;
        QV4::CompiledData::Location location;
        bool composite;
        bool needsCreation;
        int majorVersion;
        int minorVersion;
        QQmlType *type;
        QmlCompilation *component;
    };

    QHash<int, TypeReference> typeReferences;

    struct ScriptReference
    {
        QV4::CompiledData::Location location;
        QString qualifier;
    };

    QList<ScriptReference> scripts;
};

#endif // QMLCOMPILATION_H
