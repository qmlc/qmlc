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

#include "defaultpropertymerger.h"
#include "qmctypecompiler.h"

DefaultPropertyMerger::DefaultPropertyMerger(QmcTypeCompiler *compiler)
    : compiler(compiler),
      qmlObjects(*compiler->qmlObjects()),
      propertyCaches(compiler->propertyCaches())
{

}

void DefaultPropertyMerger::mergeDefaultProperties()
{
    for (int i = 0; i < qmlObjects.count(); ++i)
        mergeDefaultProperties(i);
}

void DefaultPropertyMerger::mergeDefaultProperties(int objectIndex)
{
    // qqmltypeloader.cpp:
    QQmlPropertyCache *propertyCache = propertyCaches.at(objectIndex);
    if (!propertyCache)
        return;

    QmlIR::Object *object = qmlObjects.at(objectIndex);

    QString defaultProperty = object->indexOfDefaultProperty != -1 ? propertyCache->parent()->defaultPropertyName() : propertyCache->defaultPropertyName();
    QmlIR::Binding *bindingsToReinsert = 0;
    QmlIR::Binding *tail = 0;

    QmlIR::Binding *previousBinding = 0;
    QmlIR::Binding *binding = object->firstBinding();
    while (binding) {
        if (binding->propertyNameIndex == 0 || compiler->stringAt(binding->propertyNameIndex) != defaultProperty) {
            previousBinding = binding;
            binding = binding->next;
            continue;
        }

        QmlIR::Binding *toReinsert = binding;
        binding = object->unlinkBinding(previousBinding, binding);

        if (!tail) {
            bindingsToReinsert = toReinsert;
            tail = toReinsert;
        } else {
            tail->next = toReinsert;
            tail = tail->next;
        }
        tail->next = 0;
    }

    binding = bindingsToReinsert;
    while (binding) {
        QmlIR::Binding *toReinsert = binding;
        binding = binding->next;
        object->insertSorted(toReinsert);
    }
}
