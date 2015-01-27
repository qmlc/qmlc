/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qmcdebugger.h"
#include "qmcdebugging.h"
#include "qmcdebugdata.h"
#include <private/qv4object_p.h>
#include <private/qv4functionobject_p.h>
#include <private/qv4function_p.h>
#include <private/qv4instr_moth_p.h>
#include <private/qv4runtime_p.h>
#include <iostream>

#include <algorithm>
#include <QRegExp>
#include <QDebug>

using namespace QV4;
using namespace QV4::Debugging;

QV4::Debugging::Debugger *makeQmcDebugger(QV4::ExecutionEngine *engine)
{
    return new QmcDebugger(engine);
}

void QmcDebuggingInit()
{
    QV4::ExecutionEngine::debuggerMaker = &makeQmcDebugger;
}

namespace {
class EvalJob: public Debugger::Job
{
    QV4::ExecutionEngine *engine;
    const QString &script;

public:
    EvalJob(QV4::ExecutionEngine *engine, const QString &script)
        : engine(engine)
        , script(script)
    {}

    ~EvalJob() {}

    void run()
    {
        // TODO
        qDebug() << "Evaluating script:" << script;
        Q_UNUSED(engine);
    }

    bool resultAsBoolean() const
    {
        return true;
    }
};

class GatherSourcesJob: public Debugger::Job
{
    QV4::ExecutionEngine *engine;
    const int seq;

public:
    GatherSourcesJob(QV4::ExecutionEngine *engine, int seq)
        : engine(engine)
        , seq(seq)
    {}

    ~GatherSourcesJob() {}

    void run()
    {
        QStringList sources;

        foreach (QV4::CompiledData::CompilationUnit *unit, engine->compilationUnits) {
            QString fileName = unit->fileName();
            if (!fileName.isEmpty()) {
                // Change name back to source so Qt Creator does not need to.
                QRegExp reqmc(QLatin1String("qmc$"));
                fileName.replace(reqmc, QLatin1String("qml"));
                QRegExp rejsc(QLatin1String("jsc$"));
                fileName.replace(rejsc, QLatin1String("js"));
                sources.append(fileName);
            }
        }

        Debugger *debugger = engine->debugger;
        QMetaObject::invokeMethod(debugger->agent(), "sourcesCollected", Qt::QueuedConnection,
                                  Q_ARG(QV4::Debugging::Debugger*, debugger),
                                  Q_ARG(QStringList, sources),
                                  Q_ARG(int, seq));
    }
};
}

QmcDebugger::QmcDebugger(QV4::ExecutionEngine *engine)
    : Debugger(engine)
{
}

QmcDebugger::~QmcDebugger()
{
}

void QmcDebugger::gatherSources(int requestSequenceNr)
{
    QMutexLocker locker(&m_lock);

    m_gatherSources = new GatherSourcesJob(m_engine, requestSequenceNr);
    if (m_state == Paused) {
        runInEngine_havingLock(m_gatherSources);
        delete m_gatherSources;
        m_gatherSources = 0;
    }
}

QmcDebugger::ExecutionState QmcDebugger::currentExecutionState() const
{
    ExecutionState state;
    state.fileName = getFileName(engine()->currentContext());
    state.lineNumber = engine()->currentContext()->lineNumber;

    return state;
}

static CallContext *findContext(ExecutionContext *ctxt, int frame)
{
    // This by-passes the checkBreakpoint call context since function is NULL.
    while (ctxt) {
        CallContext *cCtxt = ctxt->asCallContext();
        if (cCtxt && cCtxt->function) {
            if (frame < 1)
                return cCtxt;
            --frame;
        }
        ctxt = ctxt->parent;
    }

    return 0;
}

static CallContext *findScope(ExecutionContext *ctxt, int scope)
{
    // ctxt is from findContext, hence is not checkBreakpoint call context.
    for (; scope > 0 && ctxt; --scope)
        ctxt = ctxt->outer;

    return ctxt ? ctxt->asCallContext() : 0;
}

