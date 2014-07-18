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

#ifndef IRFUNCTIONCLEANSER_H
#define IRFUNCTIONCLEANSER_H

#include <QVector>

#include <private/qv4jsir_p.h>

class QmcTypeCompiler;

class IRFunctionCleanser : public QV4::IR::StmtVisitor, public QV4::IR::ExprVisitor
{
public:
    IRFunctionCleanser(QmcTypeCompiler *typeCompiler, const QVector<int> &functionsToRemove);

    void clean();

private:
    virtual void visitClosure(QV4::IR::Closure *closure);

    virtual void visitTemp(QV4::IR::Temp *) {}

    virtual void visitMove(QV4::IR::Move *s) {
        s->source->accept(this);
        s->target->accept(this);
    }

    virtual void visitConvert(QV4::IR::Convert *e) { e->expr->accept(this); }
    virtual void visitPhi(QV4::IR::Phi *) { }

    virtual void visitExp(QV4::IR::Exp *s) { s->expr->accept(this); }

    virtual void visitJump(QV4::IR::Jump *) {}
    virtual void visitCJump(QV4::IR::CJump *s) { s->cond->accept(this); }
    virtual void visitRet(QV4::IR::Ret *s) { s->expr->accept(this); }

    virtual void visitConst(QV4::IR::Const *) {}
    virtual void visitString(QV4::IR::String *) {}
    virtual void visitRegExp(QV4::IR::RegExp *) {}
    virtual void visitName(QV4::IR::Name *) {}
    virtual void visitUnop(QV4::IR::Unop *e) { e->expr->accept(this); }
    virtual void visitBinop(QV4::IR::Binop *e) { e->left->accept(this); e->right->accept(this); }
    virtual void visitCall(QV4::IR::Call *e) {
        e->base->accept(this);
        for (QV4::IR::ExprList *it = e->args; it; it = it->next)
            it->expr->accept(this);
    }

    virtual void visitNew(QV4::IR::New *e) {
        e->base->accept(this);
        for (QV4::IR::ExprList *it = e->args; it; it = it->next)
            it->expr->accept(this);
    }

    virtual void visitSubscript(QV4::IR::Subscript *e) {
        e->base->accept(this);
        e->index->accept(this);
    }

    virtual void visitMember(QV4::IR::Member *e) {
        e->base->accept(this);
    }

private:
    QmcTypeCompiler *compiler;
    QV4::IR::Module *module;
    const QVector<int> &functionsToRemove;

    QVector<int> newFunctionIndices;
};

#endif // IRFUNCTIONCLEANSER_H
