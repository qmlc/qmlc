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

#ifndef JSBINDINGEXPRESSIONSIMPLIFIER_H
#define JSBINDINGEXPRESSIONSIMPLIFIER_H

#include <QHash>
#include <QString>
#include <QVector>
#include <QList>

#include <private/qv4jsir_p.h>

class QmcTypeCompiler;

class JSBindingExpressionSimplifier : public QV4::IR::StmtVisitor
{
public:
    JSBindingExpressionSimplifier(QmcTypeCompiler *typeCompiler);

    void reduceTranslationBindings();

private:
    void reduceTranslationBindings(int objectIndex);

    virtual void visitMove(QV4::IR::Move *move);
    virtual void visitJump(QV4::IR::Jump *) {}
    virtual void visitCJump(QV4::IR::CJump *) { discard(); }
    virtual void visitExp(QV4::IR::Exp *) { discard(); }
    virtual void visitPhi(QV4::IR::Phi *) {}
    virtual void visitRet(QV4::IR::Ret *ret);

    void visitFunctionCall(const QString *name, QV4::IR::ExprList *args, QV4::IR::Temp *target);

    void discard() { _canSimplify = false; }

    bool simplifyBinding(QV4::IR::Function *function, QmlIR::Binding *binding);
    bool detectTranslationCallAndConvertBinding(QmlIR::Binding *binding);

    QmcTypeCompiler *compiler;

    const QList<QmlIR::Object*> &qmlObjects;
    QV4::IR::Module *jsModule;

    bool _canSimplify;
    const QString *_nameOfFunctionCalled;
    QVector<int> _functionParameters;
    int _functionCallReturnValue;

    QHash<int, QV4::IR::Expr*> _temps;
    int _returnValueOfBindingExpression;
    int _synthesizedConsts;

    QVector<int> irFunctionsToRemove;
};

#endif // JSBINDINGEXPRESSIONSIMPLIFIER_H
