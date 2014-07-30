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
 * Author: Mikko Hurskainen <mikko.hurskainen@nomovok.com>
 */

#ifndef JSC_H
#define JSC_H

#include <QObject>
#include <QDataStream>

#include "compiler.h"
#include "qmccompiler_global.h"

namespace QV4 {
namespace IR {
struct Module;
}

namespace Compiler {
struct JSUnitGenerator;
}

namespace CompiledData {
class CompilationUnit;
}
}

class QQmlEngine;

class QMCCOMPILERSHARED_EXPORT ScriptC : public Compiler
{
    Q_OBJECT
public:
    explicit ScriptC(QQmlEngine *engine, QObject *parent = 0);
    virtual ~ScriptC();

protected:
    virtual bool compileData();
    virtual bool createExportStructures();
private:
    QV4::CompiledData::CompilationUnit* precompile(QV4::IR::Module *module, QV4::Compiler::JSUnitGenerator *unitGenerator);
};

#endif // JSC_H