void QmcDebugger::collectArgumentsInContext(Collector *collector, int frameNr, int scopeNr)
{
    if (state() != Paused)
        return;

    class ArgumentCollectJob: public Job
    {
        QV4::ExecutionEngine *engine;
        Collector *collector;
        int frameNr;
        int scopeNr;

    public:
        ArgumentCollectJob(QV4::ExecutionEngine *engine, Collector *collector, int frameNr, int scopeNr)
            : engine(engine)
            , collector(collector)
            , frameNr(frameNr)
            , scopeNr(scopeNr)
        {}

        ~ArgumentCollectJob() {}

        void run()
        {
            if (frameNr < 0)
                return;

            CallContext *ctxt = findScope(findContext(engine->currentContext(), frameNr), scopeNr);
            if (!ctxt)
                return;

            Scope scope(engine);
            ScopedValue v(scope);
            int nFormals = ctxt->formalCount();
            for (unsigned i = 0, ei = nFormals; i != ei; ++i) {
                QString qName;
                if (String *name = ctxt->formals()[nFormals - i - 1])
                    qName = name->toQString();
                v = ctxt->argument(i);
                collector->collect(qName, v);
            }
        }
    };

    ArgumentCollectJob job(m_engine, collector, frameNr, scopeNr);
    runInEngine(&job);
}

/// Same as \c retrieveArgumentsFromContext, but now for locals.
void QmcDebugger::collectLocalsInContext(Collector *collector, int frameNr, int scopeNr)
{
    if (state() != Paused)
        return;

    class LocalCollectJob: public Job
    {
        QV4::ExecutionEngine *engine;
        Collector *collector;
        int frameNr;
        int scopeNr;

    public:
        LocalCollectJob(QV4::ExecutionEngine *engine, Collector *collector, int frameNr, int scopeNr)
            : engine(engine)
            , collector(collector)
            , frameNr(frameNr)
            , scopeNr(scopeNr)
        {}

        void run()
        {
            if (frameNr < 0)
                return;

            CallContext *ctxt = findScope(findContext(engine->currentContext(), frameNr), scopeNr);
            if (!ctxt)
                return;

            Scope scope(engine);
            ScopedValue v(scope);
            for (unsigned i = 0, ei = ctxt->variableCount(); i != ei; ++i) {
                QString qName;
                if (String *name = ctxt->variables()[i])
                    qName = name->toQString();
                v = ctxt->locals[i];
                collector->collect(qName, v);
            }
        }
    };

    LocalCollectJob job(m_engine, collector, frameNr, scopeNr);
    runInEngine(&job);
}

bool QmcDebugger::collectThisInContext(Collector *collector, int frame)
{
    if (state() != Paused)
        return false;

    class ThisCollectJob: public Job
    {
        QV4::ExecutionEngine *engine;
        Collector *collector;
        int frameNr;
        bool *foundThis;

    public:
        ThisCollectJob(QV4::ExecutionEngine *engine, Collector *collector, int frameNr, bool *foundThis)
            : engine(engine)
            , collector(collector)
            , frameNr(frameNr)
            , foundThis(foundThis)
        {}

        void run()
        {
            *foundThis = myRun();
        }

        bool myRun()
        {
            ExecutionContext *ctxt = findContext(engine->currentContext(), frameNr);
            while (ctxt) {
                if (CallContext *cCtxt = ctxt->asCallContext())
                    if (cCtxt->activation)
                        break;
                ctxt = ctxt->outer;
            }

            if (!ctxt)
                return false;

            Scope scope(engine);
            ScopedObject o(scope, ctxt->asCallContext()->activation);
            collector->collect(o);
            return true;
        }
    };

    bool foundThis = false;
    ThisCollectJob job(m_engine, collector, frame, &foundThis);
    runInEngine(&job);
    return foundThis;
}

void QmcDebugger::collectThrownValue(Collector *collector)
{
    if (state() != Paused || !m_engine->hasException)
        return;

    class ThisCollectJob: public Job
    {
        QV4::ExecutionEngine *engine;
        Collector *collector;

    public:
        ThisCollectJob(QV4::ExecutionEngine *engine, Collector *collector)
            : engine(engine)
            , collector(collector)
        {}

        void run()
        {
            Scope scope(engine);
            ScopedValue v(scope, engine->exceptionValue);
            collector->collect(QStringLiteral("exception"), v);
        }
    };

    ThisCollectJob job(m_engine, collector);
    runInEngine(&job);
}

void QmcDebugger::collectReturnedValue(Collector *collector) const
{
    if (state() != Paused)
        return;

    Scope scope(m_engine);
    ScopedObject o(scope, m_returnedValue);
    collector->collect(o);
}

