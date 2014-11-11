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
#include <private/qqmljslexer_p.h>
#include <private/qqmljsparser_p.h>

#include "scriptc.h"
#include "qmlcompilation.h"
#include "qmcinstructionselection.h"


ScriptC::ScriptC(QQmlEngine *engine, QObject *parent) :
    Compiler(engine, parent)
{
}

ScriptC::~ScriptC()
{
}

bool ScriptC::compileData()
{
    compilation()->type = QMC_JS;
    // from QQmlScriptBlob::dataReceived
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
    QV4::CompiledData::CompilationUnit *unit = precompile(&irUnit.jsModule, &irUnit.jsGenerator);
    if (unit)
        unit->ref();

    if (!errors.isEmpty()) {
        if (unit)
            unit->deref();
        appendErrors(errors);
        return false;
    }
    if (!unit)
        return false;
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

QV4::CompiledData::CompilationUnit* ScriptC::precompile(QV4::IR::Module *module, QV4::Compiler::JSUnitGenerator *unitGenerator)
{
    QQmlEnginePrivate *enginePrivate = QQmlEnginePrivate::get(compilation()->engine);
    QV4::ExecutionEngine *v4 = QV8Engine::getV4((QJSEngine *)compilation()->engine);

    // qv4script.cpp:346
    using namespace QQmlJS;
    using namespace QQmlJS::AST;

    QQmlJS::Engine ee;
    QQmlJS::Lexer lexer(&ee);
    lexer.setCode(compilation()->code, /*line*/1, /*qml mode*/true);
    QQmlJS::Parser parser(&ee);

    parser.parseProgram();

    QList<QQmlError> errors;

    foreach (const QQmlJS::DiagnosticMessage &m, parser.diagnosticMessages()) {
        if (m.isWarning()) {
            qWarning("%s:%d : %s", qPrintable(compilation()->url.toString()), m.loc.startLine, qPrintable(m.message));
            return NULL;
        }

        QQmlError error;
        error.setUrl(compilation()->url);
        error.setDescription(m.message);
        error.setLine(m.loc.startLine);
        error.setColumn(m.loc.startColumn);
        errors << error;
    }

    if (!errors.isEmpty()) {
        appendErrors(errors);
        return 0;
    }

    Program *program = AST::cast<Program *>(parser.rootNode());
    if (!program) {
        // if parsing was successful, and we have no program, then
        // we're done...:
        return 0;
    }

    QQmlJS::Codegen cg(/*strict mode*/false);
    cg.generateFromProgram(compilation()->url.toString(), compilation()->code, program, module, QQmlJS::Codegen::EvalCode);
    errors = cg.qmlErrors();
    if (!errors.isEmpty()) {
        appendErrors(errors);
        return 0;
    }

    QScopedPointer<QmcInstructionSelection> isel(
                new QmcInstructionSelection(enginePrivate, v4->executableAllocator,
                                        module, unitGenerator));
    isel->setUseFastLookups(false);
    QV4::CompiledData::CompilationUnit *ret =  isel->compile(/*generate unit data*/false);
    compilation()->linkData = isel->linkData();
    compilation()->exceptionReturnLabels = isel->exceptionReturnLabelsData();
    compilation()->exceptionPropagationJumps = isel->exceptionPropagationJumpsData();
#if CPU(ARM_THUMB2)
        compilation()->jumpsToLinkData = isel->linkRecordData();
        compilation()->unlinkedCodeData = isel->unlinkedCodeData();
#endif
    return ret;
}

bool ScriptC::createExportStructures()
{
    return true;
}
