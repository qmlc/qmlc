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

#include <private/qv8engine_p.h>
#include <private/qv4engine_p.h>
#include <private/qqmlirbuilder_p.h>
#include <private/qv4script_p.h>
#include <private/qqmlcompiler_p.h>

#include "jsc.h"
#include "qmlcompilation.h"


JSC::JSC(QObject *parent) :
    Compiler(parent)
{
}

JSC::~JSC()
{
}

bool JSC::compileData()
{
    compilation()->type = QMC_JS;
    // from QQmlScriptBlob::dataReceived
    QV4::ExecutionEngine *v4 = QV8Engine::getV4((QJSEngine *)compilation()->engine);
    QmlIR::Document irUnit(false);
    QQmlJS::DiagnosticMessage metaDataError;
    irUnit.extractScriptMetaData(compilation()->code, &metaDataError);
    if (!metaDataError.message.isEmpty()) {
        QQmlError e;
        e.setUrl(compilation()->url);
        e.setLine(metaDataError.loc.startLine);
        e.setColumn(metaDataError.loc.startColumn);
        e.setDescription(metaDataError.message);
        appendError(e);
        return false;
    }

    QList<QQmlError> errors;
    QV4::CompiledData::CompilationUnit *unit = QV4::Script::precompile(&irUnit.jsModule, &irUnit.jsGenerator, v4, compilation()->url, compilation()->code, &errors);
    if (unit)
        unit->ref();

    if (!errors.isEmpty()) {
        if (unit)
            unit->deref();
        appendErrors(errors);
        return false;
    }
    irUnit.javaScriptCompilationUnit = unit;

    QmlIR::QmlUnitGenerator qmlGenerator;
    QV4::CompiledData::QmlUnit *qmlUnit = qmlGenerator.generate(irUnit);
    Q_ASSERT(!unit->data);
    Q_ASSERT((void*)qmlUnit == (void*)&qmlUnit->header);
    // The js unit owns the data and will free the qml unit.
    unit->data = &qmlUnit->header;
    compilation()->unit = unit;
    compilation()->qmlUnit = qmlUnit;
    compilation()->name = compilation()->urlString;
    return true;
}

bool JSC::createExportStructures()
{
}
