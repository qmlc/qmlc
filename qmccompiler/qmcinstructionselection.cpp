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

#include "qmcinstructionselection.h"
#include <private/qv4ssa_p.h>

using namespace QV4;
using namespace QV4::IR;
using namespace QV4::JIT;

QmcInstructionSelection::QmcInstructionSelection(QQmlEnginePrivate *qmlEngine, QV4::ExecutableAllocator *execAllocator,
                                                 QV4::IR::Module *module, QV4::Compiler::JSUnitGenerator *jsGenerator)
    : QV4::JIT::InstructionSelection(qmlEngine, execAllocator, module, jsGenerator)
{
}

void QmcInstructionSelection::run(int functionIndex)
{
    IR::Function *function = irModule->functions[functionIndex];
    QVector<Lookup> lookups;
    qSwap(_function, function);

    IR::Optimizer opt(_function);
    opt.run(qmlEngine);

#ifdef REGALLOC_IS_SUPPORTED
    static const bool withRegisterAllocator = qgetenv("QV4_NO_REGALLOC").isEmpty();
    if (opt.isInSSA() && withRegisterAllocator) {
        RegisterAllocator(getIntRegisters(), getFpRegisters()).run(_function, opt);
    } else
#endif // REGALLOC_IS_SUPPORTED
    {
        if (opt.isInSSA())
            // No register allocator available for this platform, or env. var was set, so:
            opt.convertOutOfSSA();
        ConvertTemps().toStackSlots(_function);
    }
    IR::Optimizer::showMeTheCode(_function);
    QSet<IR::Jump *> removableJumps = opt.calculateOptionalJumps();
    qSwap(_removableJumps, removableJumps);

    Assembler* oldAssembler = _as;
    _as = new Assembler(this, _function, executableAllocator, 6); // 6 == max argc for calls to built-ins with an argument array

    _as->enterStandardStackFrame();

#ifdef ARGUMENTS_IN_REGISTERS
    _as->move(_as->registerForArgument(0), Assembler::ContextRegister);
#else
    _as->loadPtr(addressForArgument(0), Assembler::ContextRegister);
#endif

    const int locals = _as->stackLayout().calculateJSStackFrameSize();
    _as->loadPtr(Address(Assembler::ContextRegister, qOffsetOf(ExecutionContext, engine)), Assembler::ScratchRegister);
    _as->loadPtr(Address(Assembler::ScratchRegister, qOffsetOf(ExecutionEngine, jsStackTop)), Assembler::LocalsRegister);
    _as->addPtr(Assembler::TrustedImm32(sizeof(QV4::Value)*locals), Assembler::LocalsRegister);
    _as->storePtr(Assembler::LocalsRegister, Address(Assembler::ScratchRegister, qOffsetOf(ExecutionEngine, jsStackTop)));

    int lastLine = 0;
    for (int i = 0, ei = _function->basicBlocks.size(); i != ei; ++i) {
        IR::BasicBlock *nextBlock = (i < ei - 1) ? _function->basicBlocks[i + 1] : 0;
        _block = _function->basicBlocks[i];
        _as->registerBlock(_block, nextBlock);

        foreach (IR::Stmt *s, _block->statements) {
            if (s->location.isValid()) {
                if (int(s->location.startLine) != lastLine) {
                    Assembler::Address lineAddr(Assembler::ContextRegister, qOffsetOf(QV4::ExecutionContext, lineNumber));
                    _as->store32(Assembler::TrustedImm32(s->location.startLine), lineAddr);
                    lastLine = s->location.startLine;
                }
            }
            s->accept(this);
        }
    }

    if (!_as->exceptionReturnLabel.isSet())
        visitRet(0);

    int dummySize;
    JSC::MacroAssemblerCodeRef codeRef =_as->link(&dummySize);
    compilationUnit->codeRefs[functionIndex] = codeRef;

    qSwap(_function, function);
    delete _as;
    _as = oldAssembler;
    qSwap(_removableJumps, removableJumps);

}
