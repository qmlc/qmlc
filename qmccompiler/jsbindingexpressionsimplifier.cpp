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

#include <private/qqmlirbuilder_p.h>

#include "jsbindingexpressionsimplifier.h"

#include "qmctypecompiler.h"
#include "irfunctioncleanser.h"

JSBindingExpressionSimplifier::JSBindingExpressionSimplifier(QmcTypeCompiler *typeCompiler)
    : compiler(typeCompiler),
      qmlObjects(*typeCompiler->qmlObjects()),
      jsModule(typeCompiler->jsIRModule())
{

}

void JSBindingExpressionSimplifier::reduceTranslationBindings()
{
    for (int i = 0; i < qmlObjects.count(); ++i)
        reduceTranslationBindings(i);
    if (!irFunctionsToRemove.isEmpty()) {
        IRFunctionCleanser cleanser(compiler, irFunctionsToRemove);
        cleanser.clean();
    }
}

void JSBindingExpressionSimplifier::reduceTranslationBindings(int objectIndex)
{
    const QmlIR::Object *obj = qmlObjects.at(objectIndex);

    for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
        if (binding->type != QV4::CompiledData::Binding::Type_Script)
            continue;

        const int irFunctionIndex = obj->runtimeFunctionIndices->at(binding->value.compiledScriptIndex);
        QV4::IR::Function *irFunction = jsModule->functions.at(irFunctionIndex);
        if (simplifyBinding(irFunction, binding)) {
            irFunctionsToRemove.append(irFunctionIndex);
            jsModule->functions[irFunctionIndex] = 0;
            delete irFunction;
        }
    }
}

void JSBindingExpressionSimplifier::visitMove(QV4::IR::Move *move)
{
    QV4::IR::Temp *target = move->target->asTemp();
    if (!target || target->kind != QV4::IR::Temp::VirtualRegister) {
        discard();
        return;
    }

    if (QV4::IR::Call *call = move->source->asCall()) {
        if (QV4::IR::Name *n = call->base->asName()) {
            if (n->builtin == QV4::IR::Name::builtin_invalid) {
                visitFunctionCall(n->id, call->args, target);
                return;
            }
        }
        discard();
        return;
    }

    if (QV4::IR::Name *n = move->source->asName()) {
        if (n->builtin == QV4::IR::Name::builtin_qml_id_array
            || n->builtin == QV4::IR::Name::builtin_qml_imported_scripts_object
            || n->builtin == QV4::IR::Name::builtin_qml_context_object
            || n->builtin == QV4::IR::Name::builtin_qml_scope_object) {
            // these are free of side-effects
            return;
        }
        discard();
        return;
    }

    if (!move->source->asTemp() && !move->source->asString() && !move->source->asConst()) {
        discard();
        return;
    }

    _temps[target->index] = move->source;
}

void JSBindingExpressionSimplifier::visitFunctionCall(const QString *name, QV4::IR::ExprList *args, QV4::IR::Temp *target)
{
    // more than one function call?
    if (_nameOfFunctionCalled) {
        discard();
        return;
    }

    _nameOfFunctionCalled = name;

    _functionParameters.clear();
    while (args) {
        int slot;
        if (QV4::IR::Temp *param = args->expr->asTemp()) {
            if (param->kind != QV4::IR::Temp::VirtualRegister) {
                discard();
                return;
            }
            slot = param->index;
            _functionParameters.append(slot);
        } else if (QV4::IR::Const *param = args->expr->asConst()) {
            slot = --_synthesizedConsts;
            Q_ASSERT(!_temps.contains(slot));
            _temps[slot] = param;
            _functionParameters.append(slot);
        }
        args = args->next;
    }

    _functionCallReturnValue = target->index;
}

void JSBindingExpressionSimplifier::visitRet(QV4::IR::Ret *ret)
{
    // nothing initialized earlier?
    if (_returnValueOfBindingExpression != -1) {
        discard();
        return;
    }
    QV4::IR::Temp *target = ret->expr->asTemp();
    if (!target || target->kind != QV4::IR::Temp::VirtualRegister) {
        discard();
        return;
    }
    _returnValueOfBindingExpression = target->index;
}

