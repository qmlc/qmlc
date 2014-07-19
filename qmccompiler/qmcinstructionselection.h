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

#ifndef QMCINSTRUCTIONSELECTION_H
#define QMCINSTRUCTIONSELECTION_H

#include <QList>

#include <private/qv4isel_masm_p.h>

#include "qmcfile.h"

namespace QV4 {
namespace IR {
struct Module;
}
namespace Compiler {
struct JSUnitGenerator;
}
}

class QmcInstructionSelection : public QV4::JIT::InstructionSelection
{
public:
    QmcInstructionSelection(QQmlEnginePrivate *qmlEngine, QV4::ExecutableAllocator *execAllocator,
                            QV4::IR::Module *module, QV4::Compiler::JSUnitGenerator *jsGenerator);

    virtual void run(int functionIndex);

    const QList<QVector<QmcUnitCodeRefLinkCall > >& linkData() const { return linkedCalls; }

private:

    QList<QVector<QmcUnitCodeRefLinkCall > > linkedCalls;

};

#endif // QMCINSTRUCTIONSELECTION_H
