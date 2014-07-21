/*!
 * Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 * Contact: http://www.qt-project.org/legal
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

#include <QVector>

#include <private/qqmlirbuilder_p.h>

#include "irfunctioncleanser.h"

#include "qmctypecompiler.h"

IRFunctionCleanser::IRFunctionCleanser(QmcTypeCompiler *typeCompiler, const QVector<int> &functionsToRemove)
    : compiler(typeCompiler),
      module(typeCompiler->jsIRModule()),
      functionsToRemove(functionsToRemove)
{
}

void IRFunctionCleanser::clean()
{
    QVector<QV4::IR::Function*> newFunctions;
    newFunctions.reserve(module->functions.count() - functionsToRemove.count());

    newFunctionIndices.resize(module->functions.count());

    for (int i = 0; i < module->functions.count(); ++i) {
        QV4::IR::Function *f = module->functions.at(i);
        Q_ASSERT(f || functionsToRemove.contains(i));
        if (f) {
            newFunctionIndices[i] = newFunctions.count();
            newFunctions << f;
        }
    }

    module->functions = newFunctions;

    foreach (QV4::IR::Function *function, module->functions) {
        foreach (QV4::IR::BasicBlock *block, function->basicBlocks()) {
            foreach (QV4::IR::Stmt *s, block->statements()) {
                s->accept(this);
            }
        }
    }

    foreach (QmlIR::Object *obj, *compiler->qmlObjects()) {
        if (!obj->runtimeFunctionIndices)
            continue;
        for (int i = 0; i < obj->runtimeFunctionIndices->count; ++i)
            (*obj->runtimeFunctionIndices)[i] = newFunctionIndices[obj->runtimeFunctionIndices->at(i)];
    }
}

void IRFunctionCleanser::visitClosure(QV4::IR::Closure *closure)
{
    closure->value = newFunctionIndices.at(closure->value);
}
