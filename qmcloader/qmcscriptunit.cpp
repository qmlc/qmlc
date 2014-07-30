/*!
 * Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 * Contact: http://www.qt-project.org/legal
 * Copyright (C) 2014 Nomovok Ltd. All rights reserved.
 * Contact: info@nomovok.com
 *
 * This file may be used under the terms of the GNU Lesser
 * General Public License version 2.1 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU Lesser General Public License version 2.1 requirements
 * will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * In addition, as a special exception, Digia and other copyright holders
 * give you certain additional rights.  These rights are described in
 * the Digia Qt LGPL Exception version 1.1, included in the file
 * LGPL_EXCEPTION.txt in this package.
 */

#include "qmcscriptunit.h"
#include "qmcunit.h"
#include "qmcloader.h"

QmcScriptUnit::QmcScriptUnit(QmcUnit *qmcUnit, QQmlTypeLoader *typeLoader)
    : QQmlScriptBlob(qmcUnit->url, typeLoader),
      unit(qmcUnit)
{
}

QmcScriptUnit::~QmcScriptUnit()
{
    foreach (QmcUnit *dependency, dependencies) {
        dependency->blob->release();
    }
    dependencies.clear();

    delete unit;
}

bool QmcScriptUnit::initialize()
{
    if (isComplete())
        return true;
    setStatus(Complete);

    initializeFromCompilationUnit(unit->compilationUnit, false);

    // add imports
    // qqmltypeloader.cpp:2808
    // TBD
    for (quint32 i = 0; i < unit->qmlUnit->nImports; ++i) {
        const QV4::CompiledData::Import *import = unit->qmlUnit->importAt(i);
        if (import->type == QV4::CompiledData::Import::ImportScript) {
            QmcScriptUnit* script = unit->loader->getScript(stringAt(import->uriIndex), unit->loadedUrl);
            if (!script)
                return false;
            dependencies.append(script->unit);
            scriptImported(script, import->location, stringAt(import->qualifierIndex), QString());

        } else if (import->type == QV4::CompiledData::Import::ImportLibrary) {
            if (!addImport(import, &unit->errors))
                return false;
        } else {
            QQmlError error;
            error.setDescription("Unsupported import type");
            error.setColumn(import->location.column);
            error.setLine(import->location.line);
            error.setUrl(unit->url);
            return false;
        }
    }

    foreach (QmcUnit *unit, dependencies) {
        if (unit->type == QMC_JS) {
            if (!((QmcScriptUnit *)unit->blob)->initialize())
                return false;
        } else {
            return false;
        }
    }

    // call done
    done();
    if (!isError())
        return true;
    else
        return false;
}
