/*!
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
 * In addition, as a special exception, copyright holders
 * give you certain additional rights.  These rights are described in
 * the Digia Qt LGPL Exception version 1.1, included in the file
 * LGPL_EXCEPTION.txt in this package.
 *
 * Author: Ismo Karkkainen <ismo.karkkainen@nomovok.com>
 */

#ifndef QMCDEBUGGER_H
#define QMCDEBUGGER_H

#include <private/qv4debugging_p.h>
#include <private/qv4context_p.h>

#include <QObject>
#include <QQmlError>

#include "qmcdebugging.h"
#include "qmcfile.h"


class QMCDEBUGGERSHARED_EXPORT QmcDebugger : public QV4::Debugging::Debugger
{
public:
    QmcDebugger(QV4::ExecutionEngine *engine);
    ~QmcDebugger();

    void gatherSources(int requestSequenceNr);

    ExecutionState currentExecutionState() const;

    void collectArgumentsInContext(Collector *collector, int frameNr = 0, int scopeNr = 0);
    void collectLocalsInContext(Collector *collector, int frameNr = 0, int scopeNr = 0);
    bool collectThisInContext(Collector *collector, int frame = 0);
    void collectThrownValue(Collector *collector);
    void collectReturnedValue(Collector *collector) const;
    QVector<QV4::ExecutionContext::ContextType> getScopeTypes(int frame = 0) const;
    void resume(Speed speed);

    void maybeBreakAtInstruction();

    void enteringFunction();
    void leavingFunction(const QV4::ReturnedValue &retVal);
    void aboutToThrow();

private:
    QString getFileName(QV4::ExecutionContext *ctxt) const;
    int getLineNumber(QV4::ExecutionContext *ctxt) const;
    QmcDebug *debugBreakCheckContext(QV4::ExecutionContext *ctxt) const;
    void prepareContextPauseAndWait(QV4::Debugging::PauseReason reason);
    QV4::ExecutionContext *topmostContextWithCompilationUnit() const;
};

#endif // QMCDEBUGGER_H
