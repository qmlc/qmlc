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

#include <QQmlScriptString>

#include "scriptstringscanner.h"

#include <private/qqmlpropertycache_p.h>
#include <private/qqmlirbuilder_p.h>

#include "qmctypecompiler.h"

ScriptStringScanner::ScriptStringScanner(QmcTypeCompiler *typeCompiler)
    : compiler(typeCompiler),
      qmlObjects(*typeCompiler->qmlObjects()),
      propertyCaches(typeCompiler->propertyCaches())
{

}

void ScriptStringScanner::scan()
{
    const int scriptStringMetaType = qMetaTypeId<QQmlScriptString>();
    for (int i = 0; i < qmlObjects.count(); ++i) {
        QQmlPropertyCache *propertyCache = propertyCaches.at(i);
        if (!propertyCache)
            continue;

        const QmlIR::Object *obj = qmlObjects.at(i);

        QmlIR::PropertyResolver resolver(propertyCache);
        QQmlPropertyData *defaultProperty = obj->indexOfDefaultProperty != -1 ? propertyCache->parent()->defaultProperty() : propertyCache->defaultProperty();

        for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
            if (binding->type != QV4::CompiledData::Binding::Type_Script)
                continue;
            bool notInRevision = false;
            QQmlPropertyData *pd = binding->propertyNameIndex != 0 ? resolver.property(compiler->stringAt(binding->propertyNameIndex), &notInRevision) : defaultProperty;
            if (!pd || pd->propType != scriptStringMetaType)
                continue;

            QmlIR::CompiledFunctionOrExpression *foe = obj->functionsAndExpressions->slowAt(binding->value.compiledScriptIndex);
            if (foe)
                foe->disableAcceleratedLookups = true;

            QString script = compiler->bindingAsString(obj, binding->value.compiledScriptIndex);
            binding->stringIndex = compiler->registerString(script);
        }
    }
}