bool JSBindingExpressionSimplifier::simplifyBinding(QV4::IR::Function *function, QmlIR::Binding *binding)
{
    _canSimplify = true;
    _nameOfFunctionCalled = 0;
    _functionParameters.clear();
    _functionCallReturnValue = -1;
    _temps.clear();
    _returnValueOfBindingExpression = -1;
    _synthesizedConsts = 0;

    // It would seem unlikely that function with some many basic blocks (after optimization)
    // consists merely of a qsTr call or a constant value return ;-)
    if (function->basicBlocks.count() > 10)
        return false;

    foreach (QV4::IR::BasicBlock *bb, function->basicBlocks) {
        foreach (QV4::IR::Stmt *s, bb->statements) {
            s->accept(this);
            if (!_canSimplify)
                return false;
        }
        if (!_canSimplify)
            return false;
    }

    if (_returnValueOfBindingExpression == -1)
        return false;

    if (_canSimplify) {
        if (_nameOfFunctionCalled) {
            if (_functionCallReturnValue != _returnValueOfBindingExpression)
                return false;
            return detectTranslationCallAndConvertBinding(binding);
        }
    }

    return false;
}

bool JSBindingExpressionSimplifier::detectTranslationCallAndConvertBinding(QmlIR::Binding *binding)
{
    if (*_nameOfFunctionCalled == QStringLiteral("qsTr")) {
        QString translation;
        QV4::CompiledData::TranslationData translationData;
        translationData.number = -1;
        translationData.commentIndex = 0; // empty string

        QVector<int>::ConstIterator param = _functionParameters.constBegin();
        QVector<int>::ConstIterator end = _functionParameters.constEnd();
        if (param == end)
            return false;

        QV4::IR::String *stringParam = _temps[*param]->asString();
        if (!stringParam)
            return false;

        translation = *stringParam->value;

        ++param;
        if (param != end) {
            stringParam = _temps[*param]->asString();
            if (!stringParam)
                return false;
            translationData.commentIndex = compiler->registerString(*stringParam->value);
            ++param;

            if (param != end) {
                QV4::IR::Const *constParam = _temps[*param]->asConst();
                if (!constParam || constParam->type != QV4::IR::SInt32Type)
                    return false;

                translationData.number = int(constParam->value);
                ++param;
            }
        }

        if (param != end)
            return false;

        binding->type = QV4::CompiledData::Binding::Type_Translation;
        binding->stringIndex = compiler->registerString(translation);
        binding->value.translationData = translationData;
        return true;
    } else if (*_nameOfFunctionCalled == QStringLiteral("qsTrId")) {
        QString id;
        QV4::CompiledData::TranslationData translationData;
        translationData.number = -1;
        translationData.commentIndex = 0; // empty string, but unused

        QVector<int>::ConstIterator param = _functionParameters.constBegin();
        QVector<int>::ConstIterator end = _functionParameters.constEnd();
        if (param == end)
            return false;

        QV4::IR::String *stringParam = _temps[*param]->asString();
        if (!stringParam)
            return false;

        id = *stringParam->value;

        ++param;
        if (param != end) {
            QV4::IR::Const *constParam = _temps[*param]->asConst();
            if (!constParam || constParam->type != QV4::IR::SInt32Type)
                return false;

            translationData.number = int(constParam->value);
            ++param;
        }

        if (param != end)
            return false;

        binding->type = QV4::CompiledData::Binding::Type_TranslationById;
        binding->stringIndex = compiler->registerString(id);
        binding->value.translationData = translationData;
        return true;
    } else if (*_nameOfFunctionCalled == QStringLiteral("QT_TR_NOOP") || *_nameOfFunctionCalled == QStringLiteral("QT_TRID_NOOP")) {
        QVector<int>::ConstIterator param = _functionParameters.constBegin();
        QVector<int>::ConstIterator end = _functionParameters.constEnd();
        if (param == end)
            return false;

        QV4::IR::String *stringParam = _temps[*param]->asString();
        if (!stringParam)
            return false;

        ++param;
        if (param != end)
            return false;

        binding->type = QV4::CompiledData::Binding::Type_String;
        binding->stringIndex = compiler->registerString(*stringParam->value);
        return true;
    } else if (*_nameOfFunctionCalled == QStringLiteral("QT_TRANSLATE_NOOP")) {
        QVector<int>::ConstIterator param = _functionParameters.constBegin();
        QVector<int>::ConstIterator end = _functionParameters.constEnd();
        if (param == end)
            return false;

        ++param;
        if (param == end)
            return false;

        QV4::IR::String *stringParam = _temps[*param]->asString();
        if (!stringParam)
            return false;

        ++param;
        if (param != end)
            return false;

        binding->type = QV4::CompiledData::Binding::Type_String;
        binding->stringIndex = compiler->registerString(*stringParam->value);
        return true;
    }
    return false;
}
