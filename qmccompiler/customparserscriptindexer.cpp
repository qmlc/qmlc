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

#include "customparserscriptindexer.h"

#include <private/qqmlirbuilder_p.h>
#include <private/qv4compileddata_p.h>

#include "qmctypecompiler.h"

CustomParserScriptIndexer::CustomParserScriptIndexer(QmcTypeCompiler *typeCompiler)
    : compiler(typeCompiler),
      qmlObjects(*typeCompiler->qmlObjects()),
      customParsers(typeCompiler->customParserCache())
{
}

void CustomParserScriptIndexer::annotateBindingsWithScriptStrings()
{
    scanObjectRecursively(compiler->rootObjectIndex());
}

void CustomParserScriptIndexer::scanObjectRecursively(int objectIndex, bool annotateScriptBindings)
{
    const QmlIR::Object * const obj = qmlObjects.at(objectIndex);
    if (!annotateScriptBindings)
        annotateScriptBindings = customParsers.contains(obj->inheritedTypeNameIndex);
    for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
        if (binding->type >= QV4::CompiledData::Binding::Type_Object) {
            scanObjectRecursively(binding->value.objectIndex, annotateScriptBindings);
            continue;
        } else if (binding->type != QV4::CompiledData::Binding::Type_Script)
            continue;
        if (!annotateScriptBindings)
            continue;
        const QString script = compiler->bindingAsString(obj, binding->value.compiledScriptIndex);
        binding->stringIndex = compiler->registerString(script);
    }
}
