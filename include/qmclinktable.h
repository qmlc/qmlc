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

#ifndef QMCLINKTABLE_H
#define QMCLINKTABLE_H

#include <private/qv4runtime_p.h>

struct QmcLinkEntry {
    const char *name;
    void *addr;
};

#define QMC_LINK_TABLE_ADD_NS(x) QV4::Runtime::x
#define QMC_LINK_TABLE_STR(x) "Runtime::" #x
#define QMC_LINK_TABLE_ENTRY_RUNTIME(x) { (const char *)QMC_LINK_TABLE_STR(x), (void *)QMC_LINK_TABLE_ADD_NS(x) }

// table to link objects
// this table can be used to resolve functions, it is id -> object mapping to maximize performance in linking phase
// when adding new calls, add to end of the list to maintain compatibility
const QmcLinkEntry QMC_LINK_TABLE[] = {
    // call
    QMC_LINK_TABLE_ENTRY_RUNTIME(callGlobalLookup),
    QMC_LINK_TABLE_ENTRY_RUNTIME(callActivationProperty),
    QMC_LINK_TABLE_ENTRY_RUNTIME(callProperty),
    QMC_LINK_TABLE_ENTRY_RUNTIME(callPropertyLookup),
    QMC_LINK_TABLE_ENTRY_RUNTIME(callElement),
    QMC_LINK_TABLE_ENTRY_RUNTIME(callValue),

    // construct
    QMC_LINK_TABLE_ENTRY_RUNTIME(constructGlobalLookup),
    QMC_LINK_TABLE_ENTRY_RUNTIME(constructActivationProperty),
    QMC_LINK_TABLE_ENTRY_RUNTIME(constructProperty),
    QMC_LINK_TABLE_ENTRY_RUNTIME(constructPropertyLookup),
    QMC_LINK_TABLE_ENTRY_RUNTIME(constructValue),

    // set & get
    QMC_LINK_TABLE_ENTRY_RUNTIME(setActivationProperty),
    QMC_LINK_TABLE_ENTRY_RUNTIME(setProperty),
    QMC_LINK_TABLE_ENTRY_RUNTIME(setElement),
    QMC_LINK_TABLE_ENTRY_RUNTIME(getProperty),
    QMC_LINK_TABLE_ENTRY_RUNTIME(getActivationProperty),
    QMC_LINK_TABLE_ENTRY_RUNTIME(getElement),

    // typeof
    QMC_LINK_TABLE_ENTRY_RUNTIME(typeofValue),
    QMC_LINK_TABLE_ENTRY_RUNTIME(typeofName),
    QMC_LINK_TABLE_ENTRY_RUNTIME(typeofMember),
    QMC_LINK_TABLE_ENTRY_RUNTIME(typeofElement),

    // delete
    QMC_LINK_TABLE_ENTRY_RUNTIME(deleteElement),
    QMC_LINK_TABLE_ENTRY_RUNTIME(deleteMember),
    QMC_LINK_TABLE_ENTRY_RUNTIME(deleteName),

    // exceptions & scopes
    QMC_LINK_TABLE_ENTRY_RUNTIME(throwException),
    QMC_LINK_TABLE_ENTRY_RUNTIME(unwindException),
    QMC_LINK_TABLE_ENTRY_RUNTIME(pushWithScope),
    QMC_LINK_TABLE_ENTRY_RUNTIME(pushCatchScope),
    QMC_LINK_TABLE_ENTRY_RUNTIME(popScope),

    // closures
    QMC_LINK_TABLE_ENTRY_RUNTIME(closure),

    // function header
    QMC_LINK_TABLE_ENTRY_RUNTIME(declareVar),
    QMC_LINK_TABLE_ENTRY_RUNTIME(setupArgumentsObject),
    QMC_LINK_TABLE_ENTRY_RUNTIME(convertThisToObject),

    // literals
    QMC_LINK_TABLE_ENTRY_RUNTIME(arrayLiteral),
    QMC_LINK_TABLE_ENTRY_RUNTIME(objectLiteral),
    QMC_LINK_TABLE_ENTRY_RUNTIME(regexpLiteral),

    // foreach
    QMC_LINK_TABLE_ENTRY_RUNTIME(foreachIterator),
    QMC_LINK_TABLE_ENTRY_RUNTIME(foreachNextPropertyName),

    // unary operators
    //typedef ReturnedValue (*UnaryOperation)(const ValueRef);
    {"NOOP", (void *)qt_noop},
    QMC_LINK_TABLE_ENTRY_RUNTIME(uPlus),
    QMC_LINK_TABLE_ENTRY_RUNTIME(uMinus),
    QMC_LINK_TABLE_ENTRY_RUNTIME(uNot),
    QMC_LINK_TABLE_ENTRY_RUNTIME(complement),
    QMC_LINK_TABLE_ENTRY_RUNTIME(increment),
    QMC_LINK_TABLE_ENTRY_RUNTIME(decrement),

    // binary operators
    //typedef ReturnedValue (*BinaryOperation)(const ValueRef left, const ValueRef right);
    {"NOOP", (void *)qt_noop},
    //typedef ReturnedValue (*BinaryOperationContext)(ExecutionContext *ctx, const ValueRef left, const ValueRef right);
    {"NOOP", (void *)qt_noop},

    QMC_LINK_TABLE_ENTRY_RUNTIME(instanceof),
    QMC_LINK_TABLE_ENTRY_RUNTIME(in),
    QMC_LINK_TABLE_ENTRY_RUNTIME(add),
    QMC_LINK_TABLE_ENTRY_RUNTIME(addString),
    QMC_LINK_TABLE_ENTRY_RUNTIME(bitOr),
    QMC_LINK_TABLE_ENTRY_RUNTIME(bitXor),
    QMC_LINK_TABLE_ENTRY_RUNTIME(bitAnd),
    QMC_LINK_TABLE_ENTRY_RUNTIME(sub),
    QMC_LINK_TABLE_ENTRY_RUNTIME(mul),
    QMC_LINK_TABLE_ENTRY_RUNTIME(div),
    QMC_LINK_TABLE_ENTRY_RUNTIME(mod),
    QMC_LINK_TABLE_ENTRY_RUNTIME(shl),
    QMC_LINK_TABLE_ENTRY_RUNTIME(shr),
    QMC_LINK_TABLE_ENTRY_RUNTIME(ushr),
    QMC_LINK_TABLE_ENTRY_RUNTIME(greaterThan),
    QMC_LINK_TABLE_ENTRY_RUNTIME(lessThan),
    QMC_LINK_TABLE_ENTRY_RUNTIME(greaterEqual),
    QMC_LINK_TABLE_ENTRY_RUNTIME(lessEqual),
    QMC_LINK_TABLE_ENTRY_RUNTIME(equal),
    QMC_LINK_TABLE_ENTRY_RUNTIME(notEqual),
    QMC_LINK_TABLE_ENTRY_RUNTIME(strictEqual),
    QMC_LINK_TABLE_ENTRY_RUNTIME(strictNotEqual),

    // comparisons
    //typedef Bool (*CompareOperation)(const ValueRef left, const ValueRef right);}
    {"NOOP", (void *)qt_noop},
    QMC_LINK_TABLE_ENTRY_RUNTIME(compareGreaterThan),
    QMC_LINK_TABLE_ENTRY_RUNTIME(compareLessThan),
    QMC_LINK_TABLE_ENTRY_RUNTIME(compareGreaterEqual),
    QMC_LINK_TABLE_ENTRY_RUNTIME(compareLessEqual),
    QMC_LINK_TABLE_ENTRY_RUNTIME(compareEqual),
    QMC_LINK_TABLE_ENTRY_RUNTIME(compareNotEqual),
    QMC_LINK_TABLE_ENTRY_RUNTIME(compareStrictEqual),
    QMC_LINK_TABLE_ENTRY_RUNTIME(compareStrictNotEqual),

    //typedef Bool (*CompareOperationContext)(ExecutionContext *ctx, const ValueRef left, const ValueRef right);
    {"NOOP", (void *)qt_noop},
    QMC_LINK_TABLE_ENTRY_RUNTIME(compareInstanceof),
    QMC_LINK_TABLE_ENTRY_RUNTIME(compareIn),

    // conversions
    QMC_LINK_TABLE_ENTRY_RUNTIME(toBoolean),
    QMC_LINK_TABLE_ENTRY_RUNTIME(toDouble),
    QMC_LINK_TABLE_ENTRY_RUNTIME(toInt),
    QMC_LINK_TABLE_ENTRY_RUNTIME(doubleToInt),
    QMC_LINK_TABLE_ENTRY_RUNTIME(toUInt),
    QMC_LINK_TABLE_ENTRY_RUNTIME(doubleToUInt),

    // qml
    QMC_LINK_TABLE_ENTRY_RUNTIME(getQmlIdArray),
    QMC_LINK_TABLE_ENTRY_RUNTIME(getQmlImportedScripts),
    QMC_LINK_TABLE_ENTRY_RUNTIME(getQmlContextObject),
    QMC_LINK_TABLE_ENTRY_RUNTIME(getQmlScopeObject),
    QMC_LINK_TABLE_ENTRY_RUNTIME(getQmlSingleton),
    QMC_LINK_TABLE_ENTRY_RUNTIME(getQmlAttachedProperty),
    QMC_LINK_TABLE_ENTRY_RUNTIME(getQmlQObjectProperty),
    QMC_LINK_TABLE_ENTRY_RUNTIME(setQmlQObjectProperty),

};

#endif // QMCLINKTABLE_H
