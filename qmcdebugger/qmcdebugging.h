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

#ifndef QMCDEBUGGING_H
#define QMCDEBUGGING_H

#include <QtCore/qglobal.h>

#if defined(QMCDEBUGGER_LIBRARY)
#  define QMCDEBUGGERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QMCDEBUGGERSHARED_EXPORT Q_DECL_IMPORT
#endif

#if defined(QMC) && defined(QMC_DEBUG)
#define QMC_DEBUG_INIT QmcDebuggingInit();
void QmcDebuggingInit();
#else
#define QMC_DEBUG_INIT
#endif

#endif // QMCDEBUGGING_H