QVector<QV4::ExecutionContext::ContextType> QmcDebugger::getScopeTypes(int frame) const
{
    QVector<ExecutionContext::ContextType> types;

    if (state() != Paused)
        return types;

    CallContext *sctxt = findContext(m_engine->currentContext(), frame);
    if (!sctxt || sctxt->type < ExecutionContext::Type_SimpleCallContext)
        return types;
    CallContext *ctxt = static_cast<CallContext *>(sctxt);

    for (ExecutionContext *it = ctxt; it; it = it->outer)
        types.append(it->type);

    return types;
}

void QmcDebugger::resume(Speed speed)
{
    QMutexLocker locker(&m_lock);
    if (m_state != Paused)
        return;

    if (!m_returnedValue.isUndefined())
        m_returnedValue = Encode::undefined();

    m_currentContext = topmostContextWithCompilationUnit();
    m_stepping = speed;
    m_runningCondition.wakeAll();
}

void QmcDebugger::maybeBreakAtInstruction()
{
    if (m_runningJob) // do not re-enter when we're doing a job for the debugger.
        return;

    if (!pauseAtNextOpportunity())
        return;

    QMutexLocker locker(&m_lock);

    if (m_gatherSources) {
        m_gatherSources->run();
        delete m_gatherSources;
        m_gatherSources = 0;
    }

    switch (m_stepping) {
    case StepOver:
        if (m_currentContext != topmostContextWithCompilationUnit())
            break;
        // fall through
    case StepIn:
        prepareContextPauseAndWait(Step);
        return;
    case StepOut:
    case NotStepping:
        break;
    }

    if (m_pauseRequested) { // Serve debugging requests from the agent
        m_pauseRequested = false;
        prepareContextPauseAndWait(PauseRequest);
    } else if (m_haveBreakPoints) {
        // A little bit risky but the break check should be the only place to
        // call this function.
        quint32 sourceId = engine()->currentContext()->callData->args[1].integerValue();
        QmcDebug* debug = debugData(sourceId);
        if (!debug)
            return;
        int lineNumber = engine()->currentContext()->callData->args[0].integerValue();
        if (reallyHitTheBreakPoint(debug->sourceName, lineNumber))
            prepareContextPauseAndWait(BreakPoint);
    }
}

void QmcDebugger::enteringFunction()
{
    QMutexLocker locker(&m_lock);
    if (m_stepping == StepIn) {
        m_currentContext = m_engine->currentContext();
    }
}

void QmcDebugger::leavingFunction(const QV4::ReturnedValue &retVal)
{
    QMutexLocker locker(&m_lock);
    // Note that here we have not yet stepped out.
    if (m_stepping != NotStepping && m_currentContext == m_engine->currentContext())
    {
        m_currentContext = m_engine->currentContext()->parent;
        m_stepping = StepOver;
        m_returnedValue = retVal;
    }
}

void QmcDebugger::aboutToThrow()
{
    if (!m_breakOnThrow)
        return;

    if (m_runningJob) // do not re-enter when we're doing a job for the debugger.
        return;

    QMutexLocker locker(&m_lock);
    prepareContextPauseAndWait(Throwing);
}


QString QmcDebugger::getFileName(QV4::ExecutionContext *ctxt) const
{
    QmcDebug *info = debugBreakCheckContext(ctxt);
    if (info)
        return info->sourceName;
    return QString();
}

int QmcDebugger::getLineNumber(QV4::ExecutionContext *ctxt) const
{
    QmcDebug *info = debugBreakCheckContext(ctxt);
    if (info)
        return ctxt->callData->args[0].integerValue();
    return -1;
}

QmcDebug *QmcDebugger::debugBreakCheckContext(QV4::ExecutionContext *ctxt) const
{
    // Some sanity checks.
    if (ctxt->callData->argc != 2)
        return NULL;
    if (!ctxt->callData->args[1].isInteger())
        return NULL;
    // If ctxt is not break check context, returns NULL.
    return debugData(ctxt->callData->args[1].integerValue());
}

void QmcDebugger::prepareContextPauseAndWait(PauseReason reason)
{
    // Current context is actually that of checkBreakpoint and we need to set
    // the line number of the parent context so correct info is passed.
    ExecutionContext *ctxt = findContext(engine()->currentContext(), 0);
    ctxt->lineNumber = engine()->currentContext()->callData->args[0].integerValue();
    pauseAndWait(reason);
}

QV4::ExecutionContext *QmcDebugger::topmostContextWithCompilationUnit() const
{
    QV4::ExecutionContext *ctxt = m_engine->currentContext();
    while (ctxt && !ctxt->compilationUnit)
        ctxt = ctxt->parent;
    return ctxt;
}
